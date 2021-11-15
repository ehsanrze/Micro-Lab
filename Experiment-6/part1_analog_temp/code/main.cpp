#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>

char data[20];
int adc_result;

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
    while (ADCSRA & _BV(ADSC));

    return ADC;
}

void cmd(char config)
{
    PORTC = config;
    PORTD = 0x01;
    _delay_ms(10);
    PORTD = 0x00;
}

void show_on_lcd(char data[])
{
    for (int i = 0; i < strlen(data); i++)
    {
        PORTC = data[i];
        PORTD = 0x05;
        _delay_ms(1);
        PORTD = 0x04;
    }
}

void reset_lcd()
{
    cmd(0x01);
    cmd(0x06);
    cmd(0x0c);
    cmd(0x38);
}

int main()
{
    DDRC = 0xFF;
    DDRD = 0xFF;

    reset_lcd();
    adc_init();

    while (1)
    {
        if (adc_result != adc_read(0))
        {
            // read adc form PA0
            adc_result = adc_read(0);
            itoa(adc_result, data, 10);
            reset_lcd();
            show_on_lcd("Temp is ");
            show_on_lcd(data);
        }
    }
}