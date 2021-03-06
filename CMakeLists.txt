cmake_minimum_required(VERSION 3.15)
set( PROJECT_NAME "ts-sim" )
## Set our project name
project(${PROJECT_NAME} VERSION 0.0.0 
	DESCRIPTION "Time Sharing system simulator")

set (${PROJECT_NAME}_VERSION_MAJOR 0)
set (${PROJECT_NAME}_VERSION_MINOR 0)

# Add a custom CMake Modules directory
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules
                      ${CMAKE_MODULE_PATH})

# Generate Doxygen documentation files
option(BUILD_DOC "Build documenation" ON)
option(RUN_TESTS "Run tests" ON)

if (BUILD_DOC)
    find_package(Doxygen REQUIRED)
    if (NOT DOXYGEN_FOUND)
	message(WARNING "Doxygen not found, unable to generate documatation")
    else()
        set(DOXY_IN ${CMAKE_HOME_DIRECTORY}/doc/doxyfile.in)                      ## used in the doxyfile.in
        set(DOXY_OUT ${CMAKE_HOME_DIRECTORY}/doc)    ## used in the doxyfile.in
	configure_file(${DOXY_IN} ${DOXY_OUT} @ONLY)
	add_custom_target(doxyDoc 
		COMMAND ${DOXYGEN_EXECUTABLE} ${DOXY_IN}
		WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY}/doc
                COMMENT "Building user's documentation into doxyDoc build dir..."
        )
    endif()
endif()

if (RUN_TESTS) 
# Find CppUTest.
if(CPPUTEST_HOME)
    set(CPPUTEST_INCLUDE_DIRS ${CPPUTEST_HOME}/include)
    set(CPPUTEST_LIBRARIES ${CPPUTEST_HOME}/lib)
    set(CPPUTEST_LDFLAGS CppUTest CppUTestExt)
elseif(DEFINED ENV{CPPUTEST_HOME})
    message(STATUS "Using CppUTest home: $ENV{CPPUTEST_HOME}")
    set(CPPUTEST_INCLUDE_DIRS $ENV{CPPUTEST_HOME}/include)
    set(CPPUTEST_LIBRARIES $ENV{CPPUTEST_HOME}/lib)
    set(CPPUTEST_LDFLAGS CppUTest CppUTestExt)
else()
    find_package(PkgConfig REQUIRED)
    pkg_search_module(CPPUTEST REQUIRED cpputest>=3.8)
    message(STATUS "Found CppUTest version ${CPPUTEST_VERSION}")
endif()
ENABLE_TESTING()
endif()


set(CMAKE_CXX_STANDARD 17)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /std:c++17")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2")
else()
    set(CMAKE_CXX_FLAGS "-Wall -Wextra")
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()

# Get required files and packages
find_package (Threads)
include(CheckIncludeFiles)
check_include_files(unistd.h HAVE_UNISTD_H)
check_include_files(termio.h HAVE_TERMIO_H)
if (NOT HAVE_TERMIO_H)
   check_include_files(termios.h HAVE_TERMIOS_H)
endif()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

include_directories(BEFORE ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(src/core)

## Use all the *.cpp files we found under this folder for the project
FILE(GLOB I8080_SRCS "src/i8080/i8080_system.cpp" "src/i8080/i8080_cpu.cpp")

## Define the executable
#add_dependencies(i8080 corelib)
add_executable(i8080 ${I8080_SRCS} src/i8080/main.cpp)
target_link_libraries(i8080 corelib)
target_link_libraries(i8080 ${CMAKE_THREAD_LIBS_INIT})

if (RUN_TESTS) 
message(${CMAKE_CURRENT_SOURCE_DIR})
add_executable(i8080_test ${I8080_SRCS} src/i8080/test/main.cpp)
target_include_directories(i8080_test PRIVATE "src/i8080")
target_include_directories(i8080_test PRIVATE ${CPPUTEST_INCLUDE_DIRS})
add_dependencies(i8080_test core_test)
add_test(NAME i8080_test COMMAND i8080_test 
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/i8080/test)
target_link_directories(i8080_test PRIVATE ${CPPUTEST_LIBRARIES})
target_link_libraries(i8080_test corelib)
if(MSVC)
    target_link_libraries(i8080_test winmm)
endif()
target_link_libraries(i8080_test ${CPPUTEST_LDFLAGS})
add_custom_command(TARGET i8080_test POST_BUILD 
	COMMAND i8080_test
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/i8080/test
	COMMENT "Running i8080 test" VERBATIM)
endif()


