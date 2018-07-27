-- MySQL dump 10.13  Distrib 5.1.71, for jhls-linux-gnu (x86_64)
--
-- Host: localhost    Database: infoDB
-- ------------------------------------------------------
-- Server version	5.1.71

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;


--
-- Current Database: `infoDB`
--

CREATE DATABASE /*!32312 IF NOT EXISTS*/ `infoDB` /*!40100 DEFAULT CHARACTER SET utf8 */;

USE `infoDB`;


--
-- Table structure for table `acl_template`
--

DROP TABLE IF EXISTS `acl_template`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `acl_template` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(32) DEFAULT NULL,
  `content` text,
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL,
  `create_user` varchar(32) DEFAULT '',
  `active` int(11) DEFAULT '1',
  `create_user_id` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `action`
--

DROP TABLE IF EXISTS `action`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `action` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `strategy_id` bigint(20) unsigned NOT NULL,
  `type` tinyint(1) unsigned NOT NULL,
  `file_path` text NOT NULL,
  `alarm_subject` varchar(255) NOT NULL,
  `restore_subject` varchar(255) NOT NULL,
  `alarm_template` text NOT NULL,
  `restore_template` text NOT NULL,
  `send_type` int(1) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  KEY `fk_action_strategy_id` (`strategy_id`),
  CONSTRAINT `fk_action_strategy_id` FOREIGN KEY (`strategy_id`) REFERENCES `strategy` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `action_result`
--

