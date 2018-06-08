# MYSQL_FOUND - true if library and headers were found
# MYSQL_INCLUDE_DIRS - include directories
# MYSQL_LIBRARIES - library directories

#SET(MYSQL_NAMES mysqlclient mysqlclient_r)
SET(MYSQL_NAMES  mysqlclient_r)
find_package(PkgConfig)
pkg_check_modules(PC_MYSQL QUIET ${MYSQL_NAMES})

find_path(MYSQL_INCLUDE_DIR mysql.h
    /user/include PATH_SUFFIXES mysql)

find_library(MYSQL_LIBRARY NAMES ${MYSQL_NAMES}
    HINTS ${PC_MYSQL_LIBDIR} ${PC_MYSQL_LIBRARY_DIRS} PATH_SUFFIXES mysql)

set(MYSQL_LIBRARIES ${MYSQL_LIBRARY})
set(MYSQL_INCLUDE_DIRS ${MYSQL_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(LIBMYSQL DEFAULT_MSG MYSQL_LIBRARY MYSQL_INCLUDE_DIR)

mark_as_advanced(MYSQL_INCLUDE_DIR MYSQL_LIBRARY)
