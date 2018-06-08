/*
 Source Server Type    : MySQL
 Source Server Version : 50713
 Source Database       : owl

 Target Server Type    : MySQL
 Target Server Version : 50713
 File Encoding         : utf-8

 Date: 11/08/2016 10:46:50 AM
*/

SET NAMES utf8;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
--  Table structure for `action`
-- ----------------------------
DROP TABLE IF EXISTS `action`;
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
) ENGINE=InnoDB AUTO_INCREMENT=452 DEFAULT CHARSET=utf8;

-- ----------------------------
--  Table structure for `action_result`
-- ----------------------------
DROP TABLE IF EXISTS `action_result`;
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

-- ----------------------------
--  Table structure for `action_user`
-- ----------------------------
DROP TABLE IF EXISTS `action_user`;
CREATE TABLE `action_user` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `action_id` int(10) unsigned NOT NULL,
  `user_id` int(10) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_action_id_user_id` (`action_id`,`user_id`),
  KEY `fk_action_user_user_id` (`user_id`),
  CONSTRAINT `fk_action_user_action_id` FOREIGN KEY (`action_id`) REFERENCES `action` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_action_user_user_id` FOREIGN KEY (`user_id`) REFERENCES `auth_user` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) DEFAULT CHARSET=utf8;

-- ----------------------------
--  Table structure for `action_user_group`
-- ----------------------------
DROP TABLE IF EXISTS `action_user_group`;
CREATE TABLE `action_user_group` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `action_id` int(10) unsigned NOT NULL,
  `user_group_id` int(10) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_action_id_user_group_id` (`action_id`,`user_group_id`),
  KEY `fk_action_user_group_user_group_id` (`user_group_id`),
  CONSTRAINT `fk_action_user_group_action_id` FOREIGN KEY (`action_id`) REFERENCES `action` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_action_user_group_user_group_id` FOREIGN KEY (`user_group_id`) REFERENCES `auth_group` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) AUTO_INCREMENT=636 DEFAULT CHARSET=utf8;

-- ----------------------------
--  Table structure for `execute`
-- ----------------------------
DROP TABLE IF EXISTS `execute`;
CREATE TABLE `execute` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `strategy_id` bigint(20) unsigned NOT NULL,
  `type` tinyint(1) unsigned NOT NULL,
  `command` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  KEY `fk_execute_strategy_id` (`strategy_id`),
  CONSTRAINT `fk_execute_strategy_id` FOREIGN KEY (`strategy_id`) REFERENCES `strategy` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=716 DEFAULT CHARSET=utf8;

-- ----------------------------
--  Table structure for `execute_result`
-- ----------------------------
DROP TABLE IF EXISTS `execute_result`;
CREATE TABLE `execute_result` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `strategy_event_id` bigint(20) unsigned NOT NULL,
  `execute_id` int(10) unsigned NOT NULL,
  `execute_type` tinyint(1) unsigned NOT NULL,
  `command` varchar(255) NOT NULL DEFAULT '',
  `host_id` char(32) NOT NULL,
  `result`  varchar(255) NOT NULL DEFAULT 'unexecuted',
  PRIMARY KEY (`id`),
  KEY `fk_execute_result_strategy_event_id` (`strategy_event_id`),
  KEY `fk_execute_result_execute_id` (`execute_id`),
  KEY `fk_execute_result_host_id` (`host_id`),
  CONSTRAINT `fk_execute_result_strategy_event_id` FOREIGN KEY (`strategy_event_id`) REFERENCES `strategy_event` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_execute_result_execute_id` FOREIGN KEY (`execute_id`) REFERENCES `execute` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_execute_result_host_id` FOREIGN KEY (`host_id`) REFERENCES `postinfo` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) AUTO_INCREMENT=40000 DEFAULT CHARSET=utf8;

