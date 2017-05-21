#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>


#ifdef __cplusplus
 extern "C" {
#endif

enum {
  GPIO_A_NUM = 0,
  GPIO_B_NUM = 1,
  GPIO_C_NUM = 2,
  GPIO_D_NUM = 3,
};

#define HWREG(x)                                (*((volatile uint32_t *)(x)))

// GPIO base addresses
#define GPIO_A_DEV              0x400d9000
#define GPIO_B_DEV              0x400da000
#define GPIO_C_DEV              0x400db000
#define GPIO_D_DEV              0x400dc000

// Input/Output Selection
#define IOC_PXX_SEL             0x400d4000
#define IOC_PXX_OVER            0x400d4080
#define IOC_PA0_SEL             0x400D4000  // Peripheral select control
#define IOC_PA1_SEL             0x400D4004  // Peripheral select control
#define IOC_PA0_OVER            0x400D4080
#define IOC_PA1_OVER            0x400D4084
#define IOC_OVERRIDE_OE         0x00000008  // PAD Config Override Output Enable
#define IOC_OVERRIDE_DIS        0x00000000  // PAD Config Override Disabled

// Peripheral select values
#define IOC_SEL_UART0_TXD       (0)
#define IOC_SEL_UART1_RTS       (1)
#define IOC_SEL_UART1_TXD       (2)
#define IOC_SEL_SSI0_TXD        (3)
#define IOC_SEL_SSI0_CLKOUT     (4)
#define IOC_SEL_SSI0_FSSOUT     (5)
#define IOC_SEL_SSI0_STXSER_EN  (6)
#define IOC_SEL_SSI1_TXD        (7)
#define IOC_SEL_SSI1_CLKOUT     (8)
#define IOC_SEL_SSI1_FSSOUT     (9)
#define IOC_SEL_SSI1_STXSER_EN  (10)
#define IOC_SEL_I2C_CMSSDA      (11)
#define IOC_SEL_I2C_CMSSCL      (12)
#define IOC_SEL_GPT0_ICP1       (13)
#define IOC_SEL_GPT0_ICP2       (14)
#define IOC_SEL_GPT1_ICP1       (15)
#define IOC_SEL_GPT1_ICP2       (16)
#define IOC_SEL_GPT2_ICP1       (17)
#define IOC_SEL_GPT2_ICP2       (18)
#define IOC_SEL_GPT3_ICP1       (19)
#define IOC_SEL_GPT3_ICP2       (20)

// GPIO offsets
#define GPIO_DATA               0x00000000
#define GPIO_DIR                0x00000400
#define GPIO_IS                 0x00000404
#define GPIO_IBE                0x00000408
#define GPIO_IEV                0x0000040C
#define GPIO_IE                 0x00000410
#define GPIO_RIS                0x00000414
#define GPIO_MIS                0x00000418
#define GPIO_IC                 0x0000041C
#define GPIO_AFSEL              0x00000420
#define GPIO_GPIOLOCK           0x00000520
#define GPIO_GPIOCR             0x00000524
#define GPIO_PMUX               0x00000700
#define GPIO_P_EDGE_CTRL        0x00000704
#define GPIO_USB_CTRL           0x00000708
#define GPIO_PI_IEN             0x00000710
#define GPIO_IRQ_DETECT_ACK     0x00000718
#define GPIO_USB_IRQ_ACK        0x0000071C
#define GPIO_IRQ_DETECT_UNMASK  0x00000720


// GPIO Macros
#define GPIO_PIN_MASK(n)                 ( 1 << (n) )
#define GPIO_PORT_TO_DEV(port)           ( GPIO_A_DEV + ((port) << 12) )
#define GPIO_CONTROL_HW(port, pin)       ( HWREG(GPIO_PORT_TO_DEV(port) + GPIO_AFSEL) |=  GPIO_PIN_MASK(pin) )
#define GPIO_CONTROL_SW(port, pin)       ( HWREG(GPIO_PORT_TO_DEV(port) + GPIO_AFSEL) &= ~GPIO_PIN_MASK(pin) )
#define GPIO_SET_OUTPUT(port, pin)       ( HWREG(GPIO_PORT_TO_DEV(port) + GPIO_DIR) |=    GPIO_PIN_MASK(pin) )
#define GPIO_SET_INPUT(port, pin)        ( HWREG(GPIO_PORT_TO_DEV(port) + GPIO_DIR) &=   ~GPIO_PIN_MASK(pin) )
#define GPIO_READ_PIN(port, pin)         ( HWREG(GPIO_PORT_TO_DEV(port) + GPIO_DATA +    (GPIO_PIN_MASK(pin) << 2)) )
#define GPIO_SET_PIN(port, pin)          ( HWREG(GPIO_PORT_TO_DEV(port) + GPIO_DATA +    (GPIO_PIN_MASK(pin) << 2)) = 0xFF )
#define GPIO_CLEAR_PIN(port, pin)        ( HWREG(GPIO_PORT_TO_DEV(port) + GPIO_DATA +    (GPIO_PIN_MASK(pin) << 2)) = 0x00 )

// LED PIN definitions
#define LED0_PIN        0
#define LED1_PIN        1
#define LED2_PIN        2
#define LED3_PIN        3

// LED Macros
#define LED0_ON         GPIO_SET_PIN(GPIO_C_NUM, LED0_PIN)
#define LED0_OFF        GPIO_CLEAR_PIN(GPIO_C_NUM, LED0_PIN)
#define LED1_ON         GPIO_SET_PIN(GPIO_C_NUM, LED1_PIN)
#define LED1_OFF        GPIO_CLEAR_PIN(GPIO_C_NUM, LED1_PIN)
#define LED2_ON         GPIO_SET_PIN(GPIO_C_NUM, LED2_PIN)
#define LED2_OFF        GPIO_CLEAR_PIN(GPIO_C_NUM, LED2_PIN)
#define LED3_ON         GPIO_SET_PIN(GPIO_C_NUM, LED3_PIN)
#define LED3_OFF        GPIO_CLEAR_PIN(GPIO_C_NUM, LED3_PIN)
#define LED_ALL_OFF     LED0_OFF;   \
                        LED1_OFF;   \
                        LED2_OFF;   \
                        LED3_OFF
#define LED_ALL_ON      LED0_ON;    \
                        LED1_ON;    \
                        LED2_ON;    \
                        LED3_ON

// LED functions
void cc2538LedsInit(void);

#ifdef __cplusplus
} // end extern "C"
#endif
#endif // GPIO_H_
