#include "pch.h"

#include "UIEnqueue.h"

namespace wxutils {

namespace detail {

const wxEventType functionEventType = wxNewEventType();

FunctionEventBase::FunctionEventBase()
    : wxEvent(0, functionEventType)
{}

class FunctionEventHandler : public wxEvtHandler {
    MSTD_SINGLETON_INLINE_DEFINITION(FunctionEventHandler);
public:
    bool ProcessEvent(wxEvent & evt)
    {
        static_cast<FunctionEventBase&>(evt).execute();
        return true;
    }
private:
    FunctionEventHandler()
    {
    }
};

void enqueueFunctionEvent(wxEvent & evt)
{
    FunctionEventHandler::instance().AddPendingEvent(evt);
}

}

}
