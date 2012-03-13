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
