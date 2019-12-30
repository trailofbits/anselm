cmake_minimum_required(VERSION 3.4.3)

if (NOT DEFINED ENV{LLVM_HOME})
	message(FATAL_ERROR "$LLVM_HOME is not defined")
else ()
	set(ENV{LLVM_DIR} $ENV{LLVM_HOME}/lib/cmake/llvm)
endif()

find_package(LLVM REQUIRED CONFIG)
add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})

add_library(Graph MODULE
	Graph.cpp
	Pattern.cpp
)

set_target_properties(Graph PROPERTIES
	CXX_STANDARD 20
	COMPILE_FLAGS "-fno-rtti"
	LINK_FLAGS "-Wl,-znodelete"
)
