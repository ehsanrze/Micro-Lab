#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>
#include <avr/interrupt.h>

#define LCD_Data PORTC
#define LCD_Config PORTB
#define NOTIF_PORT PORTD

#define GREEN_LIGHT 4
#define RED_LIGHT 5

#define KEYPAD_PORT PORTA
#define KEYPAD_DDR DDRA
#define KEYPAD_PIN PINA

#define BAUD 9600                              // define baud
#define BAUDRATE ((F_CPU) / (BAUD * 16UL) - 1) // set baudrate value for UBRR

char keypad[13] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '*', '0', '#', '-'};
char username[10] = "123";
char password[10] = "123";
char temp[10] = "-";
char year[10] = "21", month[10] = "10", day[10] = "12";
int seconds = 0;
int minutes = 0;
int hours = 0;
char cast_numbers[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
volatile uint32_t timer0_overflows = 0;

// send data with UART
void uart_transmit(unsigned int data)
{
    while (!(UCSRA & (1 << UDRE)))
        ;
    UDR = data;
}

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

uint8_t get_key_pressed()
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

    return 12;
}

void get_number(char* data)
{
    char temp_data[10] = "";
    char key = '-';
    int i = 0;
    while (1)
    {
        _delay_ms(200);
        key = keypad[get_key_pressed()];
        if (key == '*' || key == '#')
        {
            break;
        }
        if (key != '-')
        {
            temp_data[i] = key;
            show_on_lcd(key);
            i++;
        }
    }
    strncpy(data, temp_data, 10);
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

void show_date()
{
    show_on_lcd("20");
    show_on_lcd(year);
    show_on_lcd('-');
    show_on_lcd(month);
    show_on_lcd('-');
    show_on_lcd(day);
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

// need authentication
void home()
{
    reset_lcd();
    show_on_lcd("Temp is: ");
    show_on_lcd(temp);
    go_to_line_lcd(2);
    show_on_lcd("Date: ");
    show_date();
    go_to_line_lcd(3);
    show_on_lcd("Time: ");
    show_clock();
    go_to_line_lcd(4);
    show_on_lcd("Admin: Press * ");
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
        _delay_ms(200);
        key = keypad[get_key_pressed()];
        if (key == '*' || key == '#')
        {
            break;
        }
        if (key != '-')
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
            _delay_ms(200);
            key = keypad[get_key_pressed()];
            if (key == '*' || key == '#')
            {
                break;
            }
            if (key != '-')
            {
                input_password[i] = key;
                i++;
                show_on_lcd('*');
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

// setup counter of stepper motor
void set_stepper_motor()
{
    reset_lcd();
    char key = '-';
    int i = 1;
    char input_stepper_motor[10];
    int stepper_motor;
    show_on_lcd("Setup stepper motor ");
    go_to_line_lcd(2);
    get_number(input_stepper_motor);
    stepper_motor = atoi(input_stepper_motor);
    uart_transmit(stepper_motor);
}

// setup date of system
void set_date()
{
    reset_lcd();
    show_on_lcd("Year, Month, Day: ");
    go_to_line_lcd(2);
    get_number(year);
    go_to_line_lcd(3);
    get_number(month);
    go_to_line_lcd(4);
    get_number(day);
    home();
}

// setup time of system
void set_time()
{
    char input_hours[10], input_minutes[10], input_seconds[10];
    reset_lcd();
    show_on_lcd("Hour,Minute,Second:");
    go_to_line_lcd(2);
    get_number(input_hours);
    hours = atoi(input_hours);
    go_to_line_lcd(3);
    get_number(input_minutes);
    minutes = atoi(input_minutes);
    go_to_line_lcd(4);
    get_number(input_seconds);
    seconds = atoi(input_seconds);
    home();
}

void counter()
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
    home();
    TCNT0 = 0;
    timer0_overflows = 0;
}

void admin()
{
    char key = '-';
    reset_lcd();
    show_on_lcd("1- Set Date");
    go_to_line_lcd(2);
    show_on_lcd("2- Set Time");
    go_to_line_lcd(3);
    show_on_lcd("3- Set Stepper motor");
    go_to_line_lcd(4);
    show_on_lcd("4- Back");
    while (key == '-')
    {
        key = keypad[get_key_pressed()];
    }
    switch (key)
    {
    case '1':
        set_date();
        break;

    case '2':
        set_time();
        break;

    case '3':
        set_stepper_motor();
        break;

    case '4':
        home();
        break;
    }
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

// Initial UART
void uart_init(void)
{
    UBRRH = (BAUDRATE >> 8);
    UBRRL = BAUDRATE;
    UCSRB |= (1 << TXEN) | _BV(RXEN) | (1 << RXCIE);     //enable receiver (interrupt) and transmitter
    UCSRC |= (1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1); // 8bit data format
}

// Interrupt for receiver
ISR(USART_RXC_vect)
{
    itoa(UDR, temp, 10);
    home();
}

int main()
{
    DDRC = 0xFF;
    DDRD = 0xFF;
    DDRB = 0xFF;

    initial_timer0();
    uart_init();
    reset_lcd();
    login();
    sei();

    while (1)
    {
        if (get_key_pressed() != 12)
        {
            admin();
        }
        if (timer0_overflows >= 122)
        {
            if (TCNT0 >= 9)
            {
                counter();
            }
        }
    }
}