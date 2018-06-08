SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/build/modules")

SET(SOURCES
    task_management_service_main.c
    config_task_message_handler.c
)

ADD_EXECUTABLE(task_management_service_app ${SOURCES})
TARGET_LINK_LIBRARIES(task_management_service_app json-c pthread rdkafka)
TARGET_LINK_LIBRARIES(task_management_service_app message_helper minIni common)

FIND_PACKAGE(MYSQL REQUIRED)
INCLUDE_DIRECTORIES(${MYSQL_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(task_management_service_app ${MYSQL_LIBRARIES})
