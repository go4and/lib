#include "pch.h"

#include "Defines.h"

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

void nslogWrite(LogLevel level, const char * out, size_t len)
{
    @autoreleasepool {
        NSString * string = [[NSString alloc] initWithBytes:out length:len encoding:NSUTF8StringEncoding];
        NSLog(@"%@", string);
        [string release];
    }
}

}
