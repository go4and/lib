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

const filesize_t sizeUnknown = -1;
const filesize_t sizeNotFound = -2;
const int codeCancelled = 1000;
const int codeOpenFailed = 1001;
const int codeDone = 1002;

struct DownloadProgress {
    filesize_t complete;
    filesize_t total;
    
    DownloadProgress(filesize_t c, filesize_t t)
        : complete(c), total(t)
    {
    }
};

typedef std::function<void(size_t, int)> DownloadListener;

class DownloadManager : boost::noncopyable {
public:
    explicit DownloadManager();
    ~DownloadManager();
    
    void setConcurrentWorkers(size_t value);
    void setMaxChunks(size_t value);
    void setMinChunkSize(filesize_t value);
    
    size_t download(const std::string & url, const boost::filesystem::path & dest, int priority, const DownloadListener & listener, const char * state, size_t stateLen);
    inline size_t download(const std::string & url, const boost::filesystem::path & dest, int priority, const DownloadListener & listener)
    {
        return download(url, dest, priority, listener, 0, 0);
    }
    void cancel(size_t id);
    DownloadProgress getProgress(size_t id);
    bool getState(size_t id, std::vector<char> & out);
private:
    class Impl;
    boost::scoped_ptr<Impl> impl_;
};

}