-- ----------------------------
--  Table structure for `strategy`
-- ----------------------------
DROP TABLE IF EXISTS `strategy`;
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
  `create_time` DATETIME NOT NULL,
  `enable` tinyint(1) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_name` (`name`),
  KEY `idx_pid` (`pid`)
) ENGINE=InnoDB AUTO_INCREMENT=193 DEFAULT CHARSET=utf8;

-- ----------------------------
--  Table structure for `strategy_event`
-- ----------------------------
DROP TABLE IF EXISTS `strategy_event`;
CREATE TABLE `strategy_event` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `strategy_id` bigint(20) unsigned NOT NULL,
  `strategy_name` varchar(255) NOT NULL,
  `priority` int(1) unsigned NOT NULL,
  `cycle` int(10) unsigned NOT NULL,
  `alarm_count` int(4) unsigned NOT NULL,
  `expression` varchar(255) NOT NULL,
  `create_time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `update_time` DATETIME NOT NULL,
  `count` int(4) unsigned NOT NULL DEFAULT '1',
  `status` int(1) unsigned NOT NULL DEFAULT '1',
  `host_id` char(32) NOT NULL,
  `host_cname` varchar(255) NOT NULL DEFAULT '',
  `host_name` varchar(255) NOT NULL,
  `ip` varchar(16) NOT NULL,
  `sn` varchar(128) NOT NULL DEFAULT '',
  `process_user` char(255) NOT NULL DEFAULT '',
  `process_comments` text,
  `process_time` DATETIME NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=5513 DEFAULT CHARSET=utf8;

DROP TRIGGER IF EXISTS `strategy_event_insert_trigger`;
delimiter //
CREATE TRIGGER `strategy_event_insert_trigger` BEFORE INSERT ON `strategy_event` FOR EACH ROW BEGIN
	SET NEW.update_time = NOW();
	SET NEW.process_time = NOW();
END //

-- ----------------------------
--  Table structure for `strategy_group`
-- ----------------------------
DROP TABLE IF EXISTS `strategy_group`;
CREATE TABLE `strategy_group` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `strategy_id` bigint(20) unsigned NOT NULL,
  `group_id` int(10) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_strategy_id_group_id` (`strategy_id`,`group_id`),
  KEY `fk_strategy_group_group_id` (`group_id`),
  CONSTRAINT `fk_strategy_group_group_id` FOREIGN KEY (`group_id`) REFERENCES `device_group` (`group_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_strategy_group_strategy_id` FOREIGN KEY (`strategy_id`) REFERENCES `strategy` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) AUTO_INCREMENT=298 DEFAULT CHARSET=utf8;

-- ----------------------------
--  Table structure for `strategy_host`
-- ----------------------------
DROP TABLE IF EXISTS `strategy_host`;
CREATE TABLE `strategy_host` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `strategy_id` bigint(20) unsigned NOT NULL,
  `host_id` char(32) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_strategy_id_host_id` (`strategy_id`,`host_id`),
  KEY `fk_strategy_host_host_id` (`host_id`),
  CONSTRAINT `fk_strategy_host_host_id` FOREIGN KEY (`host_id`) REFERENCES `postinfo` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_strategy_host_strategy_id` FOREIGN KEY (`strategy_id`) REFERENCES `strategy` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) AUTO_INCREMENT=440 DEFAULT CHARSET=utf8;

-- ----------------------------
--  Table structure for `trigger`
-- ----------------------------
DROP TABLE IF EXISTS `trigger`;
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
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_strategy_id_index_index` (`strategy_id`,`index`),
  CONSTRAINT `fk_strategy_strategy_id` FOREIGN KEY (`strategy_id`) REFERENCES `strategy` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=259 DEFAULT CHARSET=utf8;

-- ----------------------------
--  Table structure for `trigger_event`
-- ----------------------------
DROP TABLE IF EXISTS `trigger_event`;
CREATE TABLE `trigger_event` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `strategy_event_id` bigint(20) unsigned NOT NULL,
  `index` char(10) NOT NULL,
  `metric` varchar(128) NOT NULL,
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

SET FOREIGN_KEY_CHECKS = 1;
