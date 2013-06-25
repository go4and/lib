/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
pragma once

#include <boost/config.hpp>

#include <boost/cstdint.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/intrusive_ptr.hpp>

#include "config.hpp"

#include "mutex.hpp"
#include "pointer_cast.hpp"
#include "reference_counter.hpp"
#include "singleton.hpp"

#include "buffers_fwd.hpp"

namespace boost {

struct default_user_allocator_new_delete;

template <typename UserAllocator>
class pool;

class mutex;

}

namespace mstd {

class pool;
class buffers;
class buffer;

namespace detail {
    class MSTD_DECL buffer_releaser {
    public:
        void operator()(buffer * buf) const;
    };
}

class MSTD_DECL buffer : public boost::noncopyable, public mstd::reference_counter<buffer, detail::buffer_releaser> {
public:
    inline char * ptr()
    {
        return mstd::pointer_cast<char*>(this) + sizeof(*this);
    }

    inline char * data()
    {
        return ptr();
    }

    size_t buffer_size() const;    
private:
    // TODO optimize size?
    buffer(pool * p)
        : pool_(p) {}

    pool * pool_;

    friend class buffers;
    friend class detail::buffer_releaser;
};

class scoped_allocator;

class MSTD_DECL buffers : public mstd::singleton<buffers> {
public:
    size_t pools() const;
    size_t allocated() const;
    size_t reserved() const;

    pbuffer take(size_t size);

    buffers();
    ~buffers();

    void status(std::ostream & out);
#if BOOST_WINDOWS
    void lazy_release(bool value);
    void direct(bool value);
#endif
private:
    inline static buffer* create(void* dest, pool* src) { return new (dest) buffer(src); }
    inline static pool* getPool(buffer* b) { return b->pool_; }
    
    class impl;

    boost::scoped_ptr<impl> impl_;

    friend class pool;
    friend class scoped_allocator;
    friend class impl;

    MSTD_SINGLETON_DECLARATION(buffers);
};

class MSTD_DECL scoped_allocator {
public:
    explicit scoped_allocator(buffers & bufs);

    pbuffer operator()(size_t size);
private:
    buffers::impl & bufs_;
    mstd::lock_guard<mstd::mutex> guard_;
};

size_t malloc_size(void * ptr);

}
