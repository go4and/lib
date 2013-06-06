#include "pch.h"

#include "JSON.h"

MLOG_DECLARE_LOGGER(json);

namespace mnet {

namespace {

class Builder {
    typedef boost::property_tree::ptree Tree;

public:
    explicit Builder(Tree & t) : 
        tree_(t)
    {}

    void start()
    {
        push();
    }

    void end()
    {
        pop();
    }

    void name(const char * s, size_t l)
    {
        name_.assign(s, l);
    }

    void value(const char* s, size_t l)
    {
        add()->data().assign(s, l);
    }
private:
    void push()
    {
        nodes_.push_back(nodes_.empty() ? &tree_ : add());
    }

    Tree * add()
    {
        Tree* tr = &nodes_.back()->push_back(Tree::value_type(name_, Tree()))->second;
        name_.clear();
        return tr;
    }

    void pop()
    {
        nodes_.pop_back();
        name_.clear();
    }

    Tree & tree_;
    std::vector<Tree *> nodes_;
    std::string name_;
};

int valString(void * ctx, const unsigned char * s, size_t l)
{
    reinterpret_cast<Builder*>(ctx)->value(reinterpret_cast<const char*>(s), l);
    return true;
}

int valNumber(void * ctx, const char * s, size_t l)
{
    return valString(ctx, reinterpret_cast<const unsigned char *>(s), l);
}

int childName(void * ctx, const unsigned char * s, size_t l)
{
    reinterpret_cast<Builder*>(ctx)->name(reinterpret_cast<const char *>(s), l);
    return true;
}

int childStart(void * ctx)
{
    reinterpret_cast<Builder*>(ctx)->start();
    return true;
}

int childEnd(void * ctx)
{
    reinterpret_cast<Builder*>(ctx)->end();
    return true;
}

yajl_callbacks callbacks = {
    NULL,
    NULL,
    NULL,
    NULL,
    &valNumber,
    &valString,
    &childStart,
    &childName,
    &childEnd,
    &childStart,
    &childEnd
};

}

void parseJSON(const char * input, size_t len, boost::property_tree::ptree & tree, ParseError & error)
{
    Builder builder(tree);
    mstd::handle_base<yajl_handle, mstd::global_fun_traits<yajl_handle, void, &yajl_free> > hand(yajl_alloc(&callbacks, NULL, &builder));
    const unsigned char * buf = mstd::pointer_cast<const unsigned char*>(input);
    yajl_status res = yajl_parse(*hand, buf, len);

    if(res == yajl_status_error)
    {
        unsigned char * err = yajl_get_error(*hand, 1, buf, len);
        error.init(mstd::pointer_cast<char*>(err));
        yajl_free_error(*hand, err);
    }
}

void parseJSON(const boost::filesystem::path & path, boost::property_tree::ptree & tree, ParseError & error)
{
    FILE * file = mstd::wfopen(path, "rb");
    if(file)
    {
        
#if BOOST_WINDOWS
        struct _stat64 stat;
        if(!_fstat64(_fileno(file), &stat))
#else
        struct stat stat;
        if(!fstat(fileno(file), &stat))
#endif
        {
            size_t size = static_cast<size_t>(stat.st_size);
            if(size)
            {
                std::vector<char> buffer(size);
                fread(&buffer[0], 1, size, file);
                parseJSON(&buffer[0], size, tree, error);
            } else
                parseJSON(0, 0, tree, error);
        } else
            error.init("Unable to get file size");
        fclose(file);
    } else
        error.init("Unable to open file");
}

}
