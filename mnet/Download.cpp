/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
)
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
    impl_.reset();
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
