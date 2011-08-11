#if defined(_MSC_VER)
#pragma warning(disable: 4100)
#pragma warning(disable: 4127)
#pragma warning(disable: 4244)
#endif

#if !defined(_STLP_NO_IOSTREAMS)

#include <boost/random.hpp>

#include <boost/date_time/posix_time/posix_time_types.hpp>

#include <boost/thread/tss.hpp>

#include "random.hpp"
#include "token_generator.hpp"

namespace mstd {

class token_generator_base::Impl {
public:
    template<class T>
    Impl(const T & t)
        : chars_(t)
    {}

    std::string next(size_t len)
    {
        Rand * rand = tss_.get();
        if(!rand)
            tss_.reset(rand = new Rand(chars_.c_str(), chars_.length()));

        std::string result;
        result.resize(len);
        std::generate(result.begin(), result.end(), RandRef(rand));
        return result;
    }
private:
    class Rand : public boost::noncopyable {
    public:
        Rand(const char * chars, size_t len)
            : chars_(chars),
              generator_(seed()),
              distribution_(0, static_cast<int>(len) - 1)
        {}

        char operator()()
        {
            return chars_[distribution_(generator_)];
        }
    private:    
        typedef boost::mt19937 generator_type;
        typedef boost::uniform_int<> distribution_type;

        const char * chars_;
        generator_type generator_;
        distribution_type distribution_;
    };

    class RandRef {
    public:
        explicit RandRef(Rand * r)
            : rand_(r) {}

        char operator()()
        {
            return (*rand_)();
        }
    private:
        Rand * rand_;
    };

    std::string chars_;
    boost::thread_specific_ptr<Rand> tss_;
};

token_generator_base::token_generator_base(const char * source)
    : impl_(new Impl(source))
{
}

token_generator_base::token_generator_base(const std::string & source)
    : impl_(new Impl(source))
{
}

token_generator_base::~token_generator_base()
{
}

std::string token_generator_base::next(size_t len)
{
    return impl_->next(len);
}

token_generator::token_generator()
    : token_generator_base("0123456789abcdefghijklmnopqrstuvwxyz")
{
}

}

#endif
