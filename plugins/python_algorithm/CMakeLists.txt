find_package(Python COMPONENTS Interpreter Development)
add_library(python_algorithm MODULE python_algorithm.cpp)
target_link_libraries(python_algorithm smm-2ng-plugin-header jsonlib)
target_include_directories(python_algorithm  PUBLIC ${Python_INCLUDE_DIRS})
target_link_libraries(python_algorithm ${Python_LIBRARIES})