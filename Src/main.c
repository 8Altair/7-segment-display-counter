#include <stdint.h>

/// --- Base addresses ---
#define PERIPHERAL_BASE       	0x40000000
#define AHB1_PERIPHERAL_BASE    (PERIPHERAL_BASE + 0x20000)
#define RCC_BASE         		(AHB1_PERIPHERAL_BASE + 0x3800)
#define GPIOA_BASE       		(AHB1_PERIPHERAL_BASE + 0x0000)
#define GPIOD_BASE       		(AHB1_PERIPHERAL_BASE + 0x0C00)

// --- Registers ---
#define RCC_AHB1ENR   (*(volatile uint32_t *)(RCC_BASE + 0x30))  // AHB1 enable (GPIO clocks)
#define GPIOE_MODER   (*(volatile uint32_t *)(GPIOA_BASE + 0x00))  // Mode
#define GPIOE_IDR     (*(volatile uint32_t *)(GPIOA_BASE + 0x10)) // Input data
#define GPIOD_MODER   (*(volatile uint32_t *)(GPIOD_BASE + 0x00))  // Mode
#define GPIOD_BSRR    (*(volatile uint32_t *)(GPIOD_BASE + 0x18))  // Bit set/reset (atomic)
#define GPIOD_ODR     (*(volatile uint32_t *)(GPIOD_BASE + 0x14))	// Output data

// --- LED on PD13 ---
#define LED_PIN       13
#define LED_SET       (1 << LED_PIN)	// BS
#define LED_RESET     (1 << (LED_PIN + 16))	// BR

#define BUTTON_PIN       0	// PA0 (blue button)
#define BUTTON_MASK      (1 << BUTTON_PIN)

// --- Crude delay loops (tune as needed) ---
// Start values assuming ~16 MHz HSI and no compiler optimizations.
#define LOOP_1S   ((16 * 1000000 / 10) + (-1000000))   // ~1 second (modify last number)
#define LOOP_DELAY (LOOP_1S/10)  // ~100 ms on-time

// --- 7-segment PD0–PD6 = a–g ---
#define SEGMENT_MASK  (0x7F)  // Bits 0–6

// Digit to segment lookup table (common cathode)
const uint8_t digit_to_segments[10] =
{
    /* 0 */ 0b00111111, // a b c d e f
    /* 1 */ 0b00000110, // b c
    /* 2 */ 0b01011011, // a b d e g
    /* 3 */ 0b01001111, // a b c d g
    /* 4 */ 0b01100110, // b c f g
    /* 5 */ 0b01101101, // a c d f g
    /* 6 */ 0b01111101, // a c d e f g
    /* 7 */ 0b00000111, // a b c
    /* 8 */ 0b01111111, // All
    /* 9 */ 0b01101111  // a b c d f g
};

int main(void)
{
	// Enable GPIOA + GPIOD clocks (RCC_AHB1ENR: bits 0=GPIOAEN, 3=GPIODEN)
    RCC_AHB1ENR |= (1 << 0) | (1 << 3);

    // PD13 as general-purpose output (MODER13 = 01b)
    GPIOD_MODER &= ~(3 << (LED_PIN * 2));
    GPIOD_MODER |=  (1 << (LED_PIN * 2));

    /* Make PA0 an input (button) */
    GPIOE_MODER &= ~(3 << (0 * 2));

    // Configure PD0–PD6 (segments a–g) as outputs
    for (uint32_t i = 0; i <= 6; i++)
    {
    	GPIOD_MODER &= ~(3 << (i * 2));
        GPIOD_MODER |=  (1 << (i * 2));
    }

	uint8_t button_previous = 0;
	uint8_t digit = 0;

	while (1)
	{
		// Show digit on 7-segment
		GPIOD_ODR = (GPIOD_ODR & ~SEGMENT_MASK) | (~digit_to_segments[digit] & SEGMENT_MASK);

		// Delay ~1s + check for button during that period
		for (volatile uint32_t i = 0; i < LOOP_1S; i++)
		{
			uint8_t button_now = (GPIOE_IDR & BUTTON_MASK) ? 1 : 0;

			if (button_now && !button_previous)
			{
				for (volatile uint32_t d = 0; d < LOOP_DELAY; d++)
				{ /* tiny loop for ~20ms */ }
				if (GPIOE_IDR & BUTTON_MASK)
				{
					//digit = 0;	 // Reset to zero
					digit = (digit + 1) % 10;	// Increment
				    GPIOD_ODR = (GPIOD_ODR & ~SEGMENT_MASK) | (~digit_to_segments[digit] & SEGMENT_MASK);
				}
			}

			button_previous = button_now;
		}
		// Advance digit 0→9
		digit++;
		if (digit > 9) digit = 0;
	}
}
