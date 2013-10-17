/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

#include "Config.h"

class wxString;
class wxWindow;
class wxFilePickerCtrl;

namespace wxutils {

WXUTILS_DECL 
wxFilePickerCtrl * createFilePicker(wxWindow * parent,
                                    const wxString & path,
                                    const wxString & button,
                                    const wxString & hint,
                                    const wxString & wildcard);

}
