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

class ChunkedBuffer {
public:
    explicit ChunkedBuffer(size_t chunkSize = 0x4000)
        : chunkSize_(chunkSize), rchunk_(allocChunk()), rpos_(chunkBegin(rchunk_)), wchunk_(rchunk_), wpos_(chunkBegin(wchunk_)), tail_(rchunk_)
    {
        chunkSetNext(rchunk_, 0);
    }

    ~ChunkedBuffer()
    {
        char * current = rchunk_;
        while(current)
        {
            char * next = chunkNext(current);
            free(current);
            current = next;
        }
    }

    bool empty() const
    {
        return wpos_ == rpos_;
    }

    void consume(size_t len)
    {
        char * chunkEnd = this->chunkEnd(rchunk_);
        rpos_ += len;
        BOOST_ASSERT(rpos_ <= chunkEnd);
        if(rpos_ == wpos_)
            rpos_ = wpos_ = chunkBegin(rchunk_);
        else if(rpos_ == chunkEnd)
        {
            char * next = chunkNext(rchunk_);
            BOOST_ASSERT(next);
            chunkSetNext(rchunk_, 0);
            chunkSetNext(tail_, rchunk_);
            rchunk_ = next;
            rpos_ = chunkBegin(rchunk_);
        }
    }

    std::pair<char*, size_t> readyChunk()
    {
        if(rchunk_ == wchunk_)
            return std::make_pair(rpos_, wpos_ - rpos_);
        else
            return std::make_pair(rpos_, chunkEnd(rchunk_) - rpos_);
    }

    void append(const char * data, size_t len)
    {
        const char * end = data + len;
        while(data != end)
        {
            char * chunkEnd = this->chunkEnd(wchunk_);
            if(wpos_ == chunkEnd)
            {
                char * next = chunkNext(wchunk_);
                if(!next)
                    next = chunkSetNext(wchunk_, allocChunk());
                wchunk_ = next;
                chunkSetNext(next, 0);
                wpos_ = chunkBegin(next);
            }
            size_t size = std::min(end - data, chunkEnd - wpos_);
            memcpy(wpos_, data, size);
            data += size;
            wpos_ += size;
        }
    }
private:
    static const size_t chunkHeaderSize = sizeof(char*);
    
    inline char * chunkNext(char * chunk)
    {
        char * result;
        memcpy(&result, chunk, sizeof(char*));
        return result;
    }

    inline char * chunkSetNext(char * chunk, char * next)
    {
        memcpy(chunk, &next, sizeof(next));
        return next;
    }

    inline char * chunkBegin(char * chunk)
    {
        return chunk + chunkHeaderSize;
    }

    inline char * chunkEnd(char * chunk)
    {
        return chunk + chunkSize_;
    }

    inline char * allocChunk()
    {
        return static_cast<char*>(malloc(chunkSize_));
    }

    size_t chunkSize_;
    char * rchunk_;
    char * rpos_;
    char * wchunk_;
    char * wpos_;
    char * tail_;
};

}
