/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

#ifndef NEXUS_BUILDING

#include <vector>
#include <deque>
#include <unordered_set>

#include <boost/array.hpp>

#include <boost/mpl/find_if.hpp>
#include <boost/mpl/vector.hpp>

#include <boost/preprocessor/repetition/enum_binary_params.hpp>
#include <boost/preprocessor/repetition/repeat_from_to.hpp>

#include <boost/type_traits/is_pod.hpp>
#include <boost/type_traits/is_same.hpp>

#include <boost/range/iterator_range.hpp>

#include <boost/unordered/unordered_set_fwd.hpp>

#include <mstd/exception.hpp>

#endif

#include "Config.h"

#include "Buffer.h"
#include "Packet.h"

#include "PacketWriter.h"

namespace boost {
    template<class T>
    class reference_wrapper;

    template<class T>
    class optional;

    namespace multi_index {
        template<class Value, class Indexes, class Allocator>
        class multi_index_container;
    }
}

namespace nexus {

template<class Type>
struct PodPacker {
    typedef PodPacker<Type> type;

    BOOST_STATIC_ASSERT((boost::is_pod<Type>::value));

    static size_t packSize(const Type&)
    {
        return sizeof(Type);
    }
    
    template<class U>
    static void pack(U & out, const Type & t)
    {
        write<Type>(out, t);
    }
};

class NEXUS_DECL ShortStringRef {
public:
    typedef ShortStringRef packer;

    ShortStringRef(const std::string & str)
        : ref_(&str) {}

    size_t packSize() const;
    void pack(char *& out) const;
private:
    const std::string * ref_;
};

inline ShortStringRef shortString(const std::string & str)
{
    return ShortStringRef(str);
}

template<class Ch>
class CStringRef;

template<class Ch>
class CString;

class NEXUS_DECL CStringPacker {
public:
    static size_t packSize(CStringRef<char> input);
    static void pack(char *& out, CStringRef<char> input);

    static size_t packSize(CString<char> input);
    static void pack(char *& out, CString<char> input);

    static size_t packSize(CStringRef<wchar_t> input);
    static void pack(char *& out, CStringRef<wchar_t> input);

    static size_t packSize(CString<wchar_t> input);
    static void pack(char *& out, CString<wchar_t> input);
};

template<class Ch>
class CStringRef {
public:
    typedef CStringPacker packer;

    CStringRef(const std::basic_string<Ch> & str)
        : str_(&str) {}

    const std::basic_string<Ch> & get() const
    {
        return *str_;
    }
private:
    const std::basic_string<Ch> * str_;
};

template<class Ch>
CStringRef<Ch> cString(const std::basic_string<Ch> & str)
{
    return CStringRef<Ch>(str);
}

template<class Ch>
class CString {
public:
    typedef CStringPacker packer;

    CString(const Ch * str)
        : str_(str) {}

    const Ch * get() const
    {
        return str_;
    }
private:
    const Ch * str_;
};

template<class Ch>
CString<Ch> cString(const Ch * str)
{
    return CString<Ch>(str);
}

struct StringWithLen {
    const char * data;
    size_t len;

    StringWithLen(const char * d, size_t l)
        : data(d), len(l) {}
};

struct NEXUS_DECL StringPacker {
    static size_t packSize(const char * input);
    static void pack(char *& out, const char * input);

    static size_t packSize(const std::string & input);
    static void pack(char *& out, const std::string & input);

    static size_t packSize(const StringWithLen & input);
    static void pack(char *& out, const StringWithLen & input);
};

struct NEXUS_DECL RawDataPacker {
    static size_t packSize(const std::pair<const char*, const char*> & p);
    static void pack(char *& out, const std::pair<const char*, const char*> & p);
};

template<class Packer>
struct CollectionPacker {
    template<class Collection>
    static size_t packSize(const Collection & col)
    {
        size_t result = 2;
        for(typename Collection::const_iterator i = col.begin(), end = col.end(); i != end; ++i)
            result += Packer::packSize(*i);
        return result;
    }

