SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/build/modules")

SET(SOURCES
    influxdb_metrics_service.c
)

ADD_EXECUTABLE(influxdb_metrics_app ${SOURCES})
TARGET_LINK_LIBRARIES(influxdb_metrics_app json-c pthread rdkafka)
TARGET_LINK_LIBRARIES(influxdb_metrics_app message_helper minIni common)

FIND_PACKAGE(MYSQL REQUIRED)
INCLUDE_DIRECTORIES(${MYSQL_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(influxdb_metrics_app ${MYSQL_LIBRARIES})
