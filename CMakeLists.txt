cmake_minimum_required(VERSION 3.0)
project(noarch
	VERSION 1.0.0
	LANGUAGES C )

add_executable(noarch noarch.c)
target_link_libraries(noarch dl)