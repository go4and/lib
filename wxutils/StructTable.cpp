/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#include "pch.h"

#include "StructTable.h"

namespace wxutils {

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

    inline const wxString & renderBool(bool v)
    {
        return v ? one_ : zero_;
    }
private:
    Helper()
        : facet_(new boost::posix_time::wtime_facet()),
          zero_(L"0"), one_(L"1")
    {
        facet_->format(L"%Y-%b-%d %H:%M:%S");
        ss_.imbue(std::locale(ss_.getloc(), facet_));
    }

    std::wstringstream ss_;
    boost::posix_time::wtime_facet * facet_;
    wxString zero_;
    wxString one_;
};

}

boost::optional<wxString> renderDate(const boost::posix_time::ptime & time)
{
    if(time.is_not_a_date_time())
        return boost::optional<wxString>();
    else
        return Helper::instance().output(time);
}

const wxString & renderBool(bool v)
{
    return Helper::instance().renderBool(v);
}

void refreshGridRect(wxGrid * grid, int row1, int col1, int row2, int col2)
{
    wxRect rect = grid->CellToRect(row1, col1);
    wxRect temp = grid->CellToRect(row2, col2);
    rect.SetBottom(temp.GetBottom());
    rect.SetRight(temp.GetRight());
    rect.SetLeftTop(grid->CalcScrolledPosition(rect.GetLeftTop()));
    grid->GetGridWindow()->Refresh(false, &rect);
}

}
