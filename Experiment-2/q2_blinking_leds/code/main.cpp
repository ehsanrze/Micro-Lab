// define cpu frequency to 10Mhz
#define F_CPU 1000000UL
#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
    DDRB = 0xFF;
    DDRA = 0x00;

    while (1)
    {
        if (PINA == 0x01)
        {
            PORTB = 0xFF;
            _delay_ms(500);
            PORTB = 0x00;
            _delay_ms(500);
        }
    }
}