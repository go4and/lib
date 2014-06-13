/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

namespace nexus {

extern const int signalUser1;
extern const int signalUser2;
extern const int signalTerm;

typedef std::function<void(int)> SignalHandler;

void signalAction(int signal, bool restart, const SignalHandler & handler);

}
