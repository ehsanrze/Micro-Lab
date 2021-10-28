#include <avr/io.h>
#include <string.h>
#include <avr/delay.h>
#include <avr/interrupt.h>

volatile uint8_t timer2_overflow = 0;
int seconds = 0;
int minutes = 0;
int hours = 0;

// cast numbers to character
char cast_numbers[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

void initial_timer_2()
{
    // prescaler = 256
    TCCR2 |= _BV(CS22) | _BV(CS21);

    // counter set to 0
    TCNT2 = 0;

    // enable overflow interrupt
    TIMSK |= _BV(TOIE2);

    timer2_overflow = 0;
}

ISR(TIMER2_OVF_vect)
{
    timer2_overflow++;
}

// this function config LCD
void cmd(char config)
{
    PORTC = config;
    PORTD = 0x01; // make lcd to get commend
    _delay_ms(1);
    PORTD = 0x00; // make lcd to get data
}

void show_on_lcd(char data)
{
    PORTC = data;
    PORTD = 0x05; // high pulse
    _delay_ms(1);
    PORTD = 0x04; // low pulse
}

void reset_lcd()
{
    cmd(0x01); // clear display screen
    cmd(0x06); // set direction to left to right
    cmd(0x0c); // enable screen & disable cursor
    cmd(0x38); // set lcd to 8-bit, 2 line, 5x7 dots
}

void show_clock()
{
    show_on_lcd(cast_numbers[hours / 10]);
    show_on_lcd(cast_numbers[hours % 10]);
    show_on_lcd(':');
    show_on_lcd(cast_numbers[minutes / 10]);
    show_on_lcd(cast_numbers[minutes % 10]);
    show_on_lcd(':');
    show_on_lcd(cast_numbers[seconds / 10]);
    show_on_lcd(cast_numbers[seconds % 10]);
}

int main()
{
    DDRC = 0xFF;
    DDRD = 0xFF;

    reset_lcd();
    show_clock();
    initial_timer_2();
    sei();

    while (1)
    {
        if (timer2_overflow >= 122)
        {
            if (TCNT2 >= 18)
            {
                if (seconds >= 59)
                {
                    seconds = 0;
                    minutes++;
                }
                else
                {
                    seconds++;
                }

                if (minutes >= 59)
                {
                    minutes = 0;
                    hours++;
                }

                if (hours >= 12)
                {
                    hours = 0;
                }
                reset_lcd();
                show_clock();
                TCNT2 = 0;           // reset counter
                timer2_overflow = 0; // reset overflow counter
            }
        }
    }
    return 0;
}