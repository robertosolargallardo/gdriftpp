find_path(MONGOCXX_INCLUDE
    NAMES mongocxx/client.hpp
    PATHS ${MONGOCXX_ROOT_DIR}/include/mongocxx/v_noabi/
    DOC "The MONGOCXX include directory"
)

find_path(MONGOCXX_LIBRARY 
    NAMES libmongocxx.so
    PATHS ${MONGOCXX_ROOT_DIR}/lib
    DOC "The MONGOCXX library"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MONGOCXX DEFAULT_MSG MONGOCXX_INCLUDE MONGOCXX_LIBRARY)

if (MONGOCXX_FOUND)
    set(MONGOCXX_LIBRARY ${MONGOCXX_LIBRARY} )
    set(MONGOCXX_INCLUDE ${MONGOCXX_INCLUDE} )
    set(MONGOCXX_DEFINITIONS )
endif()


