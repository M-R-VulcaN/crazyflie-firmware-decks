/*

*/


#include "deck.h"
#include "FreeRTOS.h"
#include "system.h"
#include "task.h"
#include "pm.h"
#include "debug.h"
#include "param.h"
#include "log.h"
#include "oxygen_sensor.h"

#define OXYGEN_SENSOR_READ_AMOUNT 200


static uint8_t isOxygenSensorInit = false;

static float oxygenConcentrationPercent = 0;

static double getAvrgOxygenSensorRead()
{
    long readSum = 0;
    int i = 0;
    for(i=0;i<OXYGEN_SENSOR_READ_AMOUNT;i++)
    {
        readSum += analogRead(OXYGEN_SENSOR_PIN);
        vTaskDelay(M2T(1));

    }
    return readSum/OXYGEN_SENSOR_READ_AMOUNT;
}

static double oxygenSensorReadToVout(double oxygenSensorRead)
{
    return oxygenSensorRead * (STM32_VREF_VOLTS / STM32_ANALOG_BIT_BASE);
}

static double voutToOxygenConcentrationPercentage(double vout) 
{ 
    return vout * OXYGEN_SENSOR_CONSTANT * 100.0 + OXYGEN_SENSOR_CORRECTION_CONSTANT;
}

static double getOxygenSensorConcentrationPercentage()
{
    double averageOxygenSensorRead = getAvrgOxygenSensorRead();
    double vout = oxygenSensorReadToVout(averageOxygenSensorRead);
    DEBUG_PRINT("OXYSENS: Vout is %f\n",vout);
    return voutToOxygenConcentrationPercentage(vout);
}

static void displayOxygenSensorTask(void* ignored) 
{
    systemWaitStart();
    while(true) 
    {
        vTaskDelay(F2T(OXYGEN_SENSOR_CHECK_FREQUENCY));
        oxygenConcentrationPercent = (float)getOxygenSensorConcentrationPercentage();
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


LOG_GROUP_START(sensors)

LOG_ADD(LOG_FLOAT, oxygen, &oxygenConcentrationPercent)

LOG_GROUP_STOP(sensors)