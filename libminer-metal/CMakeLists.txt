file(GLOB sources "*.cpp")
file(GLOB headers "*.h")

execute_process(COMMAND xcrun -sdk macosx --show-sdk-path OUTPUT_VARIABLE SDK_PATH OUTPUT_STRIP_TRAILING_WHITESPACE)

message(STATUS "sdk path: ${SDK_PATH}")
set(CMAKE_OSX_SYSROOT ${SDK_PATH})

add_library(ethash-metal ${sources} ${headers})
target_link_libraries(ethash-metal ethcore ethash Boost::thread mtlpp_lib
        "-framework CoreFoundation"
        "-framework IOKit")
target_include_directories(ethash-metal PRIVATE .. ../libmtlpp ${CMAKE_CURRENT_BINARY_DIR})
