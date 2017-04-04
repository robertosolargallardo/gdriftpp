find_path(RESTBED_INCLUDE
    NAMES http.hpp
    PATHS ${RESTBED_ROOT_DIR}/include/corvusoft/restbed/
    DOC "The restbed include directory"
)

find_path(RESTBED_LIBRARY 
    NAMES librestbed.a
    PATHS ${RESTBED_ROOT_DIR}/library
    DOC "The restbed library"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(RESTBED DEFAULT_MSG RESTBED_INCLUDE RESTBED_LIBRARY)

if (RESTBED_FOUND)
    set(RESTBED_LIBRARY ${RESTBED_LIBRARY} )
    set(RESTBED_INCLUDE ${RESTBED_INCLUDE} )
    set(RESTBED_DEFINITIONS )
endif()


