package main

const (
	ERROR_CODE_OK = iota
	ERROR_CODE_REQ_EMPTY
	ERROR_CODE_FORMAT_ERROR
	ERROR_CODE_DATA_TOO_LARGE
	ERROR_CODE_UNAUTHORIZED_REQUEST
	ERROR_CODE_UNKNOWN_REQUEST

	ERROR_CODE_SAVE_REGISTER_ERROR
	ERROR_CODE_SAVE_INFO_ERROR
	ERROR_CODE_SAVE_HEARTBEAT_ERROR
	ERROR_CODE_TASK_DISTRIBUTE_ERROR
	ERROR_CODE_SAVE_UPDATE_TASK_STATUS_ERROR
	ERROR_CODE_SAVE_CMD_TASK_RESULT_ERROR
)


var errnoMap = map[int]string{
	ERROR_CODE_OK: "",
	ERROR_CODE_REQ_EMPTY: "request empty",
	ERROR_CODE_FORMAT_ERROR: "format error",
	ERROR_CODE_DATA_TOO_LARGE: "data too large",
	ERROR_CODE_UNAUTHORIZED_REQUEST: "unauthorized request",
	ERROR_CODE_UNKNOWN_REQUEST: "unknown request type",

	ERROR_CODE_SAVE_REGISTER_ERROR: "register data error",
	ERROR_CODE_SAVE_INFO_ERROR: "inform data error",
	ERROR_CODE_SAVE_HEARTBEAT_ERROR: "heartbeat data error",
	ERROR_CODE_TASK_DISTRIBUTE_ERROR: "distribute task error",
	ERROR_CODE_SAVE_UPDATE_TASK_STATUS_ERROR: "save update task status error",
	ERROR_CODE_SAVE_CMD_TASK_RESULT_ERROR: "save cmd task result error",
}

func GetErrorString(code int) string{
	msg,ok := errnoMap[code]
	if ok {
		return msg
	} else {
		return ""
	}
}
