//
// Created by jqt3o on 3/17/2022.
//
#include <Arduino.h>
#include <stdint.h>
#include "math.h"
#include "touch.h"

const int touch_pin_max = 10;
int touch_pin_count = 0;
int _touch_pins [touch_pin_max] = {0};

const int idleMax = 100;
uint16_t idleReadings[touch_pin_max][idleMax] = {0};

const int readingMax = 5;
uint16_t currentReadings[touch_pin_max][readingMax] = {0};

unsigned long lastCalibration = 0;
float channelAverages[touch_pin_max] = {0};
float channelStandardDeviations[touch_pin_max] = {0};

int readingTotal = 0;
int idleTotal = 0;
int readingIndex = 0;

uint16_t hit = 0;

float average (uint16_t values[], int size)
{
    float total = 0;
    for (int i = 0; i < size; ++i)
    {
        total += values[i];
    }
    return total/size;
}

float average(uint16_t values[], int size, int start, int stop)
{
    int count = stop < start ? (stop + size - start) : stop - start;
    float total = 0;
    for (int i = 0; i < count; ++i)
    {
        total += values[(start + i)%size];
    }

    return total/count;
}

float standardDeviation(uint16_t values[], int size)
{
    float avg = average(values, size);

    float total = 0;
    for(int i = 0; i < size; ++i)
    {
        float v = values[i] - avg;
        total += v*v;
    }

    return sqrt(total/(size - 1));
}

//initializes the touch sub system
void touch_init(int touch_pins [], int count)
{
    touch_high_volt_t high;
    touch_low_volt_t low;
    touch_volt_atten_t attn;
    touch_pad_get_voltage(&high, &low, &attn);
//    Serial.printf("voltage high: %d voltage low: %d voltage attn: %d\n", high, low, attn);

    uint16_t sleep_cycle;
    uint16_t meas_cycle;
    touch_pad_get_meas_time(&sleep_cycle, &meas_cycle);
//    Serial.printf("sleep time: %d meas cycle: %d\n", sleep_cycle, meas_cycle);

    touch_cnt_slope_t slope;
    touch_tie_opt_t tie;
    touch_pad_get_cnt_mode(static_cast<touch_pad_t>(0), &slope, &tie);
//    Serial.printf("cnt slope: %d tie opt: %d\n", slope, tie);

//    touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_0V);
    for (int i = 0; i < 9; ++i)
    {
        touch_pad_set_cnt_mode(static_cast<touch_pad_t>(i), TOUCH_PAD_SLOPE_1, TOUCH_PAD_TIE_OPT_LOW);
    }

    //Just a little safety to ensure we don't exceed the max
    touch_pin_count = min(touch_pin_max, count);
    for (int i = 0; i < touch_pin_count; ++i)
    {
        pinMode(touch_pins[i], INPUT);
    }

    memcpy(_touch_pins, touch_pins, touch_pin_count);
}

//each loop (Should be called often) will update the internal filtering
void touch_loop()
{
    //100hz
    if (millis() - lastCalibration < 10)
    {
       return;
    }

    lastCalibration = millis();
    if (idleTotal < idleMax)
    {
        idleTotal += 1;
    }

    if (readingTotal < readingMax)
    {
        readingTotal += 1;
    }

    for (int i = 0; i < touch_pin_count; ++i) {

        //Calculate out touch threshold. This should be 0 if we aren't calibrated yet
        float touchThreshold = channelAverages[i] - 4 * channelStandardDeviations[i];

        uint16_t read = touchRead(_touch_pins[i]);
        //Keep the max, because we want to normalize excessively low values
        currentReadings[i][readingIndex % readingTotal] = max(touchThreshold, (float)read);

        //If we are in the warm up phase, store and continue
        if (idleTotal < idleMax) {
            idleReadings[i][readingIndex % idleTotal] = read;
            continue;
        }

        //If we think we are idle, calibrate
        if (read >= touchThreshold) {
            idleReadings[i][readingIndex % idleTotal] = max(touchThreshold, (float)read);
            channelAverages[i] = average(idleReadings[i], idleTotal);
            channelStandardDeviations[i] = standardDeviation(idleReadings[i], idleTotal);
        }
     }

    readingIndex += 1;
}

//Get the currently determined status;
uint16_t touch_status()
{
    for (int i = 0; i < touch_pin_count; ++i)
    {
        float touchThreshold = channelAverages[i] - 4 * channelStandardDeviations[i];
        float touchAverage = average(currentReadings[i], readingTotal);

        //Check for hit
        if (touchAverage < touchThreshold) {
            //Set the associated bit to true on a hit
            hit |= 0 << i;
        }
    }
    return hit;
}
