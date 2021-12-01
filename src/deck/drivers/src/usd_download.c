#include "deck.h"
#include "FreeRTOS.h"
#include "system.h"
#include "task.h"
#include "pm.h"
#include "debug.h"
#include "app_channel.h"
#include "param.h"
#include "usd_download.h"
#include "usddeck.h"
#include "usec_time.h"

#define TASK_SIZE configMINIMAL_STACK_SIZE*4

#define TASK_PRIORITY 3

static uint8_t isUsdDownloadInit = false;

static uint8_t usdDownloadParam = false;

static paramVarId_t usdDownloadFlagParamId = {0};

static void usdDownload()
{
    uint8_t buff[APP_CHANNLE_MAX_LEN] = {0}; //create buffer for messages
    uint8_t r_data[APP_CHANNLE_R_LEN] = {0}; //create buffer for responses

    buff[0] = APP_CHANNLE_SIZE_MSG;
    uint32_t datasize = usddeckFileSize();
    memcpy(&(buff[1]), &datasize, 4); //make size message
    
    do
    {
    appchannelSendPacket(buff, APP_CHANNLE_MSG_HEADER_LEN);
    } while (!appchannelReceivePacket(r_data, APP_CHANNLE_R_LEN, 100) && usdDownloadParam); 
        //send and wait for response, if none given, send again after 100ms
        //incase respose was 0 (success) continiue 
        //incase 1 (wrong type) continiue anyway. the reason is that if not size reqyest isnt the one needed, 
        //    then the one needed is either a data msg or finished msg, both are further on and require continiuing. 
        //    can occur if first response from client was missed and the second request was sent when the client expected a data response next
        //incase 2 (not all data recived) not possble
        //incase 3 (client not running) it will exit and stop
    
    uint32_t lastAddr = 0;
    uint32_t addr = 0;
    for( addr = 0; addr < datasize && r_data[0] != APP_CHANNLE_R_NOT_RUNNING && usdDownloadParam;)
    {   //read next data in loop until all data was properly sent, client stopped running or sendAppChannle has been set to 0

    memset(buff, 0, sizeof(buff)); //reset buff
    memcpy(&buff[1], &addr, 4);
    buff[0] = APP_CHANNLE_DATA_MSG; //set data msg header

    if (addr + APP_CHANNLE_MAX_MSG_LEN <= datasize     &&  usddeckRead(addr, &buff[5], APP_CHANNLE_MAX_MSG_LEN)) 
    {   //incase remaining is bigger then len on data in msg, set as much as possble data and send
        appchannelSendPacket(buff, APP_CHANNLE_MAX_LEN);
    }
    else if(addr + APP_CHANNLE_MAX_MSG_LEN > datasize  &&  usddeckRead(addr, &buff[5], datasize - addr))
    {   //incase remaining is smaller then len on data in msg, set as much the remaining data and send
        appchannelSendPacket(buff, datasize - addr + APP_CHANNLE_MSG_HEADER_LEN);
    }

    addr += APP_CHANNLE_MAX_MSG_LEN; // increase addr

    if(addr >= datasize || addr-lastAddr >= 2000) //if reached end or sent 2000 bytes or more, check if data was recived properly and continiue
    {
        memset(buff, 0, sizeof(buff)); //reset buffer
        buff[0] = APP_CHANNLE_FINISHED_MSG;
        lastAddr = (addr <= datasize) ? addr : datasize;
        memcpy(&(buff[1]), &lastAddr, 4); //make finished msg
        
        do
        {
        appchannelSendPacket(buff, APP_CHANNLE_MSG_HEADER_LEN); 
        } while (!appchannelReceivePacket(r_data, APP_CHANNLE_R_LEN, 100) && usdDownloadParam); //send and wait for response, if none given, send again after 100ms

        if(r_data[0] == APP_CHANNLE_R_NOT_ALL_RECIVED || r_data[0] == APP_CHANNLE_R_SUCESS) //if not all data was recived properly by client
        {
        memcpy(&lastAddr, &(r_data[1]), 4); // get the addr where he stopped
        addr = lastAddr; //set it to be the next one sent in the next 3000 byteas
        }
    }
    }


}

static void listenToEmergencyLandTask(void* ignored) 
{
    systemWaitStart();
    paramVarId_t loggingParam = paramGetVarId("usd", "logging");
    while(true) // waits until 'deck.eland' equals to 1
    {
        vTaskDelay(F2T(EMERGENCY_LAND_CHECK_FREQUENCY));
        if(usdDownloadParam && !paramGetUint(loggingParam))
        {
            DEBUG_PRINT("USDLOAD: downloading...\n");
            
            usdDownload();

            usdDownloadParam = false; //turn off sendAppChannle
        }
    }
    vTaskDelete(NULL);
}


static void usdDownloadInit(DeckInfo *info)
{
    if(!isUsdDownloadInit)
    {
        DEBUG_PRINT("USDLOAD: usd download is enabled\n");

        usdDownloadFlagParamId = paramGetVarId(ELAND_FLAG_GROUP_NAME, ELAND_FLAG_PARAM_NAME);

        xTaskCreate(listenToEmergencyLandTask, "emergencyLandTask",
                TASK_SIZE, NULL, TASK_PRIORITY, NULL);

        isUsdDownloadInit = true;
    }
}

static bool usdDownloadTest()
{
    bool successFlag = true;
    paramVarId_t isUsdDeckInit = paramGetVarId("deck", "bcUSD");
    
    if (!isUsdDownloadInit) 
    {
        DEBUG_PRINT("USDLOAD: [fail] Error while initializing the usd download deck\n");
        successFlag = false;
    }
    if(!paramGetUint(isUsdDeckInit))
    {
        DEBUG_PRINT("USDLOAD: [fail] usddeck is not initialized\n");
        successFlag = false;
    }

    return successFlag;
}

const DeckDriver usdDownload_driver = {
    .vid = USD_DOWNLOAD_VID,
    .pid = USD_DOWNLOAD_PID,
    .name = "usd_download",

    .init = usdDownloadInit,
    .test = usdDownloadTest,
};

DECK_DRIVER(usdDownload_driver);

PARAM_GROUP_START(deck)

PARAM_ADD(PARAM_UINT8 | PARAM_RONLY, usdload, &isUsdDownloadInit)

PARAM_GROUP_STOP(deck)

PARAM_GROUP_START(usd)

PARAM_ADD(PARAM_UINT8, download, &usdDownloadParam)

PARAM_GROUP_STOP(usd)