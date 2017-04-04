find_path(BSONCXX_INCLUDE_DIR
    NAMES bsoncxx/types.hpp
    PATHS ${BSONCXX_ROOT_DIR}/include/bsoncxx/v_noabi/
    DOC "The BSONCXX include directory"
)

find_library(BSONCXX_LIBRARY 
    NAMES bsoncxx
    PATHS ${BSONCXX_ROOT_DIR}
    DOC "The BSONCXX library"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(BSONCXX DEFAULT_MSG BSONCXX_INCLUDE_DIR BSONCXX_LIBRARY)

if (BSONCXX_FOUND)
    set(BSONCXX_LIBRARIES ${BSONCXX_LIBRARY} )
    set(BSONCXX_INCLUDE_DIRS ${BSONCXX_INCLUDE_DIR} )
    set(BSONCXX_DEFINITIONS )
endif()


