#include "../include/joystick.h"
#include "../include/adc.h"
#include <stdint.h>

uint8_t joystickX_read()
{
    return adc_read(0);
}
uint8_t joystickY_read()
{
    return adc_read(1);
}


uint8_t joystickMinX = 76;
uint8_t joystickMaxX = 255;
uint8_t joystickMinY = 63;
uint8_t joystickMaxY = 255;
uint8_t median = 170;

int16_t joystick_to_percent(uint8_t input)
{
    if (input)
    if (input >= median) 
    {
        return ((int16_t)(input - median) * 100) /
               (joystickMaxX - median);
    } 
    else 
    {
        return -((int16_t)(median - input) * 100) /
                (median - joystickMinX);
    }
}

    // typical value for joystick is 2.5V, our ADC reports around 172 though, so well go from there
    //return (input - joystickMinX)/( joystickMaxX - joystickMinX)

#include "../include/utils.h"
void joystick_calibration()
{
    // for a certain time we just need to log the values and take the min max and average or something and decide what to set the values to
    
    uint8_t delay = 100;
    int8_t time = 10000;
    uint8_t loop_count = 100;
    uint8_t xPos[loop_count];
    uint8_t yPos[loop_count];
    printf("set count to %u", loop_count);

    for (int i = 0; i < loop_count; i++)
    {
        xPos[i] = joystickX_read();
        yPos[i] = joystickY_read();
        printf("bruh2");
        _delay_ms(100);
    }
    median = find_median_uint8(xPos, loop_count);
    printf("set median to %u", median);
};




void printJoystickXY()
{
    
}