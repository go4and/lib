#include "pch.h"

#include "HTTP.h"

#include "DownloadManager.h"

MLOG_DECLARE_LOGGER(mnet_download_manager);

namespace mnet {

namespace {

const filesize_t invalidPosition = std::numeric_limits<size_t>::max();

MSTD_DEFINE_ENUM_EX(ChunkState, cs, (Waiting)(Active)(Stopping)(Complete));

struct Chunk {
    mstd::atomic<filesize_t> begin;
    mstd::atomic<filesize_t> end;
    mstd::atomic<ChunkState> state;
    int priority;
    
    FILE * out;
    
    bool valid() const { return begin != invalidPosition; }

    Chunk(int p)
        : begin(invalidPosition), end(invalidPosition), state(csWaiting), priority(p)
    {
    }
};

class DirectWriter {
public:
    explicit DirectWriter(Chunk & chunk)
        : chunk_(chunk)
    {
    }
    
    size_t operator()(const char * buf, size_t size) const
    {
        auto & begin = chunk_.begin;
        filesize_t len = std::min<filesize_t>(std::max<filesize_t>(chunk_.end - begin, 0), size);
        FILE * out = chunk_.out;
#if BOOST_WINDOWS
        _fseeki64(out, begin, SEEK_SET);
#else
        fseeko(out, begin, SEEK_SET);
#endif
        size_t written = fwrite(buf, 1, static_cast<size_t>(len), out);
        BOOST_VERIFY(written == len);
        fflush(out);
        begin += len;

        // MLOG_MESSAGE(Debug, "write(" << len << "), written: " << written << ", state: " << name(chunk_.state));

        return (chunk_.state == csActive) ? static_cast<size_t>(len) : 0;
    }
private:
    Chunk & chunk_;
};

class Task;

class TaskContext {
public:
    virtual size_t maxChunks() = 0;
    virtual filesize_t minChunkSize() = 0;
    virtual void finished(Task & task, Chunk & chunk, int ec) = 0;
};

class Task : boost::noncopyable {
public:
    explicit Task(int id, TaskContext & context, const std::string & url, const boost::filesystem::path & dest, int priority, const DownloadListener & listener, const char * state, size_t stateLen)
        : id_(id), context_(context), url_(url), dest_(dest), priority_(priority), listener_(listener), out_(0), cancelled_(false), active_(0)
    {
        if(state && stateLen)
        {
            out_ = mstd::wfopen_append(dest);
            if(out_)
            {
                const char * end = state + stateLen;
                memcpy(&size_, state, sizeof(size_));
                state += sizeof(size_);
                while(state != end)
                {
                    chunks_.push_back(Chunk(priority));
                    Chunk & chunk = chunks_.back();
                    filesize_t temp;
                    memcpy(&temp, state, sizeof(temp));
                    state += sizeof(size_);
                    chunk.begin = temp;
                    memcpy(&temp, state, sizeof(temp));
                    state += sizeof(size_);
                    chunk.end = temp;
                    chunk.state = csWaiting;
                    chunk.out = out_;
                }
                return;
            }
        }

        size_ = sizeUnknown;
        chunks_.reserve(context.maxChunks());
        chunks_.push_back(Chunk(priority));
    }

    ~Task()
    {
        if(out_)
            fclose(out_);
    }

    DownloadProgress getProgress()
    {
        if(size_ == sizeUnknown)
            return DownloadProgress(0, size_);
        else {
            filesize_t complete = size_;
            for(const auto & chunk : chunks_)
                complete -= std::max<filesize_t>(chunk.end - chunk.begin, 0);
            return DownloadProgress(complete, size_);
        }
    }

    bool getState(std::vector<char> & out)
    {
        if(size_ == sizeUnknown)
            return false;
        out.resize(sizeof(filesize_t) + chunks_.size() * sizeof(filesize_t) * 2);
        char * pos = &out[0];
        memcpy(pos, &size_, sizeof(size_));
        pos += sizeof(size_);
        for(const auto & chunk : chunks_)
            if(chunk.state != csComplete)
            {
                filesize_t temp = chunk.begin;
                memcpy(pos, &temp, sizeof(temp));
                pos += sizeof(temp);
                temp = chunk.end;
                memcpy(pos, &temp, sizeof(temp));
                pos += sizeof(temp);
            }
        BOOST_ASSERT(pos - &out[0] <= out.size());
        out.resize(pos - &out[0]);
        return true;
    }

    int id() const { return id_; }
    const std::string & url() const { return url_; }
    const boost::filesystem::path & path() const { return dest_; }
    int priority() const { return priority_; }
    void cancel() { cancelled_ = true; }
    bool cancelled() const { return cancelled_; }
    size_t chunkIndex(Chunk & chunk) const { return &chunk - &chunks_[0]; }
    size_t active() const { return active_; }

