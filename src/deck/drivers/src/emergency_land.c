/*
This code controls the emergency landing option. The ermergency land option is controlled via the parameter named 'deck.eland'.
If the param equals to 1 then the emergency landing code is launched. 
The param is changed whenever the crazyflie receives a high digitalRead from the gpio pin 1 which is connected to the NRF's emergency land 
output pin. Whenever the NRF sends a high signal at DECK_GPIO_IO1 then the code sets the param 'deck.eland' to 1 which causes it to launch the 
emergency landing operation.
*/


#include "deck.h"
#include "FreeRTOS.h"
#include "system.h"
#include "task.h"
#include "pm.h"
#include "debug.h"
#include "param.h"
#include "emergency_land.h"




static uint8_t isEmergencyLandInit = false;

static uint8_t emergencyLandingParam = false;

static paramVarId_t emergencyLandingFlagParamId = {0};



//This function listens to the EMERGENCY_LAND_PIN in the frequency of EMERGENCY_LAND_CHECK_FREQUENCY
//if the pin is HIGH then the function starts the emergency landing operation.
static void listenToEmergencyLandTask(void* ignored) 
{
    systemWaitStart();

    while(!paramGetUint(emergencyLandingFlagParamId)) // waits until 'deck.eland' equals to 1
    {
        vTaskDelay(F2T(EMERGENCY_LAND_CHECK_FREQUENCY));
        if(digitalRead(EMERGENCY_LAND_PIN)) 
        {
            DEBUG_PRINT("ELAND: received emergency landing from NRF!\n");

            paramSetInt(emergencyLandingFlagParamId, true);
        }
    }

    DEBUG_PRINT("ELAND: emergency landing!\n");
    //Todo: add emergency landing code 

    vTaskDelete(NULL); //End current task
}


static void emergencyLandInit(DeckInfo *info)
{
    if(!isEmergencyLandInit)
    {
        DEBUG_PRINT("ELAND: emergency landing is enabled\n");
        pinMode(EMERGENCY_LAND_PIN, INPUT);    

        emergencyLandingFlagParamId = paramGetVarId(ELAND_FLAG_GROUP_NAME, ELAND_FLAG_PARAM_NAME);

        xTaskCreate(listenToEmergencyLandTask, "emergencyLandTask",
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
    .vid = EMERGENCY_LAND_VID,
    .pid = EMERGENCY_LAND_PID,
    .name = "emergency_land",


    .usedGpio = EMERGENCY_LAND_PIN_USING,

    .init = emergencyLandInit,
    .test = emergencyLandTest,
};

DECK_DRIVER(emergencyLand_driver);

PARAM_GROUP_START(deck)

PARAM_ADD(PARAM_UINT8 | PARAM_RONLY, eland, &isEmergencyLandInit)

PARAM_GROUP_STOP(deck)

PARAM_GROUP_START(emergency)

PARAM_ADD(PARAM_UINT8, land, &emergencyLandingParam)

PARAM_GROUP_STOP(emergency)