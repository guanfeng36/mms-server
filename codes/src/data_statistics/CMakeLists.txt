SET(FILES
    mms_daemon.py
    mms_device_status_statistics.py
)

add_custom_command(OUTPUT ${FILES}
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/mms_daemon.py ${EXECUTABLE_OUTPUT_PATH}
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/mms_device_status_statistics.py ${EXECUTABLE_OUTPUT_PATH}
)

ADD_CUSTOM_TARGET(copy_pyscript ALL DEPENDS ${FILES})

