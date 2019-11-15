/*
 * "Hello World" example.
 *
 * This example prints 'Hello from Nios II' to the STDOUT stream. It runs on
 * the Nios II 'standard', 'full_featured', 'fast', and 'low_cost' example
 * designs. It runs with or without the MicroC/OS-II RTOS and requires a STDOUT
 * device in your system's hardware.
 * The memory footprint of this hosted application is ~69 kbytes by default
 * using the standard reference design.
 *
 * For a reduced footprint version of this template, and an explanation of how
 * to reduce the memory footprint for a given application, see the
 * "small_hello_world" template.
 *
 */

#include <stdio.h>
#include "system.h"
#include "altera_avalon_pio_regs.h"
#include "sys/alt_irq.h"


int background();
void tight_polling();
void interrupt();
static void my_ISR (void* context, alt_u32 id);

int main()
{
	// Print mode 0 for interrupts, 1 for tight polling
	int mode = IORD(SWITCH_PIO_BASE, 0) & 0x01;
	if(mode == 0){
			printf("Interrupt mode selected\n");
		} else {
			printf("Tight polling mode selected\n");
	}
	while(1) {
		int curr = IORD(SWITCH_PIO_BASE, 0) & 0x01;
		if(curr != mode){
			int i;
			mode = curr;
			// Debouncing
			for(i = 0; i < 10000; i++);
			if(curr == 0){
				printf("Interrupt mode selected\n");
			} else {
				printf("Tight polling mode selected\n");
			}
		}
		// Change mode, 0 for interrupts, 1 for tight polling
		if((IORD(BUTTON_PIO_BASE, 0) & 0x1) == 0){
			mode ? tight_polling() : interrupt();
		}
	}

	return 0;
}

void interrupt() {
	// Telling the HAL (hardware extension layer) exception handling system which ISR to execute for a given hardware interrupt
	alt_irq_register( STIMULUS_IN_IRQ, (void *)0, my_ISR);
	// setting up interrupt mode
	IOWR(STIMULUS_IN_BASE, 2, 1);
	int j;
	for (j = 2; j <= 2500; j += 2) {
		int background_counter = 0;
		// disables the egm
		IOWR(EGM_BASE,0,0);
		// setting period
		IOWR(EGM_BASE,2,j);
		// setting pulsewidth
		IOWR(EGM_BASE, 3, j/2);
		// enables the egm
		IOWR(EGM_BASE,0,1);
		// Setting response to 0 initially
		IOWR(RESPONSE_OUT_BASE,0,0);
		// Turning LEDs off initially
		IOWR(LED_PIO_BASE,0,0x00);
		// Note- During an interrupt (rising edge of stimulus in), leave background
		// and handle interrupt
		while(IORD(EGM_BASE, 1)) {
			background();
			background_counter++;
		}
		int avg_latency = IORD(EGM_BASE, 4);
		int missed_pulses = IORD(EGM_BASE, 5);
		int mul_pulses = IORD(EGM_BASE, 6);
		printf("%d,%d,%d,%d,%d,%d\n", j, j/2, background_counter, avg_latency, missed_pulses, mul_pulses);
	}
}

static void my_ISR (void* context, alt_u32 id) {
	//ISR code to be executed during an interrupt
	IOWR(RESPONSE_OUT_BASE,0,1);
	IOWR(RESPONSE_OUT_BASE,0,0);
	// Clearing the interrupt pending bit
	IOWR(STIMULUS_IN_BASE,3,0x0);
}

void tight_polling() {
	// turning off interrupt mode
	IOWR(STIMULUS_IN_BASE, 2, 0);
	int j;
	for (j = 2; j <= 2500; j += 2) {
		int background_counter = 0;
		IOWR(EGM_BASE,0,0);
		// setting period
		IOWR(EGM_BASE,2,j);
		// setting pulse width
		IOWR(EGM_BASE, 3, j/2);
		IOWR(EGM_BASE,0,1);
		// Setting response to 0 initially
		IOWR(RESPONSE_OUT_BASE,0,0);
		// Turning LEDs off initially
		IOWR(LED_PIO_BASE,0,0x00);
		int stimulus = 0;
		int falling_edge = 1;
		int rising_edge = 0;
		int first = 1, is_rising = 0;
		int counter = 0;
		while(IORD(EGM_BASE, 1)){
				stimulus = IORD(STIMULUS_IN_BASE,0);
				if (rising_edge && stimulus == 0) {
					falling_edge = 1;
					rising_edge = 0;
				}
				// rising edge
				if (falling_edge && (stimulus == 1)) {
					IOWR(RESPONSE_OUT_BASE,0,1);
					IOWR(RESPONSE_OUT_BASE,0,0);
					rising_edge = 1;
					falling_edge = 0;
					first --;
					is_rising = 1;
				}
				// falling edge
				if (first == 0) {
					if (rising_edge && stimulus == 0) {
						falling_edge = 1;
						rising_edge = 0;
						counter++;
						background();
						background_counter++;
					}
					else {
						counter++;
						background();
						background_counter++;
					}
				}
				// consequent rising edges
				else if (rising_edge && is_rising){
					int i;
					is_rising = 0;
					for(i = 0; i < counter-1; i++){
						background();
						background_counter++;
					}
				}
			}
		int avg_latency = IORD(EGM_BASE, 4);
		int missed_pulses = IORD(EGM_BASE, 5);
		int mul_pulses = IORD(EGM_BASE, 6);
		printf("%d,%d,%d,%d,%d,%d\n", j, j/2, background_counter, avg_latency, missed_pulses, mul_pulses);
	}
}

int background()
{

	int j;
	int x = 0;
	int grainsize = 4;
	int g_taskProcessed = 0;
	int enabled = IORD(EGM_BASE, 1);
	if (enabled) {

		IOWR(LED_PIO_BASE,0,0x01);
		for(j = 0; j < grainsize; j++)
		{
		g_taskProcessed++;
		}
		IOWR(LED_PIO_BASE,0,0x00);

	}
	return x;
}
