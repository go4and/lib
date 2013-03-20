file(GLOB cppfiles "src/*.cpp")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(mptree${BINARY_SUFFIX} ${cppfiles})

