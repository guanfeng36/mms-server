/*
author: feng.guan
*/

package main

import (
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"log"
	"os/signal"
	cluster "github.com/bsm/sarama-cluster"
)

func init() {
	os.Chdir(filepath.Dir(os.Args[0]))
	os.Mkdir("logs", 0755)
	runtime.GOMAXPROCS(runtime.NumCPU())
}

func startConsumer() {
	config := cluster.NewConfig()
	config.Consumer.Return.Errors = true
	config.Group.Return.Notifications = true

	brokers := []string{GlobalConfig.KAFKA_BROKERS}
	topics := []string{"task_publish_service"}
	group := string("publish_srv_group")
	
	consumer, err := cluster.NewConsumer(brokers, group, topics, config)
	if err != nil {
        panic(err)
	}
	defer consumer.Close()

	signals := make(chan os.Signal, 1)
	signal.Notify(signals, os.Interrupt)

	go func() {
        for err := range consumer.Errors() {
            log.Printf("Error: %s\n", err.Error())
        }
	}()

	go func() {
        for ntf := range consumer.Notifications() {
            log.Printf("Rebalanced: %+v\n", ntf)
        }
	}()

	for {
		select {
		case msg, ok := <-consumer.Messages():
			if ok {
				handleMessage(msg.Value)
				consumer.MarkOffset(msg, "") // mark message as processed
			}
		case <-signals:
			return
		}
    }
}

func main() {
	if err := InitGlobalConfig(); err != nil {
        fmt.Fprintf(os.Stderr, "failed to init global config:", err)
		return
	}
	if err := InitLog(); err != nil {
        fmt.Fprintf(os.Stderr, "failed to init log:", err)
        return
	}

	if err := InitPublishServer(); err != nil {
        fmt.Fprintf(os.Stderr, "failed to init publish server:", err)
        return
	}

	startConsumer()
}
