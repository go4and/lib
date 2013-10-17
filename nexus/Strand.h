/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

#ifndef NEXUS_BUILDING
#include <boost/asio/io_service.hpp>

#include <mstd/spinlock.hpp>
#endif

namespace nexus {

template<class Mutex = mstd::spinlock>
class Strand : boost::noncopyable, public boost::asio::detail::operation {
private:
    class OnDispatchExit;
    class StrandRef;
public:
    explicit Strand(boost::asio::io_service & ios)
        : boost::asio::detail::operation(&Strand::doComplete),
          ios_(boost::asio::use_service<boost::asio::detail::io_service_impl>(ios)), locked_(false)
    {
    }

    template<class Handler>
    void dispatch(BOOST_ASIO_MOVE_ARG(Handler) handler)
    {
        if (boost::asio::detail::call_stack<Strand>::contains(this))
        {
            boost::asio::detail::fenced_block b(boost::asio::detail::fenced_block::full);
            boost_asio_handler_invoke_helpers::invoke(handler, handler);
            return;
        }

        typedef boost::asio::detail::completion_handler<Handler> op;
        typename op::ptr p = { boost::addressof(handler), boost_asio_handler_alloc_helpers::allocate(sizeof(op), handler), 0 };
        p.p = new (p.v) op(handler);

        bool dispatchImmediately = doDispatch(p.p);
        boost::asio::detail::operation* o = p.p;
        p.v = p.p = 0;

        if(dispatchImmediately)
        {
            typename boost::asio::detail::call_stack<Strand>::context ctx(this);

            OnDispatchExit onExit(*this);
            boost::asio::detail::completion_handler<Handler>::do_complete(&ios_, o, boost::system::error_code(), 0);
        }
    }

    template <typename Handler>
    void post(Handler handler)
    {
        typedef boost::asio::detail::completion_handler<Handler> op;
        typename op::ptr p = { boost::addressof(handler), boost_asio_handler_alloc_helpers::allocate(sizeof(op), handler), 0 };
        p.p = new (p.v) op(handler);

        doPost(p.p);
        p.v = p.p = 0;
    }

    template <typename Handler>
    boost::asio::detail::wrapped_handler<StrandRef, Handler> wrap(Handler handler)
    {
        return boost::asio::detail::wrapped_handler<StrandRef, Handler>(StrandRef(*this), handler);
    }
private:
    bool doDispatch(boost::asio::detail::operation* op)
    {
        bool can_dispatch = ios_.can_dispatch();
        mutex_.lock();
        if (can_dispatch && !locked_)
        {
            locked_ = true;
            mutex_.unlock();
            return true;
        }

        if (locked_)
        {
            waiting_queue_.push(op);
            mutex_.unlock();
        } else {
            locked_ = true;
            mutex_.unlock();
            ready_queue_.push(op);
            ios_.post_immediate_completion(this);
        }

        return false;
    }

    void doPost(boost::asio::detail::operation* op)
    {
        mutex_.lock();
        if(locked_)
        {
            waiting_queue_.push(op);
            mutex_.unlock();
        } else {
            locked_ = true;
            mutex_.unlock();
            ready_queue_.push(op);
            ios_.post_immediate_completion(this);
        }
    }

    static void doComplete(boost::asio::detail::io_service_impl * owner, boost::asio::detail::operation* base, const boost::system::error_code & ec, std::size_t bytes_transferred)
    {
        if (owner)
        {
            Strand * strand = static_cast<Strand*>(base);

            typename boost::asio::detail::call_stack<Strand>::context ctx(strand);

            OnDoCompleteExit onExit(*strand);

            while(boost::asio::detail::operation* o = strand->ready_queue_.front())
            {
                strand->ready_queue_.pop();
                o->complete(*owner, ec, 0);
            }
        }
    }

    class OnDispatchExit {
    public:
        explicit OnDispatchExit(Strand<Mutex> & strand)
            : strand_(strand)
        {
        }

        ~OnDispatchExit()
        {
            strand_.mutex_.lock();
            strand_.ready_queue_.push(strand_.waiting_queue_);
            bool more_handlers = strand_.locked_ = !strand_.ready_queue_.empty();
            strand_.mutex_.unlock();

            if (more_handlers)
                strand_.ios_.post_immediate_completion(&strand_);
        }
    private:
        Strand<Mutex> & strand_;
    };

    class OnDoCompleteExit {
    public:
        explicit OnDoCompleteExit(Strand<Mutex> & strand)
            : strand_(strand)
        {
        }

        ~OnDoCompleteExit()
        {
            strand_.mutex_.lock();
            strand_.ready_queue_.push(strand_.waiting_queue_);
            bool more_handlers = strand_.locked_ = !strand_.ready_queue_.empty();
            strand_.mutex_.unlock();

            if (more_handlers)
                strand_.ios_.post_private_immediate_completion(&strand_);
        }
    private:
        Strand<Mutex> & strand_;
    };
    
    class StrandRef {
    public:
        explicit StrandRef(Strand & strand)
            : strand_(strand)
        {
        }

        template<class Handler>
        void dispatch(BOOST_ASIO_MOVE_ARG(Handler) handler)
        {
            strand_.dispatch(BOOST_ASIO_MOVE_CAST(Handler)(handler));
        }
    private:
        Strand & strand_;
    };

    boost::asio::detail::io_service_impl & ios_;
    Mutex mutex_;
    bool locked_;
    boost::asio::detail::op_queue<boost::asio::detail::operation> waiting_queue_;
    boost::asio::detail::op_queue<boost::asio::detail::operation> ready_queue_;
};

}
