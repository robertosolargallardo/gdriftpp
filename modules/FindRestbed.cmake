find_path(RESTBED_INCLUDE_DIR
    NAMES http.hpp
    PATHS ${RESTBED_ROOT_DIR}/include/corvusoft/restbed/
    DOC "The Restbed include directory"
)

find_library(RESTBED_LIBRARY 
    NAMES restbed
    PATHS ${RESTBED_ROOT_DIR}/library
    DOC "The Restbed library"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(RESTBED DEFAULT_MSG RESTBED_INCLUDE_DIR RESTBED_LIBRARY)

if (RESTBED_FOUND)
    set(RESTBED_LIBRARIES ${RESTBED_LIBRARY} )
    set(RESTBED_INCLUDE_DIRS ${RESTBED_INCLUDE_DIR} )
    set(RESTBED_DEFINITIONS )
endif()


