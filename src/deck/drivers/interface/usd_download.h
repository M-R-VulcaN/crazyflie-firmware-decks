#ifndef __USD_DOWNLOAD__
#define __USD_DOWNLOAD__


#define EMERGENCY_LAND_CHECK_FREQUENCY 5
#define USD_DOWNLOAD_VID 2
#define USD_DOWNLOAD_PID 3
#define ELAND_FLAG_GROUP_NAME "usd"
#define ELAND_FLAG_PARAM_NAME "download"

#define APP_CHANNLE_MAX_LEN 30 //the max bytes that can be sent by appchannel
#define APP_CHANNLE_MAX_MSG_LEN 25 //the max bytes that can be sent by appchannel data (minus the header)

#define APP_CHANNLE_SIZE_MSG 0 //number representing a size type msg
#define APP_CHANNLE_DATA_MSG 1 //number representing a data type msg
#define APP_CHANNLE_FINISHED_MSG 2 //number representing a data ack request type msg (finished type)
#define APP_CHANNLE_MSG_HEADER_LEN 5 //len of msg header 

#define APP_CHANNLE_R_LEN 5 //length of client response
#define APP_CHANNLE_R_SUCESS 0 //response type "success"
#define APP_CHANNLE_R_WRONG_SENT 1 //response type "wrong type sent"
#define APP_CHANNLE_R_NOT_ALL_RECIVED 2 //response type "not all data properly recived by client"
#define APP_CHANNLE_R_NOT_RUNNING 3 //response type "client isnt currently running"

#endif