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

# include core_test programs to run.
add_subdirectory(core_test)
#add_executable(core_test core_test/main.cpp)
#target_link_libraries(core_test corelib)
#target_link_libraries(core_test UnitTest++)
#target_include_directories(core_test PRIVATE ${UnitTest++_DIR})
#add_test(NAME core_test COMMAND core_test)
#add_custom_command(TARGET core_test POST_BUILD
#       COMMAND core_test 
#       COMMENT "Running core test" VERBATIM)

# include core_test programs to run.
#add_executable(opt_test core_test/opt-test.cpp)
#target_link_libraries(opt_test corelib)
#target_link_libraries(opt_test UnitTest++)
#target_include_directories(opt_test PRIVATE ${UnitTest++_DIR})
#add_test(NAME opt_test COMMAND opt_test)
#add_custom_command(TARGET opt_test POST_BUILD
#       COMMAND opt_test 
#       COMMENT "Running option test" VERBATIM)
project(cppcheck-analysis)

#set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "")
#set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "")
#find(CppCheck)
add_custom_target(cppcheck COMMAND cppcheck
        --enable=warning,performance,portability,information,missingInclude 
        --suppress=missingIncludeSystem
        --std=c++17 
        --verbose 
        -I${CORE_PATH}
        ${CORE_PATH}
        )
# Build documentation for core library

