package main

import (
    "time"
	zmq "github.com/pebbe/zmq4"
)

type PubServer struct {
	Sock *zmq.Socket
}

var pub_server *PubServer

func sendHB() error {
    t := time.NewTicker(30 * time.Second)
    for {
        select {
            case <-t.C:
            pub_server.PublishMessage("__hb__", "")
         }
     }
}

func InitPublishServer() error {
	pub_server = &PubServer{}
	pub_server.Sock,_ = zmq.NewSocket(zmq.PUB)
	err := pub_server.Sock.Bind(GlobalConfig.PUBLISH_ADDR)

    go sendHB()
	return err
}

func (this *PubServer)PublishMessage(topic string, msg string) {
	_,err := this.Sock.SendMessage(topic, msg)
	if err != nil {
		lg.Error("failed to publish msg:", topic, msg)
		lg.Error(err.Error())
	} else {
		lg.Debug("succefully publish msg[%s] to topic[%s]", msg, topic)
	}
}
