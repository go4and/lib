#pragma once

namespace mstd {

namespace math {

template<class coord_type>
struct AnyPoint {
    coord_type x;
    coord_type y;
    
    AnyPoint(coord_type xx, coord_type yy)
        : x(xx), y(yy) {}
};

template<class T>
inline T abs(T t) { return t >= 0 ? t : -t; }

template<class T>
inline T sqr(T t) { return t * t; }

template<class O1, class O2>
inline auto sdist(const O1 & o1, const O2 & o2) -> decltype((o1.x - o2.x) * (o1.x - o2.x) + (o1.y - o2.y) * (o1.y - o2.y))
{
    return sqr(o1.x - o2.x) + sqr(o1.y - o2.y);
}

template<class O1, class O2>
inline auto dist(const O1 & o1, const O2 & o2) -> decltype(std::sqrt(sdist(o1, o2)))
{
    return std::sqrt(sdist(o1, o2));
}

template<class T, class O1, class O2>
inline T dist(const O1 & o1, const O2 & o2)
{
    return std::sqrt(static_cast<T>(sdist(o1, o2)));
}

template<class O>
inline auto length(const O & o) -> decltype(std::sqrt(sqr(o.x) + sqr(o.y)))
{
    return std::sqrt(sqr(o.x) + sqr(o.y));
}

template<class P>
inline P normalize(const P & p)
{
    auto len = length(p);
    P result;
    result.x = p.x / len;
    result.y = p.y / len;
    return result;
}

template<class O>
inline auto slength(const O & o) -> decltype(sqr(o.x) + sqr(o.y)) 
{
    return sqr(o.x) + sqr(o.y);
}

template<class O1, class O2>
inline O1 diff(const O1 & o1, const O2 & o2)
{
    return O1(o1.x - o2.x, o1.y - o2.y);
}

#ifdef ENGINE_WITH_CHIPMUNK
inline cpVect diff(const cpVect & o1, const cpVect & o2)
{
    return cpv(o1.x - o2.x, o1.y - o2.y);
}
#endif

template<class result_type, class O1, class O2>
inline result_type diff(const O1 & o1, const O2 & o2)
{
    return result_type(o1.x - o2.x, o1.y - o2.y);
}

template<class O1, class O2>
inline auto dotProduct(const O1 & o1, const O2 & o2) -> decltype(o1.x * o2.x + o1.y * o2.y)
{
    return o1.x * o2.x + o1.y * o2.y;
}

template<class O1, class O2>
inline auto crossProduct(const O1 & o1, const O2 & o2) -> decltype(o1.x * o2.y - o1.y * o2.x)
{
    return o1.x * o2.y - o1.y * o2.x;
}

template<class O1, class O2, class O3>
inline auto crossProduct(const O1 & o1, const O2 & o2, const O3 & o3) -> decltype((o2.x - o1.x) * (o3.y - o1.y) - (o2.y - o1.y) * (o3.x - o1.x))
{
    return (o2.x - o1.x) * (o3.y - o1.y) - (o2.y - o1.y) * (o3.x - o1.x);
}

template<class O1, class O2, class O3>
inline bool inRow(const O1 & o1, const O2 & o2, const O3 & o3)
{
    auto crossProduct = (o2.x - o1.x) * (o3.y - o1.y) - (o2.y - o1.y) * (o3.x - o1.x);
    return crossProduct == 0;
}

template<class O1, class O2, class O3, class S>
inline bool inRow(const O1 & o1, const O2 & o2, const O3 & o3, const S & eps)
{
    auto crossProduct = (o2.x - o1.x) * (o3.y - o1.y) - (o2.y - o1.y) * (o3.x - o1.x);
    return crossProduct >= -sqr(eps) && crossProduct <= sqr(eps);
}

template<class Poly>
inline auto doubleArea(const Poly & poly) -> decltype(crossProduct(poly.front(), poly.back()))
{
    typedef decltype(crossProduct(poly.front(), poly.back())) result_type;
    result_type result = 0;
    if(!poly.empty())
    {
        const auto * p1 = &poly.back();
        for(auto i = poly.begin(), end = poly.end(); i != end; ++i)
        {
            const auto * p2 = &*i;
            result += crossProduct(*p1, *p2);
            p1 = p2;
        }
    }
    return result;
}

template<class Poly, class Point>
bool contains(const Poly & poly, const Point & p)
{
    bool result = false;
    const auto * prev = &poly.back();
    for(auto i = poly.begin(), end = poly.end(); i != end; ++i)
    {
        const auto * cur = &*i;
        if(cur->y > prev->y)
        {
            auto dy = cur->y - prev->y;
            if(p.y >= prev->y && p.y < cur->y &&
               (p.x - prev->x) * dy <= (p.y - prev->y) * (cur->x - prev->x))
               result = !result;
        } else {
            auto dy = cur->y - prev->y;
            if(p.y < prev->y && p.y >= cur->y &&
               (p.x - prev->x) * dy >= (p.y - prev->y) * (cur->x - prev->x))
               result = !result;
        }
        prev = cur;
    }
    return result;
}

template<class O1, class O2, class O3>
inline auto lineSDist(const O1 & o1, const O2 & o2, const O3 & o3, bool withStart) -> decltype(sdist(o1, o3))
{
    typedef decltype(sdist(o1, o3)) result_type;
    result_type result = sdist(o1, o3);
    if(withStart)
        result = std::min(result, sdist(o1, o2));
    AnyPoint<decltype(o1.x - o2.x)> e(o1.x - o2.x, o1.y - o2.y);
    AnyPoint<decltype(o3.x - o2.x)> z(o3.x - o2.x, o3.y - o2.y);
    result_type s = dotProduct(e, z);
    if(s > 0)
    {
        result_type zl = slength(z);
        if(s < zl)
        {
            result_type d = (slength(e) * zl - sqr(s)) / zl;
            result = std::min(d, result);
        }
    }
    return result;
}
    
template<class O1, class O2, class O3, class Out>
inline auto lineSDist(const O1 & o1, const O2 & o2, const O3 & o3, bool withStart, Out & out) -> decltype(sdist(o1, o3))
{
    typedef decltype(sdist(o1, o3)) result_type;
    result_type result = sdist(o1, o3);
    out = o3;
    if(withStart)
    {
        result_type temp = sdist(o1, o2);
        if(temp < result)
        {
            result = temp;
            out = o2;
        }
    }
    O1 e = diff(o1, o2);
    O3 z = diff(o3, o2);
    result_type s = dotProduct(z, e);
    if(s > 0)
    {
        result_type zl = slength(z);
        if(s < zl)
        {
            result_type t = s / zl;
            e.x = o2.x + z.x * t;
            e.y = o2.y + z.y * t;
            result_type d = sdist(e, o1);
            if(d < result)
            {
                result = d;
                out = e;
            }
        }
    }
    return result;
}

template<class Poly, class O>
inline auto polySDist(const Poly & poly, const O & o) -> decltype(sdist(*poly.begin(), o))
{
    typedef decltype(sdist(*poly.begin(), o)) result_type;
    auto i = poly.begin(), end = poly.end();
    if(i != end)
    {
        auto * p = &poly.back();
        auto * c = &*i;
        result_type result = lineSDist(o, *p, *c, false);
        p = c;
        while(++i != end)
        {
            c = &*i;
            result_type current = lineSDist(o, *p, *c, false);
            if(current < result)
                result = current;
            p = c;
        }
        return result;
    } else
        return -1.0;
}

template<class Poly, class O, class Out>
inline auto polySDist(const Poly & poly, const O & o, Out & out) -> decltype(sdist(*poly.begin(), o))
{
    typedef decltype(sdist(*poly.begin(), o)) result_type;
    auto i = poly.begin(), end = poly.end();
    if(i != end)
    {
        auto * p = &poly.back();
        auto * c = &*i;
        result_type result = lineSDist(o, *p, *c, false, out);
        p = c;
        while(++i != end)
        {
            c = &*i;
            Out temp;
            result_type current = lineSDist(o, *p, *c, false, temp);
            if(current < result)
            {
                out = temp;
                result = current;
            }
            p = c;
        }
        return result;
    } else
        return -1.0;
}

template<class Point, class Poly, class S>
bool within(const Point & point, const Poly & poly, const S & eps)
{
    S d = polySDist(poly, point);
    if(d < sqr(eps))
        return eps > 0;
    return boost::geometry::within(point, poly);
}

namespace detail {
    template<class T>
    struct CmpY {
        template<class U>
        bool operator()(const T & t, const U & u) const
        {
            return t.y < u;
        }
        
