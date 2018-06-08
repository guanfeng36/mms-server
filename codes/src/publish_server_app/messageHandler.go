package main

import (
	"encoding/json"
	"fmt"
)

type MsgData struct {
	Identify string `json:"identify"`
	MsgBody  struct {
		Type string `json:"type"`
		Topic string   `json:"topic"`
		Data interface{}  `json:"data"`
	} `json:"msg_body"`
} 

func handleMessage(msg_str []byte){
	fmt.Println(string(msg_str))
	msg := &MsgData{}
	err := json.Unmarshal(msg_str, &msg)

	if err != nil {
		lg.Error("cannot Unmarshal msg:" , msg_str)
		lg.Error(err.Error())
		return
	}

	if msg.MsgBody.Topic == "" {
		lg.Error("cannot find topic parameter")
		return
	}

	var pub_msg []byte
	pub_msg, err = json.Marshal(msg.MsgBody.Data)
	if err != nil {
		lg.Error("cannot Marshal data:" , msg.MsgBody.Data)
		lg.Error(err.Error())
		return
	}
	pub_server.PublishMessage(msg.MsgBody.Topic, string(pub_msg))
}
