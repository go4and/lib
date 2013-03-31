#if defined(_MSC_VER)
#pragma once
#endif

#include <wchar.h>

#include <boost/config.hpp>

#ifdef __OBJC__
#include <Cocoa/Cocoa.h>
#endif

#if BOOST_WINDOWS
#include <Windows.h>
#endif

#include <map>
#include <vector>

#include <boost/bind.hpp>
#include <boost/cast.hpp>
#include <boost/format.hpp>
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/unordered_map.hpp>

#include <boost/move/move.hpp>

#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/shared_memory_object.hpp>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/hashed_index.hpp>

#if __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <wx/dc.h>
#include <wx/filepicker.h>
#include <wx/fontutil.h>
#include <wx/gbsizer.h>
#include <wx/icon.h>
#include <wx/image.h>
#include <wx/msgdlg.h>
#include <wx/settings.h>
#include <wx/stattext.h>
#include <wx/timer.h>

#include <mstd/cstdint.hpp>
#include <mstd/enum_set.hpp>
#include <mstd/filesystem.hpp>
#include <mstd/itoa.hpp>
#include <mstd/pointer_cast.hpp>
#include <mstd/process.hpp>
#include <mstd/singleton.hpp>

#include <mlog/Dumper.h>
#include <mlog/Logging.h>

#define BUILDING_WXUTILS
