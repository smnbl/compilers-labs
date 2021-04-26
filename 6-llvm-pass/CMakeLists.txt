cmake_minimum_required(VERSION 3.16.3)

project(Micro_C_Compiler)

# Find fmt
find_package(fmt REQUIRED)

message(STATUS "Found fmt ${fmt_VERSION}")
message(STATUS "Using fmt in ${fmt_DIR}")

# Find LLVM
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in ${LLVM_DIR}")

if (NOT ${LLVM_ENABLE_ASSERTIONS})
    message(WARNING "Using an LLVM build without assertions")
endif()

llvm_map_components_to_libnames(LLVM_LIBRARIES
    core
    )

# list of all targets that need to be built
set(MICROCC_ALL_TARGETS  ast    runtime boundscheck microcc)

function(add_microcc_library name)
    if ("${name}" IN_LIST MICROCC_ALL_TARGETS)
        add_library(${ARGV})
    else()
        message(STATUS "Using pre-built library '${name}' instead of building from source")
    endif()
endfunction()

# lexer
add_microcc_library(lexer
    src/lexer/lexer.cpp
    src/lexer/token.cpp
    )

# ast
add_microcc_library(ast
    src/ast/prettyprinter.cpp
    )

# parser
add_microcc_library(parser
    src/parser/parser.cpp
    )

# sema
add_microcc_library(sema
    src/sema/collectfuncdeclspass.cpp
    src/sema/scoperesolutionpass.cpp
    src/sema/typecheckingpass.cpp
    src/sema/util.cpp
    )

# codegen llvm
add_microcc_library(codegen-llvm
    src/codegen-llvm/codegen-llvm.cpp
    src/codegen-llvm/util.cpp
    )

# micro-C runtime
add_microcc_library(runtime
    src/runtime/runtime.cpp
    )

# passes
add_microcc_library(boundscheck
    SHARED
    src/boundscheck/pass.cpp
    )

# driver
add_executable(microcc
    src/driver/main.cpp
    )

target_link_libraries(microcc PUBLIC lexer ast parser sema codegen-llvm)

# set properties common to all targets
foreach(TARGET ${MICROCC_ALL_TARGETS})
    target_include_directories(${TARGET} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
    target_include_directories(${TARGET} PRIVATE "${LLVM_INCLUDE_DIRS}")

    # NOTE: We must NOT link with the LLVM libraries for LLVM passes, otherwise
    # we would get an error "option debug-pass registered more than once", as
    # these llvm::cl::opts are linked in with opt and with our pass!
    if (NOT ${TARGET} STREQUAL "boundscheck")
        target_link_libraries(${TARGET} PRIVATE "${LLVM_LIBRARIES}")
    endif()

    target_link_libraries(${TARGET} PRIVATE fmt::fmt)

    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib")
        target_link_directories(${TARGET} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/lib")
    endif()

    if (NOT ${LLVM_ENABLE_RTTI})
        target_compile_options(${TARGET} PRIVATE "-fno-rtti")
        target_link_options(${TARGET} PRIVATE "-fno-rtti")
    endif()
endforeach()

