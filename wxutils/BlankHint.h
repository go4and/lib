#pragma once

namespace wxutils {

class BlankHint {
public:
    explicit BlankHint(const wxString & blankText)
        : blankText_(blankText)
    {
    }

    void operator()(wxPaintEvent & evt) const;
private:
    wxString blankText_;
};

}
