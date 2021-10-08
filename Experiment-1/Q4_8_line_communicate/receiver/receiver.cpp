#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/delay.h>
#include <string.h>

char recived_data[200];

// this function config LCD
void cmd(char config)
{
    PORTC = config;
    PORTD = 0x01; // make lcd to get commend
    _delay_ms(10);
    PORTD = 0x00; // make lcd to get data
}

void show_on_lcd(char data[])
{
    for (int i = 0; i < strlen(data); i++)
    {
        if (i % 32 == 0)
        {
            cmd(0x01); // reset screen
            cmd(0x80); // send cursor to first line
        }
        else if (i % 16 == 0)
        {
            cmd(0xC0); // send cursor to second line
        }
        PORTC = data[i];
        PORTD = 0x05; // high pulse
        _delay_ms(10);
        PORTD = 0x04; // low pulse
    }

    // reset screen when message done.
    cmd(0x01);
    cmd(0x80);
}

void reset_lcd()
{
    cmd(0x01); // clear display screen
    cmd(0x06); // set direction to left to right
    cmd(0x0c); // enable screen & disable cursor
    cmd(0x38); // set lcd to 8-bit, 2 line, 5x7 dots
}

int main()
{
    DDRC = 0xFF;
    DDRD = 0xFF;
    DDRA = 0x00;

    reset_lcd();

    // wait until signal from sender
    while (PINA == 0x00)
    {
    }

    for (int i = 0; i <= strlen(recived_data); i++)
    {
        recived_data[i] = PINA;
        _delay_ms(5);
    }

    while (1)
    {
        show_on_lcd(recived_data);
    }
    return 0;
}