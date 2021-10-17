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
        if (PINA == 0x00)
        {
            for (int i = 0; i <= 7; i++)
            {
                if(PINA == 0x01){
                    break;
                }
                PORTB = _BV(i);
                _delay_ms(500);
            }
        }
    }
}