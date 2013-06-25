#include "pch.h"

#include "BlankHint.h"

namespace wxutils {

void BlankHint::operator()(wxPaintEvent & evt) const
{
    wxTextCtrl * ctrl = static_cast<wxTextCtrl*>(evt.GetEventObject());
    if(ctrl->GetValue().empty() && !ctrl->HasFocus())
    {
        wxPaintDC dc(ctrl);
        dc.SetPen(*wxWHITE_PEN);
        dc.SetBrush(*wxWHITE_BRUSH);
        dc.DrawRectangle(wxPoint(0, 0), ctrl->GetSize());
        dc.SetTextForeground(wxColour(0xc0, 0xc0, 0xc0));
        wxRect rect = ctrl->GetClientRect();
        rect.SetLeft(rect.GetLeft() + 1);
        dc.DrawLabel(blankText_, rect, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);
    } else
        evt.Skip();
}

}
