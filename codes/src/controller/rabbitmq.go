package main

import (
	//"fmt"
	"github.com/streadway/amqp"
)

const (
	AMQP_MSG_TYPE_ALARM_EXECUTE = "distribute_cmd_task"
)

func AmqpSend(url string, exchange string, queue string, body []byte) error {
	connection, err := amqp.Dial(url)
	if err != nil {
		lg.Error(err.Error())
		return err
	}
	defer connection.Close()

	channel, err := connection.Channel()
	if err != nil {
		lg.Error(err.Error())
		return err
	}
	defer channel.Close()

	q, err := channel.QueueDeclare(
		queue,
		true,
		false,
		false,
		false,
		nil,
	)

	if err != nil {
		lg.Error(err.Error())
		return err
	}

	err = channel.Publish(
		exchange,
		q.Name,
		false,
		false,
		amqp.Publishing{
			Headers:         amqp.Table{},
			ContentType:     "text/plain",
			ContentEncoding: "",
			Body:            body,
		})
	if err != nil {
		lg.Error(err.Error())
		return err
	}

	return nil
}
