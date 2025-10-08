// button_interrupt.c
// Josh Brake
// jbrake@hmc.edu
// 10/31/22

#include "../src/main.h"

int volatile debug_interrupt_trigd = 0; // global debug var

// Function used by printf to send characters to the laptop
int _write(int file, char *ptr, int len) {
  int i = 0;
  for (i = 0; i < len; i++) {
    ITM_SendChar((*ptr++));
  }
  return len;
}


int main(void) {
    // Enable LED as output
    gpioEnable(GPIO_PORT_B);
    pinMode(LED_PIN, GPIO_OUTPUT);

    // Enable button as input
    gpioEnable(GPIO_PORT_A);
    pinMode(BUTTON_PIN, GPIO_INPUT);
    GPIOA->PUPDR |= _VAL2FLD(GPIO_PUPDR_PUPD4, 0b01); // Set PA4 as pull-up

    // Initialize timer
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
    initTIM(DELAY_TIM);

    /////////// Enable SYSCFG clock domain in RCC ///////////
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    // Configure EXTICR2 for the input button interrupt
    SYSCFG->EXTICR[1] |= _VAL2FLD(SYSCFG_EXTICR2_EXTI4, 0b000); // Select PA4 (bits EXTI4[3:0])

    /////////// Configure interrupt for falling edge of GPIO pin for button ///////////
    // Configure mask bit
    EXTI->IMR1 |= (1 << gpioPinOffset(BUTTON_PIN)); // Configure the mask bit
    // Disable rising edge trigger
    EXTI->RTSR1 &= ~(1 << gpioPinOffset(BUTTON_PIN));// Disable rising edge trigger
    // Enable falling edge trigger
    EXTI->FTSR1 |= (1 << gpioPinOffset(BUTTON_PIN));// Enable falling edge trigger
    // Turn on EXTI interrupt in NVIC_ISER
    NVIC->ISER[0] |= (1 << EXTI4_IRQn); // EXTI4 for PA4 // EXTI4_IRQn = 10

    // Enable interrupts globally
    __enable_irq();

    printf("DEBUG: Interrupts configured.\n");

    while(1){   
        printf("Loop.\n");
        delay_millis(TIM2, 200);
    }

}

// Interrupt request handler
void EXTI4_IRQHandler(void){
    printf("DEBUG: EXTI4 interrupt triggered.\n");
    // Check that the button was what triggered our interrupt
    if (EXTI->PR1 & (1 << gpioPinOffset(BUTTON_PIN))){
        printf("DEBUG: Entered EXTI4 if statement.\n");
        // If so, clear the interrupt (NB: Write 1 to reset.)
        EXTI->PR1 |= (1 << gpioPinOffset(BUTTON_PIN));

        // Then toggle the LED
        togglePin(LED_PIN);
        printf("DEBUG: LED toggled.\n");

    }
}
