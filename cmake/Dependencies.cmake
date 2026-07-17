include(FetchContent)

# System packages
find_package(OpenGL REQUIRED)
find_package(Protobuf REQUIRED)
find_package(absl REQUIRED)
find_package(zstd REQUIRED)
find_package(OpenAL REQUIRED)
find_package(harfbuzz REQUIRED)
find_package(Freetype REQUIRED)
find_package(SDL3 REQUIRED)


# Third-party libraries
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.3 
    
)
FetchContent_MakeAvailable(glm)
FetchContent_Declare(
    soil2
    GIT_REPOSITORY https://github.com/SpartanJ/SOIL2.git
    GIT_TAG 1.31  
    
)
FetchContent_MakeAvailable(soil2)
FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(tomlplusplus)


if (WIN32)

    set(_BUILD_SHARED_LIBS_SAVED ${BUILD_SHARED_LIBS})
    set(BUILD_SHARED_LIBS ON)

    FetchContent_Declare(
    onetbb                           
    GIT_REPOSITORY https://github.com/uxlfoundation/oneTBB.git
    GIT_TAG v2023.0.0
    )
    
    set(BUILD_TESTING OFF CACHE BOOL "Build tests" FORCE)
    set(TBB_TEST OFF CACHE BOOL "Build TBB tests" FORCE)
    FetchContent_MakeAvailable(onetbb)

    set(BUILD_SHARED_LIBS ${_BUILD_SHARED_LIBS_SAVED})
    unset(_BUILD_SHARED_LIBS_SAVED)

endif()

