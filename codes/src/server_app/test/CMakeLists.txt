SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/test)
ADD_EXECUTABLE(server_test server_test.c)
TARGET_LINK_LIBRARIES(server_test message_helper)
TARGET_LINK_LIBRARIES(server_test json-c)

ADD_EXECUTABLE(dummy_server dummy_server.c)
TARGET_LINK_LIBRARIES(dummy_server message_helper)
TARGET_LINK_LIBRARIES(dummy_server json-c)

ADD_CUSTOM_TARGET(
        copy_test_data 
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/data
        ${EXECUTABLE_OUTPUT_PATH}/data)
ADD_DEPENDENCIES(server_test copy_test_data)
