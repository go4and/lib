file(GLOB cppfiles "src/*.cpp")

add_library(mptree${BINARY_SUFFIX} ${cppfiles})

