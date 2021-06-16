set(TEST_APP_NAME core_test)
set(TEST_SOURCES
     OptionsTest.cpp
     ConfigTest.cpp
     ConfigLexerTest.cpp
     MemoryTest.cpp
     EventTest.cpp
     main.cpp 
     )

include_directories(${CPPUTEST_INCLUDE_DIRS} ../src/core)
add_executable(${TEST_APP_NAME} ${TEST_SOURCES})
add_test(NAME ${TEST_APP_NAME} COMMAND ${TEST_APP_NAME})
target_link_directories(${TEST_APP_NAME} PRIVATE ${CPPUTEST_LIBRARIES})
if (MSVC)
     target_link_libraries(${TEST_APP_NAME} winmm)
endif()
target_link_libraries(${TEST_APP_NAME} corelib ${CPPUTEST_LDFLAGS})
add_custom_command(TARGET ${TEST_APP_NAME} COMMAND ${TEST_APP_NAME} -v POST_BUILD)
