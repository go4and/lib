/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
namespace mstd {

template<class T>
class PImpl {
protected:
    PImpl() : impl_(new Impl) {}
    
    BOOST_PP_REPEAT_FROM_TO(
        1, BOOST_PP_INC(MSTD_PIMPL_MAX_ARITY),
        MSTD_PIMPL_PRIVATE_CTR_DEF, _ )

    ~PImpl() {}

    class Impl;
    
    const Impl & impl() const
    {
        return *impl_;
    }
    
    Impl & impl()
    {
        return *impl_;
    }

    void copyFrom(const Impl & src)
    {
        boost::scoped_ptr<Impl> newImpl(new Impl(src));
        impl_.swap(newImpl);
    }

    void swap(PImpl & rhs)
    {
        impl_.swap(rhs.impl_);
    }
private:    
    boost::scoped_ptr<Impl> impl_;
};

#undef MSTD_PIMPL_PRIVATE_CTR_DEF

}
