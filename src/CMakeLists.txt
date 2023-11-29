add_executable(testproject main.cpp)

target_link_libraries(testproject PRIVATE 
	project_options
	project_warnings)
