#if defined(_MSC_VER)
#pragma once
#endif

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
#include <wx/gbsizer.h>
#include <wx/icon.h>
#include <wx/image.h>
#include <wx/msgdlg.h>
#include <wx/stattext.h>
#include <wx/timer.h>

#include <mstd/cstdint.hpp>
#include <mstd/enum_set.hpp>
#include <mstd/pointer_cast.hpp>
#include <mstd/singleton.hpp>

#define BUILDING_WXUTILS