    void notify(int err)
    {
        if(chunks_[0].valid())
        {
            bool allDone = true;
            for(const auto & chunk : chunks_)
                if(chunk.begin < chunk.end)
                {
                    allDone = false;
                    break;
                }
            if(allDone)
                err = 0;
        }
        listener_(id(), err);
    }

    std::vector<Chunk> & chunks() { return chunks_; }

    void launch(Chunk & chunk)
    {
        chunk.state = csActive;
        ++active_;
        if(!chunk.valid())
            Request().url(url_).sizeHandler(boost::bind(&Task::sizeReceived, this, _1, _2)).run();
        else
            Request().url(url_).range(chunk.begin, chunk.end).directWriter(DirectWriter(chunk), boost::bind(&Task::chunkDone, this, _1, boost::ref(chunk))).run();
    }
private:
    void sizeReceived(int ec, filesize_t size)
    {
        MLOG_DEBUG("sizeReceived(" << ec << ", " << size << ")");

        BOOST_ASSERT(chunks_.size() == 1);
        BOOST_ASSERT(!chunks_[0].valid());

        --active_;
        chunks_[0].state = csComplete;
        if(!ec)
        {
            if(!cancelled_)
            {
                size_ = size;
                {
                    boost::filesystem::path temp = dest_;
                    temp.remove_filename();
                    boost::system::error_code ec;
                    create_directories(temp, ec);
                }
                out_ = mstd::wfopen_append(dest_);
                if(out_)
                {
                    size_t maxChunks = context_.maxChunks();
                    filesize_t minChunkSize = context_.minChunkSize();
                    size_t chunks = std::min<size_t>((size + minChunkSize - 1) / minChunkSize, maxChunks);
                    filesize_t chunkSize = size / chunks;
                    size_t bigChunks = size - chunkSize * chunks;
                    filesize_t pos = 0;
                    chunks_.resize(chunks, Chunk(priority_));
                    for(size_t i = 0; i != chunks; ++i)
                    {
                        filesize_t currentSize = chunkSize + (i < bigChunks ? 1 : 0);
                        chunks_[i].begin = pos;
                        pos += currentSize;
                        chunks_[i].end = pos;
                        chunks_[i].state = csWaiting;
                        chunks_[i].out = out_;
                    }
                    BOOST_ASSERT(pos == size);
                } else
                    ec = codeOpenFailed;
            } else
                ec = codeCancelled;
        }

        context_.finished(*this, chunks_[0], ec);
    }

    void chunkDone(int ec, Chunk & chunk)
    {
        MLOG_DEBUG("chunkDone[" << id_ << "](" << ec << ", " << chunkIndex(chunk) << ")");

        --active_;
        chunk.state = csComplete;
        if(ec == 623)
            ec = 0;
        if(!ec)
        {
            if(!cancelled_)
            {
                Chunk * best = 0;
                filesize_t bestLeft = 0;
                for(auto & c : chunks_)
                    if(&c != &chunk)
                    {
                        filesize_t left = c.end - c.begin;
                        if(left > bestLeft)
                        {
                            best = &c;
                            bestLeft = left;
                        }
                    }
                if(best && bestLeft >= context_.minChunkSize())
                {
                    chunk.end = best->end;
                    chunk.begin = (best->begin + best->end) / 2;
                    best->end = chunk.begin;
                    chunk.state = csWaiting;
                } else {
                    chunk.begin = 0;
                    chunk.end = 0;
                    ec = codeDone;
                }
            } else
                ec = codeCancelled;
        }

        context_.finished(*this, chunk, ec);
    }

    int id_;
    TaskContext & context_;
    std::string url_;
    boost::filesystem::path dest_;
    int priority_;
    DownloadListener listener_;
    filesize_t size_;
    std::vector<Chunk> chunks_;
    FILE * out_;
    bool cancelled_;
    size_t active_;
};

struct ComparePriority {
    bool operator()(const Task & lhs, const Task & rhs) const
    {
        return lhs.priority() > rhs.priority();
    }
};

}

class DownloadManager::Impl : public TaskContext {
public:
    explicit Impl()
        : concurrentWorkers_(5), maxChunks_(5), minChunkSize_(1 << 19), taskCounter_(0)
    {
        MLOG_INFO("Impl()");
    }

    ~Impl()
    {
        MLOG_INFO("~Impl()");
    }

    void setConcurrentWorkers(size_t value)
    {
        MLOG_INFO("setConcurrentWorkers(" << value << ")");
        
        concurrentWorkers_ = value;
    }
    
    void setMaxChunks(size_t value)
    {
        MLOG_INFO("setMaxChunks(" << value << ")");
        
        maxChunks_ = value;
    }
    
    void setMinChunkSize(filesize_t value)
    {
        MLOG_INFO("setMinChunkSize(" << value << ")");
        
        minChunkSize_ = value;
    }

