#include "pch.h"

#if BOOST_WINDOWS
#include <io.h>
#endif

#include "CurlUtils.h"
#include "DownloadStats.h"
#include "HTTP.h"

#include "Download.h"

MLOG_DECLARE_LOGGER(download);

namespace mnet {

namespace download_impl {

class Worker;

struct Chunk {
    filesize_t begin;
    filesize_t end;
    boost::scoped_ptr<Worker> worker;
    boost::intrusive::set_member_hook<> hook;
    
    filesize_t size() const { return end - begin; }

    Chunk(filesize_t b, filesize_t e)
        : begin(b), end(e) {}
};

class Worker {
public:
    Worker(CURLM * curlm, FILE * out, mstd::atomic<filesize_t> & downloaded, Chunk * chunk,
           const std::string & url)
        : curlm_(curlm), out_(out), downloaded_(downloaded), chunk_(chunk), curl_(createCurl(url))
    {    
        MLOG_MESSAGE(Debug, "Worker(" << url << ")");

        char buf[0x40];
        char * p = buf;
        mstd::itoa(chunk->begin, p);
        p += strlen(p);
        *p++ = '-';
        mstd::itoa(chunk->end - 1, p);
        curl_easy_setopt(curl_, CURLOPT_RANGE, buf);

        MLOG_MESSAGE(Debug, "downloading from: " << chunk->begin << ", to: " << chunk->end << ", len: " << chunk->end - chunk->begin);

        curl_easy_setopt(curl_, CURLOPT_NOPROGRESS, 0);

        curl_easy_setopt(curl_, CURLOPT_PRIVATE, this);
        curl_easy_setopt(curl_, CURLOPT_PROGRESSFUNCTION, &Worker::progress);
        curl_easy_setopt(curl_, CURLOPT_PROGRESSDATA, this);
        curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, &Worker::write);
        curl_easy_setopt(curl_, CURLOPT_WRITEDATA, this);
        curl_easy_setopt(curl_, CURLOPT_HEADER, 0);
        curl_easy_setopt(curl_, CURLOPT_HEADERFUNCTION, appendToString);
        curl_easy_setopt(curl_, CURLOPT_WRITEHEADER, &header_);

        curl_multi_add_handle(curlm_, curl_);
    }

    CURL * curl()
    {
        return curl_;
    }

    filesize_t speed() const
    {
        double result;
        curl_easy_getinfo(curl_, CURLINFO_SPEED_DOWNLOAD, &result);
        return static_cast<filesize_t>(result);
    }

    ~Worker()
    {
        if(curl_)
        {
            curl_multi_remove_handle(curlm_, curl_);
            curl_easy_cleanup(curl_);
        }
    }
private:
    static int progress(void * x, double t, double d, double ultotal, double ulnow)
    {
        return 0;
    }

    static size_t write(const char* buf, size_t size, size_t nmemb, Worker * worker)
    {
        size *= nmemb;
        filesize_t & begin = worker->chunk_->begin;
        filesize_t len = std::min<filesize_t>(worker->chunk_->end - begin, size);
        FILE * out = worker->out_;
#if BOOST_WINDOWS
        _fseeki64(out, begin, SEEK_SET);
#else
        fseeko(out, begin, SEEK_SET);
#endif
        size_t written = fwrite(buf, static_cast<size_t>(len), 1, out);
        fflush(out);
        begin += len;
        worker->downloaded_ += len;
        MLOG_MESSAGE(Debug, "write(" << len << ", " << worker->downloaded_ << "), written: " << written);
        return static_cast<size_t>(len);
    }

    CURLM * curlm_;
    FILE * out_;
    mstd::atomic<filesize_t> & downloaded_;
    Chunk * chunk_;
    CURL * curl_;
    std::string header_;
};

}

using namespace download_impl;

