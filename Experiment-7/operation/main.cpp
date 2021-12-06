#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdlib.h>

#define F_CPU 8000000

#define BAUD 9600
#define BAUDRATE ((F_CPU) / (BAUD * 16UL) - 1)

int adc_result;
int steps[] = {0x09, 0x0A, 0x06, 0x05};
int step_index = 0;
volatile uint32_t timer0_overflows = 0;
bool motor_status = false;
int motor_cycle = 0;

void adc_init()
{
    ADCSRA = _BV(ADEN);
}

int adc_read(int channel)
{
    channel &= _BV(MUX2) | _BV(MUX1) | _BV(MUX0);
    ADMUX = (ADMUX & 0xF8) | channel;

    ADCSRA |= _BV(ADSC);

    while (ADCSRA & _BV(ADSC))
        ;

    return ADC;
}

void initial_timer0()
{
    TCCR0 |= _BV(CS02);

    TCNT0 = 0;

    TIMSK |= _BV(TOIE0);

    timer0_overflows = 0;
}

ISR(TIMER0_OVF_vect)
{
    timer0_overflows++;
}

void uart_init(void)
{
    UBRRH = (BAUDRATE >> 8);
    UBRRL = BAUDRATE;
    UCSRB |= (1 << TXEN) | _BV(RXEN) | (1 << RXCIE);
    UCSRC |= (1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1); 
}

void uart_transmit(unsigned int data)
{
    while (!(UCSRA & (1 << UDRE)));
    UDR = data;
}

ISR(USART_RXC_vect)
{
    motor_cycle = UDR * 4;
    motor_status = true;
}

int main()
{
    DDRD = 0xFF;
    DDRB = 0xFF;
    DDRA = 0x00;

    uart_init();
    initial_timer0();
    sei();
    adc_init();

    while (1)
    {
        if (adc_result != adc_read(0))
        {
            adc_result = adc_read(0);
            uart_transmit(adc_result);
        }

        if (timer0_overflows >= 61 and motor_status == true)
        {
            if (TCNT0 >= 9)
            {
                PORTB = steps[step_index];
                (step_index >= 3) ? step_index = 0 : step_index++;
                TCNT0 = 0;
                timer0_overflows = 0;
                (motor_cycle == 1) ? motor_status = false : motor_cycle--;
            }
        }
    }
}