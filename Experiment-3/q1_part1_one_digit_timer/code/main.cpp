#define F_CPU = 8000000
#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint8_t overflows;
volatile uint8_t seconds = 0;

// this function map decimal to 7seg display
int bcd_7seg(int digit)
{
    char array[] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90};
    return array[digit];
}

// initialize timer1
void initial_timer_1()
{
    // prescaler = 8
    TCCR1B |= _BV(CS11);

    // counter set to 0
    TCNT1 = 0;

    // enable overflow interrupt
    TIMSK |= _BV(TOIE1);

    sei();

    overflows = 0;
}


ISR(TIMER1_OVF_vect)
{
    overflows++;
}

int main(void)
{
    DDRC = 0xFF; // connect 7seg to PORTC
    DDRD = 0xFF;

    PORTD = _BV(PORTD7); //turn on 7seg
    PORTC = bcd_7seg(0);

    initial_timer_1();

    while (1)
    {
        if (overflows >= 15)
        {
            if (TCNT1 >= 16960)
            {
                (seconds > 8) ? seconds = 0 : seconds++;
                PORTC = bcd_7seg(seconds);

                // reset counter & overflow
                TCNT1 = 0;
                overflows = 0;
            }
        }
    }
}