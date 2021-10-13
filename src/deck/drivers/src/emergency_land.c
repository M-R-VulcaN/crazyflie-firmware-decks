#include "deck.h"
#include "FreeRTOS.h"
#include "task.h"
#include "pm.h"
#include "debug.h"
#include "param.h"

#define TASK_SIZE configMINIMAL_STACK_SIZE
#define EMERGENCY_LAND_CHECK_FREQUENCY 10

#define TASK_PRIORITY 3
#define EMERGENCY_LAND_PIN DECK_GPIO_IO1

static bool isEmergencyLandInit = false;
static bool emergencyLandingFlag = false;

static void listenToEmergencyLand(void* data) 
{
    systemWaitStart();

    while(!emergencyLandingFlag)
    {
        if(digitalRead(DECK_GPIO_IO1))
        {
            DEBUG_PRINT("ELAND: emergency landing!\n");
            //Todo: add emergency landing code 
            emergencyLandingFlag = true;
        }
        vTaskDelay(F2T(EMERGENCY_LAND_CHECK_FREQUENCY));   
    }

    vTaskDelete(NULL); //End current task
}


static void emergencyLandInit(DeckInfo *info)
{
    if(!isEmergencyLandInit)
    {
        DEBUG_PRINT("ELAND: emergency landing is enabled\n");
        pinMode(EMERGENCY_LAND_PIN, INPUT);    

        xTaskCreate(listenToEmergencyLand, "emergencyLandTask",
                TASK_SIZE, NULL, TASK_PRIORITY, NULL);

        isEmergencyLandInit = true;
    }
}

static bool emergencyLandTest()
{
    bool successFlag = true;

    if (!isEmergencyLandInit) 
    {
        DEBUG_PRINT("ELAND: Error while initializing the emergency landing deck\n");
        successFlag = false;
    }

    return successFlag;
}

const DeckDriver emergencyLand_driver = {
    .vid = 1,
    .pid = 1,
    .name = "emergency_land",


    .usedGpio = DECK_USING_IO_1,

    .init = emergencyLandInit,
    .test = emergencyLandTest,
};

DECK_DRIVER(emergencyLand_driver);

PARAM_GROUP_START(deck)

PARAM_ADD(PARAM_UINT8, eland, &emergencyLandingFlag)
PARAM_GROUP_STOP(deck)
