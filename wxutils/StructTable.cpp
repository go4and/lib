/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
include "pch.h"

#include "StructTable.h"

namespace wxutils {

wxString StructTableBase::zero_ = L"0";
wxString StructTableBase::one_ = L"1";

namespace {

class Helper {
    MSTD_SINGLETON_INLINE_DEFINITION(Helper);
public:
    wxString output(const boost::posix_time::ptime & time)
    {
        ss_.str(std::wstring());
        ss_ << time;
        return ss_.str();
    }
private:
    Helper()
        : facet_(new boost::posix_time::wtime_facet())
    {
        facet_->format(L"%Y-%b-%d %H:%M:%S");
        ss_.imbue(std::locale(ss_.getloc(), facet_));
    }

    std::wstringstream ss_;
    boost::posix_time::wtime_facet * facet_;
};

}

boost::optional<wxString> renderDate(const boost::posix_time::ptime & time)
{
    if(time.is_not_a_date_time())
        return boost::optional<wxString>();
    else
        return Helper::instance().output(time);
}

}
