#include <avr/io.h>
#include <string.h>
#include <avr/delay.h>

char data[200] = "Hello World!";

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
        PORTC = data[i];
        PORTD = 0x05; // high pulse
        _delay_ms(10);
        PORTD = 0x04; // low pulse
    }
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

    reset_lcd();
    show_on_lcd(data);

    while (1)
    {
    }
    return 0;
}