INCLUDE_DIRECTORIES(
    ./
)

SET(SOURCES
    message_sender.c
    message_subscriber.c
    async_server.c
    publish_server.c
)

ADD_LIBRARY(message_helper ${SOURCES})
TARGET_LINK_LIBRARIES(message_helper ${LIBCZMQ_LIBRARIES})

ADD_SUBDIRECTORY(test)
