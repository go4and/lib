/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#include "pch.h"

#include "Colors.h"

namespace wxutils {

namespace {

typedef uint32_t ColorCode;

struct MapItem {
    std::wstring wname;
    std::string name;
    ColorCode value;
};

class WNameTag;
class NameTag;
class ValueTag;

typedef boost::multi_index_container<
            MapItem,
            boost::multi_index::indexed_by<
                boost::multi_index::hashed_unique<
                    boost::multi_index::tag<WNameTag>,
                    boost::multi_index::member<MapItem, std::wstring, &MapItem::wname>
                >,
                boost::multi_index::hashed_unique<
                    boost::multi_index::tag<NameTag>,
                    boost::multi_index::member<MapItem, std::string, &MapItem::name>
                >,
                boost::multi_index::hashed_unique<
                    boost::multi_index::tag<ValueTag>,
                    boost::multi_index::member<MapItem, ColorCode, &MapItem::value>
                >
            >
        > StdColors;

ColorCode colorCode(wxColor val)
{
    return val.Red() | (static_cast<uint32_t>(val.Green()) << 8) | (static_cast<uint32_t>(val.Blue()) << 16);
}

class WXUTILS_DECL Data : public mstd::singleton<Data> {
public:
    StdColors value;
private:
    Data()
    {
        add(L"black",   wxColor(0x000000UL));
        add(L"maroon",  wxColor(0x000080));
        add(L"green",   wxColor(0x008000));
        add(L"olive",   wxColor(0x008080));
        add(L"navy",    wxColor(0x800000));
        add(L"purple",  wxColor(0x800080));
        add(L"teal",    wxColor(0x808000));
        add(L"gray",    wxColor(0x808080));
        add(L"silver",  wxColor(0xC0C0C0));
        add(L"red",     wxColor(0x0000ff));
        add(L"lime",    wxColor(0x00ff00));
        add(L"yellow",  wxColor(0x00ffff));
        add(L"blue",    wxColor(0xff0000));
        add(L"fuchsia", wxColor(0xff00ff));
        add(L"aqua",    wxColor(0xffff00));
        add(L"white",   wxColor(0xffffff));
    }

    void add(const wchar_t * name, const wxColor & val)
    {
        MapItem item;
        item.wname = name;
        item.name.assign(item.wname.begin(), item.wname.end());
        item.value = colorCode(val);
        value.insert(item);
    }
    
    MSTD_SINGLETON_DECLARATION(Data);
};

}

wxColor string2color(const std::wstring & value)
{
    if(value.empty())
        return 0xffffff;
    if(value[0] == '#') {
        uint32_t result = 0;
        std::wstring::const_iterator i = value.begin();
        ++i;
        for(; i != value.end(); ++i)
        {
            result = (result << 4);
            wchar_t c = *i;
            if(c >= L'0' && c <= L'9')
                result += c - '0';
            else if(c >= L'a' && c <= L'f')
                result += c - L'a' + 10;
            else if(c >= L'A' && c <= L'F')
                result += c - L'A' + 10;
        }
        return wxColor(((result & 0xff0000) >> 16) | ((result & 0xff) << 16) | (result & 0xff00));
    } else {
        const StdColors & colors = Data::instance().value;
        StdColors::const_iterator i = colors.find(value);
        return i != colors.end() ? i->value : 0xffffff;
    }
}

wxColor string2color(const std::string & value)
{
    if(value.empty())
        return 0xffffff;
    if(value[0] == '#') {
        uint32_t result = 0;
        std::string::const_iterator i = value.begin();
        ++i;
        for(; i != value.end(); ++i)
        {
            result = (result << 4);
            wchar_t c = *i;
            if(c >= L'0' && c <= L'9')
                result += c - '0';
            else if(c >= L'a' && c <= L'f')
                result += c - L'a' + 10;
            else if(c >= L'A' && c <= L'F')
                result += c - L'A' + 10;
        }
        return wxColor(((result & 0xff0000) >> 16) | ((result & 0xff) << 16) | (result & 0xff00));
    } else {
        const auto & colors = Data::instance().value.get<NameTag>();
        auto i = colors.find(value);
        if(i != colors.end())
            return i->value;
        else
            return mstd::str2int10<unsigned long>(value);
    }
}

std::wstring color2string(const wxColor & value)
{
    typedef boost::multi_index::index<StdColors, ValueTag>::type Index;
    const Index & index = Data::instance().value.get<ValueTag>();
    Index::const_iterator i = index.find(colorCode(value));
    if(i != index.end())
        return i->wname;
    else {
        const wchar_t * ht = L"0123456789abcdef";
        wchar_t buf[0x10];
        buf[0] = L'#';
        buf[1] = ht[value.Red() >> 4];
        buf[2] = ht[value.Red() & 0xf];
        buf[3] = ht[value.Green() >> 4];
        buf[4] = ht[value.Green() & 0xf];
        buf[5] = ht[value.Blue() >> 4];
        buf[6] = ht[value.Blue() & 0xf];
        buf[7] = 0;
        return buf;
    }
}

}
