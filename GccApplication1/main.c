/**
 * Copyright (c) 2019, Evgeny Bykov <evg-bikov@yandex.ru>
 * ATtiny13A
 * Antikrot tone generator.
 * --
 * Settings:
 *  F_CPU=1200000
 */

#define F_CPU 1200000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <time.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

#define LED_ON PORTB |= (1 << PB3);
#define LED_OFF PORTB &= ~(1 << PB3);
#define PWM_Connect TCCR0A |= (1<<COM0A0); // connect PWM pin to Channel A of Timer0
#define PWM_Disconnect TCCR0A &= ~(1<<COM0A0); // disconnect PWM pin to Channel A of Timer0
#define PWM_Pin_OFF PORTB &= ~(1 << PB0);	// switch off PWM pin

uint8_t counter=0;	

typedef struct s_note {
	uint8_t OCRxn; // 0..255
	uint8_t N;
} note_t;

void tone(uint32_t ret)
{
	note_t *val;
	val = (note_t *)&ret;
	TCCR0B = (TCCR0B & ~((1<<CS02)|(1<<CS01)|(1<<CS00))) | val->N;
	OCR0A = val->OCRxn - 1; // set the OCRnx
}

void stop(void)
{
	TCCR0B &= ~((1<<CS02)|(1<<CS01)|(1<<CS00)); // stop the timer
}

void sleep()
{
	//watchdog init
	wdt_reset(); // reset watchdog timer
	wdt_enable(WDTO_4S); // enable watchdog 4 seconds
	WDTCR |= _BV(WDTIE); // enable watchdog interrupt
	sei(); // enable interrupt
	set_sleep_mode(SLEEP_MODE_PWR_DOWN); // set power down level
	
	while(1) 
	{
		sleep_enable(); // sleep enable
		sleep_cpu(); // go to sleep
	}
}

ISR (WDT_vect) 
{
	uint8_t i;
	WDTCR |= _BV(WDTIE); // enable watchdog interrupt
	LED_ON;
	_delay_ms(20);
	counter++;
	LED_OFF;

	if (counter>225) // 15 minutes
	{
		LED_ON;
		wdt_disable();
		PWM_Connect;
		
		for (i=1;i<4;i++)
		{
				tone((rand()%800)+300);
				_delay_ms(2000);
				stop();
		}
		PWM_Disconnect;
		PWM_Pin_OFF;
		LED_OFF;
		counter=0;
		wdt_enable(WDTO_4S);
		sei();
		WDTCR |= _BV(WDTIE);
		sleep_enable(); // sleep enable
		sleep_cpu(); // go to sleep
	}
}

int main(void)
{
	uint16_t i;

	DDRB = 0b00001001; // set PWM and LED pin as OUTPUT
	PORTB = 0b00000000; // set all pins to LOW
	TCCR0A |= (1<<WGM01); // set timer mode to Fast PWM
	PWM_Connect;

	stop();
	_delay_ms(1500);

	LED_ON;
	PWM_Connect;
	for (i=300;i<1100;i++)
	{
		tone(i);
		_delay_ms(10);
	}
	stop();
	PWM_Disconnect;
	PWM_Pin_OFF;
	LED_OFF;

	sleep(); // go to sleep, save planet :)
}