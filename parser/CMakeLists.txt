cmake_minimum_required(VERSION 3.14)
project(Parser CXX)

set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

BISON_TARGET(ParserGen parser.y
        ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.c
        DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.h
        COMPILE_FLAGS "--verbose"
)

FLEX_TARGET(LexerGen lexer_reader.l
        ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c
)

ADD_FLEX_BISON_DEPENDENCY(LexerGen ParserGen)

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
)

set(BISON_SRC ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.c)
set(FLEX_SRC  ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c)

set_source_files_properties(${BISON_SRC} ${FLEX_SRC} PROPERTIES LANGUAGE CXX)
set_source_files_properties(${FLEX_SRC} PROPERTIES COMPILE_FLAGS "-Wno-unused-function")

add_executable(parser
        main.cpp
        Parser.cpp
        ${BISON_SRC}
        ${FLEX_SRC}
)

# ── Size reduction ────────────────────────────────────────────────────────────
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(parser PRIVATE
            -Os
            -ffunction-sections
            -fdata-sections
    )
    if(APPLE)
        target_link_options(parser PRIVATE -Wl,-dead_strip)
    else()
        target_link_options(parser PRIVATE -Wl,--gc-sections -Wl,-s)
    endif()
endif()