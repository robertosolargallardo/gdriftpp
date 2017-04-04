find_path(GDRIFT_INCLUDE
    NAMES Simulator.h
    PATHS ${GDRIFT_ROOT_DIR}/include/gdrift++
    DOC "The gdrift++ directory"
)
find_path(GDRIFT_LIBRARY
    NAMES libgdrift++.so
    PATHS ${GDRIFT_ROOT_DIR}/lib
    DOC "The gdrift++ directory"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GDRIFT DEFAULT_MSG GDRIFT_INCLUDE GDRIFT_LIBRARY)

if (GDRIFT_FOUND)
    set(GDRIFT_LIBRARY ${GDRIFT_LIBRARY})
    set(GDRIFT_INCLUDE ${GDRIFT_INCLUDE})
    set(GDRIFT_DEFINITIONS)
endif()


