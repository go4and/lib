/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
namespace wxutils {

template<class Base, class Data>
class AttachData : public Base {
public:
    AttachData() {}

    BOOST_PP_REPEAT_FROM_TO(
        1, BOOST_PP_INC(WXUTILS_ATTACH_DATA_MAX_ARITY),
        WXUTILS_ATTACH_DATA_PRIVATE_CTR_DEF, _)

    const Data & attachedData() const
    {
        return data_;
    }

    Data & attachedData()
    {
        return data_;
    }

    void listen(const boost::function<void()> & listener)
    {
        deathListener_ = listener;
    }

    ~AttachData()
    {
        if(!deathListener_.empty())
            deathListener_();
    }
private:
    Data data_;
    boost::function<void()> deathListener_;
};

}
