set(target libxsmm)

add_compile_definitions(
        _WINDLL
        LIBXSMM_BUILD
#        LIBXSMM_BUILD_EXT
)

file(GLOB SRC_FILES "*.cpp" "*.c")

message("*** source files for ${target} are: ${SRC_FILES}")

add_library(${target} SHARED ${SRC_FILES})

#target_link_directories(${target} ${ASMJIT_LIBDIR})

