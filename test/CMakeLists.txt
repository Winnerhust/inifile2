enable_testing()

add_subdirectory(googletest)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/googletest/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)

add_executable(inifile_test inifile_test.cpp)
add_compile_options("-g -Wall")
target_link_libraries(inifile_test inifile gtest gtest_main)
add_dependencies(inifile_test inifile gtest gtest_main)


#规避make test不生效的问题
add_test(NAME testcase COMMAND inifile_test)
#设置-V，可以在执行测试用例的同时将用例信息打印出来
add_custom_target(test COMMAND ${CMAKE_CTEST_COMMAND} -V DEPENDS inifile_test)

