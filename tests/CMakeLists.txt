target_sources(test
  PUBLIC
    readtests.cc
    integrationtests.cc
    symbolicexpressionparsertests.cc
)

# TODO Restrict to only test exe
add_subdirectory(include/Catch2)

target_link_libraries(test PRIVATE Catch2::Catch2WithMain)

# TODO remove
target_include_directories(test PRIVATE ${CMAKE_SOURCE_DIR}/include)
