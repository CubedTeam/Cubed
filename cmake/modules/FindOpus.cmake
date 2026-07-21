include(FindPackageHandleStandardArgs)

find_path(OPUS_INCLUDE_DIR
    NAMES opus/opus.h opus.h
)

find_library(OPUS_LIBRARY
    NAMES opus
)

find_package_handle_standard_args(
    Opus
    REQUIRED_VARS
        OPUS_INCLUDE_DIR
        OPUS_LIBRARY
)

if(Opus_FOUND)
    add_library(Opus::Opus UNKNOWN IMPORTED)

    set_target_properties(Opus::Opus PROPERTIES
        IMPORTED_LOCATION "${OPUS_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${OPUS_INCLUDE_DIR}"
    )
endif()