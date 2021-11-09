/*

*/


#include "deck.h"
#include "FreeRTOS.h"
#include "system.h"
#include "task.h"
#include "pm.h"
#include "debug.h"
#include "param.h"
#include "oxygen_sensor.h"




static uint8_t isOxygenSensorInit = false;

static void displayOxygenSensorTask(void* ignored) 
{
    systemWaitStart();
    int oxygenLevel = 0;
    while(true) 
    {
        vTaskDelay(F2T(OXYGEN_SENSOR_CHECK_FREQUENCY));
        oxygenLevel = analogRead(OXYGEN_SENSOR_PIN);
        {
            DEBUG_PRINT("OXYSENS: oxygen level is %d\n", oxygenLevel);
        }
    }
}

static void oxygenSensorInit(DeckInfo *info)
{
    if(!isOxygenSensorInit)
    {
        DEBUG_PRINT("OXYSENS: oxygen sensor is enabled\n");
        pinMode(OXYGEN_SENSOR_PIN, INPUT);    

        xTaskCreate(displayOxygenSensorTask, "oxygenSensorTask",
                TASK_SIZE, NULL, TASK_PRIORITY, NULL);

        isOxygenSensorInit = true;
    }
}

static bool oxygenSensorTest()
{
    bool successFlag = true;
    
    if (!isOxygenSensorInit) 
    {
        DEBUG_PRINT("OXYSENS: Error while initializing the emergency landing deck\n");
        successFlag = false;
    }

    return successFlag;
}

const DeckDriver oxygenSensor_driver = {
    .vid = OXYGEN_SENSOR_VID,
    .pid = OXYGEN_SENSOR_PID,
    .name = "oxygen_sensor",

    .usedGpio = OXYGEN_SENSOR_PIN_USING,

    .init = oxygenSensorInit,
    .test = oxygenSensorTest,
};

DECK_DRIVER(oxygenSensor_driver);

PARAM_GROUP_START(deck)

PARAM_ADD(PARAM_UINT8 | PARAM_RONLY, oxysens, &isOxygenSensorInit)

PARAM_GROUP_STOP(deck)