    template<class Collection>
    static void pack(char *& out, const Collection & col)
    {
        size_t count = 0;
        char * oldOut = out;
        out += 2;
        for(typename Collection::const_iterator i = col.begin(), end = col.end(); i != end; ++i, ++count)
            Packer::pack(out, *i);
        write<boost::uint16_t>(oldOut, count);
    }
};

template<class T>
class SinglePacker {
public:
    typedef SinglePacker packer;
    
    explicit SinglePacker(const T & t)
        : value_(t) {}

    size_t packSize() const;
    void pack(char *& out) const;
private:
    const T & value_;
};

template<class T>
SinglePacker<T> single(const T & t)
{
    return SinglePacker<T>(t);
}

template<class Packer>
struct OptionalPacker {
    template<class Value>
    static size_t packSize(const Value & value)
    {
        return value ? Packer::packSize(value.get()) : 0;
    }

    template<class Value>
    static void pack(char *& out, const Value & value)
    {
        if(value)
            Packer::pack(out, value.get());
    }
};

struct DerefPacker {
    template<class T>
    static size_t packSize(const T & t);
    template<class T>
    static void pack(char *& out, const T & t);
};

template<class Packer>
struct ReferencePacker {
    template<class Reference>
    static size_t packSize(const Reference & ref)
    {
        return Packer::packSize(ref.get());
    }

    template<class Reference>
    static void pack(char *& out, const Reference & ref)
    {
        Packer::pack(out, ref.get());
    }
};

class NEXUS_DECL BufferAsString {
public:
    typedef BufferAsString packer;

    BufferAsString(const Buffer & value);

    size_t packSize() const;
    void pack(char *& pos) const;
private:
    const Buffer & value_;
};

inline BufferAsString asString(const Buffer & buffer)
{
    return BufferAsString(buffer);
}

typedef boost::mpl::vector<
            boost::mpl::pair<   std::string, StringPacker>,
            boost::mpl::pair<std::pair<const char*, const char*>, RawDataPacker>,
            boost::mpl::pair<std::pair<char*, char*>, RawDataPacker>
        > CustomPackers;

template<class Pair, class Type>
struct CustomPackerHelper {
    typedef typename Pair::second type;
};

template<class Impl>
struct ForwardPacker {
    static size_t packSize(const Impl & impl)
    {
        return impl.packSize();
    }
    
