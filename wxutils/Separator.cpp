#include "pch.h"

#include "Separator.h"

namespace wxutils {

void addSeparator(wxGridBagSizer * psizer, int y, wxWindow * parent, const std::wstring & title)
{
    wxPanel * result = new wxPanel(parent);
    wxBoxSizer * sizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(new wxPanel(result, wxID_ANY, wxDefaultPosition, wxSize(32, 4), wxTAB_TRAVERSAL | wxBORDER_SUNKEN, title.c_str()), wxSizerFlags().Center());
    sizer->Add(new wxStaticText(result, wxID_ANY, title.c_str()), wxSizerFlags().Center().Border(wxALL & ~wxTOP, 4));
    sizer->Add(new wxPanel(result, wxID_ANY, wxDefaultPosition, wxSize(32, 4), wxTAB_TRAVERSAL | wxBORDER_SUNKEN, title.c_str()), wxSizerFlags().Center().Proportion(1));
    result->SetSizerAndFit(sizer);
    psizer->Add(result, wxGBPosition(y, 0), wxGBSpan(1, 5), wxTOP | wxEXPAND, 4);
}

}
