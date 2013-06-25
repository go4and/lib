/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#include "pch.h"

#include "Widget.h"

namespace wxutils {

class Widget::Visitor {
public:
    template<class V>
    static typename V::result_type apply(const V & visitor, Widget & widget)
    {
        switch(widget.which_) {
        case 1:
            return visitor(static_cast<wxWindow*>(widget.pointer_));
        case 2:
            return visitor(static_cast<wxSizer*>(widget.pointer_));
        }
        return typename V::result_type();
    }
};

Widget::Widget()
    : which_(0), pointer_(0) {}

Widget::Widget(wxWindow * window)
    : which_(1), pointer_(window) {}

Widget::Widget(wxSizer *sizer)
    : which_(2), pointer_(sizer) {}

template<class Sizer>
class AddWidget {
public:
    typedef wxSizerItem * result_type;

    explicit AddWidget(Sizer * sizer)
        : sizer_(sizer) {}

    template<class T>
    wxSizerItem * operator()(T * widget) const
    {
        return sizer_->Add(widget);
    }
private:
    Sizer * sizer_;
};

wxSizerItem * Widget::addTo(wxSizer *sizer)
{
    return Visitor::apply(AddWidget<wxSizer>(sizer), *this);
}

wxGBSizerItem * Widget::addTo(wxGridBagSizer * sizer)
{
    return boost::polymorphic_downcast<wxGBSizerItem*>(Visitor::apply(AddWidget<wxGridBagSizer>(sizer), *this));
}

class DestroyWidget {
public:
    typedef void result_type;

    explicit DestroyWidget(wxSizer * sizer)
        : sizer_(sizer) {}

    void operator()(wxWindow * window) const
    {
        window->Destroy();
    }
    
    void operator()(wxSizer * sizer) const
    {
        sizer->Clear(true);
        sizer_->Remove(sizer);
    }
private:
    wxSizer * sizer_;
};

void Widget::destroy(wxSizer * sizer)
{
    Visitor::apply(DestroyWidget(sizer), *this);
}

Widget::operator safe_bool() const
{
    return which_ ? &Dummy::f : 0;
}

}
