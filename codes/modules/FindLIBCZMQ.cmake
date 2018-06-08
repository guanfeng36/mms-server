# LIBCZMQ_FOUND - true if library and headers were found
# LIBCZMQ_INCLUDE_DIRS - include directories
# LIBCZMQ_LIBRARIES - library directories

find_package(PkgConfig)
pkg_check_modules(PC_LIBCZMQ QUIET czmq)

find_path(LIBCZMQ_INCLUDE_DIR czmq.h
	HINTS ${PC_LIBCZMQ_INCLUDEDIR} ${PC_LIBCZMQ_INCLUDE_DIRS} PATH_SUFFIXES libczmq)

find_library(LIBCZMQ_LIBRARY NAMES czmq libczmq
	HINTS ${PC_LIBCZMQ_LIBDIR} ${PC_LIBCZMQ_LIBRARY_DIRS})

set(LIBCZMQ_LIBRARIES ${LIBCZMQ_LIBRARY})
set(LIBCZMQ_INCLUDE_DIRS ${LIBCZMQ_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(LIBCZMQ DEFAULT_MSG LIBCZMQ_LIBRARY LIBCZMQ_INCLUDE_DIR)

mark_as_advanced(LIBCZMQ_INCLUDE_DIR LIBCZMQ_LIBRARY)
