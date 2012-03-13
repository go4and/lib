#include "pch.h"

#include "Separator.h"

namespace wxutils {

#if BOOST_WINDOWS
void addSeparator(wxGridBagSizer * psizer, int y, wxWindow * parent, const std::wstring & title)
{
    wxPanel * result = new wxPanel(parent);
    wxBoxSizer * sizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(new wxPanel(result, 0, 0, 32, 4, wxTAB_TRAVERSAL | wxBORDER_SUNKEN, title), wxSizerFlags().Center());
    sizer->Add(new wxStaticText(result, wxID_ANY, title), wxSizerFlags().Center().Border(wxALL & ~wxTOP, 4));
    sizer->Add(new wxPanel(result, 0, 0, 32, 4, wxTAB_TRAVERSAL | wxBORDER_SUNKEN, title), wxSizerFlags().Center().Proportion(1));
    result->SetSizerAndFit(sizer);
    psizer->Add(result, wxGBPosition(y, 0), wxGBSpan(1, 5), wxTOP | wxEXPAND, 4);
}
#endif

}