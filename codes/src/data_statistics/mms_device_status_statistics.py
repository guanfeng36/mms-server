#coding=utf-8

from datetime import datetime
from datetime import timedelta
import MySQLdb
import os, sys, sched, time

MYSQL_USER = "root"
MYSQL_PWD = "abc123"
MYSQL_HOST = "127.0.0.1"
MYSQL_PORT = 3306;
MYSQL_DATABASE = "infoDB"

# period(day) of data to keep
DATA_RETAIN_PERIOD_DAYS = 7
# period(second) of every two statistics action
DATA_COLLECT_INTERVAL_SECS = 1800

def init_db():
    ret_val = True;
    conn = None;
    try:
        conn = MySQLdb.connect(
            host = MYSQL_HOST,
            port = MYSQL_PORT,
            user = MYSQL_USER,
            passwd = MYSQL_PWD
            )
        cur = conn.cursor();
        #创建 DB
        cur.execute("CREATE DATABASE IF NOT EXISTS %s DEFAULT CHARSET utf8 COLLATE utf8_general_ci" % (MYSQL_DATABASE));
        conn.select_db(MYSQL_DATABASE);

        cur.execute("CREATE TABLE IF NOT EXISTS device_status_history(id INT PRIMARY KEY AUTO_INCREMENT,  timestamp datetime NOT NULL, postinfo_id varchar(20) NOT NULL, status INT(1) unsigned NOT NULL) ENGINE=InnoDB DEFAULT CHARSET=utf8")

        cur.close()
        conn.commit()
    except MySQLdb.Error,e:
        ret_val = False;
        print "Mysql Error %d: %s" % (e.args[0], e.args[1]);
    finally:
        if conn:
            conn.close();
    return ret_val;

def get_device_status_statistics_data():
    print "start to collect device status data..."
    current_time = datetime.now().strftime('%Y/%m/%d %H:%M:%S');

    data_retain_time = (datetime.now() - timedelta(days=DATA_RETAIN_PERIOD_DAYS)).strftime('%Y/%m/%d %H:00:00');
    print "current time: %s, data retain time: %s" % (current_time, data_retain_time)

    conn = None;
    try:
        conn = MySQLdb.connect(
            host = MYSQL_HOST,
            port = MYSQL_PORT,
            user = MYSQL_USER,
            passwd = MYSQL_PWD,
            db = MYSQL_DATABASE
            )
        cur = conn.cursor();
        cur.execute("insert into device_status_history(timestamp, postinfo_id, status) select '%s', event.host_id, min(min_priority) from ( select   host_id, min(priority) as min_priority from strategy_event where status != 3 group by host_id union select host_id, min(priority) as min_priority from notice_event where status != 3 group by host_id) as event group by event.host_id" %(current_time));
        # cur.execute("insert into device_status_history(timestamp, postinfo_id, status) select '%s',  host_id, min(priority) as min_priority from strategy_event where status != 3 group by host_id" %(current_time));
        cur.execute("delete from device_status_history where timestamp < '%s'" %(data_retain_time));

        cur.close()
        conn.commit()
        print "Succeed to save data into mysql!"
    except MySQLdb.Error,e:
        print "Mysql Error %d: %s" % (e.args[0], e.args[1]);
    finally:
        if conn:
            conn.close();

schedule = sched.scheduler(time.time, time.sleep);

def perform_command():
    get_device_status_statistics_data();
    schedule.enter(DATA_COLLECT_INTERVAL_SECS, 0, perform_command, ());

import mms_daemon
class ClientDaemon(mms_daemon.CDaemon):
    def __init__(self, name, save_path, stdin=os.devnull, stdout=os.devnull, stderr=os.devnull, home_dir='.', umask=022, verbose=1):
        mms_daemon.CDaemon.__init__(self, save_path, stdin, stdout, stderr, home_dir, umask, verbose)
        self.name = name

    def run(self, **kwargs):
        if not init_db():
            sys.stderr.write("Cannot init mysql database, and exit...");
        else: 
            get_device_status_statistics_data();
            schedule.enter(DATA_COLLECT_INTERVAL_SECS, 0, perform_command, ());
            schedule.run();

if __name__ == '__main__':
    help_msg = 'Usage: python %s <start|stop|restart|status|start-foreground>' % sys.argv[0]
    if len(sys.argv) != 2:
        print help_msg
        sys.exit(1)
    p_name = 'device_status_statistics'
    pid_fn = '/tmp/mms_device_status_statistics.pid'
    log_fn = '/var/log/mms_device_status_statistics.log'
    err_fn = '/var/log/mms_device_status_statistics.err.log'
    cD = ClientDaemon(p_name, pid_fn, stdout=log_fn, stderr=err_fn, verbose=1)

    if sys.argv[1] == 'start':
        cD.start()
    elif sys.argv[1] == 'start-foreground':
        pid = str(os.getpid())
        file(cD.pidfile, 'w+').write('%s\n' % pid)
        cD.run()
    elif sys.argv[1] == 'stop':
        cD.stop()
    elif sys.argv[1] == 'restart':
        cD.restart()
    elif sys.argv[1] == 'status':
        alive = cD.is_running()
        if alive:
            print 'process [%s] is running ......' % cD.get_pid()
        else:
            print 'daemon process [%s] stopped' %cD.name
    else:
        print 'invalid argument!'
        print help_msg
