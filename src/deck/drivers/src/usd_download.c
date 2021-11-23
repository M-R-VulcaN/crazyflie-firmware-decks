
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

#define TASK_SIZE configMINIMAL_STACK_SIZE*3

#define TASK_PRIORITY 3

static uint8_t isUsdDownloadInit = false;

static uint8_t usdDownloadParam = false;

static paramVarId_t usdDownloadFlagParamId = {0};

static uint32_t logSize = 0;

static void listenToEmergencyLandTask(void* ignored) 
{
    systemWaitStart();

    while(true) // waits until 'deck.eland' equals to 1
    {
        vTaskDelay(F2T(EMERGENCY_LAND_CHECK_FREQUENCY));
        if(paramGetUint(usdDownloadFlagParamId))
        {
            DEBUG_PRINT("USDLOAD: downloading...\n");
            logSize = usddeckFileSize();
            if(0==logSize)
            {
                DEBUG_PRINT("USDLOAD: [fail] logging isn't stopped\n");
                continue;
            }
            


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
    // else 
    // {
    //     paramVarId_t isLoggingPossibleParam = paramGetVarId("usd", "canLog");
    //     if(!paramGetUint(isLoggingPossibleParam))
    //     {
    //         DEBUG_PRINT("USDLOAD: [fail] cannot log\n");
    //         successFlag = false;
    //     }
    // }

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