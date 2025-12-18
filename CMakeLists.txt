# Define a library for this submodule
add_library(M_ESTP STATIC)

set(M_ROOT_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

# Specify the source files
target_sources(M_ESTP
PUBLIC
	${M_ROOT_PATH}/src/estp.cpp
)

# Specify the include directories for this submodule
target_include_directories(M_ESTP 
PUBLIC
    ${M_ROOT_PATH}/inc
)