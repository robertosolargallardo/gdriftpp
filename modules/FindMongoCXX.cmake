find_path(MONGOCXX_INCLUDE_DIR
    NAMES mongocxx/client.hpp
    PATHS ${MONGOCXX_ROOT_DIR}/include/mongocxx/v_noabi/
    DOC "The MONGOCXX include directory"
)

find_library(MONGOCXX_LIBRARY 
    NAMES mongocxx
    PATHS ${MONGOCXX_ROOT_DIR}
    DOC "The MONGOCXX library"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MONGOCXX DEFAULT_MSG MONGOCXX_INCLUDE_DIR MONGOCXX_LIBRARY)

if (MONGOCXX_FOUND)
    set(MONGOCXX_LIBRARIES ${MONGOCXX_LIBRARY} )
    set(MONGOCXX_INCLUDE_DIRS ${MONGOCXX_INCLUDE_DIR} )
    set(MONGOCXX_DEFINITIONS )
endif()