class Download::Impl {
public:
    explicit Impl(Download * parent, const std::string & url, const boost::filesystem::wpath & localfile, filesize_t size, const Listener & listener)
        : parent_(parent), url_(url), localfile_(localfile), size_(size), listener_(listener), active_(false),
          prevElapsed_(boost::posix_time::seconds(0)), elapsed_(0)
    {
        boost::filesystem::ifstream inp(metaFile(), std::ios::binary);
        if(inp)
        {
            std::streambuf * buf = inp.rdbuf();
            std::streampos size = buf->pubseekoff(0, std::ios::end);
            if(size)
            {
                size_t chunks = static_cast<size_t>(size / (sizeof(filesize_t) * 2));
                buf->pubseekoff(0, std::ios::beg);
                std::vector<filesize_t> buffer(chunks * 2);
                buf->sgetn(mstd::pointer_cast<char*>(&buffer[0]), chunks * sizeof(filesize_t) * 2);
                for(size_t i = 0; i != chunks; ++i)
                {
                    filesize_t begin = buffer[i * 2];
                    filesize_t end = buffer[i * 2 + 1];
                    MLOG_MESSAGE(Debug, "read chunk: " << begin << ", " << end);
                    Chunk & chunk = *(new Chunk(begin, end));
                    chunks_.insert(chunk);
                }
            }
            filesize_t p = 0;
            for(Chunks::const_iterator i = chunks_.begin(); i != chunks_.end(); ++i)
            {
                downloaded_ += i->begin - p;
                p = i->end;
            }
            if(!chunks_.empty())
            {
                Chunks::iterator i = chunks_.end();
                --i;
                size_ = i->end;
                if(!i->size())
                    chunks_.erase_and_dispose(i, mstd::delete_disposer());
            }
       } else
            downloaded_ = 0;
    }

    boost::filesystem::wpath metaFile()
    {
        boost::filesystem::wpath meta = localfile_;
#if BOOST_VERSION >= 104600
        meta.replace_extension(mstd::wfname(meta.extension()) + L".meta");
#else
        meta.replace_extension(meta.extension() + L".meta");
#endif
        return meta;
    }

    void cleanup()
    {
        try {
            boost::filesystem::remove(metaFile());
        } catch(boost::filesystem::filesystem_error & e) {
            MLOG_MESSAGE(Error, "filesystem error: " << e.what());
        }
    }

    void start()
    {
        boost::lock_guard<boost::mutex> lock(mutex_);
        if(!active_)
        {
            active_ = true;
            thread_ = boost::move(boost::thread(&Impl::execute, this));
        }
    }

    void stop()
    {
        boost::lock_guard<boost::mutex> lock(mutex_);
        if(active_)
            thread_.interrupt();
    }

    DownloadStats stats() const
    {
        DownloadStats result = { downloaded_, size_, speed_, boost::posix_time::milliseconds(elapsed_) };
        return result;
    }

