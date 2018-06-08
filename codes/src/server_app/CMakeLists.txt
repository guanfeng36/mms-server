SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/build/modules")

SET(SOURCES
    server_app_main.c
    resp_sender_service_func.c
)

ADD_EXECUTABLE(server_app ${SOURCES})
TARGET_LINK_LIBRARIES(server_app json-c pthread rdkafka)
TARGET_LINK_LIBRARIES(server_app message_helper minIni common)

ADD_SUBDIRECTORY(test)
