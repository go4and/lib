#pragma once

#include "DownloadDefines.h"

namespace mnet {

struct DownloadStats;

class Download {
public:
    typedef boost::function<void(Download*, DownloadAction, int)> Listener; 

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
