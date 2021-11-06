#include <avr/io.h>
#include <avr/delay.h>
#include <avr/interrupt.h>

#define F_CPU = 8000000

// signals of stepper motor
int steps[] = {0x09, 0x0A, 0x06, 0x05};
int step_index = 0;
volatile uint32_t timer0_overflows = 0;

void initial_timer0()
{
    // prescaler = 256
    TCCR0 |= _BV(CS02);

    TCNT0 = 0;

    TIMSK |= _BV(TOIE0);

    timer0_overflows = 0;
}

ISR(TIMER0_OVF_vect)
{
    timer0_overflows++;
}

int main()
{
    DDRA = 0xFF;
    DDRC = 0x00;

    while (PINC == _BV(PINC2))
    {
    }
    
    initial_timer0();
    sei();

    while (1)
    {
        // timer with 0.5s
        if (timer0_overflows >= 61)
        {
            if (TCNT0 >= 9)
            {
                PORTA = steps[step_index];
                (step_index >= 3) ? step_index = 0 : step_index++;
                TCNT0 = 0;
                timer0_overflows = 0;
            }
        }
    }
    return 0;
}