    template<class U>
    static void pack(U & pos, const Impl & impl)
    {
        impl.pack(pos);
    }
};

template<class Type>
struct CustomPackerHelper<boost::mpl::void_, Type> {
    typedef typename Type::packer impl_type;
    typedef typename
        boost::mpl::if_<
            boost::is_same<impl_type, Type>,
            ForwardPacker<impl_type>,
            impl_type
        >::type type;
};

template<class Type>
struct CustomPacker {
    typedef typename boost::mpl::find_if<CustomPackers, boost::is_same<boost::mpl::first<boost::mpl::_>, Type> >::type step1_type;
    typedef typename boost::mpl::deref<step1_type>::type step2_type;
    typedef typename CustomPackerHelper<step2_type, Type>::type type;
};

template<class T>
struct GetPacker {
    typedef typename boost::mpl::if_<boost::mpl::and_<boost::mpl::not_<boost::is_pointer<T> >, boost::is_pod<T> >,
                                     PodPacker<boost::mpl::_>, CustomPacker<boost::mpl::_> >::type step1_type;
    typedef typename boost::mpl::apply<step1_type, T>::type type;
};

template<>
struct GetPacker<const char*> {
    typedef StringPacker type;
};

template<class T>
struct GetPacker<std::vector<T> > {
    typedef typename GetPacker<typename boost::remove_cv<T>::type>::type packer;
    typedef CollectionPacker<packer> type;
};

template<class T>
struct GetPacker<std::deque<T> > {
    typedef typename GetPacker<typename boost::remove_cv<T>::type>::type packer;
    typedef CollectionPacker<packer> type;
};

template<class T, class Hasher, class Eq>
struct GetPacker<std::unordered_set<T, Hasher, Eq> > {
    typedef typename GetPacker<typename boost::remove_cv<T>::type>::type packer;
    typedef CollectionPacker<packer> type;
};

template<class Value, class Indexes, class Allocator>
struct GetPacker<boost::multi_index::multi_index_container<Value, Indexes, Allocator> > {
    typedef typename GetPacker<typename boost::remove_cv<Value>::type>::type packer;
    typedef CollectionPacker<packer> type;
};

template<class It>
struct GetPacker<boost::iterator_range<It> > {
    typedef typename std::iterator_traits<It>::value_type value_type;
    typedef typename GetPacker<typename boost::remove_cv<value_type>::type>::type packer;
    typedef CollectionPacker<packer> type;
};

template<class T>
struct GetPacker<boost::optional<T> > {
    typedef typename GetPacker<typename boost::remove_cv<typename boost::remove_reference<T>::type>::type>::type packer;
    typedef OptionalPacker<packer> type;
};

template<class T>
struct GetPacker<T *> {
    typedef typename GetPacker<typename boost::remove_cv<T>::type>::type packer;
    typedef DerefPacker type;
};

template<class T>
struct GetPacker<boost::shared_ptr<T> > {
    typedef typename GetPacker<typename boost::remove_cv<T>::type>::type packer;
    typedef DerefPacker type;
};

template<class T>
struct GetPacker<boost::intrusive_ptr<T> > {
    typedef typename GetPacker<typename boost::remove_cv<T>::type>::type packer;
    typedef DerefPacker type;
};

template<class T, size_t n>
struct GetPacker<boost::array<T, n> > {
    typedef typename GetPacker<typename boost::remove_cv<T>::type>::type packer;
    typedef CollectionPacker<packer> type;
};

template<class T>
struct GetPacker<boost::reference_wrapper<T> > {
    typedef typename GetPacker<typename boost::remove_cv<T>::type>::type packer;
    typedef ReferencePacker<packer> type;
};

template<class T>
size_t SinglePacker<T>::packSize() const
{
    return 2 + GetPacker<T>::type::packSize(value_);
}

template<class T>
void SinglePacker<T>::pack(char *& pos) const
{
    write<boost::uint16_t>(pos, 1);
    GetPacker<T>::type::pack(pos, value_);
}

struct GetSize {
    typedef size_t result_type;

    template<class T>
    size_t operator()(const T & t, size_t val) const
    {
        typedef typename GetPacker<T>::type packer;
        return val + packer::packSize(t);
    }
};

class Pack {
public:
    explicit Pack(char *& pos)
        : pos_(&pos) {}

    template<class T>
    void operator()(const T & t) const
    {
        typedef typename GetPacker<T>::type packer;
        packer::pack(*pos_, t);
    }
private:
    char ** pos_;
};

template<class Tuple>
struct TuplePacker {
    static size_t packSize(const Tuple & tuple)
    {
        return accumulate(tuple, 0, GetSize());
    }

