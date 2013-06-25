/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
archive_read_support_format_tar(a);
    archive_read_support_compression_gzip(a);

	if ((r = archive_read_open_memory(a, data, size)))
    {
		MLOG_MESSAGE(Error, "archive_read_open_memory(): " << archive_error_string(a) << ", " << r);
        return false;
    }
    boost::filesystem::wpath current = boost::filesystem::current_path();
    BOOST_SCOPE_EXIT((&current)) {
        boost::filesystem::current_path(current);
    } BOOST_SCOPE_EXIT_END;
    try {
        create_directories(dest);
        current_path(dest);
    } catch(boost::filesystem::filesystem_error&) {
        return false;
    }
	for (;;) {
		r = archive_read_next_header(a, &entry);
		if (r == ARCHIVE_EOF)
			break;
		if (r != ARCHIVE_OK)
        {
			MLOG_ERROR("archive_read_next_header: " << archive_error_string(a));
            return false;
        }
		if (do_extract)
        {
            /*boost::filesystem::wpath entryfname = dest / archive_entry_pathname_w(entry);
            std::wstring entryNameStr = mstd::wfname(entryfname);
            archive_entry_copy_pathname_w(entry, entryNameStr.c_str());*/
			r = archive_write_header(ext, entry);
			if (r != ARCHIVE_OK)
				MLOG_WARNING("archive_write_header: " << archive_error_string(ext));
			else {
				copy_data(a, ext);
				r = archive_write_finish_entry(ext);
				if (r != ARCHIVE_OK)
                {
                    MLOG_ERROR("archive_write_finish_entry: " << archive_error_string(ext));
                    return false;
                }
			}
            
		}
	}
    return true;
}

}

bool readFile(const boost::filesystem::wpath & input, std::vector<char> & out)
{
    MLOG_DEBUG("readFile(" << mstd::utf8fname(input) << ')');

    FILE * dummy = mstd::wfopen(input, "rb");
    if(dummy)
    {
        fseek(dummy, 0, SEEK_END);
        long size = ftell(dummy);
        fseek(dummy, 0, SEEK_SET);
        out.resize(size);
        fread(&out[0], 1, size, dummy);
        fclose(dummy);
        
        MLOG_DEBUG("readFile, size = " << size);
        return true;
    }
    
    MLOG_DEBUG("readFile, failed");
    return false;
}

bool untar(const boost::filesystem::wpath & source, const boost::filesystem::wpath & dest)
{
    std::vector<char> inp;
    if(readFile(source, inp))
    {        
        std::vector<char> out;
        if(source.extension() == ".gz")
        {
            ungzip(&inp[0], inp.size(), out);
            std::vector<char>().swap(inp);
        } else {
            inp.swap(out);
        }
        return extract(&out[0], out.size(), dest, true, ARCHIVE_EXTRACT_TIME);
    }
    return false;
}

}