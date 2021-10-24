#define F_CPU = 8000000
#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint8_t overflows;
volatile uint8_t overflow_timer_0;
volatile uint8_t seconds[4];
volatile uint8_t state = 0;

// convert digit to 7seg pins
char array[] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90};

void reset_timer()
{
    seconds[0] = 0;
    seconds[1] = 0;
    seconds[2] = 0;
    seconds[3] = 0;
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

    overflows = 0;
}

// initialize timer0
void initial_timer_0()
{
    // prescaler = 8
    TCCR0 |= _BV(CS11);

    // counter set to 0
    TCNT0 = 0;

    // enable overflow interrupt
    TIMSK |= _BV(TOIE0);

    overflow_timer_0 = 0;
}

ISR(TIMER1_OVF_vect)
{
    overflows++;
}

ISR(TIMER0_OVF_vect)
{
    overflow_timer_0++;
}

int main(void)
{
    DDRC = 0xFF; // connect 7seg to PORTC
    DDRD = 0xFF;

    PORTD = _BV(PORTD0); //turn on 7seg

    initial_timer_1();
    initial_timer_0();
    sei();

    reset_timer();

    while (1)
    {
        // 1s timer for increase timer
        if (overflows >= 15)
        {
            if (TCNT1 >= 16960)
            {
                // first digit overflow check
                if (seconds[0] > 8)
                {
                    seconds[0] = 0;
                    seconds[1]++;
                }
                else
                {
                    seconds[0]++;
                }

                // second digit overflow check
                if (seconds[1] > 8)
                {
                    seconds[1] = 0;
                    seconds[2]++;
                }

                // third digit overflow check
                if (seconds[2] > 8)
                {
                    seconds[2] = 0;
                    seconds[3]++;
                }

                // fourth digit overflow check
                if (seconds[3] > 8)
                {
                    reset_timer();
                }

                // reset counter & overflow
                TCNT1 = 0;
                overflows = 0;
            }
        }


        // 0.02 timer for switch between 7segs
        if (overflow_timer_0 >= 78)
        {
            if (TCNT0 >= 32)
            {
                PORTC = array[seconds[state]];
                PORTD = _BV(state);
                (state > 2) ? state = 0 : state++;

                TCNT0 = 0;
                overflow_timer_0 = 0;
            }
        }
    }
}