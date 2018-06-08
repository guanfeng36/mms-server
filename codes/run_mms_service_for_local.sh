#!/bin/sh

if (! test "$(id -u)" == 0)
then
    echo "Need to be root!"
    exit -1;
fi

echo "kill server_app/publish_server_app/inform_service_app/heartbeat_service_app/task_management_service_app ..."
killall -9 server_app;
killall -9 publish_server_app;
killall -9 heartbeat_service_app;
killall -0 register_service_app;
killall -9 inform_service_app;
killall -9 task_management_service_app;
killall -9 event_service_app;

bin/register_service_app &
bin/heartbeat_service_app &
bin/inform_service_app &
bin/task_management_service_app &
bin/event_service_app &
bin/publish_server_app &
bin/server_app &
