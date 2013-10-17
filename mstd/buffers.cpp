/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#if !defined(_STLP_NO_IOSTREAMS)

#if defined(_MSC_VER)
#pragma warning(disable: 4100)
#pragma warning(disable: 4244)
#pragma warning(disable: 4396)
#pragma warning(disable: 4512)
#endif

#if defined(__APPLE__)
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif

#include <vector>

#include <iostream>

#include <boost/config.hpp>

#include <boost/pool/pool.hpp>
#include <boost/ptr_container/ptr_unordered_map.hpp>

#include <boost/thread/tss.hpp>

#include "buffers.hpp"

namespace mstd {

namespace {

atomic<size_t> reserved_(0);

struct allocator {
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;

    static char * malloc BOOST_PREVENT_MACRO_SUBSTITUTION(const size_type bytes)
    {
        void * result = ::malloc(bytes);
        reserved_ += malloc_size(result);
        return static_cast<char*>(result);
    }

    static void free BOOST_PREVENT_MACRO_SUBSTITUTION(char * const block)
    {
        if(block)
        {
            reserved_ -= malloc_size(block);
            ::free(block);
        }
    }
};

}

class pool {
public:
    pool(size_t block_size, size_t blocks, buffers::impl * buffers)
        : impl_(block_size, blocks), buffers_(buffers), used_(0), allocations_(0)
    {
    }

    inline size_t get_requested_size() const
    {
        return impl_.get_requested_size();
    }

    inline void * malloc()
    {
        ++used_;
        ++allocations_;
        return impl_.malloc();
    }

    inline void free(void * buf)
    {
        --used_;
        impl_.free(buf);
    }

    inline size_t used() const
    {
        return used_;
    }

    inline size_t allocations() const
    {
        return allocations_;
    }

    inline void release(buffer * buf);
private:
    typedef boost::pool<allocator> impl_type;
    impl_type impl_;
    buffers::impl * buffers_;
    mstd::atomic<size_t> used_;
    mstd::atomic<size_t> allocations_;
};

namespace {

class pending_release {
public:
    typedef std::vector<buffer*>::const_iterator const_iterator;

    pending_release()
    {
        buffers_.reserve(0x100);
    }

    inline bool add(pool * p, buffer * buf)
    {
        buffers_.push_back(buf);
        return (total_ += p->get_requested_size()) >= (1 << 22);
    }

    inline const_iterator begin() const
    {
        return buffers_.begin();
    }

    inline const_iterator end() const
    {
        return buffers_.end();
    }

    void clear()
    {
        total_ = 0;
        buffers_.clear();
    }
private:
    size_t total_;
    std::vector<buffer*> buffers_;
};

}

size_t buffer::buffer_size() const
{
    return pool_->get_requested_size() - sizeof(*this);
}

class buffers::impl {
public:
    impl(buffers * bufs)
        : buffers_(bufs), allocated_(0)
#if BOOST_WINDOWS
        , lazyRelease_(true), direct_(false)
#endif
    {
        fillLimits();
    }

    ~impl()
    {
    }

    buffer * take(size_t size)
    {
        std::pair<void*, pool*> res;
#if BOOST_WINDOWS
        if(!direct_)
#endif
        {
            mstd::lock_guard<mstd::mutex> lock(mutex_);
            res = guarded_alloc(size, lock);
#if BOOST_WINDOWS
        } else {
            res.first = new char[sizeof(buffer) + size];
            res.second = 0;
#endif
        }

        BOOST_ASSERT(res.first);
        return buffers::create(res.first, res.second);
    }

    buffer * guarded_take(size_t size, mstd::lock_guard<mstd::mutex> & lock)
    {
        std::pair<void*, pool*> res = guarded_alloc(size, lock);

        BOOST_ASSERT(res.first);
        return buffers::create(res.first, res.second);
    }

    void release(pool * p, buffer * buf)
    {
#if BOOST_WINDOWS
        if(lazyRelease_)
        {
#endif
            pending_release * pending = pending_release_.get();
            if(!pending)
                pending_release_.reset(pending = new pending_release);
            if(pending->add(p, buf))
                release_pending(pending);
#if BOOST_WINDOWS
        } else {
            mstd::lock_guard<mstd::mutex> lock(mutex_);

            buf->~buffer();
            p->free(buf);
            allocated_ -= p->get_requested_size();
        }
#endif
    }

    size_t count_pools()
    {
        mstd::lock_guard<mstd::mutex> lock(mutex_);
        return pools_.size();
    }

    size_t allocated()
    {
        return allocated_;
    }

    size_t reserved()
    {
        return reserved_;
    }

