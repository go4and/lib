#pragma once

namespace nexus {

extern const int signalUser1;
extern const int signalUser2;
extern const int signalTerm;

typedef boost::function<void(int)> SignalHandler;

void signalAction(int signal, bool restart, const SignalHandler & handler);

}
