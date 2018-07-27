package main

import (
	"encoding/json"
	"strings"
	"github.com/Shopify/sarama"
)

type CommonMsg struct {
	Identify string `json:"identify"`
	MsgBody  struct {
		Type    string `json:"type"`
	} `json:"msg_body"`
}

type RegisterMsg struct {
	Identify string `json:"identify"`
	MsgBody  struct {
		Type    string `json:"type"`
		MacAddr string `json:"mac_addr"`
		DeviceCode string `json:"device_code"`
		Ipv4 string `json:"ipv4"`
	} `json:"msg_body"`
}

type RegisterResponseMsg struct {
	Identify string `json:"identify"`
	MsgBody  struct {
		RespCode int `json:"resp_code"`
		RespMsg string `json:"resp_msg"`
		Token string `json:"token"`
		DeviceID string `json:"device_id"`
	} `json:"msg_body"`
}

func handleMessage(msg_str []byte){
	response_msg := RegisterResponseMsg{}

	msg := &CommonMsg{}
	err := json.Unmarshal(msg_str, &msg)
	if err == nil {
		response_msg.Identify = msg.Identify
		if (strings.Compare(msg.MsgBody.Type, "register") == 0) {
			register_msg := &RegisterMsg{}
			err = json.Unmarshal(msg_str, &register_msg)
			if err == nil {
				device_info := &DeviceInfo{}
				for device_info.ID == "" {
					device_info.ID = UniqueId()
				}
				device_info.MacAddr = register_msg.MsgBody.MacAddr
				device_info.DeviceCode = register_msg.MsgBody.DeviceCode
				device_info.Ipv4 = register_msg.MsgBody.Ipv4
				err = mydb.CreateRegisterDevice(device_info)
				if err == nil {
					response_msg.MsgBody.RespCode = ERROR_CODE_OK
					response_msg.MsgBody.DeviceID = device_info.ID
					response_msg.MsgBody.Token = AUTH_ENCODE_KEY
					lg.Debug("device(%s) registered succeddfully", device_info.ID)
				} else {
					lg.Error("failed to save register info to db")
					lg.Error(err.Error())
					response_msg.MsgBody.RespCode = ERROR_CODE_SAVE_REGISTER_ERROR
					response_msg.MsgBody.RespMsg = GetErrorString(response_msg.MsgBody.RespCode)
				}
			} else {
				lg.Error("failed to Unmarshal registe msg:", string(msg_str))
				lg.Error(err.Error())
				response_msg.MsgBody.RespCode = ERROR_CODE_FORMAT_ERROR;
				response_msg.MsgBody.RespMsg = GetErrorString(response_msg.MsgBody.RespCode)
			}
		} else {
			lg.Error("unknown msg type:", string(msg.MsgBody.Type))
			lg.Error(err.Error())
			response_msg.MsgBody.RespCode = ERROR_CODE_UNKNOWN_REQUEST;
			response_msg.MsgBody.RespMsg = GetErrorString(response_msg.MsgBody.RespCode)
		}
	} else {
		lg.Error("registe msg:", string(msg_str))
		lg.Error(err.Error())
		response_msg.MsgBody.RespCode = ERROR_CODE_FORMAT_ERROR;
		response_msg.MsgBody.RespMsg = GetErrorString(response_msg.MsgBody.RespCode)
	}

	sendResponse(response_msg)
}

func sendResponse(msg RegisterResponseMsg) {
	msg_str, err := json.Marshal(msg)
	if err != nil {
		lg.Error("Failed to encode amqp msg to json.")
		return
	}

	config := sarama.NewConfig()
	config.Producer.RequiredAcks = sarama.WaitForAll
	config.Producer.Partitioner = sarama.NewRandomPartitioner
	config.Producer.Return.Successes = true

	msg_to_send := &sarama.ProducerMessage{}
	msg_to_send.Topic = "response_sender_service"
	msg_to_send.Partition = int32(-1)
	msg_to_send.Value = sarama.ByteEncoder(msg_str)

	producer, err := sarama.NewSyncProducer(strings.Split(GlobalConfig.KAFKA_BROKERS, ","), config)
	if err != nil {
		lg.Error("Failed to produce message: %s", err)
		return
	}
	defer producer.Close()

	_, _, err = producer.SendMessage(msg_to_send)
	if err != nil {
		lg.Error("Failed to produce message: ", err)
		return
	}
}
