#set(GDRIFT++_ROOT_DIR /usr/local)
find_path(GDRIFT_INCLUDE_DIR
    NAMES Simulator.h
    PATHS ${GDRIFT_ROOT_DIR}/include/gdrift++
    DOC "The gdrift++ directory"
)

find_library(GDRIFT_LIBRARY 
    NAMES gdrift++
    PATHS ${GDRIFT_ROOT_DIR}/lib
    DOC "The gdrift++ library"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GDRIFT DEFAULT_MSG GDRIFT_INCLUDE_DIR GDRIFT_LIBRARY)

if (GDRIFT_FOUND)
    set(GDRIFT_LIBRARIES ${GDRIFT_LIBRARY} )
    set(GDRIFT_INCLUDE_DIRS ${GDRIFT_INCLUDE_DIR} )
    set(GDRIFT_DEFINITIONS )
endif()


