//
// Created by jqt3o on 3/17/2022.
//
#include "touch.h"

int touch_pin_count = 0;

const int idleMax = 100;
uint16_t idleReadings[TOUCH_PIN_COUNT][idleMax] = {0};

const int readingMax = 5;
uint16_t currentReadings[TOUCH_PIN_COUNT][readingMax] = {0};

unsigned long lastCalibration = 0;
float channelAverages[TOUCH_PIN_COUNT] = {0};
float channelStandardDeviations[TOUCH_PIN_COUNT] = {0};

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
void touch_init(int [] touch_pins, int count)
{
    touch_high_volt_t high;
    touch_low_volt_t low;
    touch_volt_atten_t attn;
    touch_pad_get_voltage(&high, &low, &attn);
    uint16_t sleep_cycle;
    uint16_t meas_cycle;
//    Serial.printf("voltage high: %d voltage low: %d voltage attn: %d\n", high, low, attn);

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

    touch_pin_count = count;
    for (int i = 0; i < touch_pin_count; ++i)
    {
        pinMode(touch_pins[i], INPUT);
    }
}

//each loop (Should be called often) will update the internal filtering
void touch_loop()
{
    if (idleTotal < idleMax)
    {
        idleTotal += 1;
    }

    if (readingTotal < readingMax)
    {
        readingTotal += 1;
    }
    for (int i = 0; i < TOUCH_PIN_COUNT; ++i) {
        uint16_t read = touchRead(TOUCH_PINS[i]);
        currentReadings[i][readingIndex % readingTotal] = read;

        float touchAverage = average(currentReadings[i], readingTotal);
        float touchThreshold = channelAverages[i] - 4*channelStandardDeviations[i];

        //Check for hit
        //averages and std default to 0, so this will never be true if we aren't calibrated
        if (touchAverage < touchThreshold)
        {
            //Set the associated bit to true on a hit
            hit |= 1 << i;
        }

        if (idleTotal < idleMax) {
            touchReadings[i][readingIndex%idleTotal] = read;
        }

        //Only calibrates once per second, and only if we have a full reading array, and if we aren't currently being tapped
        if (read >= touchThreshold && idleTotal == idleMax)
        {
            idleReadings[i][readingIndex%idleTotal] = read;
            lastCalibration = millis();
//            for (int i = 0; i < TOUCH_PIN_COUNT; ++i)
//            {
            channelAverages[i] = average(idleReadings[i], idleTotal);
            channelStandardDeviations[i] = standardDeviation(idleReadings[i], idleTotal);
//            }
        }
    }
    readingIndex += 1;
}

//Get the currently determined status;
uint16_t touch_status()
{
    return hit;
}
