#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>

#define LCD_Data PORTC
#define LCD_Config PORTD
#define NOTIF_PORT PORTB

#define GREEN_LIGHT 4
#define RED_LIGHT 5

#define KEYPAD_PORT PORTA
#define KEYPAD_DDR DDRA
#define KEYPAD_PIN PINA

char keypad[12] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '*', '0', '#'};
char username[10] = "123";
char password[10] = "123";
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

void show_on_lcd(char data)
{
    LCD_Data = data;
    LCD_Config = 0x05;
    _delay_ms(1);
    LCD_Config = 0x04;
}

void go_to_line_lcd(int line_number)
{
    switch (line_number)
    {
    case 1:
        lcd_config_cmd(0x80);
        break;
    case 2:
        lcd_config_cmd(0xC0);
        break;
    case 3:
        lcd_config_cmd(0x94);
        break;
    case 4:
        lcd_config_cmd(0xD4);
        break;

    default:
        break;
    }
}

uint8_t GetKeyPressed()
{
    _delay_ms(500);
    while (1)
    {
        uint8_t r, c;

        KEYPAD_PORT |= 0X0F;
        for (c = 0; c < 3; c++)
        {
            KEYPAD_DDR &= ~(0X7F);
            KEYPAD_DDR |= (0X40 >> c);
            for (r = 0; r < 4; r++)
            {
                if (!(KEYPAD_PIN & (0X08 >> r)))
                {
                    return (r * 3 + c);
                }
            }
        }
        // return 0XFF; //Indicate No key pressed
    }
}

void notification(int status)
{
    switch (status)
    {
    case 0:
        NOTIF_PORT &= ~(1UL << RED_LIGHT);
        NOTIF_PORT |= _BV(GREEN_LIGHT);
        break;
    case 1:
        NOTIF_PORT &= ~(1UL << GREEN_LIGHT);
        NOTIF_PORT |= _BV(RED_LIGHT);
        break;
    default:
        break;
    }
}

// need authentication
void home()
{
    reset_lcd();
    show_on_lcd("User ID: ");
    show_on_lcd(username);
    go_to_line_lcd(2);
    show_on_lcd("Date: 2021-09-10");
    go_to_line_lcd(3);

    show_on_lcd("Time: 13:40:01");
    go_to_line_lcd(4);
    show_on_lcd("Admin: Press *");
}

void login()
{
    char input_username[10] = "";
    char input_password[10] = "";
    char key;
    int i = 0;
    reset_lcd();
    show_on_lcd("Enter user ID:");
    go_to_line_lcd(2);
    while (1)
    {
        key = keypad[GetKeyPressed()];
        if (key == '*' || key == '#')
        {
            break;
        }
        else
        {
            input_username[i] = key;
            i++;
            show_on_lcd(key);
        }
    }

    i = 0; // clear index of char array for record password

    if (strcmp(input_username, username) == 0)
    {
        reset_lcd();
        show_on_lcd("Enter your password: ");
        go_to_line_lcd(2);
        while (1)
        {
            key = keypad[GetKeyPressed()];
            if (key == '*' || key == '#')
            {
                break;
            }
            else
            {
                input_password[i] = key;
                i++;
                show_on_lcd(key);
            }
        }

        if (strcmp(input_password, password) == 0)
        {
            notification(0);
            home();
        }
        else
        {
            notification(1);
            reset_lcd();
            show_on_lcd("Password Incorrect");
            _delay_ms(1000);
            login();
        }
    }
    else
    {
        notification(1);
        reset_lcd();
        show_on_lcd("Username Incorrect");
        _delay_ms(1000);
        login();
    }
}

int main()
{
    DDRC = 0xFF;
    DDRD = 0xFF;
    DDRB = 0xFF;
    reset_lcd();
    login();

    while (1)
    {
    }
}