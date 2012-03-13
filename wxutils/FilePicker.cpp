#include "pch.h"

#include "FilePicker.h"

namespace wxutils {

namespace {

class FilePickerCtrl : public wxFilePickerCtrl {
public:
    explicit FilePickerCtrl(const wxString & label)
        : label_(label) {}
private:
    wxFileDirPickerWidgetBase *CreatePicker(wxWindow *parent,
                                            const wxString& path,
                                            const wxString& message,
                                            const wxString& wildcard)
    {
        return new wxFilePickerWidget(parent, wxID_ANY,
                                      label_,
                                      path, message, wildcard,
                                      wxDefaultPosition, wxDefaultSize,
                                      GetPickerStyle(GetWindowStyle()));
    }

    wxString label_;
};

}

wxFilePickerCtrl * createFilePicker(wxWindow * parent, const wxString & path,
                                    const wxString & button, const wxString & hint,
                                    const wxString & wildcard)
{
    wxFilePickerCtrl * result = new FilePickerCtrl(button);
    result->Create(parent, wxID_ANY, path, hint, wildcard);
    return result;
}
                                           
}
