/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

#if !defined(BUILDING_WXUTILS)
#include <wx/defs.h>
#include <wx/timer.h>
#endif

#include "Config.h"

namespace wxutils {

namespace detail {

    class FunctionEventBase : public wxEvent {
    public:
        FunctionEventBase();
        virtual void execute() const = 0;
        virtual ~FunctionEventBase() {}
    };

    template<class F>
    class FunctionEvent : public FunctionEventBase {
    public:
        explicit FunctionEvent(const F & f)
            : f_(f)
        {
        }

        wxEvent * Clone() const
        {
            return new FunctionEvent<F>(*this);
        }

        virtual ~FunctionEvent() {}
    private:
        void execute() const
        {
            f_();
        }

        mutable F f_;
    };

    void enqueueFunctionEvent(wxEvent & evt);
}

template<class F>
void uiEnqueue(const F & f)
{
    detail::FunctionEvent<F> evt(f);
    detail::enqueueFunctionEvent(evt);
}

struct UIEnqueuer {
    typedef void result_type;

    template<class F>
    void operator()(const F & f) const
    {
        uiEnqueue(f);
    }
};

namespace detail {

    template<class F>
    class FunctionTimer : public wxTimer {
    public:
        explicit FunctionTimer(int interval, const F & f)
            : f_(f), stopped_(false)
        {
            if(wxIsMainThread())
                Start(interval);
            else
                uiEnqueue(std::bind(&wxTimer::Start, this, interval, false));
        }

        void Notify();

        virtual ~FunctionTimer() {}
    private:
        F f_;
        bool stopped_;
    };

    template<class T>
    class Delete {
    public:
        explicit Delete(T * t)
            : t_(t) {}
        
        void operator()() const
        {
            delete t_;
        }
    private:
        T * t_;
    };

    template<class F>
    void FunctionTimer<F>::Notify()
    {
        if(stopped_)
            return;
        if(!f_())
        {
            stopped_ = true;
            Stop();
            uiEnqueue(Delete<FunctionTimer<F> >(this));
        }
    }
}

template<class F>
void addTimer(int interval, const F & f)
{
    new detail::FunctionTimer<F>(interval, f);
}

template<class F>
void addTimer(const boost::posix_time::time_duration & interval, const F & f)
{
    addTimer(interval.total_milliseconds(), f);
}

template<class F>
class UIInvoker {
public:
    UIInvoker(const F & f)
        : f_(f) {}

    void operator()() const
    {
        uiEnqueue(f_);
    }

    template<class A>
    void operator()(const A & a) const
    {
        uiEnqueue(std::bind(f_, a));
    }

    template<class A, class B>
    void operator()(const A & a, const B & b) const
    {
        uiEnqueue(std::bind(f_, a, b));
    }

    template<class A, class B, class C>
    void operator()(const A & a, const B & b, const C & c) const
    {
        uiEnqueue(std::bind(f_, a, b, c));
    }

    template<class A, class B, class C, class D>
    void operator()(const A & a, const B & b, const C & c, const D & d) const
    {
        uiEnqueue(std::bind(f_, a, b, c, d));
    }
private:
    F f_;
};

template<class F>
UIInvoker<F> uiInvoker(const F & f)
{
    return UIInvoker<F>(f);
}

}
