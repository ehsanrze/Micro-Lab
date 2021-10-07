#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/delay.h>
#include <string.h>

char read_from_eeprom[200];

// example data to save in EEPROM
char data[200] = "Hello, this message sends from Ehsan Rezaee with student number 972023015 from one microcontroller that name is 'sender' to another microcontroller 'receiver' and shows a message in LCD.";

int main()
{
    DDRC = 0xFF;
    PORTC = 0x00;

    // write and read from EEPROM
    eeprom_write_block(data, 0, strlen(data));
    eeprom_read_block(read_from_eeprom, 0, strlen(data));

    // this delay for sync timing of two microcontroller
    _delay_ms(300);

    for (int i = 0; i <= strlen(read_from_eeprom); i++)
    {
        PORTC = read_from_eeprom[i];
        _delay_ms(5);
    }

    while (1)
    {
        PORTC = 0x00;
    }
    return 0;
}