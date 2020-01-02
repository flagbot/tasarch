cmake_minimum_required(VERSION 3.13)
project(noarch
	VERSION 1.0.0)

######################
# helper functions	 #
######################
include(CMakeParseArguments)

function(GLOB_CPP folder)

	set(GC_ONEVALUEARGS VAR_PREFIX)
	set(GC_MULTIVALUEARGS GLOBS)

	cmake_parse_arguments(GC_ARG
		"${GC_ONEVALUEARGS}"
		"${GC_MULTIVALUEARGS}"
		"${ARGN}")

	if (NOT GC_ARG_VAR_PREFIX)
		set(GC_ARG_VAR_PREFIX ${folder})
    endif()

	file(GLOB ${GC_ARG_VAR_PREFIX}_CPP
		"${PROJECT_SOURCE_DIR}/${folder}/*.cc"
		"${PROJECT_SOURCE_DIR}/${folder}/*.cpp"
	)

	file(GLOB ${GC_ARG_VAR_PREFIX}_HDR
		"${PROJECT_SOURCE_DIR}/${folder}/*.h"
		"${PROJECT_SOURCE_DIR}/${folder}/*.hpp"
	)

	set(${GC_ARG_VAR_PREFIX}_SRC "${${GC_ARG_VAR_PREFIX}_HDR}" "${${GC_ARG_VAR_PREFIX}_CPP}" PARENT_SCOPE)

endfunction()

add_subdirectory(gambatte-libretro)

######################
# tasarch target	 #
######################

find_package(PkgConfig)
pkg_check_modules(GTKMM REQUIRED gtkmm-3.0 libarchive NO_CMAKE_PATH)

glob_cpp(gui)

glob_cpp(emulator)

glob_cpp(movie)

list(APPEND CMAKE_MODULE_PATH
     ${PROJECT_SOURCE_DIR}/GCR_CMake/macros)

include(GlibCompileResourcesSupport)

file(GLOB RESOURCES
	"${PROJECT_SOURCE_DIR}/resources/*.*"
)

SOURCE_GROUP(resources FILES ${RESOURCES})

set(RESOURCES_GEN_DIR "${PROJECT_SOURCE_DIR}/resources/generated")

set(APPLICATION_NAME tasarch)

list(TRANSFORM RESOURCES REPLACE "${PROJECT_SOURCE_DIR}/" "")

compile_gresources(RESOURCE_FILE
                   XML_OUT
				   TYPE EMBED_C
				   RESOURCES ${RESOURCES})
				   
file(GLOB RESOURCES_GEN
	"${RESOURCES_GEN_DIR}/*.c"
)

add_custom_target(resources ALL DEPENDS ${RESOURCE_FILE} ${RESOURCES})

message(STATUS "Resource File: ${RESOURCE_FILE} ${RESOURCES} ${GTKMM_INCLUDE_DIRS}")

# SOURCE_GROUP(${folder} FILES ${${GC_ARG_VAR_PREFIX}_SRC})

SOURCE_GROUP(TREE ${PROJECT_SOURCE_DIR} PREFIX "" FILES ${emulator_SRC} ${gui_SRC} ${movie_SRC} ${RESOURCES})

add_executable(${APPLICATION_NAME} main.cpp ${emulator_SRC} ${gui_SRC} ${movie_SRC} ${RESOURCES} ${RESOURCE_FILE})
target_include_directories(${APPLICATION_NAME} PRIVATE ${GTKMM_INCLUDE_DIRS} ${libarchive_INCLUDE_DIRS} /usr/local/opt/freetype/include/freetype2)
target_link_libraries(${APPLICATION_NAME} dl ${GTKMM_LIBRARIES} ${libarchive_LIBRARIES})
target_compile_options(${APPLICATION_NAME} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-std=c++14 -DSOURCE_CODE_ROOT="${PROJECT_SOURCE_DIR}/">)
target_link_directories(${APPLICATION_NAME} PRIVATE ${GTKMM_LIBRARY_DIRS})
add_dependencies(${APPLICATION_NAME} gambatte_libretro resources)

######################
# noarch target		 #
######################

# find_package(glfw3 3.3 REQUIRED)
# find_package(OpenGL REQUIRED)
# find_package(GLEW REQUIRED)

# add_executable(noarch noarch.c)
# target_include_directories(noarch PRIVATE ${GLFW_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR} ${GLEW_INCLUDE_DIRS})
# target_link_libraries(noarch dl glfw ${OPENGL_gl_LIBRARY} GLEW::GLEW)
# add_dependencies(noarch gambatte_libretro)
