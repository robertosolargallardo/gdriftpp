INCLUDE(ExternalProject)

set(EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external)

ExternalProject_Add(
   libgdrift
   GIT_REPOSITORY  https://github.com/robertosolargallardo/libgdrift.git
   CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION}
)
