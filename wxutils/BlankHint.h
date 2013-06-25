<<<<<<< HEAD
#pragma once
=======
/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
pragma once
>>>>>>> 751fc585e47bb4774a5a26110f78678513d85781

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
