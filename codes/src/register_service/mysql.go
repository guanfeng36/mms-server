package main

import (
	"database/sql"
	"fmt"
	"time"
	_ "github.com/go-sql-driver/mysql"
)

var mydb *db

type db struct {
	*sql.DB
}

func InitMysqlConnPool() error {
	var err error
	var conn *sql.DB
	conn, err = sql.Open("mysql", fmt.Sprintf("%s:%s@tcp(%s)/%s?charset=utf8&parseTime=true&loc=Local",
		GlobalConfig.MYSQL_USER, GlobalConfig.MYSQL_PASSWORD, GlobalConfig.MYSQL_ADDR + ":" + GlobalConfig.MYSQL_PORT, GlobalConfig.MYSQL_DBNAME))
	if err != nil {
		return err
	}
	err = conn.Ping()
	if err != nil {
		return err
	}
	conn.SetMaxIdleConns(GlobalConfig.MYSQL_MAX_IDLE_CONN)
	conn.SetMaxOpenConns(GlobalConfig.MYSQL_MAX_CONN)
	mydb = &db{conn}
	return nil
}

func (this *db) IsDeviceExisted(device_id string) (bool, error) {
	var mac_addr string;
	err := this.QueryRow("select mac_addr from postinfo where id = ?", device_id).Scan(&mac_addr)
	if err != nil {
		if err == sql.ErrNoRows {
			return false, nil;
		} else {
			lg.Error(err.Error())
			return true, err;
		}
	}
	return true, nil;
}

func (this *db) CreateRegisterDevice(device_info *DeviceInfo) (error){
	stmt, err := this.Prepare("INSERT INTO `postinfo`(`id`, `mac_addr`, `device_code`, `ipv4`, `reg_time`) VALUES (?, ?, ?, ?, ?)")
	if err != nil {
		lg.Error(err.Error())
		return err
	}

	_, err = stmt.Exec(
		device_info.ID,
		device_info.MacAddr,
		device_info.DeviceCode,
		device_info.Ipv4,
		time.Now())

	if err != nil {
		lg.Error(err.Error())
		return err
	}
	
	return nil
}
