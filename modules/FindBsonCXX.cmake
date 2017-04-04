find_path(BSONCXX_INCLUDE
    NAMES bsoncxx/types.hpp
    PATHS ${BSONCXX_ROOT_DIR}/include/bsoncxx/v_noabi/
    DOC "The bsoncxx include directory"
)

find_path(BSONCXX_LIBRARY 
    NAMES libbsoncxx.so
    PATHS ${BSONCXX_ROOT_DIR}/lib
    DOC "The bsoncxx library"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(BSONCXX DEFAULT_MSG BSONCXX_INCLUDE BSONCXX_LIBRARY)

if (BSONCXX_FOUND)
    set(BSONCXX_LIBRARY ${BSONCXX_LIBRARY} )
    set(BSONCXX_INCLUDE ${BSONCXX_INCLUDE} )
    set(BSONCXX_DEFINITIONS )
endif()