        template<class U>
        bool operator()(const U & u, const T & t) const
        {
            return u < t.y;
        }
    };
}

template<class Point, class Vertex, class S>
inline bool onEdge(const Point & point, const Vertex & p1, const Vertex & p2, const S & eps)
{
    auto d = p2.y - p1.y;
    if(std::abs(d) < eps)
        return std::abs(point.y - p1.y) < eps && point.x > std::min(p1.x, p2.x) - eps && point.x < std::max(p1.x, p2.x) + eps;
    else {
        auto t = (point.y - p1.y) / d;
        return std::abs(point.x - (p1.x + (p2.x - p1.x) * t)) < eps;
    }
}

template<class Point, class Poly, class Edges, class S>
inline bool onEdge(const Point & point, const Poly & poly, const Edges & edges, const S & eps)
{
    for(auto j = edges.begin(), jend = edges.end(); j != jend; ++j)
    {
        auto & p1 = poly[*j];
        auto & p2 = *j == 0 ? poly.back() : poly[*j - 1];
        if(onEdge(point, p1, p2, eps))
            return true;
    }
    return false;
}

template<class Point, class Poly, class Mapping, class S>
inline bool within(const Point & point, const Poly & poly, const Mapping & mapping, const S & ieps)
{
    auto i = std::lower_bound(mapping.begin(), mapping.end(), point.y, detail::CmpY<decltype(*mapping.begin())>());
    if(i == mapping.begin())
    {
        if(ieps < 0 || point.y < i->y - ieps)
            return false;
        return onEdge(point, poly, i->edges, ieps);
    }
    if(i == mapping.end())
    {
        if(ieps < 0)
            return false;
        --i;
        if(point.y > i->y + ieps)
            return false;
        if(!i->edges.empty() && onEdge(point, poly, i->edges, ieps))
            return true;
        --i;
        return onEdge(point, poly, i->edges, ieps);
    }
    S eps = std::abs(ieps);
    if(i->y - point.y < eps)
    {
        if(onEdge(point, poly, i->edges, eps))
            return ieps > 0;
    }
    --i;
    if(onEdge(point, poly, i->edges, eps))
        return ieps > 0;
    bool result = false;
    for(auto j = i->edges.begin(), jend = i->edges.end(); j != jend; ++j)
    {
        auto & p1 = poly[*j];
        auto & p2 = *j == 0 ? poly.back() : poly[*j - 1];
        auto d = p2.y - p1.y;
        if(std::abs(d) > eps)
        {
            auto t = (point.y - p1.y) / d;
            if(point.x + eps > p1.x + (p2.x - p1.x) * t)
                result = !result;
        }
    }
    return result;
}

template<class T, class Gen>
T randomRound(double a, Gen & gen)
{
    double i = std::floor(a);
    a -= i;
    return static_cast<T>(i) + (gen() > a ? 0 : 1);
}

template<class T, class Gen>
T randomInt(double a, double b, Gen & gen)
{
    return randomRound<T>(gen() * (b - a) + a, gen);
}

}

using math::sqr;
using math::sdist;
using math::diff;
using math::dotProduct;
using math::crossProduct;
using math::doubleArea;
using math::contains;
using math::lineSDist;
using math::polySDist;
using math::within;
using math::randomRound;
using math::randomInt;

}
