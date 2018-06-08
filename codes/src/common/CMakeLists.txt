SET(LIBSOURCES
    common.c
    log.c
    base64.c
    crypt.c
    message.c
    kafka_util.c
)

ADD_LIBRARY(common ${LIBSOURCES})
TARGET_LINK_LIBRARIES(common zlog ${GLIB2_LIBRARIES})

ADD_SUBDIRECTORY(sample)

ADD_CUSTOM_TARGET(
        copy_common_conf 
        COMMAND ${CMAKE_COMMAND} -E copy 
        ${CMAKE_CURRENT_SOURCE_DIR}/conf/jhtm_log_conf 
        ${CONFIGURE_OUTPUT_PATH}/jhtm_log_conf)
ADD_DEPENDENCIES(common copy_common_conf)
