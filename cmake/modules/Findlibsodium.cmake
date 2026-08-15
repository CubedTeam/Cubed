include(FindPackageHandleStandardArgs)

# AI-generated: Normalize supported libsodium providers.
find_package(unofficial-sodium CONFIG QUIET)

if(TARGET unofficial-sodium::sodium)
    set(libsodium_LINK_SOURCE unofficial-sodium::sodium)
    set(libsodium_VERSION "${unofficial-sodium_VERSION}")
endif()

if(NOT libsodium_LINK_SOURCE)
    find_package(PkgConfig QUIET)

    if(PkgConfig_FOUND)
        pkg_check_modules(PC_libsodium QUIET IMPORTED_TARGET libsodium)
    endif()

    if(TARGET PkgConfig::PC_libsodium)
        set(libsodium_LINK_SOURCE PkgConfig::PC_libsodium)
        set(libsodium_VERSION "${PC_libsodium_VERSION}")
    endif()
endif()

if(NOT libsodium_LINK_SOURCE)
    find_path(libsodium_INCLUDE_DIR
        NAMES sodium.h
    )

    find_library(libsodium_LIBRARY
        NAMES sodium libsodium
    )

    if(libsodium_INCLUDE_DIR AND libsodium_LIBRARY)
        set(libsodium_LINK_SOURCE "${libsodium_LIBRARY}")
    endif()
endif()

find_package_handle_standard_args(libsodium
    REQUIRED_VARS libsodium_LINK_SOURCE
    VERSION_VAR libsodium_VERSION
)

if(libsodium_FOUND AND NOT TARGET libsodium::libsodium)
    if(libsodium_INCLUDE_DIR AND libsodium_LIBRARY)
        add_library(libsodium::libsodium UNKNOWN IMPORTED)
        set_target_properties(libsodium::libsodium PROPERTIES
            IMPORTED_LOCATION "${libsodium_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${libsodium_INCLUDE_DIR}"
        )
    else()
        add_library(libsodium::libsodium INTERFACE IMPORTED)
        set_target_properties(libsodium::libsodium PROPERTIES
            INTERFACE_LINK_LIBRARIES "${libsodium_LINK_SOURCE}"
        )
    endif()
endif()

mark_as_advanced(
    libsodium_INCLUDE_DIR
    libsodium_LIBRARY
)
