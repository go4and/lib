#pragma once

#ifndef MCRYPT_BUILDING
#include <string>
#include <vector>

#include <boost/optional.hpp>
#endif

#include "Config.h"

namespace mcrypt {

    namespace detail {
        extern MCRYPT_DECL const char * hexTable;

        template<class Out, class Input>
        Out visualizeImpl(const Input & data)
        {
            Out result;
            result.reserve(data.size() * 2);
            for(typename Input::const_iterator i = data.begin(), end = data.end(); i != end; ++i)
            {
                unsigned char b = *i;
                result.push_back(hexTable[b >> 4]);
                result.push_back(hexTable[b & 0xf]);
            }
            return result;
        }
    }

    template<class Container>
    std::string visualize(const Container & data)
    {
        return detail::visualizeImpl<std::string>(data);
    }

    template<class Container>
    std::wstring wvisualize(const Container & data)
    {
        return detail::visualizeImpl<std::wstring>(data);
    }

    template<class Container>
    std::string visualize(const boost::optional<Container> & opt)
    {
        return opt ? visualize(opt.get()) : std::string();
    }

    template<class Container>
    std::wstring wvisualize(const boost::optional<Container> & opt)
    {
        return opt ? wvisualize(opt.get()) : std::wstring();
    }
    
    MCRYPT_DECL void bfcrypt(const std::string & password, const char * begin, const char * end, std::vector<char> & out);
    MCRYPT_DECL void bfdecrypt(const std::string & password, const char * begin, const char * end, std::vector<char> & out);
}
