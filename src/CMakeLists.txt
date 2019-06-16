
set(SOURCES
    format-parse.c
    re.c
)

add_library(replacetag ${SOURCES})
target_include_directories(replacetag PUBLIC ${FP_ROOT_DIR}/src)

if(FP_EXAMPLE)
    add_executable(parse main.c)
    target_link_libraries(parse replacetag)
endif(FP_EXAMPLE)


# if(FP_BUILD_CPP OR FP_EXAMPLE_CPP)
#     add_library(formatparser++ format-parser.cpp)
#     target_link_libraries(formatparser++ formatparser)
# endif(FP_BUILD_CPP OR FP_EXAMPLE_CPP)


# if(FP_EXAMPLE_CPP)
#     add_executable(parse++ main.cpp)
#     target_link_libraries(parse++ formatparser++)
# endif(FP_EXAMPLE_CPP)
