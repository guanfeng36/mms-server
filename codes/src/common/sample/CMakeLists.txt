SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/tools)

SET(ENCRYPT_SOURCES
    encrypt_tool.c
)

ADD_EXECUTABLE(encrypt_tool ${ENCRYPT_SOURCES})
TARGET_LINK_LIBRARIES(encrypt_tool common m ssl crypto)
