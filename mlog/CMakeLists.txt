file(GLOB cppfiles "*.cpp")

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  list(APPEND cppfiles MacUtils.mm)

  macro(FIND_AND_ADD_FRAMEWORK fwname appname)
    find_library(FRAMEWORK_${fwname}
        NAMES ${fwname}
        PATHS ${CMAKE_OSX_SYSROOT}/System/Library
        PATH_SUFFIXES Frameworks
        NO_DEFAULT_PATH)
    if( ${FRAMEWORK_${fwname}} STREQUAL FRAMEWORK_${fwname}-NOTFOUND)
        MESSAGE(ERROR ": Framework ${fwname} not found")
    else()
        TARGET_LINK_LIBRARIES(${appname}${BINARY_SUFFIX} ${FRAMEWORK_${fwname}})
        # MESSAGE(STATUS "Framework ${fwname} found at ${FRAMEWORK_${fwname}}")
    endif()
  endmacro(FIND_AND_ADD_FRAMEWORK)

endif()

set(CMAKE_CXX_FLAGS "-DBOOST_SYSTEM_NO_DEPRECATED")

add_library(mlog${BINARY_SUFFIX} ${cppfiles})
target_link_libraries(mlog${BINARY_SUFFIX} mstd${BINARY_SUFFIX})

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
   find_and_add_framework(Foundation mlog)
endif()
