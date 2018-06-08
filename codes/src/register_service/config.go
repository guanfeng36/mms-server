package main

import (
	"github.com/Unknwon/goconfig"
)
const AUTH_ENCODE_KEY = "qWuYhJBMhGbFhLpIhuHgBNMk"

const (
	CONFIG_FILE_PATH                 = "/etc/jh_mms_conf"
	DEFAULT_MYSQL_ADDR               = "127.0.0.1"
	DEFAULT_MYSQL_PORT               = "3306"
	DEFAULT_MYSQL_USER               = "root"
	DEFAULT_MYSQL_DBNAME             = "infoDB"
	DEFAULT_MYSQL_PASSWORD           = ""
	DEFAULT_MAX_CONN                 = 20
	DEFAULT_MAX_IDLE_CONN            = 5

    DEFAULT_KAFKA_BROKERS            = "127.0.0.1:9092"

    DEFAULT_LOG_FILE                 = "./logs/register_service.log"
    DEFAULT_LOG_EXPIRE_DAYS          = 7
    DEFAULT_LOG_LEVEL                = 7
)

var GlobalConfig *Config

type Config struct {
	//MYSQL CONFIG
	MYSQL_ADDR          string //mysql ip地址
	MYSQL_PORT          string //mysql port端口
	MYSQL_USER          string //mysql 登陆用户名
	MYSQL_PASSWORD      string //mysql 登陆密码
	MYSQL_DBNAME        string //mysql 数据库名称
    MYSQL_MAX_IDLE_CONN int    //mysql 最大空闲连接数
    MYSQL_MAX_CONN      int    //mysql 最大连接数

	LOG_FILE        string //日志保存目录
	LOG_LEVEL       int    //日志级别
	LOG_EXPIRE_DAYS int    //日志保留天数

	KAFKA_BROKERS string
}

func InitGlobalConfig() error {
	cfg, err := goconfig.LoadConfigFile(CONFIG_FILE_PATH)
	if err != nil {
		return err
	}
	GlobalConfig = &Config{
		MYSQL_ADDR:               cfg.MustValue("mysql", "host", DEFAULT_MYSQL_ADDR),
		MYSQL_PORT:               cfg.MustValue("mysql", "port", DEFAULT_MYSQL_PORT),
		MYSQL_USER:               cfg.MustValue("mysql", "username", DEFAULT_MYSQL_USER),
		MYSQL_PASSWORD:           cfg.MustValue("mysql", "password", DEFAULT_MYSQL_PASSWORD),
		MYSQL_DBNAME:             cfg.MustValue("mysql", "dbname", DEFAULT_MYSQL_DBNAME),
		MYSQL_MAX_CONN:           cfg.MustInt("mysql", "mysql_max_conn", DEFAULT_MAX_CONN),
		MYSQL_MAX_IDLE_CONN:      cfg.MustInt("mysql", "mysql_max_idle_conn", DEFAULT_MAX_IDLE_CONN),

		LOG_FILE:                 cfg.MustValue("log", "log_file", DEFAULT_LOG_FILE),
		LOG_EXPIRE_DAYS:          cfg.MustInt("log", "log_expire_days", DEFAULT_LOG_EXPIRE_DAYS),
		LOG_LEVEL:                cfg.MustInt("log", "log_level", DEFAULT_LOG_LEVEL),

		KAFKA_BROKERS:            cfg.MustValue("kafka", "brokers", DEFAULT_KAFKA_BROKERS),
	}
	return nil
}
