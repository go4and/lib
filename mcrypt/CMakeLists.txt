file(GLOB cppfiles "*.cpp")
add_library(mcrypt${BINARY_SUFFIX} ${cppfiles})
