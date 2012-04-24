#include "pch.h"

#include <Foundation/Foundation.h>

namespace mlog {

std::string doDocumentsFolder()
{
    NSArray *paths = NSSearchPathForDirectoriesInDomains
                        (NSDocumentDirectory, NSUserDomainMask, YES);
    if ([paths count] > 0)
        return [[paths objectAtIndex: 0] UTF8String];
    const char * home = getenv("HOME");
    if(home)
        return mstd::utf8fname(boost::filesystem::wpath(mstd::deutf8(home)) / L"Documents");
    return "Documents";
}

std::string documentsFolder()
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    std::string result = doDocumentsFolder();
    [pool release];
    return result;
}

}
