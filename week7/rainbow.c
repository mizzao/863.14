/*
 * rainbow.c
 *
 * Created: 10/24/2014 12:25:22 PM
 *  Author: Andrew Mao
 */ 

#define F_CPU 20000000    // AVR clock frequency in Hz, used by util/delay.h
#include <avr/io.h>
#include <util/delay.h>

#define output(directions,pin) (directions |= pin) // set port direction for output
#define set(port,pin) (port |= pin) // set port pin
#define clear(port,pin) (port &= (~pin)) // clear port pin
#define pin_test(pins,pin) (pins & pin) // test for port pin
#define bit_test(byte,bit) (byte & (1 << bit)) // test for bit set
#define led_delay(ms) _delay_ms(ms) // LED delay

#define led_port PORTA
#define led_toggle PINA // Useful for toggling on/off without tracking existing value
#define led_direction DDRA
#define led_red (1 << PA3)
#define led_green (1 << PA2)
#define led_blue (1 << PA7)

// The colors that we will cycle through, with linear interpolation. These are
// somewhat like RGB colors, but not quite, as they scale brightness for PWM. 
// {127, 0, 0} is not a dark red; it's a dim red. The following values were
// obtained through trial-and-error  for (1k, 1k, 499) resistors on (R, G, B).
const int COLORS[][3] = {
	{ 200, 0, 0 }, // red	
	{ 120, 45, 0 }, // orange
	{ 100, 75, 0 }, // yellow	
	{ 0, 150, 0 }, // green	
	{ 0, 0, 255 }, // blue	
	{ 100, 0, 180 } // violet	
};

#define NCOLORS (sizeof(COLORS) / sizeof(COLORS[0]))
#define INTERP_BITS 6
#define INTERP_STEPS (1 << INTERP_BITS)

int brightness[] = { 255, 255, 255 };

// Run a software pwm cycle as quickly as the processor will allow.
// The faster this function runs, the smoother the PWM.
void pwmCycle() {
	// Turn everything on.
	set(led_toggle, led_red | led_green | led_blue);	

	// Turn everything off after the right duty cycle.
	int i;
	for(i = 0; i < 256; i++) {
		if (i == brightness[0]) set(led_toggle, led_red);
		if (i == brightness[1]) set(led_toggle, led_green);
		if (i == brightness[2]) set(led_toggle, led_blue);
	}	
}

// The number of PWM intervals between brightness adjustments.
#define CYCLES 40

void pwm() {
	int j;
	for( j = 0; j < CYCLES; j++ ) {
		pwmCycle();						
	}
}

int main(void) {

	// set clock divider to /1
	CLKPR = (1 << CLKPCE);
	CLKPR = (0 << CLKPS3) | (0 << CLKPS2) | (0 << CLKPS1) | (0 << CLKPS0);
	
	// initialize LED pins - set as output		
	output(led_direction, led_red | led_green | led_blue);	

	// Turn everything off
	// they are driven low, so set to 1
	set(led_port, led_red | led_green | led_blue);	
		
	int color = 0;
	int *oldColor, *newColor;	
	int i, j;

	// main loop - cycle through the colors 	
	while (1) {
		oldColor = COLORS[color];
		color = (color + 1) % NCOLORS;
		newColor = COLORS[color];

		// Transition the current color toward this color, in INTERP_STEPS.		
		for( i = 0; i < INTERP_STEPS; i++ ) {
			// Are these multiply operations going to be too slow? Seems okay.
			int l = INTERP_STEPS - i;			
			for( j = 0; j < 3; j++ ) {
				brightness[j] = (oldColor[j] * l + newColor[j] * i) >> INTERP_BITS;
			}					
			
			// Run some PWM cycles with the current colors.
			pwm();
		}

	}
}