    void status(std::ostream & out)
    {
        mstd::lock_guard<mstd::mutex> lock(mutex_);
        out << "Allocated bytes: " << allocated_ << std::endl;
        out << "Pools: " << pools_.size() << std::endl;
        size_t used = 0;
        for(pools::const_iterator i = pools_.begin(), end = pools_.end(); i != end; ++i)
        {
            const pool * p = i->second;
            size_t cur = p->used();
            used += cur;
            out << "size: " << p->get_requested_size() << ", used: " << cur << ", allocations: " << p->allocations() << std::endl;
        }
        out << "Totally used: " << used << std::endl;
    }
#if BOOST_WINDOWS
    void lazy_release(bool value)
    {
        lazyRelease_ = value;
    }

    void direct(bool value)
    {
        direct_ = value;
    }
#endif
private:
    inline void release_pending(pending_release * pending)
    {
        mstd::lock_guard<mstd::mutex> lock(mutex_);

        for(pending_release::const_iterator i = pending->begin(), end = pending->end(); i != end; ++i)
        {
            buffer * buf = *i;
            pool * p = buffers::getPool(buf);

            buf->~buffer();
            p->free(buf);
            allocated_ -= p->get_requested_size();
        }

        pending->clear();
    }

    inline std::pair<void*, pool*> guarded_alloc(size_t size, mstd::lock_guard<mstd::mutex> & lock)
    {
        size_t idx = 0;
        while(limits_[idx] < size)
            ++idx;
        size_t granularity = granularities_[idx];
        size_t bufferSize = (size + granularity - 1) / granularity * granularity;
        bufferSize += sizeof(buffer);
        pools::iterator i = pools_.find(bufferSize);
        if(i == pools_.end())
            i = pools_.insert(bufferSize, new pool(bufferSize, blocks_[idx], this)).first;
        pool * p = i->second;
        void * raw = p->malloc();
        if(!raw)
        {
            std::cerr << "PIZDETSSSSSSSSS!!!!!11111oneoneone: " << size << ", " << bufferSize << ", " << blocks_[idx] << std::endl;
            exit(1);
        }

        allocated_ += p->get_requested_size();

        return std::make_pair(raw, p);
    }

    void fillLimits()
    {
        for(size_t g = 16; g <= 1 << 26;)
        {
            size_t old = g;
            g *= 16;
            limit(old, g, g <= 1024 ? 32 : (g <= 65536 ? 4 : 1));
        }
    }

    void limit(size_t granularity, size_t limit, size_t blocks)
    {
        granularities_.push_back(granularity);
        limits_.push_back(limit);
        blocks_.push_back(blocks);
    }

    typedef boost::ptr_unordered_map<size_t, pool> pools;

    buffers * buffers_;
    std::vector<size_t> granularities_;
    std::vector<size_t> limits_;
    std::vector<size_t> blocks_;
    mstd::mutex mutex_;
    pools pools_;
    mstd::atomic<size_t> allocated_;
    boost::thread_specific_ptr<pending_release> pending_release_;
#if BOOST_WINDOWS
    bool lazyRelease_;
    bool direct_;
#endif

    friend class scoped_allocator;
};

buffers::buffers()
{
    impl_.reset(new impl(this));
}

buffers::~buffers()
{
}

size_t buffers::pools() const
{
    return impl_->count_pools();
}

size_t buffers::allocated() const
{
    return impl_->allocated();
}

size_t buffers::reserved() const
{
    return impl_->reserved();
}

pbuffer buffers::take(size_t size)
{
    buffer * temp = impl_->take(size);
    if(temp->get_current_number_of_references())
        exit(1);
    return pbuffer(temp);
}

void buffers::status(std::ostream & out)
{
    impl_->status(out);
}

#if BOOST_WINDOWS
void buffers::lazy_release(bool value)
{
    impl_->lazy_release(value);
}

void buffers::direct(bool value)
{
    impl_->direct(value);
}
#endif

void pool::release(buffer * buf)
{
    buffers_->release(this, buf);
}

scoped_allocator::scoped_allocator(buffers & bufs)
    : bufs_(*bufs.impl_), guard_(bufs_.mutex_)
{
}

pbuffer scoped_allocator::operator()(size_t size)
{
    buffer * temp = bufs_.guarded_take(size, guard_);
    if(temp->get_current_number_of_references())
        exit(1);
    return pbuffer(temp);
}

namespace detail {

void buffer_releaser::operator()(buffer * buf) const
{
    pool * p = buf->pool_;
    if(p)
        p->release(buf);
    else
        delete [] pointer_cast<char*>(buf);
}

}

size_t malloc_size(void * ptr)
{
#if defined(BOOST_WINDOWS)
    return _msize(ptr);
#elif defined(__APPLE__)
    return ::malloc_size(ptr);
#else
    return malloc_usable_size(ptr);
#endif
}

}

#endif
