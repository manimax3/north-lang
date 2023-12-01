add_executable(north main.cpp)

find_package(fmt REQUIRED)

target_link_libraries(north PRIVATE 
	project_options
	project_warnings
	fmt::fmt)
