#include "gpio.h"

/* Initialises GPIO ports on the platform */
void cc2538LedsInit(void)
{
    GPIO_CONTROL_SW(GPIO_C_NUM, LED0_PIN);
    GPIO_CONTROL_SW(GPIO_C_NUM, LED1_PIN);
    GPIO_CONTROL_SW(GPIO_C_NUM, LED2_PIN);
    GPIO_CONTROL_SW(GPIO_C_NUM, LED3_PIN);

    GPIO_SET_OUTPUT(GPIO_C_NUM, LED0_PIN);
    GPIO_SET_OUTPUT(GPIO_C_NUM, LED1_PIN);
    GPIO_SET_OUTPUT(GPIO_C_NUM, LED2_PIN);
    GPIO_SET_OUTPUT(GPIO_C_NUM, LED3_PIN);
}
