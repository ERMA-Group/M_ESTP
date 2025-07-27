# Define a library for this submodule
add_library(M_ESTF STATIC)

set(M_ROOT_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

# Specify the source files
target_sources(M_ESTF
PUBLIC
	${M_ROOT_PATH}/src/estf.c
)

# Specify the include directories for this submodule
target_include_directories(M_ESTF 
PUBLIC
    ${M_ROOT_PATH}/src
    ${M_ROOT_PATH}/Interface
)

# This links the cfg library so the module can #include "x_cfg.h" and use cfg.c
target_link_libraries(M_ESTF
    PUBLIC
		M_ERMA
		M_ESTF_cfg
)