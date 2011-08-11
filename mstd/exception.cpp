#if !defined(_STLP_NO_IOSTREAMS)

#if defined(_MSC_VER)
#pragma warning(disable: 4512)
#endif

#include <boost/exception/diagnostic_information.hpp>

#include "exception.hpp"

namespace mstd {

void BoostExceptionOut::out(std::ostream & stream) const
{
    stream << mstd::get_error_message(*exc_) << " (" << diagnostic_information(*exc_) << ")";
}

void StdExceptionOut::out(std::ostream &stream) const
{
    stream << exc_->what();
}

}

#endif
