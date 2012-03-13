#include "pch.h"

#include "RectUtils.h"

namespace wxutils {

wxRect intersect(const wxRect & lhs, const wxRect & rhs)
{
    wxPoint leftTop(std::max(lhs.x, rhs.x), std::max(lhs.y, rhs.y));
    wxPoint rightBottom(std::min(lhs.x + lhs.width, rhs.x + rhs.width),
                        std::min(lhs.y + lhs.height, rhs.y + rhs.height));
    return wxRect(leftTop, wxSize(std::max(rightBottom.x - leftTop.x, 0), std::max(rightBottom.y - leftTop.y, 0)));
}

bool contains(const wxRect & rect, const wxPoint & point)
{
    return point.x >= rect.x && point.x < rect.x + rect.width &&
           point.y >= rect.y && point.y < rect.y + rect.height;
}

#if BOOST_WINDOWS
RECT toRect(const wxRect & rect)
{
    RECT result = { rect.x, rect.y, rect.GetRight(), rect.GetBottom() };
    return result;
}
#endif

void order(wxPoint & lt, wxPoint & rb)
{
    int t = std::min(lt.x, rb.x);
    rb.x = t ^ lt.x ^ rb.x;
    lt.x = t;

    t = std::min(lt.y, rb.y);
    rb.y = t ^ lt.y ^ rb.y;
    lt.y = t;
}

bool contains(const std::vector<wxPoint> & points, const wxPoint & point)
{
    return !points.empty() && contains(points.size(), &points[0], point);
}

bool contains(int n, const wxPoint * poly, const wxPoint & p)
{
    bool result = false;
    wxPoint prev = poly[n - 1];
    for(int i = 0; i != n; ++i)
    {
        wxPoint cur = poly[i];
        if(cur.y > prev.y)
        {
            boost::int64_t dy = cur.y - prev.y;
            if(p.y >= prev.y && p.y < cur.y && 
               (p.x - prev.x) * dy <= static_cast<boost::int64_t>(p.y - prev.y) * (cur.x - prev.x))
               result = !result;
        } else {
            boost::int64_t dy = cur.y - prev.y;
            if(p.y < prev.y && p.y >= cur.y && 
               (p.x - prev.x) * dy >= static_cast<boost::int64_t>(p.y - prev.y) * (cur.x - prev.x))
               result = !result;
        }
        prev = cur;
    }
    return result;
}

}
