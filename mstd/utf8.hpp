/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

#include <iterator>
#include <memory>

#if !defined(_STLP_NO_IOSTREAMS)
#include <iosfwd>
#endif

#include <boost/assert.hpp>
#include <boost/static_assert.hpp>

#include <boost/type_traits/remove_cv.hpp>
#include <boost/type_traits/remove_reference.hpp>

#include <boost/utility/enable_if.hpp>

namespace mstd {

    namespace detail {

        template<size_t sz>
        struct ones_helper {
            static const unsigned int value = ones_helper<sz / 2>::value ^ (ones_helper<sz / 2>::value << (8 * sz / 2));
        };

        template<>
        struct ones_helper<1> {
            static const unsigned int value = 0xff;
        };

        template<class T>
        struct ones {
            static const T value = ones_helper<sizeof(T)>::value;
        };
        
        template<class Iterator>
        struct iterator_value_type {
            typedef typename std::iterator_traits<Iterator>::reference temp_type;
            typedef typename boost::remove_reference<typename boost::remove_cv<temp_type>::type>::type type;
        };
        
#if !defined(_STLP_NO_IOSTREAMS)
        template<class Char>
        struct iterator_value_type<std::ostreambuf_iterator<Char> > {
            typedef Char type;
        };

        template<class Container>
        struct iterator_value_type<std::back_insert_iterator<Container> > {
            typedef typename Container::value_type type;
        };
#endif
    }
    
    const size_t max_utf8_length = 3;
    
    template<class In>
    typename boost::disable_if_c<sizeof(In) < 2, size_t>::type
    utf8_length(In begin, In end)
    {
        typedef typename detail::iterator_value_type<In>::type value_type;
        
        static const value_type ones = detail::ones<value_type>::value;

        size_t result = 0;

        for (; begin != end; ++begin)
        {
            value_type c = *begin;
            if(c & (ones ^ 0x7f))
                break;
            ++result;
        }

        for (; begin != end; ++begin)
        {
            value_type c = *begin;
            if (!(c & (ones ^ 0x7f)))
                ++result;
            else if (c & (ones ^ 0x07ff))
                result += 3;
            else
                result += 2;
        }

        return result;
    }

    template<class In, class Out>
    Out utf8(In begin, In end, Out out)
    {
        typedef typename detail::iterator_value_type<In>::type value_type;
        typedef typename detail::iterator_value_type<Out>::type out_type;
        
        static const value_type ones = detail::ones<value_type>::value;

        for (; begin != end; ++begin)
        {
            value_type c = *begin;
            if(c & (ones ^ 0x7f))
                break;
            *out = static_cast<out_type>(c);
            ++out;
        }

        for (; begin != end; ++begin)
        {
            value_type c = *begin;
            if (!(c & (ones ^ 0x7f))) {
                *out = static_cast<out_type>(c);
                ++out;
            } else if (c & (ones ^ 0x07ff)) {
                *out = static_cast<out_type>(0xe0 | ((c >> 12) & 0x0f));
                *++out = static_cast<out_type>(0x80 | ((c >> 6) & 0x3f));
                *++out = static_cast<out_type>(0x80 | (c & 0x3f));
                ++out;
            } else {
                *out = static_cast<out_type>(0xc0 | ((c >> 6) & 0x1f));
                *++out = static_cast<out_type>(0x80 | (c & 0x3f));
                ++out;
            }
        }

        return out;
    }

    template<class In, class Out>
    Out deutf8(In begin, In end, Out out)
    {
        typedef typename detail::iterator_value_type<In>::type value_type;
        typedef typename detail::iterator_value_type<Out>::type out_type;

        static const value_type ones = detail::ones<value_type>::value;

        for (; begin != end; ++begin)
        {
            value_type c = *begin;
            if(c & (ones ^ 0x7f))
                break;
            *out = static_cast<out_type>(c);
            ++out;
        }

        for (; begin != end; ++begin)
        {
            value_type c = *begin;
            switch((c >> 4) & 0xf) {
                case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7:
                    *out = static_cast<out_type>(c);
                    break;
                case 12: case 13:
                    ++begin;
                    BOOST_ASSERT(begin != end);
                    BOOST_ASSERT((*begin & 0xc0) == 0x80);
                    if(begin == end || (*begin & 0xc0) != 0x80)
                        return out;
                    *out = (static_cast<out_type>(c & 0x1f) << 6) ^ static_cast<out_type>(*begin & 0x3f);
                    break;
                case 14:
                    {
                        ++begin;
                        BOOST_ASSERT(begin != end);
                        BOOST_ASSERT((*begin & 0xc0) == 0x80);
                        if(begin == end || (*begin & 0xc0) != 0x80)
                            return out;
                        out_type c2 = static_cast<out_type>(*begin);
                        ++begin;
                        BOOST_ASSERT(begin != end);
                        BOOST_ASSERT((*begin & 0xc0) == 0x80);
                        if(begin == end || (*begin & 0xc0) != 0x80)
                            return out;
                        *out = (static_cast<out_type>(c & 0x0f) << 12) ^ ((c2 & 0x3f) << 6) ^ static_cast<out_type>(*begin & 0x3f);
                    }
                    break;
                default:
                    BOOST_ASSERT(false);
                    *out = 0;
                    break;
            }
            ++out;
        }
        
        return out;
    }

}
