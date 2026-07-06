add_library(dr_libs STATIC
    dr_libs.cpp
)

target_include_directories(dr_libs PUBLIC
    ${CMAKE_SOURCE_DIR}/third_party/dr_libs/include
)
