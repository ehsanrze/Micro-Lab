#include <avr/io.h>
#include <avr/delay.h>
#include <string.h>
#include <avr/interrupt.h>

#define F_CPU = 8000000

#define MOTOR PORTA
#define SENSOR PINC

#define LCD_Data PORTB
#define LCD_Config PORTD

int state = 0; // 0: stop, 1: move, 2:turn right, 3: turn left
void lcd_config_cmd(char config)
{
    LCD_Data = config;
    LCD_Config = 0x01;
    _delay_ms(10);
    LCD_Config = 0x00;
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
    for (int i = 0; i < strlen(data); i++)
    {
        LCD_Data = data[i];
        LCD_Config = 0x05;
        _delay_ms(1);
        LCD_Config = 0x04;
    }
}

void move()
{
    MOTOR = _BV(0) | _BV(3);
}

void turn_right()
{
    MOTOR = _BV(1) | _BV(3);
}

void turn_left()
{
    MOTOR = _BV(0) | _BV(2);
}

void stop()
{
    MOTOR = 0;
}

int main()
{
    DDRA = 0xFF;
    DDRB = 0xFF;
    DDRD = 0xFF;
    DDRC = 0x00;

    reset_lcd();
    show_on_lcd("Welcome...");

    while (1)
    {
        // check motor is on/off
        if (SENSOR & _BV(6))
        {
            if (SENSOR & _BV(6) and !(SENSOR & 0x3F) and state != 1)
            {
                state = 1;
                reset_lcd();
                show_on_lcd("MOVING...");
                move();
            }
            if ((SENSOR & 0x7) and state != 2)
            {
                state = 2;
                reset_lcd();
                show_on_lcd("TURNING RIGHT...");
                turn_right();
            }
            else if ((SENSOR & 0x38) and state != 3)
            {
                state = 3;
                reset_lcd();
                show_on_lcd("TURNING LEFT...");
                turn_left();
            }
        }
        else if (state != 0)
        {
            state = 0;
            reset_lcd();
            show_on_lcd("STOP...");
            stop();
        }
    }
    return 0;
}