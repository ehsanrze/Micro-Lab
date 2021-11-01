#include <avr/io.h>
#include <string.h>
#include <util/delay.h>
#include <avr/interrupt.h>

volatile uint8_t timer1_overflow = 0;
int seconds = 0;
int minutes = 0;
int hours = 0;

// cast numbers to character
char cast_numbers[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

void initial_timer_1()
{
    // prescaler = 8
    TCCR1B |= _BV(CS11);

    // counter set to 0
    TCNT1 = 0;

    // enable overflow interrupt
    TIMSK |= _BV(TOIE1);

    timer1_overflow = 0;
}

ISR(TIMER1_OVF_vect)
{
    timer1_overflow++;
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
    reset_lcd();
    show_on_lcd(cast_numbers[hours / 10]);
    show_on_lcd(cast_numbers[hours % 10]);
    show_on_lcd(':');
    show_on_lcd(cast_numbers[minutes / 10]);
    show_on_lcd(cast_numbers[minutes % 10]);
    show_on_lcd(':');
    show_on_lcd(cast_numbers[seconds / 10]);
    show_on_lcd(cast_numbers[seconds % 10]);
}

// setup hours -> minutes -> seconds sequentially
void setup_alarm()
{
    while (PINA != _BV(2))
    {
        if (PINA == _BV(0) && hours < 24)
        {
            hours++;
            show_clock();
            _delay_ms(500);
        }

        if (PINA == _BV(1) && hours > 0)
        {
            hours--;
            show_clock();
            _delay_ms(500);
        }
    }

    _delay_ms(1000);

    while (PINA != _BV(2))
    {
        if (PINA == _BV(0) && minutes < 59)
        {
            minutes++;
            show_clock();
            _delay_ms(500);
        }

        if (PINA == _BV(1) && minutes > 0)
        {
            minutes--;
            show_clock();
            _delay_ms(500);
        }
    }

    _delay_ms(1000);
    while (PINA != _BV(2))
    {
        if (PINA == _BV(0) && seconds < 59)
        {
            seconds++;
            show_clock();
            _delay_ms(500);
        }

        if (PINA == _BV(1) && seconds > 0)
        {
            seconds--;
            show_clock();
            _delay_ms(500);
        }
    }
}

// show message when alarm finished
void finished_alarm()
{
    reset_lcd();
    char message[] = "finished";
    for (int i = 0; i < sizeof(message); i++)
    {
        show_on_lcd(message[i]);
    }
    _delay_ms(1000);
}

int main()
{
    DDRC = 0xFF;
    DDRD = 0xFF;
    DDRA = 0x00;

    reset_lcd();
    show_clock();
    initial_timer_1();
    sei();

    setup_alarm();
    while (1)
    {
        if (timer1_overflow >= 15)
        {
            if (TCNT1 >= 16960)
            {
                if (seconds >= 1)
                {
                    seconds--;
                }
                if (seconds <= 0 && minutes >= 1)
                {
                    seconds = 59;
                    minutes--;
                }
                if (seconds <= 0 && minutes <= 0 && hours >= 1)
                {
                    minutes = 59;
                    seconds = 59;
                    hours--;
                }
                if (hours <= 0 && minutes <= 0 && seconds <= 0)
                {
                    finished_alarm();
                    show_clock();
                    setup_alarm();
                }

                show_clock();
                TCNT1 = 0;           // reset counter
                timer1_overflow = 0; // reset overflow counter
            }
        }
    }
    return 0;
}