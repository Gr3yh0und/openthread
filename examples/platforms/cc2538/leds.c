/*
 * leds.c
 *
 *  Created on: 17 May 2017
 *      Author: Michael Morscher, morscher@hm.edu
 */

#include "gpio.h"
#include "leds.h"

void cc2538LedsInit(void)
{
    cc2538GpioSoftwareControl(GPIO_C_NUM, LED0_PIN);
    cc2538GpioSoftwareControl(GPIO_C_NUM, LED1_PIN);
    cc2538GpioSoftwareControl(GPIO_C_NUM, LED2_PIN);
    cc2538GpioSoftwareControl(GPIO_C_NUM, LED3_PIN);

    cc2538GpioDirOutput(GPIO_C_NUM, LED0_PIN);
    cc2538GpioDirOutput(GPIO_C_NUM, LED1_PIN);
    cc2538GpioDirOutput(GPIO_C_NUM, LED2_PIN);
    cc2538GpioDirOutput(GPIO_C_NUM, LED3_PIN);

    cc2538GpioIocOver(GPIO_C_NUM, LED0_PIN, IOC_OVERRIDE_DIS);
    cc2538GpioIocOver(GPIO_C_NUM, LED1_PIN, IOC_OVERRIDE_DIS);
    cc2538GpioIocOver(GPIO_C_NUM, LED2_PIN, IOC_OVERRIDE_DIS);
    cc2538GpioIocOver(GPIO_C_NUM, LED3_PIN, IOC_OVERRIDE_DIS);
}