    static void pack(char *& pos, const Tuple & tuple)
    {
        for_each(tuple, Pack(pos));
    }
};

#define NEXUS_PACKET_PACKER_MAX_ARITY 25

#define NEXUS_PACKET_PACKER_ADD_SIZE(z, n, data) \
    + GetPacker<T##n>::type::packSize(x##n) \
    /**/

#define NEXUS_PACKET_PACKER_TUPLE_SIZE_DEF(z, n, data) \
    template <BOOST_PP_ENUM_PARAMS(n, typename T)> \
    size_t tupleSize(BOOST_PP_ENUM_BINARY_PARAMS(n, const T, & x)) \
    { \
        return 0 BOOST_PP_REPEAT(n, NEXUS_PACKET_PACKER_ADD_SIZE, ~); \
    } \
    /**/

BOOST_PP_REPEAT_FROM_TO(
    1, BOOST_PP_INC(NEXUS_PACKET_PACKER_MAX_ARITY),
    NEXUS_PACKET_PACKER_TUPLE_SIZE_DEF, ~ )

#define NEXUS_PACKET_PACKER_DO_PACK(z, n, data) \
    GetPacker<T##n>::type::pack(pos, x##n); \
    /**/

#define NEXUS_PACKET_PACKER_TUPLE_PACK_DEF(z, n, data) \
    template <BOOST_PP_ENUM_PARAMS(n, typename T)> \
    void tuplePack(char *& pos, BOOST_PP_ENUM_BINARY_PARAMS(n, const T, & x)) \
    { \
        BOOST_PP_REPEAT_FROM_TO_##z(0, n, NEXUS_PACKET_PACKER_DO_PACK, _); \
    } \
    /**/

BOOST_PP_REPEAT_FROM_TO(
    1, BOOST_PP_INC(NEXUS_PACKET_PACKER_MAX_ARITY),
    NEXUS_PACKET_PACKER_TUPLE_PACK_DEF, _ )

template<class T>
size_t DerefPacker::packSize(const T & t)
{
    return t ? nexus::tupleSize(*t) : 0;
}

template<class T>
void DerefPacker::pack(char *& out, const T & t)
{
    if(t)
        nexus::tuplePack(out, *t);
}

class InvalidPackSizeTag;
typedef mstd::own_exception<InvalidPackSizeTag> InvalidPackSizeException;
class PacketCodeTag;
typedef boost::error_info<PacketCodeTag, int> PacketCodeInfo;
class ExpectedSizeTag;
typedef boost::error_info<ExpectedSizeTag, size_t> ExpectedSizeInfo;
class FoundSizeTag;
typedef boost::error_info<FoundSizeTag, size_t> FoundSizeInfo;

#define NEXUS_PACKET_PACKER_INIT_POS() \
    nexus::Buffer result(size); \
    char * pos = result.data(); \
    char * start = pos; \
    /**/

#define NEXUS_PACKET_PACKER_RETURN() \
    (void) start; \
    BOOST_ASSERT(static_cast<size_t>(pos - start) == size); \
    return result; \
    /**/

#define NEXUS_PACKET_PACKER_PACK_DEF(z, n, data) \
    template <BOOST_PP_ENUM_PARAMS(n, typename T)> \
    Buffer pack(BOOST_PP_ENUM_BINARY_PARAMS(n, const T, & x)) \
    { \
        size_t size = nexus::tupleSize(BOOST_PP_ENUM_PARAMS(n, x)); \
        NEXUS_PACKET_PACKER_INIT_POS(); \
        nexus::tuplePack(pos, BOOST_PP_ENUM_PARAMS(n, x)); \
        NEXUS_PACKET_PACKER_RETURN(); \
    } \
    /**/

BOOST_PP_REPEAT_FROM_TO(
    1, BOOST_PP_INC(NEXUS_PACKET_PACKER_MAX_ARITY),
    NEXUS_PACKET_PACKER_PACK_DEF, _ )

#define NEXUS_PACKET_PACKER_PACK_VEC_DEF(z, n, data) \
    template <BOOST_PP_ENUM_PARAMS(n, typename T)> \
    void pack(std::vector<char> & out, BOOST_PP_ENUM_BINARY_PARAMS(n, const T, & x)) \
    { \
        size_t size = nexus::tupleSize(BOOST_PP_ENUM_PARAMS(n, x)) + 4; \
        size_t oldSize = out.size(); \
        out.resize(oldSize + size); \
        char * pos = &out[oldSize]; \
        char * start = pos; \
        nexus::write(pos, static_cast<uint32_t>(size - 4)); \
        nexus::tuplePack(pos, BOOST_PP_ENUM_PARAMS(n, x)); \
        (void) start; \
        BOOST_ASSERT(static_cast<size_t>(pos - start) == size); \
    } \
    /**/

BOOST_PP_REPEAT_FROM_TO(
    1, BOOST_PP_INC(NEXUS_PACKET_PACKER_MAX_ARITY),
    NEXUS_PACKET_PACKER_PACK_VEC_DEF, _ )

void badExpectedSize(PacketCode code, size_t expectedSize, size_t size);

inline Buffer packCD(PacketCode code, size_t expectedSize)
{
    size_t size = 1;
    if(size != expectedSize + 1)
        badExpectedSize(code, expectedSize, size);
    NEXUS_PACKET_PACKER_INIT_POS();
    write<unsigned char>(pos, code);
    NEXUS_PACKET_PACKER_RETURN();
}

#define NEXUS_PACKET_PACKER_PACK_CD_DEF(z, n, data) \
    template <BOOST_PP_ENUM_PARAMS(n, typename T)> \
    Buffer packCD(PacketCode code, size_t expectedSize, BOOST_PP_ENUM_BINARY_PARAMS(n, const T, & x)) \
    { \
        size_t size = 1 + nexus::tupleSize(BOOST_PP_ENUM_PARAMS(n, x)); \
        if(size != expectedSize + 1) \
            badExpectedSize(code, expectedSize, size); \
        NEXUS_PACKET_PACKER_INIT_POS(); \
        write<unsigned char>(pos, code); \
        nexus::tuplePack(pos, BOOST_PP_ENUM_PARAMS(n, x)); \
        NEXUS_PACKET_PACKER_RETURN(); \
    }

BOOST_PP_REPEAT_FROM_TO(
    1, BOOST_PP_INC(NEXUS_PACKET_PACKER_MAX_ARITY),
    NEXUS_PACKET_PACKER_PACK_CD_DEF, _ )

inline Buffer packCSD(PacketCode code, size_t len = 0)
{
    size_t size = 3;
    NEXUS_PACKET_PACKER_INIT_POS();
    write<boost::uint8_t>(pos, code);
    write<boost::uint16_t>(pos, 0);
    NEXUS_PACKET_PACKER_RETURN();
}

#define NEXUS_PACKET_PACKER_PACK_CSD_DEF(z, n, data) \
    template <BOOST_PP_ENUM_PARAMS(n, typename T)> \
    Buffer packCSD(PacketCode code, size_t len, BOOST_PP_ENUM_BINARY_PARAMS(n, const T, & x)) \
    { \
        if(len > 0x7fff) \
        { \
            size_t size = len + 5; \
            NEXUS_PACKET_PACKER_INIT_POS(); \
            write<boost::uint8_t>(pos, code); \
            write<boost::uint16_t>(pos, (len & 0x7fff) | 0x8000); \
            write<boost::uint16_t>(pos, len >> 15); \
            nexus::tuplePack(pos, BOOST_PP_ENUM_PARAMS(n, x)); \
            NEXUS_PACKET_PACKER_RETURN(); \
        } else { \
            size_t size = len + 3; \
            NEXUS_PACKET_PACKER_INIT_POS(); \
            write<boost::uint8_t>(pos, code); \
            write<boost::uint16_t>(pos, len); \
            nexus::tuplePack(pos, BOOST_PP_ENUM_PARAMS(n, x)); \
            NEXUS_PACKET_PACKER_RETURN(); \
        } \
    } \
    /**/

BOOST_PP_REPEAT_FROM_TO(
    1, BOOST_PP_INC(NEXUS_PACKET_PACKER_MAX_ARITY),
    NEXUS_PACKET_PACKER_PACK_CSD_DEF, _ )

template<class T>
class Unarray {
public:
    typedef Unarray packer;

    explicit Unarray(const T & t)
        : value_(&t) {}

    size_t packSize() const
    {
        size_t result = 0;
        for(typename T::const_iterator i = value_->begin(), end = value_->end(); i != end; ++i)
            result += nexus::tupleSize(*i);
        return result;
    }

    void pack(char *& pos) const
    {
        for(typename T::const_iterator i = value_->begin(), end = value_->end(); i != end; ++i)
            nexus::tuplePack(pos, *i);
    }
private:
    const T * value_;
};

template<class T>
Unarray<T> unarray(const T & t)
{
    return Unarray<T>(t);
}

}
