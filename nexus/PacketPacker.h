/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
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