    ~Impl()
    {
        {
            boost::unique_lock<boost::mutex> lock(mutex_);
            if(thread_.joinable())
            {
                thread_.interrupt();
                lock.unlock();
                thread_.join();
            }
        }
        chunks_.clear_and_dispose(mstd::delete_disposer());
    }
private:
    void execute()
    {
        MLOG_MESSAGE(Info, "execute(" << url_ << ")");

        {
            boost::filesystem::wpath temp(localfile_);
            temp.remove_filename();
            create_directories(temp);
        }

        int code = 0;
        bool notFound = false;
        if(chunks_.empty())
        {
            code = getRemoteFileSizeEx(url_, size_);
            if(!code)
                chunks_.insert(*new Chunk(0, size_));
            if(code == 404)
                notFound = true;
        }
        
        bool interrupted = false;
        if(!code)
        {
            boost::posix_time::ptime start = boost::posix_time::microsec_clock::universal_time();
            boost::posix_time::ptime lastMetaSave = start;
            {
                FILE * meta = mstd::wfopen(metaFile(), "rb+");
                if(!meta)
                    meta = mstd::wfopen(metaFile(), "wb");
                BOOST_SCOPE_EXIT((meta)) {
                    if(meta)
                        fclose(meta);
                } BOOST_SCOPE_EXIT_END;
                try {
                    FILE * out = mstd::wfopen(localfile_, "rb+");
                    if(!out)
                        out = mstd::wfopen(localfile_, "wb");
                    if(!out)
                        throw std::exception();
                    BOOST_SCOPE_EXIT((out)) {
                        if(out)
                            fclose(out);
                    } BOOST_SCOPE_EXIT_END;
                    out_ = out;

                    CurlMultiHandle multi(curl_multi_init());
                    curlm_ = *multi;
                    makeWorkers(5);
                    saveMetaState(meta);
                    fd_set readfs, writefs, excfs;
                    int maxfd;
                    int retries = 100;
                    while(!(interrupted = boost::this_thread::interruption_requested()))
                    {
                        int rh = 0;
                        while(curl_multi_perform(curlm_, &rh) == CURLM_CALL_MULTI_PERFORM);
                        CURLMsg * msg;
                        while((msg = curl_multi_info_read(curlm_, &rh)) != 0)
                        {
                            if(msg->msg == CURLMSG_DONE)
                            {
                                Worker * worker;
                                curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &worker);
                                long code;
                                curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &code);
                                if(code >= 300)
                                    MLOG_MESSAGE(Warning, "done with code: " << code);
                                else
                                    MLOG_MESSAGE(Notice, "done with code: " << code);
                                bool error = msg->data.result != CURLE_WRITE_ERROR && (msg->data.result != CURLE_OK || code != 200);
                                if(error)
                                {
                                    if(code == 404)
                                    {
                                        notFound = true;
                                        retries = 0;
                                    } else if(retries > 0)
                                    {
                                        if(code == 301 || code == 302)
                                            getRemoteFileSizeEx(url_, size_);
                                        else                                 
                                            --retries;
                                    }
                                }
                                for(Chunks::iterator i = chunks_.begin(), end = chunks_.end(); i != end; ++i)
                                    if(i->worker.get() == worker)
                                    {
                                        if(i->begin == i->end)
                                            chunks_.erase_and_dispose(i, mstd::delete_disposer());
                                        else
                                            i->worker.reset();
                                        break;
                                    }
                                if(retries)
                                {
                                    makeWorkers(1);
                                    lastMetaSave = boost::posix_time::min_date_time;
                                }
                            }
                        }
                        if(notFound)
                            break;
                        filesize_t speed = 0;
                        for(Chunks::const_iterator i = chunks_.begin(), end = chunks_.end(); i != end; ++i)
                            if(i->worker)
                                speed += i->worker->speed();
                        speed_ = speed;
                        boost::posix_time::ptime now = boost::posix_time::microsec_clock::universal_time();
                        elapsed_ = (prevElapsed_ + (now - start)).total_milliseconds();
                        if(now - lastMetaSave > boost::posix_time::seconds(5))
                        {
                            saveMetaState(meta);
                            lastMetaSave = now;
                        }

                        FD_ZERO(&readfs);
                        FD_ZERO(&writefs);
                        FD_ZERO(&excfs);
                        maxfd = 0;
                        CURLMcode cr = curl_multi_fdset(curlm_, &readfs, &writefs, &excfs, &maxfd);
                        if(cr != CURLM_OK)
                            MLOG_ERROR("curl_multi_fdset failed: " << cr);
                        if(maxfd == -1)
                            break;
                        timeval timeout = { 0, 10000 };
                        int res = select(maxfd + 1, &readfs, &writefs, &excfs, &timeout);
                        if(res == -1)
                            break;
                    }
                } catch(boost::thread_interrupted&) {
                    interrupted = true;
                } catch(std::exception&)
                {
                }
                saveMetaState(meta);
            }
            boost::posix_time::ptime stop = boost::posix_time::microsec_clock::universal_time();
            prevElapsed_ += stop - start;
            for(Chunks::iterator i = chunks_.begin(), end = chunks_.end(); i != end; ++i)
                i->worker.reset();
        }
        DownloadAction action = downloaded_ == size_ ? daFinished : (interrupted ? daStopped : daFailed);
        code = action == daFailed ? (notFound ? 404 : 400) : 200;
        MLOG_MESSAGE(Debug, "download done: " << action << ", code: " << code);
        listener_(parent_, action, code);
        boost::lock_guard<boost::mutex> lock(mutex_);
        active_ = false;
    }

    void saveMetaState(FILE * out)
    {
        if(out)
        {
            filesize_t * buf = static_cast<filesize_t*>(alloca((chunks_.size() + 1) * 2 * sizeof(filesize_t)));
            filesize_t * p = buf;
            for(Chunks::iterator i = chunks_.begin(), end = chunks_.end(); i != end; ++i)
            {
                *p++ = i->begin;
                *p++ = i->end;
            }
            if(!chunks_.empty())
            {
                Chunks::iterator i = chunks_.end();
                --i;
                if(i->end != size_)
                {
                    *p++ = size_;
                    *p++ = size_;
                }
            }
            size_t size = (p - buf) * sizeof(*buf);
            fseek(out, 0, SEEK_SET);
            size_t written = fwrite(buf, size, 1, out);
            if(written != size)
                MLOG_ERROR("mata save failed: " << written << " vs " << size);
#if BOOST_WINDOWS
            _chsize(_fileno(out), size);
#else
            int res = ftruncate(fileno(out), size);
            if(res < 0)
            {
                int err = errno;
                MLOG_ERROR("failed to truncate meta: " << err);
            }
#endif
            fflush(out);
        }
    }

    void makeWorkers(size_t n)
    {
        Chunk ** freeChunks = static_cast<Chunk**>(alloca((n + 1) * sizeof(Chunk*)));
        Chunk ** p = freeChunks;
        for(Chunks::const_iterator i = chunks_.begin(), end = chunks_.end(); i != end && static_cast<size_t>(p - freeChunks) < n; ++i)
        {
            if(!i->worker)
                *p++ = const_cast<Chunk*>(&*i);
        }
        if(p == freeChunks)
        {
            Chunk * maxChunk = 0;
            for(Chunks::const_iterator i = chunks_.begin(), end = chunks_.end(); i != end; ++i)
            {
                if(!maxChunk || i->end - i->begin > maxChunk->end - maxChunk->begin)
                    maxChunk = const_cast<Chunk*>(&*i);
            }
            if(!maxChunk)
            {
                MLOG_MESSAGE(Notice, "no more chunks");
                return;
            }
            *p++ = maxChunk;
            ++n;
        }
        if(static_cast<size_t>(p - freeChunks) < n)
        {
            Chunk * maxChunk = *freeChunks;
            for(Chunk ** i = freeChunks; ++i != p;)
            {
                if((*i)->end - (*i)->begin > maxChunk->end - maxChunk->begin)
                    maxChunk = *i;
            }
            filesize_t chunkSize = maxChunk->end - maxChunk->begin;
            if(chunkSize > 10000) // do not split small chunks
            {
                size_t l = n - (p - freeChunks) + 1;
                filesize_t blockSize = chunkSize / l;
                filesize_t extra = chunkSize - blockSize * l;
                filesize_t o = maxChunk->begin;
                for(size_t i = 0; i != l; ++i)
                {
                    filesize_t e = o + blockSize;
                    if(extra)
                    {
                        ++e;
                        --extra;
                    }
                    if(i == 0)
                    {
                        maxChunk->end = e;
                    } else {
                        Chunk * chunk = new Chunk(o, e);
                        chunks_.insert(*chunk);
                        *p++ = chunk;
                    }
                    o = e;
                }
            }
        }
        for(Chunk ** i = freeChunks; i != p; ++i)
        {
            Chunk * chunk = *i;
            if(!chunk->worker/* && (i + 1 == p)*/)
                chunk->worker.reset(new Worker(curlm_, out_, downloaded_, chunk, url_));
        }
    }

    struct CompareChunks {
        bool operator()(const Chunk & lhs, const Chunk & rhs) const
        {
            return lhs.begin < rhs.begin;
        }
    };

    typedef boost::intrusive::multiset<Chunk,
                                       boost::intrusive::member_hook<Chunk, boost::intrusive::set_member_hook<>, &Chunk::hook>,
                                       boost::intrusive::compare<CompareChunks> > Chunks;

    Download * parent_;
    std::string url_;
    boost::filesystem::wpath localfile_;
    filesize_t size_;
    Listener listener_;

    mutable boost::mutex mutex_;
    boost::thread thread_;
    bool active_;
    mstd::atomic<filesize_t> downloaded_;
    mstd::atomic<filesize_t> speed_;
    Chunks chunks_;
    CURLM * curlm_;
    FILE * out_;
    boost::posix_time::time_duration prevElapsed_;
    int64_t elapsed_;
};

Download::Download(const std::string & url, const boost::filesystem::wpath & localfile, filesize_t size, const Listener & listener)
{
    impl_.reset(new Impl(this, url, localfile, size, listener));
}

Download::~Download()
{
}

void Download::cleanup()
{
    impl_->cleanup();
}

void Download::start()
{
    impl_->start();
}

void Download::stop()
{
    impl_->stop();
}

DownloadStats Download::stats() const
{
    return impl_->stats();
}

}
