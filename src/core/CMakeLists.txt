#
# Core system library.
#
#   All simulators should link to corelib
project(corelib)

# Define sources and include files.
set(CORE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB CORE_SRC CONFIGURE_DEPENDS "*.cpp")
file(GLOB CORE_INCLUDE CONFIGURE_DEPENDS "*.h")

# Include files for library
include_directories(BEFORE ${CORE_PATH})
add_library(corelib STATIC ${CORE_SRC})
target_include_directories(corelib PUBLIC ${CORE_PATH})

if(RUN_TESTS)
# include core_test programs to run.
add_subdirectory(core_test)
endif()
project(cppcheck-analysis)

add_custom_target(cppcheck COMMAND cppcheck
        --enable=warning,performance,portability,information,missingInclude 
        --suppress=missingIncludeSystem
        --std=c++17 
        --verbose 
        -I${CORE_PATH}
        ${CORE_PATH}
        )

