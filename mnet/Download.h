/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

#include "DownloadDefines.h"

namespace mnet {

struct DownloadStats;

class Download {
public:
    typedef std::function<void(Download*, DownloadAction, int)> Listener; 

    explicit Download(const std::string & url, const boost::filesystem::wpath & localfile, filesize_t size, const Listener & listener);
    ~Download();

    void cleanup();

    void start();
    void stop();

    DownloadStats stats() const;    
private:
    class Impl;
    boost::scoped_ptr<Impl> impl_;
};

}
