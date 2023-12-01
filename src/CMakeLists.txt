add_executable(north main.cpp)

target_link_libraries(north PRIVATE 
	project_options
	project_warnings)