    int download(const std::string & url, const boost::filesystem::path & dest, int priority, const DownloadListener & listener, const char * state, size_t stateLen)
    {
        ++taskCounter_;

        MLOG_DEBUG("download(" << url << ", " << dest << ", " << priority << ", " << mlog::dump(state, stateLen) << "), id = " << taskCounter_);

        tasks_.emplace_back(taskCounter_, *this, url, dest, (priority << 0x10) - taskCounter_, listener, state, stateLen);
        --(id2task_[taskCounter_] = tasks_.end());
        tasks_.sort(ComparePriority());
        process();

        return taskCounter_;
    }
    
    void cancel(int id)
    {
        MLOG_DEBUG("cancel(" << id << ")");

        auto i = id2task_.find(id);
        if(i != id2task_.end())
        {
            auto & task = *i->second;
            task.cancel();
            bool hasPending = false;
            for(auto & chunk : task.chunks())
                if(chunk.state == csActive)
                {
                    hasPending = true;
                    chunk.state = csStopping;
                    auto i = std::find(active_.begin(), active_.end(), &chunk);
                    if(i != active_.end())
                        active_.erase(i);
                    else
                        BOOST_ASSERT(false);
                } else if(chunk.state == csStopping)
                    hasPending = true;
                else if(chunk.state == csWaiting)
                    chunk.state = csComplete;
            if(!hasPending)
                taskDone(task, codeCancelled);
        }
    }
    
    DownloadProgress getProgress(int id)
    {
        auto i = id2task_.find(id);
        if(i != id2task_.end())
            return i->second->getProgress();
        else
            return DownloadProgress(0, sizeNotFound);
    }

    bool getState(size_t id, std::vector<char> & out)
    {
        auto i = id2task_.find(id);
        if(i != id2task_.end())
            return i->second->getState(out);
        else
            return false;
    }
private:
    void process()
    {
        MLOG_DEBUG("process()");

        for(auto & task : tasks_)
        {
            auto & chunks = task.chunks();
            for(auto & chunk : chunks)
                if(chunk.state == csWaiting)
                {
                    bool start = false;
                    if(active_.size() >= concurrentWorkers_)
                    {
                        for(auto i = active_.begin(), end = active_.end(); i != end; ++i)
                        {
                            Chunk * activeChunk = *i;
                            BOOST_ASSERT(activeChunk->state == csActive);
                            if(activeChunk->priority < task.priority())
                            {
                                activeChunk->state = csStopping;
                                active_.erase(i);
                                start = true;
                                break;
                            }
                        }
                    } else
                        start = true;
                    if(start)
                    {
                        task.launch(chunk);
                        active_.push_back(&chunk);
                    } else
                        return;
                }
        }
    }

    void finished(Task & task, Chunk & chunk, int ec)
    {
        MLOG_DEBUG("finished(" << task.id() << ", " << task.chunkIndex(chunk) << ")");

        auto i = std::find(active_.begin(), active_.end(), &chunk);
        if(i != active_.end())
            active_.erase(i);
        if(chunk.state == csComplete && !task.active())
        {
            bool hasPending = false;
            for(auto & chunk : task.chunks())
                if(chunk.state != csComplete)
                {
                    hasPending = true;
                    break;
                }
            if(!hasPending)
                taskDone(task, ec);
        }

        process();
    }

    void taskDone(Task & task, int ec)
    {
        MLOG_DEBUG("taskDone(" << task.id() << ")");

        task.notify(ec);

        auto i = id2task_.find(task.id());
        if(i != id2task_.end())
        {
            tasks_.erase(i->second);
            id2task_.erase(i);
        } else
            BOOST_ASSERT(false);
    }

    size_t maxChunks()
    {
        return maxChunks_;
    }

    filesize_t minChunkSize()
    {
        return minChunkSize_;
    }

    size_t concurrentWorkers_;
    size_t maxChunks_;
    filesize_t minChunkSize_;
    int taskCounter_;
    
    typedef std::list<Task> Tasks;
    Tasks tasks_;
    boost::unordered_map<int, Tasks::iterator> id2task_;
    std::vector<Chunk*> active_;
};

DownloadManager::DownloadManager()
    : impl_(new Impl)
{
}

DownloadManager::~DownloadManager()
{
}

void DownloadManager::setConcurrentWorkers(size_t value)
{
    impl_->setConcurrentWorkers(value);
}

void DownloadManager::setMaxChunks(size_t value)
{
    impl_->setMaxChunks(value);
}

void DownloadManager::setMinChunkSize(filesize_t value)
{
    impl_->setMinChunkSize(value);
}

size_t DownloadManager::download(const std::string & url, const boost::filesystem::path & dest, int priority, const DownloadListener & listener, const char * state, size_t stateLen)
{
    return impl_->download(url, dest, priority, listener, state, stateLen);
}

void DownloadManager::cancel(size_t id)
{
    impl_->cancel(id);
}

DownloadProgress DownloadManager::getProgress(size_t id)
{
    return impl_->getProgress(id);
}

bool DownloadManager::getState(size_t id, std::vector<char> & out)
{
    return impl_->getState(id, out);
}

}
