#include "pch.h"

#if !defined(BOOST_WINDOWS)
#include <signal.h>
#endif

#include "Signals.h"

namespace nexus {

#if !BOOST_WINDOWS
const int signalUser1 = SIGUSR1;
const int signalUser2 = SIGUSR2;
const int signalTerm = SIGTERM;
#else
const int signalUser1 = 10;
const int signalUser2 = 12;
const int signalTerm = 15;
#endif

class SignalsManager : public mstd::singleton<SignalsManager> {
public:
    void install(size_t signal, bool restart, const SignalHandler & handler)
    {
        handlers_.resize(std::max(signal + 1, handlers_.size()));
        handlers_[signal] = handler;

#if !BOOST_WINDOWS
        struct sigaction nsa;
        memset(&nsa, 0, sizeof(nsa));

        nsa.sa_handler = &signalHandler;
        sigemptyset(&nsa.sa_mask);
        nsa.sa_flags = restart ? SA_RESTART : 0;

        int result = sigaction(signal, &nsa, 0);
        if(result == -1)
            std::cerr << "Failed to setup signal action for " << signal << ": " << errno << std::endl;
        else
            std::cout << "Setup signal action: " << signal << std::endl;
#else
        if(static_cast<int>(signal) == signalTerm)
        {
            SetConsoleCtrlHandler(&SignalsManager::consoleCtrlHandler, TRUE);
        }
#endif
    }
private:
    bool process(size_t signal)
    {
        if(signal < handlers_.size() && !handlers_[signal].empty())
        {
            handlers_[signal](signal);
            return true;
        } else
            return false;
    }

#if !BOOST_WINDOWS
    static void signalHandler(int signal)
    {
        instance().process(signal);
    }
#else
    static BOOL WINAPI consoleCtrlHandler(DWORD type)
    {
        switch(type) {
        case CTRL_C_EVENT:
            if(instance().process(signalTerm))
                return true;
            break;
        }
        return false;
    }
#endif

    std::vector<SignalHandler> handlers_;
    
    friend class mstd::singleton<SignalsManager>;
};

void signalAction(int signal, bool restart, const SignalHandler & handler)
{
    SignalsManager::instance().install(signal, restart, handler);
}

}
