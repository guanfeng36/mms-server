INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

ADD_SUBDIRECTORY(common)
ADD_SUBDIRECTORY(message_helper)
ADD_SUBDIRECTORY(minIni)

ADD_SUBDIRECTORY(server_app)
ADD_SUBDIRECTORY(inform_service)
ADD_SUBDIRECTORY(heartbeat_service)
ADD_SUBDIRECTORY(task_management_service)
ADD_SUBDIRECTORY(event_service)
ADD_SUBDIRECTORY(influxdb_metrics_service)

ADD_SUBDIRECTORY(data_statistics)
