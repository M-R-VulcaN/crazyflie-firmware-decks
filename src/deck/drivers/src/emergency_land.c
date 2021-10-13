#include "emergency_land.h"
#include "deck.h"
#include "FreeRTOS.h"
#include "task.h"
#include "pm.h"
#include "debug.h"
#define TASK_SIZE configMINIMAL_STACK_SIZE
#define TASK_DELAY 100
#define TASK_PRIORITY 3

static bool isEmergencyLandInit = false;

static void listenToEmergencyLand(void* data) 
{
    systemWaitStart();
    int readRes = 0;
    while(1)
    {
        readRes = digitalRead(DECK_GPIO_IO1);
        if(readRes)
        {
            DEBUG_PRINT("emergency landing!");
        }
        vTaskDelay(TASK_DELAY);   
    }
}

static void emergencyLandInit(DeckInfo *info)
{
    if(!isEmergencyLandInit)
    {
        DEBUG_PRINT("emergency landing is enabled\n");
        pinMode(DECK_GPIO_IO1, INPUT);     
        xTaskCreate(listenToEmergencyLand, "emergencyLandTask",
                TASK_SIZE, NULL, TASK_PRIORITY, NULL);
    }
}

static bool emergencyLandTest()
{
    return true;
}

const DeckDriver emergencyLand_driver = {
    .vid = 0,
    .pid = 0,
    .name = "emergency_land",


    .usedGpio = DECK_USING_IO_1,

    .init = emergencyLandInit,
    .test = emergencyLandTest,
};

DECK_DRIVER(emergencyLand_driver);
