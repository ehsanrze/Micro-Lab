#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdlib.h>

#define F_CPU = 8000000

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

void reset_lcd()
{
    lcd_config_cmd(0x01);
    lcd_config_cmd(0x06);
    lcd_config_cmd(0x0c);
    lcd_config_cmd(0x38);
}

void show_on_lcd(char data[])
{
    reset_lcd();
    for (int i = 0; i < strlen(data); i++)
    {
        PORTC = data[i];
        PORTD = 0x05;
        _delay_ms(1);
        PORTD = 0x04;
    }
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

void setup_temp()
{
    char cast_temp[20];
    show_on_lcd("Default temp=25");
    while (PINA != _BV(PINC3))
    {
        if (PINA == _BV(PINC1))
        {
            temp++;
            itoa(temp, cast_temp, 10);
            show_on_lcd(cast_temp);
            _delay_ms(1000);
        }

        if (PINA == _BV(PINC2))
        {
            temp--;
            itoa(temp, cast_temp, 10);
            show_on_lcd(cast_temp);
            _delay_ms(1000);
        }
    }
    show_on_lcd("Submited...");
    _delay_ms(1000);
}

int main()
{
    DDRC = 0xFF;
    DDRD = 0xFF;
    DDRB = 0xFF;
    DDRA = 0x00;

    setup_temp();
    initial_timer0();
    sei();
    reset_lcd();
    adc_init();

    while (1)
    {
        if (adc_result != adc_read(0))
        {
            // read adc form PA0
            adc_result = adc_read(0);
            itoa(adc_result, data, 10);
            show_on_lcd("Temp is ");
            show_on_lcd(data);
            adc_result >= temp ? motor_status = true : motor_status = false;
        }

        // timer with 0.5s
        if (timer0_overflows >= 61 and motor_status == true)
        {
            if (TCNT0 >= 9)
            {
                PORTB = steps[step_index];
                (step_index >= 3) ? step_index = 0 : step_index++;
                TCNT0 = 0;
                timer0_overflows = 0;
            }
        }
    }
}