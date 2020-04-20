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

add_library(Anselm MODULE
	Anselm.cpp
	Context.cpp
)

set_target_properties(Anselm PROPERTIES
	CXX_STANDARD 17
	COMPILE_FLAGS "-fno-rtti"
	LINK_FLAGS "-Wl,-znodelete"
)
