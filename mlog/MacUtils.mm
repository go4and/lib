#include "pch.h"

#include "Defines.h"
#include "Utils.h"

#include <Foundation/Foundation.h>

#if !defined(MLOG_NO_LOGGING)

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

void nslogWrite(LogLevel level, const char * out, size_t len)
{
    @autoreleasepool {
        NSLog(@"%.*s", static_cast<int>(len), out);
    }
}

std::ostream & operator<<(std::ostream & out, const OutObjC & objc)
{
    @autoreleasepool {
        NSString * temp = [NSString stringWithFormat:@"%@", objc.value()];
        out << [temp UTF8String];
    }
    return out;
}

}

#endif
