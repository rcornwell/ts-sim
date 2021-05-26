cmake_minimum_required(VERSION 3.15)
set( PROJECT_NAME "ts-sim" )
## Set our project name
project(${PROJECT_NAME} VERSION 0.0.0 
	DESCRIPTION "Time Sharing system simulator")

# Generate Doxygen documentation files
option(BUILD_DOC "Build documenation" ON)

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

# Add a custom CMake Modules directory
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules
                      ${CMAKE_MODULE_PATH})

ENABLE_TESTING()

set(CMAKE_CXX_STANDARD 17)
set(UnitTest++_DIR "/usr/include/UnitTest++")
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_subdirectory(src/core)

## Use all the *.cpp files we found under this folder for the project
FILE(GLOB I8080_SRCS "src/i8080/i8080_system.cpp" "src/i8080/i8080_cpu.cpp")

## Define the executable
add_executable(i8080 ${I8080_SRCS} src/i8080/main.cpp)
add_executable(i8080_test ${I8080_SRCS} src/i8080/test/main.cpp)
add_test(NAME i8080_test COMMAND i8080_test ../src/i8080/test)
add_custom_command(TARGET i8080_test POST_BUILD 
	COMMAND i8080_test ../src/i8080/test
	COMMENT "Running i8080 test" VERBATIM)

add_dependencies(i8080_test core_test)
#add_dependencies(i8080 corelib)
target_link_libraries(i8080_test UnitTest++)
target_link_libraries(i8080_test corelib)
target_link_libraries(i8080 corelib)
target_include_directories(i8080_test PRIVATE "src/i8080")
target_include_directories(i8080_test PRIVATE ${UnitTest++_DIR})