DROP TABLE IF EXISTS `action_result`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `action_result` (
  `strategy_event_id` bigint(20) unsigned NOT NULL,
  `action_id` bigint(20) unsigned NOT NULL,
  `action_type` tinyint(1) unsigned NOT NULL,
  `action_send_type` int(1) unsigned NOT NULL,
  `user_id` int(10) unsigned NOT NULL,
  `username` char(255) NOT NULL,
  `phone` varchar(255) NOT NULL DEFAULT '',
  `mail` varchar(255) NOT NULL DEFAULT '',
  `weixin` varchar(255) NOT NULL DEFAULT '',
  `subject` varchar(255) NOT NULL,
  `content` text NOT NULL,
  `success` tinyint(1) unsigned NOT NULL,
  `response` text NOT NULL,
  KEY `fk_action_result_strategy_event_id` (`strategy_event_id`),
  CONSTRAINT `fk_action_result_strategy_event_id` FOREIGN KEY (`strategy_event_id`) REFERENCES `strategy_event` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `action_user`
--

DROP TABLE IF EXISTS `action_user`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `action_user` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `action_id` int(10) unsigned NOT NULL,
  `user_id` int(10) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_action_id_user_id` (`action_id`,`user_id`),
  KEY `fk_action_user_user_id` (`user_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `action_user_group`
--

DROP TABLE IF EXISTS `action_user_group`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `action_user_group` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `action_id` int(10) unsigned NOT NULL,
  `user_group_id` int(10) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_action_id_user_group_id` (`action_id`,`user_group_id`),
  KEY `fk_action_user_group_user_group_id` (`user_group_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `auth_user`
--

DROP TABLE IF EXISTS `auth_user`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `auth_user` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `password` varchar(128) NOT NULL,
  `last_login` datetime NOT NULL,
  `is_superuser` tinyint(1) NOT NULL,
  `username` varchar(30) NOT NULL,
  `first_name` varchar(30) NOT NULL,
  `last_name` varchar(30) NOT NULL,
  `email` varchar(75) NOT NULL,
  `is_staff` tinyint(1) NOT NULL,
  `is_active` tinyint(1) NOT NULL,
  `date_joined` datetime NOT NULL,
  `remember_token` varchar(64) DEFAULT NULL,
  `group_id` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `username` (`username`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `auth_user`
--

LOCK TABLES `auth_user` WRITE;
/*!40000 ALTER TABLE `auth_user` DISABLE KEYS */;
INSERT INTO `auth_user`(`id`, `password`, `is_superuser`,`username`,`is_staff`,`is_active`,`date_joined`,`remember_token`) VALUES (1,'$2y$10$L9quDx9KjSNgPSIhJc21IuA5EUaxyegoqKffzo2O48XbAy8pxOheK',1,'admin',0,1,NOW(),'Q0UecYiVDtBHrOWKgp2tHU0pMoqevP9Fko744tf6sstQdTfomWxEUla4V6A9');
/*!40000 ALTER TABLE `auth_user` ENABLE KEYS */;
UNLOCK TABLES;


--
-- Table structure for table `config_devices`
--

DROP TABLE IF EXISTS `config_devices`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `config_devices` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(128) NOT NULL,
  `remark` text,
  `path` varchar(128) NOT NULL DEFAULT '',
  `create_time` datetime DEFAULT NULL,
  `create_by_id` int(11) unsigned DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `name` (`name`,`create_by_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `config_history`
--

DROP TABLE IF EXISTS `config_history`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `config_history` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `operation` varchar(32) DEFAULT NULL,
  `operate_time` datetime DEFAULT NULL,
  `operate_by_id` int(11) unsigned DEFAULT NULL,
  `svn` varchar(128) DEFAULT NULL,
  `config_id` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `configuration_info`
--

DROP TABLE IF EXISTS `configuration_info`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `configuration_info` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `remark` varchar(128) DEFAULT NULL,
  `filename` varchar(256) NOT NULL DEFAULT '',
  `svn_path` varchar(256) NOT NULL,
  `svn_revision` int(11) unsigned NOT NULL,
  `config_abs_path` varchar(256) NOT NULL,
  `md5sum` varchar(64) DEFAULT NULL,
  `ftype` int(8) DEFAULT '1',
  `upload_time` datetime DEFAULT NULL,
  `upload_by_id` int(11) unsigned DEFAULT NULL,
  `full_path` varchar(256) DEFAULT NULL,
  `config_device_id` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `configure_task`
--

DROP TABLE IF EXISTS `configure_task`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `configure_task` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `configuration_id` int(11) unsigned NOT NULL,
  `title` varchar(64) NOT NULL,
  `remark` varchar(128) DEFAULT NULL,
  `post_script` varchar(256) DEFAULT NULL,
  `create_time` datetime DEFAULT NULL,
  `create_by_id` int(11) unsigned DEFAULT NULL,
  `last_edit_time` datetime DEFAULT NULL,
  `last_edit_by_id` int(11) unsigned DEFAULT NULL,
  `status` int(8) DEFAULT '1',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `configure_task_devices`
--

DROP TABLE IF EXISTS `configure_task_devices`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `configure_task_devices` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `task_id` int(11) unsigned NOT NULL,
  `device_id` varchar(64) NOT NULL,
  `group_name` varchar(128) DEFAULT NULL,
  `status` int(8) DEFAULT '1',
  `message` varchar(512) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `device_group`
--

DROP TABLE IF EXISTS `device_group`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `device_group` (
  `group_id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `group_name` varchar(128) NOT NULL,
  `remark` varchar(256) NOT NULL DEFAULT '',
  `create_time` datetime DEFAULT NULL,
  `create_by_id` int(11) unsigned DEFAULT NULL,
  `last_edit_time` datetime DEFAULT NULL,
  `last_edit_by_id` int(11) unsigned DEFAULT NULL,
  `main` int(11) DEFAULT NULL,
  PRIMARY KEY (`group_id`),
  UNIQUE KEY `group_name` (`group_name`,`create_by_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `device_group_devices`
--

DROP TABLE IF EXISTS `device_group_devices`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `device_group_devices` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `device_group_id` int(11) unsigned NOT NULL,
  `postinfo_id` varchar(64) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_devicegroup_id_postinfo_id` (`device_group_id`,`postinfo_id`),
  KEY `fk_device_group_devices_postinfo_id` (`postinfo_id`),
  CONSTRAINT `fk_device_group_devices_device_group_id` FOREIGN KEY (`device_group_id`) REFERENCES `device_group` (`group_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_device_group_devices_postinfo_id` FOREIGN KEY (`postinfo_id`) REFERENCES `postinfo` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `device_status_history`
--

DROP TABLE IF EXISTS `device_status_history`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `device_status_history` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `timestamp` datetime NOT NULL,
  `postinfo_id` varchar(64) NOT NULL,
  `status` int(1) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  KEY `fk_device_status_history_postinfo_id` (`postinfo_id`),
  CONSTRAINT `fk_device_status_history_postinfo_id` FOREIGN KEY (`postinfo_id`) REFERENCES `postinfo` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `device_vpn_info`
--

DROP TABLE IF EXISTS `device_vpn_info`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `device_vpn_info` (
  `postinfo_id` varchar(64) NOT NULL,
  `vpn_name` varchar(64) NOT NULL,
  `status` int(1) unsigned NOT NULL,
  `client_cn` varchar(64) NOT NULL,
  `client_ip` varchar(64) NOT NULL,
  `client_vpn_ip` varchar(64) NOT NULL,
  `server_cn` varchar(64) NOT NULL,
  `server_ip` varchar(64) NOT NULL,
  `vpn_state` varchar(64) NOT NULL,
  `vpn_cert_info` text,
  `vpn_cert_start_date` datetime DEFAULT NULL,
  `vpn_cert_end_date` datetime DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL,
  PRIMARY KEY (`postinfo_id`),
  KEY `fk_device_vpn_info_postinfo_id` (`postinfo_id`),
  CONSTRAINT `fk_device_vpn_info_postinfo_id` FOREIGN KEY (`postinfo_id`) REFERENCES `postinfo` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `execute`
--

DROP TABLE IF EXISTS `execute`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `execute` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `strategy_id` bigint(20) unsigned NOT NULL,
  `type` tinyint(1) unsigned NOT NULL,
  `command` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  KEY `fk_execute_strategy_id` (`strategy_id`),
  CONSTRAINT `fk_execute_strategy_id` FOREIGN KEY (`strategy_id`) REFERENCES `strategy` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `execute_result`
--

DROP TABLE IF EXISTS `execute_result`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `execute_result` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `strategy_event_id` bigint(20) unsigned NOT NULL,
  `execute_id` int(10) unsigned NOT NULL,
  `execute_type` tinyint(1) unsigned NOT NULL,
  `command` varchar(255) NOT NULL DEFAULT '',
  `host_id` char(32) NOT NULL,
  `result` varchar(255) NOT NULL DEFAULT 'unexecuted',
  PRIMARY KEY (`id`),
  KEY `fk_execute_result_strategy_event_id` (`strategy_event_id`),
  KEY `fk_execute_result_execute_id` (`execute_id`),
  KEY `fk_execute_result_host_id` (`host_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `file_feature`
--

DROP TABLE IF EXISTS `file_feature`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `file_feature` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(32) DEFAULT NULL,
  `file_name` varchar(128) DEFAULT NULL,
  `active` int(11) DEFAULT '1',
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL,
  `create_user` varchar(32) DEFAULT '',
  `create_user_id` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `fom_template`
--

DROP TABLE IF EXISTS `fom_template`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `fom_template` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(32) DEFAULT NULL,
  `content` text,
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL,
  `create_user` varchar(32) DEFAULT '',
  `active` int(11) DEFAULT '1',
  `create_user_id` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `guomi`
--

DROP TABLE IF EXISTS `guomi`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `guomi` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(32) DEFAULT NULL,
  `file_name` varchar(128) DEFAULT NULL,
  `active` int(11) DEFAULT '1',
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL,
  `create_user` varchar(32) DEFAULT '',
  `create_user_id` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `hardware_template`
--

DROP TABLE IF EXISTS `hardware_template`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hardware_template` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(32) DEFAULT NULL,
  `content` text,
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL,
  `create_user` varchar(32) DEFAULT '',
  `active` int(11) DEFAULT '1',
  `create_user_id` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `influxdb_metrics`
--

DROP TABLE IF EXISTS `influxdb_metrics`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `influxdb_metrics` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `index` varchar(128) NOT NULL,
  `name` varchar(128) NOT NULL,
  `function` varchar(32) NOT NULL,
  `dsname` varchar(32) NOT NULL,
  `order` varchar(16) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `influxdb_metrics`
--

LOCK TABLES `influxdb_metrics` WRITE;
/*!40000 ALTER TABLE `influxdb_metrics` DISABLE KEYS */;
INSERT INTO `influxdb_metrics` VALUES (1,'cpu-0/cpu-idle','CPU空闲率','derivative','','asc'),(2,'load/load','系统负载(shortterm)','mean','shortterm','asc'),(3,'load/load','系统负载(midterm)','mean','midterm','asc'),(4,'load/load','系统负载(longterm)','mean','longterm','asc'),(5,'memory/memory-free','可用内存','mean','','asc'),(6,'swap/swap-free','可用交换空间','mean','','asc'),(7,'df/df-root','根分区占用率','mean','used','asc'),(8,'disk-ops','磁盘操作频率(写)','derivative','write','asc'),(9,'disk-ops','磁盘操作频率(读)','derivative','read','asc'),(10,'disk-oct','磁盘数据吞>吐量(写)','derivative','write','asc'),(11,'disk-oct','磁盘数据吞吐量(读)','derivative','read','asc'),(12,'disk-time','磁盘操作时间(写)','derivative','write','asc'),(13,'disk-time','磁盘操作时间(读)','derivative','read','asc'),(14,'conntrack/conntrack','网络连接数','mean','','asc'),(15,'fscache-Acquire/fscache_stat-nbf','FSCache拒绝请求数-错误引起','mean','value','asc'),(16,'fscache-Acquire/fscache_stat-noc','FSCache拒绝请求数-无可>用缓存引起','mean','value','asc'),(17,'fscache-Acquire/fscache_stat-ok','FSCache请求成功数','mean','value','asc'),(18,'interface/if_octets-eth0','eth0网口吞吐量(tx)','derivative','tx','asc'),(19,'interface/if_octets-eth0','eth0网
口吞吐量(rx)','derivative','rx','asc'),(20,'interface/if_errors-eth0','eth0网口错误量(tx)','derivative','tx','asc'),(21,'interface/if_errors-eth0','eth0网口错误量(rx)','derivative','rx','asc'),(22,'irq/irq-NMI','非可屏蔽中断','mean','value','asc'),(23,'irq/irq-SPU','伪中断','mean','value','asc'),(24,'irq/irq-CAL','系统调用中断','mean','value','asc'),(25,'irq/irq-ERR','中断错误','mean','value','asc'),(26,'protocols-Tcp/protocol_counter-AttemptFails','TCP连接失败个数','mean','','asc'),(27,'protocols-Tcp/protocol_counter-CurrEstab','TCP当前连接数','mean','','asc'),(28,'protocols-Udp/protocol_counter-InErrors','UDP发包错误','mean','','asc'),(29,'protocols-Udp/protocol_counter-NoPorts','UDP收包数','mean','','asc'),(30,'tcpconns-22-local/tcp_connections-ESTABLISHED','已建立的SSH连接数','mean','','asc'),(31,'tcpconns-22-local/tcp_connections-FIN_WAIT1','FIN_WAIT1状
态的SSH连接数','mean','','asc'),(32,'tcpconns-22-local/tcp_connections-SYN_RECV','SYN_REV状态的SSH连接数','mean','','asc'),(33,'thermal-thermal_zone0/temperature-temperature','主板温度','mean','','asc'),(34,'thermal-thermal_zone1/temperature-temperature','CPU温度','mean','','asc'),(35,'entropy/entropy','系统熵值','mean','entropy','asc'),(38,'nginx/nginx_requests','Nginx处理中的请求数','mean','','asc'),(39,'nginx/nginx_conread','Nginx读连接数','mean','','asc'),(40,'nginx/nginx-conwrite','Nginx写连接数','mean','','asc');
/*!40000 ALTER TABLE `influxdb_metrics` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `log_message`
--

DROP TABLE IF EXISTS `log_message`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `log_message` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `time_stamp` datetime DEFAULT NULL,
  `message` varchar(128) DEFAULT NULL,
  `user_id` int(11) unsigned DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `metrics_translation`
--

DROP TABLE IF EXISTS `metrics_translation`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `metrics_translation` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `plugin` varchar(64) NOT NULL DEFAULT '',
  `type` varchar(64) NOT NULL DEFAULT '',
  `type_instance` varchar(64) NOT NULL DEFAULT '',
  `function` varchar(32) NOT NULL DEFAULT '',
  `orders` varchar(16) NOT NULL DEFAULT '',
  `name` varchar(64) NOT NULL DEFAULT '',
  `metric_index` varchar(128) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=111 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;


--
-- Dumping data for table `metrics_translation`
--

LOCK TABLES `metrics_translation` WRITE;
/*!40000 ALTER TABLE `metrics_translation` DISABLE KEYS */;
INSERT INTO metrics_translation(plugin,type,type_instance,function,orders,name,metric_index) values ('contextswitch','contextswitch','','derivative','asc','上下文切换','contextswitch/contextswitch' ),('cpu','cpu','idle','derivative','asc','CPU空闲率','cpu/cpu-idle' ),('cpu','cpu','interrupt','derivative','asc','CPU中断','cpu/cpu-interrupt' ),('cpu','cpu','nice','derivative','asc','CPU调度优先级','cpu/cpu-nice' ),('cpu','cpu','softirq','derivative','asc','CPU软中断','cpu/cpu-softirq' ),('cpu','cpu','steal','derivative','asc','','cpu/cpu-steal' ),('cpu','cpu','system','derivative','asc','系统CPU','cpu/cpu-system' ),('cpu','cpu','user','derivative','asc','用户CPU','cpu/cpu-user' ),('cpu','cpu','wait','derivative','asc','CPU等待','cpu/cpu-wait' ),('df','df','boot','mean','asc','boot使用率','df/df-boot' ),('df','df','dev-shm','mean','asc','dev-shm使用率','df/df-dev-shm' ),('df','df','home','mean','asc','home使用率','df/df-home' ),('df','df','root','mean','asc','root使用率','df/df-root' ),('df','df','tmp','mean','asc','tmp使用率','df/df-tmp' ),('df','df','var-tmp','mean','asc','var-tmp使用率','df/df-var-tmp' ),('df','df_inodes','free','mean','asc','','df/df_inodes-free' ),('df','df_inodes','reserved','mean','asc','','df/df_inodes-reserved' ),('df','df_inodes','used','mean','asc','','df/df_inodes-used' ),('disk','disk_merged','','derivative','asc','','disk/disk_merged' ),('disk','disk_octets','','derivative','asc','磁盘数据吞吐量','disk/disk_octets' ),('disk','disk_ops','','derivative','asc','磁盘读写频率','disk/disk_ops' ),('disk','disk_time','','derivative','asc','磁盘操作时间','disk/disk_time' ),('dns','dns_octets','','mean','asc','','dns/dns_octets' ),('dns','dns_opcode','Query','mean','asc','','dns/dns_opcode-Query' ),('dns','dns_qtype','A','mean','asc','','dns/dns_qtype-A' ),('dns','dns_qtype','AAAA','mean','asc','','dns/dns_qtype-AAAA' ),('dns','dns_qtype','PTR','mean','asc','','dns/dns_qtype-PTR' ),('dns','dns_rcode','NOERROR','mean','asc','','dns/dns_rcode-NOERROR' ),('dns','dns_rcode','NXDOMAIN','mean','asc','','dns/dns_rcode-NXDOMAIN' ),('interface','if_errors','eth0','derivative','asc','eth0网口错误','interface/if_errors-eth0' ),('interface','if_errors','lo','derivative','asc','lo网口错误','interface/if_errors-lo' ),('interface','if_errors','virbr0','derivative','asc','virbr0网口错误','interface/if_errors-virbr0' ),('interface','if_errors','virbr0-nic','derivative','asc','virbr0-nic网口错误','interface/if_errors-virbr0-nic' ),('interface','if_octets','eth0','derivative','asc','eth0网口吞吐量','interface/if_octets-eth0' ),('interface','if_octets','lo','derivative','asc','lo网口吞吐量','interface/if_octets-lo' ),('interface','if_octets','virbr0','derivative','asc','virbr0网口吞吐量','interface/if_octets-virbr0' ),('interface','if_octets','virbr0-nic','derivativ
e','asc','virbr0-nic网口吞吐量','interface/if_octets-virbr0-nic' ),('interface','if_packets','eth0','derivative','asc','','interface/if_packets-eth0' ),('interface','if_packets','lo','derivative','asc','','interface/if_packets-lo' ),('interface','if_packets','virbr0','derivative','asc','','interface/if_packets-virbr0' ),('interface','if_packets','virbr0-nic','derivative','asc','','interface/if_packets-virbr0-nic' ),('load','load','','mean','asc','系统负载','load/load' ),('memory','memory','buffered','mean','asc','缓冲存储器','memory/memory-buffered' ),('memory','memory','cached','mean','asc','CPU缓存','memory/memory-cached' ),('memory','memory','free','mean','asc','可用内存','memory/memory-free' ),('memory','memory','used','mean','asc','已用内存','memory/memory-used' ),('processes','fork_rate','','mean','asc','','processes/fork_rate' ),('processes','ps_state','blocked','mean','asc','I/O等待的进程数','processes/ps_state-blocked' ),('processes','ps_state','paging','mean','asc','分页中的进程数','processes/ps_state-paging' ),('processes','ps_state','running','mean','asc','运行中的进程数','processes/ps_state-running' ),('processes','ps_state','sleeping','mean','asc','睡眠中的进程数','processes/ps_state-sleeping' ),('processes','ps_state','stopped','mean','asc','已停止的进程数','processes/ps_state-stopped' ),('processes','ps_state','zombies','mean','asc','僵死的进程数','processes/ps_state-zombies' ),('tcpconns','tcp_connections','CLOSED','mean','asc','','tcpconns/tcp_connections-CLOSED' ),('tcpconns','tcp_connections','CLOSE_WAIT','mean','asc','','tcpconns/tcp_connections-CLOSE_WAIT' ),('tcpconns','tcp_connections','CLOSING','mean','asc','','tcpconns/tcp_connections-CLOSING' ),('tcpconns','tcp_connections','ESTABLISHED','mean','asc','','tcpconns/tcp_connections-ESTABLISHED' ),('tcpconns','tcp_connections','FIN_WAIT1','mean','asc','','tcpconns/tcp_connections-FIN_WAIT1' ),('tcpconns','tcp_connections','FIN_WAIT2','mean','asc','','tcpconns/tcp_connections-FIN_WAIT2' ),('tcpconns','tcp_connections','LAST_ACK','mean','asc','','tcpconns/tcp_connections-LAST_ACK' ),('tcpconns','tcp_connections','LISTEN','mean','asc','','tcpconns/tcp_connections-LISTEN' ),('tcpconns','tcp_connections','SYN_RECV','mean','asc','','tcpconns/tcp_connections-SYN_RECV' ),('tcpconns','tcp_connections','SYN_SENT','mean','asc','','tcpconns/tcp_connections-SYN_SENT' ),('tcpconns','tcp_connections','TIME_WAIT','mean','asc','','tcpconns/tcp_connections-TIME_WAIT' ),('uptime','uptime','','mean','asc','','uptime/uptime' ),('vmem','vmpage_action','activate','mean','asc','','vmem/vmpage_action-activate' ),('vmem','vmpage_action','alloc','mean','asc','','vmem/vmpage_action-alloc' ),('vmem','vmpage_action','deactivate','mean','asc','','vmem/vmpage_action-deactivate' ),('vmem','vmpage_action','free','mean','asc','','vmem/vmpage_action-free' ),('vmem','vmpage_action','refill','mean','asc','','vmem/vmpage_action-refill' ),('vmem','vmpage_action','scan_direct','mean','asc','','vmem/vmpage_action-scan_direct' ),('vmem','vmpage_action','scan_kswapd','mean','asc','','vmem/vmpage_action-scan_kswapd' ),('vmem','vmpage_action','steal','mean','asc','','vmem/vmpage_action-steal' ),('vmem','vmpage_faults','','mean','asc','','vmem/vmpage_faults' ),('vmem','vmpage_io','memory','mean','asc','','vmem/vmpage_io-memory' ),('vmem','vmpage_io','swap','mean','asc','','vmem/vmpage_io-swap' ),('vmem','vmpage_number','active_anon','mean','asc','','vmem/vmpage_number-active_anon' ),('vmem','vmpage_number','active_file','mean','asc','','vmem/vmpage_number-active_file' ),('vmem','vmpage_number','anon_pages','mean','asc','','vmem/vmpage_number-anon_pages' ),('vmem','vmpage_number','anon_transparent_hugepages','mean','asc','','vmem/vmpage_number-anon_transparent_hugepages' ),('vmem','vmpage_number','bounce','mean','asc','','vmem/vmpage_number-bounce' ),('vmem','vmpage_number','file_pages','mean','asc','','vmem/vmpage_number-file_pages' ),('vmem','vmpage_number','free_pages','mean','asc','','vmem/vmpage_number-free_pages' ),('vmem','vmpage_number','inactive_anon','mean','asc','','vmem/vmpage_number-inactive_anon' ),('vmem','vmpage_number','inactive_file','mean','asc','','vmem/vmpage_number-inactive_file' ),('vmem','vmpage_number','isolated_anon','mean','asc','','vmem/vmpage_number-isolated_anon' ),('vmem','vmpage_number','isolated_file','mean','asc','','vmem/vmpage_number-isolated_file' ),('vmem','vmpage_number','kernel_stack','mean','asc','','vmem/vmpage_number-kernel_stack' ),('vmem','vmpage_number','mapped','mean','asc','','vmem/vmpage_number-mapped' ),('vmem','vmpage_number','mlock','mean','asc','','vmem/vmpage_number-mlock' ),('vmem','vmpage_number','page_table_pages','mean','asc','','vmem/vmpage_number-page_table_pages' ),('vmem','vmpage_number','shmem','mean','asc','','vmem/vmpage_number-shmem' ),('vmem','vmpage_number','slab_reclaimable','mean','asc','','vmem/vmpage_number-slab_reclaimable' ),('vmem','vmpage_number','slab_unreclaimable','mean','asc','','vmem/vmpage_number-slab_unreclaimable' ),('vmem','vmpage_number','unevictable','mean','asc','','vmem/vmpage_number-unevictable' ),('vmem','vmpage_number','unstable','mean','asc','','vmem/vmpage_number-unstable' ),('vmem','vmpage_number','vmscan_write','mean','asc','','vmem/vmpage_number-vmscan_write' ),('vmem','vmpage_number','writeback','mean','asc','','vmem/vmpage_number-writeback' ),('vmem','vmpage_number','writeback_temp','mean','asc','','vmem/vmpage_number-writeback_temp' ),('swap','swap','free','mean','asc','可用交换空间','swap/swap-free' ),('swap','swap','cached','mean','asc','缓存交换空间','swap/swap-cached' ),('swap','swap','used','mean','asc','已用交换空间','swap/swap-used' ),('swap','swap_io-in','','mean','asc','','swap/swap_io-in' ),('swap','swap_io-out','','mean','asc','','swap/swap_io-out' ),('conntrack','conntrack','','mean',
'asc','网络连接数','conntrack/conntrack' ),('irq','irq','NMI','mean','asc','非可屏蔽中断','irq/irq-NMI' ),('irq','irq','SPU','mean','asc','伪中断','irq/irq-SPU' ),('irq','irq','CAL','mean','asc','系统>
调用中断','irq/irq-CAL' ),('irq','irq','ERR','mean','asc','中断错误','irq/irq-ERR' ),('entropy','entropy','','mean','asc','系统熵值','entropy/entropy' );
/*!40000 ALTER TABLE `metrics_translation` ENABLE KEYS */;
UNLOCK TABLES;


--
-- Table structure for table `notice_event`
--

DROP TABLE IF EXISTS `notice_event`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `notice_event` (
  `event_id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `host_id` varchar(64) NOT NULL,
  `event_type` varchar(64) NOT NULL,
  `create_time` datetime NOT NULL,
  `update_time` datetime NOT NULL,
  `priority` int(11) DEFAULT '1',
  `status` int(1) unsigned NOT NULL DEFAULT '1',
  `process_user` varchar(255) NOT NULL DEFAULT '',
  `remark` text,
  `event_type_cn` varchar(128) DEFAULT NULL,
  PRIMARY KEY (`event_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `notice_operation`
--

DROP TABLE IF EXISTS `notice_operation`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `notice_operation` (
  `operation_id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `event_id` int(11) unsigned NOT NULL,
  `object` varchar(255) NOT NULL,
  `operate_type` varchar(64) NOT NULL DEFAULT '',
  PRIMARY KEY (`operation_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `operation_log`
--

DROP TABLE IF EXISTS `operation_log`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `operation_log` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `user_id` int(11) DEFAULT NULL,
  `type` varchar(32) DEFAULT NULL,
  `target_id` int(11) DEFAULT NULL,
  `desc` text,
  `updated_at` timestamp NULL DEFAULT NULL,
  `created_at` timestamp NULL DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `package_history`
--

DROP TABLE IF EXISTS `package_history`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `package_history` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(512) NOT NULL,
  `repo_id` int(11) unsigned NOT NULL,
  `operation` varchar(32) DEFAULT NULL,
  `operate_time` datetime DEFAULT NULL,
  `operate_by_id` int(11) unsigned DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `fk_package_history_repo_id` (`repo_id`),
  CONSTRAINT `fk_package_history_repo_id` FOREIGN KEY (`repo_id`) REFERENCES `software_repo` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `packages`
--

DROP TABLE IF EXISTS `packages`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `packages` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(512) NOT NULL,
  `soft_name` varchar(128) DEFAULT NULL,
  `soft_version` varchar(128) DEFAULT NULL,
  `soft_release` varchar(128) DEFAULT NULL,
  `soft_vendor` varchar(128) DEFAULT NULL,
  `soft_arch` varchar(128) DEFAULT NULL,
  `repo_id` int(11) unsigned NOT NULL,
  `upload_time` datetime DEFAULT NULL,
  `upload_by_id` int(11) unsigned DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `fk_packages_repo_id` (`repo_id`),
  CONSTRAINT `fk_packages_repo_id` FOREIGN KEY (`repo_id`) REFERENCES `software_repo` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `param`
--

DROP TABLE IF EXISTS `param`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `param` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `key` varchar(128) DEFAULT NULL,
  `content` text,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `post_metrics`
--

DROP TABLE IF EXISTS `post_metrics`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `post_metrics` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `postinfo_id` varchar(64) NOT NULL DEFAULT '',
  `plugin` varchar(64) NOT NULL DEFAULT '',
  `plugin_instance` varchar(64) NOT NULL DEFAULT '',
  `type` varchar(64) NOT NULL DEFAULT '',
  `type_instance` varchar(64) NOT NULL DEFAULT '',
  `dsname` varchar(32) NOT NULL DEFAULT '',
  `metric_index` varchar(128) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `postinfo`
--

DROP TABLE IF EXISTS `postinfo`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `postinfo` (
  `id` varchar(64) NOT NULL,
  `byname` varchar(128) DEFAULT NULL,
  `mac_addr` varchar(20) DEFAULT NULL,
  `ipv4` varchar(20) DEFAULT NULL,
  `time_stamp` datetime DEFAULT NULL,
  `time_stamp_at_server` datetime DEFAULT NULL,
  `reg_time` datetime DEFAULT NULL,
  `connection_init_timestamp` datetime DEFAULT NULL,
  `retry_count` int(11) unsigned DEFAULT NULL,
  `station_num` varchar(16) DEFAULT NULL,
  `station_info` varchar(32) DEFAULT NULL,
  `address` varchar(128) DEFAULT NULL,
  `location` varchar(64) DEFAULT NULL,
  `hw_type` varchar(32) DEFAULT NULL,
  `machine_model` varchar(128) DEFAULT NULL,
  `os_version` varchar(64) DEFAULT NULL,
  `os_type` varchar(64) DEFAULT NULL,
  `heartbeat_interval` int(11) unsigned DEFAULT NULL,
  `cpu_model_name` varchar(48) DEFAULT NULL,
  `kernel` varchar(128) DEFAULT NULL,
  `interface` text,
  `network` text,
  `ps2_device` text,
  `usb_device` text,
  `com` text,
  `graphics` text,
  `libs` text,
  `software` text,
  `ssh_name` varchar(32) DEFAULT 'root',
  `ssh_password` varchar(32) DEFAULT '123456',
  `hardware_template_id` int(11) DEFAULT NULL,
  `process_white_list_id` int(11) DEFAULT NULL,
  `trust_root_id` int(11) DEFAULT NULL,
  `file_feature_id` int(11) DEFAULT NULL,
  `city` varchar(64) DEFAULT NULL,
  `device_code` varchar(64) DEFAULT NULL,
  `guomi_id` int(11) DEFAULT NULL,
  `acl_template_id` int(11) DEFAULT NULL,
  `root_acl_template_id` int(11) DEFAULT NULL,
  `root_fom_template_id` int(11) DEFAULT NULL,
  `fom_template_id` int(11) DEFAULT NULL,
  `root_file_feature_id` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `process_white_list`
--

DROP TABLE IF EXISTS `process_white_list`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `process_white_list` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(32) DEFAULT NULL,
  `content` text,
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL,
  `create_user` varchar(32) DEFAULT '',
  `active` int(11) DEFAULT '1',
  `create_user_id` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `root_acl_template`
--

DROP TABLE IF EXISTS `root_acl_template`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `root_acl_template` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(32) DEFAULT NULL,
  `content` text,
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL,
  `create_user` varchar(32) DEFAULT '',
  `active` int(11) DEFAULT '1',
  `create_user_id` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `root_file_feature`
--

DROP TABLE IF EXISTS `root_file_feature`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `root_file_feature` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(32) DEFAULT NULL,
  `file_name` varchar(128) DEFAULT NULL,
  `active` int(11) DEFAULT '1',
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL,
  `create_user` varchar(32) DEFAULT '',
  `create_user_id` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `root_fom_template`
--

DROP TABLE IF EXISTS `root_fom_template`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `root_fom_template` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(32) DEFAULT NULL,
  `content` text,
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL,
  `create_user` varchar(32) DEFAULT '',
  `active` int(11) DEFAULT '1',
  `create_user_id` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `software_repo`
--

DROP TABLE IF EXISTS `software_repo`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `software_repo` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(128) NOT NULL,
  `remark` text,
  `repo_path` varchar(128) NOT NULL,
  `create_time` datetime DEFAULT NULL,
  `create_by_id` int(11) unsigned DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `name` (`name`,`create_by_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `strategy`
--

DROP TABLE IF EXISTS `strategy`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `strategy` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(255) NOT NULL,
  `priority` int(1) unsigned NOT NULL,
  `pid` int(10) unsigned NOT NULL,
  `alarm_count` int(4) unsigned NOT NULL DEFAULT '0',
  `cycle` int(10) unsigned NOT NULL DEFAULT '5',
  `expression` varchar(255) NOT NULL,
  `description` varchar(255) DEFAULT '',
  `user_id` int(10) unsigned NOT NULL,
  `create_user` char(255) NOT NULL DEFAULT '',
  `create_time` datetime NOT NULL,
  `enable` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `group_changed` int(11) DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_name` (`name`),
  KEY `idx_pid` (`pid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `strategy_event`
--

DROP TABLE IF EXISTS `strategy_event`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `strategy_event` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `strategy_id` bigint(20) unsigned NOT NULL,
  `strategy_name` varchar(255) NOT NULL,
  `priority` int(1) unsigned NOT NULL,
  `cycle` int(10) unsigned NOT NULL,
  `alarm_count` int(4) unsigned NOT NULL,
  `expression` varchar(255) NOT NULL,
  `create_time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `update_time` datetime NOT NULL,
  `count` int(4) unsigned NOT NULL DEFAULT '1',
  `status` int(1) unsigned NOT NULL DEFAULT '1',
  `host_id` char(32) NOT NULL,
  `host_cname` varchar(255) NOT NULL DEFAULT '',
  `host_name` varchar(255) NOT NULL,
  `ip` varchar(16) NOT NULL,
  `sn` varchar(128) NOT NULL DEFAULT '',
  `process_user` char(255) NOT NULL DEFAULT '',
  `process_comments` text,
  `process_time` datetime NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;
ALTER DATABASE `infoDB` CHARACTER SET utf8 COLLATE utf8_general_ci ;
/*!50003 SET @saved_cs_client      = @@character_set_client */ ;
/*!50003 SET @saved_cs_results     = @@character_set_results */ ;
/*!50003 SET @saved_col_connection = @@collation_connection */ ;
/*!50003 SET character_set_client  = utf8 */ ;
/*!50003 SET character_set_results = utf8 */ ;
/*!50003 SET collation_connection  = utf8_general_ci */ ;
/*!50003 SET @saved_sql_mode       = @@sql_mode */ ;
/*!50003 SET sql_mode              = '' */ ;
DELIMITER ;;
/*!50003 CREATE*/ /*!50017 DEFINER=`root`@`localhost`*/ /*!50003 TRIGGER `strategy_event_insert_trigger` BEFORE INSERT ON `strategy_event` FOR EACH ROW BEGIN
	SET NEW.update_time = NOW();
	SET NEW.process_time = NOW();
END */;;
DELIMITER ;
/*!50003 SET sql_mode              = @saved_sql_mode */ ;
/*!50003 SET character_set_client  = @saved_cs_client */ ;
/*!50003 SET character_set_results = @saved_cs_results */ ;
/*!50003 SET collation_connection  = @saved_col_connection */ ;
ALTER DATABASE `infoDB` CHARACTER SET latin1 COLLATE latin1_swedish_ci ;

--
-- Table structure for table `strategy_group`
--

DROP TABLE IF EXISTS `strategy_group`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `strategy_group` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `strategy_id` bigint(20) unsigned NOT NULL,
  `group_id` int(10) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_strategy_id_group_id` (`strategy_id`,`group_id`),
  KEY `fk_strategy_group_group_id` (`group_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `strategy_host`
--

DROP TABLE IF EXISTS `strategy_host`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `strategy_host` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `strategy_id` bigint(20) unsigned NOT NULL,
  `host_id` char(32) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_strategy_id_host_id` (`strategy_id`,`host_id`),
  KEY `fk_strategy_host_host_id` (`host_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `task`
--

DROP TABLE IF EXISTS `task`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `task` (
  `task_id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `update_time` varchar(32) DEFAULT NULL,
  `update_date` date DEFAULT NULL,
  `create_time` datetime DEFAULT NULL,
  `create_by_id` int(11) unsigned DEFAULT NULL,
  `last_edit_time` datetime DEFAULT NULL,
  `last_edit_by_id` int(11) unsigned DEFAULT NULL,
  `status` varchar(32) DEFAULT NULL,
  `name` varchar(256) DEFAULT NULL,
  PRIMARY KEY (`task_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `task_cpe_map`
--

DROP TABLE IF EXISTS `task_cpe_map`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `task_cpe_map` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `cpe_id` varchar(64) NOT NULL,
  `task_id` int(11) unsigned NOT NULL,
  `transaction_id` int(11) DEFAULT NULL,
  `status` varchar(32) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `fk_task_cpe_map_task_id` (`task_id`),
  KEY `fk_task_cpe_map_cpe_id` (`cpe_id`),
  CONSTRAINT `fk_task_cpe_map_cpe_id` FOREIGN KEY (`cpe_id`) REFERENCES `postinfo` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_task_cpe_map_task_id` FOREIGN KEY (`task_id`) REFERENCES `task` (`task_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `task_packages`
--

DROP TABLE IF EXISTS `task_packages`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `task_packages` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `task_info_id` int(11) unsigned NOT NULL,
  `softwarepackageinfo_id` int(11) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_task_info_id_softwarepackageinfo_id` (`task_info_id`,`softwarepackageinfo_id`),
  KEY `fk_task_packages_softwarepackageinfo_id` (`softwarepackageinfo_id`),
  CONSTRAINT `fk_task_packages_softwarepackageinfo_id` FOREIGN KEY (`softwarepackageinfo_id`) REFERENCES `packages` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_task_packages_task_info_id` FOREIGN KEY (`task_info_id`) REFERENCES `task` (`task_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `trigger`
--

DROP TABLE IF EXISTS `trigger`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `trigger` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `strategy_id` bigint(20) unsigned NOT NULL,
  `metric` varchar(128) NOT NULL,
  `dsname` varchar(32) NOT NULL,
  `function` varchar(32) NOT NULL,
  `orders` varchar(16) NOT NULL,
  `tags` varchar(255) NOT NULL DEFAULT '',
  `number` int(10) unsigned NOT NULL DEFAULT '0',
  `index` char(10) NOT NULL,
  `name` varchar(32) NOT NULL,
  `method` varchar(10) NOT NULL,
  `symbol` char(5) NOT NULL,
  `threshold` double(255,2) NOT NULL,
  `description` varchar(255) DEFAULT '',
  `metric_dsname` varchar(256) DEFAULT '',
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_strategy_id_index_index` (`strategy_id`,`index`),
  CONSTRAINT `fk_strategy_strategy_id` FOREIGN KEY (`strategy_id`) REFERENCES `strategy` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `trigger_event`
--

DROP TABLE IF EXISTS `trigger_event`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `trigger_event` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `strategy_event_id` bigint(20) unsigned NOT NULL,
  `index` char(10) NOT NULL,
  `metric` varchar(128) NOT NULL DEFAULT '',
  `tags` varchar(255) DEFAULT NULL,
  `number` int(10) unsigned NOT NULL DEFAULT '0',
  `aggregate_tags` varchar(255) DEFAULT NULL,
  `current_threshold` double(255,2) NOT NULL,
  `method` varchar(10) NOT NULL,
  `symbol` char(5) NOT NULL,
  `threshold` double(255,2) NOT NULL,
  `triggered` tinyint(1) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `fk_trigger_event_strategy_event_id` (`strategy_event_id`),
  CONSTRAINT `fk_trigger_event_strategy_event_id` FOREIGN KEY (`strategy_event_id`) REFERENCES `strategy_event` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `trust_root`
--

DROP TABLE IF EXISTS `trust_root`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `trust_root` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(32) DEFAULT NULL,
  `content` text,
  `active` int(11) DEFAULT '1',
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL,
  `create_user` varchar(32) DEFAULT '',
  `create_user_id` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `vpn`
--

DROP TABLE IF EXISTS `vpn`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `vpn` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(32) DEFAULT NULL,
  `auth_type` varchar(32) DEFAULT 'password',
  `ipsec_policy-template` varchar(256) DEFAULT '',
  `ipsec_profile` varchar(256) DEFAULT '',
  `ipsec_sa` varchar(256) DEFAULT '',
  `ipsec_statistics` varchar(256) DEFAULT '',
  `ipsec_transform-set` varchar(256) DEFAULT '',
  `ipsec_tunnel` varchar(256) DEFAULT '',
  `ikev2_policy` varchar(256) DEFAULT '',
  `ikev2_profile` varchar(256) DEFAULT '',
  `ikev2_proposal` varchar(256) DEFAULT '',
  `ikev2_sa` varchar(256) DEFAULT '',
  `ikev2_statistics` varchar(256) DEFAULT '',
  `active` int(11) DEFAULT '1',
  `created_at` timestamp NULL DEFAULT NULL,
  `updated_at` timestamp NULL DEFAULT NULL,
  `create_user` varchar(32) DEFAULT '',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2018-06-26  3:24:22
