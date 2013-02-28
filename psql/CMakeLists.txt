file(GLOB cppfiles "*.cpp")
add_library(psql${BINARY_SUFFIX} ${cppfiles})
