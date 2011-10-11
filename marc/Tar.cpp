#include "pch.h"

#include "GZip.h"

#include "Tar.h"

MLOG_DECLARE_LOGGER(tar);

namespace marc {

namespace {

int copy_data(struct archive *ar, struct archive *aw)
{
    MLOG_DEBUG("copy_data(" << ar << ", " << aw << ')');

	int r;
	const void *buff;
	size_t size;
	off_t offset;
    
	for (;;) {
		r = archive_read_data_block(ar, &buff, &size, &offset);
		if (r == ARCHIVE_EOF)
			return (ARCHIVE_OK);
		if (r != ARCHIVE_OK)
			return (r);
		r = archive_write_data_block(aw, buff, size, offset);
		if (r != ARCHIVE_OK)
        {
            MLOG_WARNING("archive_write_data_block: " << archive_error_string(aw));
			return (r);
		}
	}
}

bool extract(void * data, size_t size, const boost::filesystem::wpath & dest, int do_extract, int flags)
{
    MLOG_MESSAGE(Debug, "extract(" << size << ", " << mstd::utf8fname(dest) << ", " << do_extract << ", " << flags << ')');

	struct archive_entry *entry;
	int r = 0;
    
    struct archive * a = archive_read_new();

    MLOG_MESSAGE(Debug, "archive_read_new: " << a);

    struct archive * ext = archive_write_disk_new();

    MLOG_MESSAGE(Debug, "archive_write_disk_new: " << ext);

    BOOST_SCOPE_EXIT((&a)(&ext)) {
        archive_read_close(a);
        archive_read_finish(a);
        archive_write_close(ext);
    } BOOST_SCOPE_EXIT_END;
    
	archive_write_disk_set_options(ext, flags);
	/*
	 * Note: archive_write_disk_set_standard_lookup() is useful
	 * here, but it requires library routines that can add 500k or
	 * more to a static executable.
	 */
	archive_read_support_format_tar(a);
    archive_read_support_compression_gzip(a);

	if (r = archive_read_open_memory(a, data, size))
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