#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdlib.h>

#define F_CPU 8000000

#define BAUD 9600                              // define baud
#define BAUDRATE ((F_CPU) / (BAUD * 16UL) - 1) // set baudrate value for UBRR

char data[20];
int adc_result;
int steps[] = {0x09, 0x0A, 0x06, 0x05};
int step_index = 0;
volatile uint32_t timer0_overflows = 0;
bool motor_status = false;
int temp = 25; // setup temperature

void adc_init()
{
    // Enable ADC
    ADCSRA = _BV(ADEN);
}

int adc_read(int channel)
{
    // select channel 0-7
    channel &= _BV(MUX2) | _BV(MUX1) | _BV(MUX0);
    ADMUX = (ADMUX & 0xF8) | channel;

    // start conversion
    ADCSRA |= _BV(ADSC);

    // wait for conversion to complete
    while (ADCSRA & _BV(ADSC))
        ;

    return ADC;
}

void lcd_config_cmd(char config)
{
    PORTC = config;
    PORTD = 0x01;
    _delay_ms(10);
    PORTD = 0x00;
}

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

// function to initialize UART
void uart_init(void)
{
    UBRRH = (BAUDRATE >> 8);
    UBRRL = BAUDRATE;                                    //set baud rate
    UCSRB |= (1 << TXEN) | (1 << RXEN);                  //enable receiver and transmitter
    UCSRC |= (1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1); // 8bit data format
}

// function to send data - NOT REQUIRED FOR THIS PROGRAM IMPLEMENTATION
void uart_transmit(unsigned int data)
{
    while (!(UCSRA & (1 << UDRE)))
        ;       // wait while register is free
    UDR = data; // load data in the register
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
            // read adc form PA0
            adc_result = adc_read(0);
            adc_result >= temp ? motor_status = true : motor_status = false;
        }

        // timer with 0.5s
        if (timer0_overflows >= 61 and motor_status == true)
        {
            if (TCNT0 >= 9)
            {
                uart_transmit(adc_result);
                PORTB = steps[step_index];
                (step_index >= 3) ? step_index = 0 : step_index++;
                TCNT0 = 0;
                timer0_overflows = 0;
            }
        }
    }
}