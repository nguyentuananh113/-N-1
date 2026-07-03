#include "i2c-lcd.h"
#include <string.h>

extern I2C_HandleTypeDef hi2c1; // dùng I2C1

#define SLAVE_ADDRESS_LCD 0x4E // địa chỉ I2C LCD (0x27 << 1 = 0x4E)

void lcd_send_cmd (char cmd)
{
    char data_u, data_l;
    uint8_t data_t[4];
    data_u = (cmd&0xf0);
    data_l = ((cmd<<4)&0xf0);
    data_t[0] = data_u|0x0C;  // en=1, rs=0
    data_t[1] = data_u|0x08;  // en=0, rs=0
    data_t[2] = data_l|0x0C;  // en=1, rs=0
    data_t[3] = data_l|0x08;  // en=0, rs=0
    HAL_I2C_Master_Transmit (&hi2c1, SLAVE_ADDRESS_LCD,(uint8_t *) data_t, 4, 100);
}

void lcd_send_data (char data)
{
    char data_u, data_l;
    uint8_t data_t[4];
    data_u = (data&0xf0);
    data_l = ((data<<4)&0xf0);
    data_t[0] = data_u|0x0D;  // en=1, rs=1
    data_t[1] = data_u|0x09;  // en=0, rs=1
    data_t[2] = data_l|0x0D;  // en=1, rs=1
    data_t[3] = data_l|0x09;  // en=0, rs=1
    HAL_I2C_Master_Transmit (&hi2c1, SLAVE_ADDRESS_LCD,(uint8_t *) data_t, 4, 100);
}

void lcd_clear (void)
{
    lcd_send_cmd (0x01);
    HAL_Delay(2);
}

void lcd_put_cur(int row, int col)
{
    switch (row)
    {
        case 0:
            lcd_send_cmd (0x80 + 0x00 + col);
            break;
        case 1:
            lcd_send_cmd (0x80 + 0x40 + col);
            break;
    }
}

void lcd_init (void)
{
    HAL_Delay(50); // chờ LCD khởi động
    lcd_send_cmd (0x30);
    HAL_Delay(5);
    lcd_send_cmd (0x30);
    HAL_Delay(1);
    lcd_send_cmd (0x30);
    HAL_Delay(10);
    lcd_send_cmd (0x20);  // 4-bit mode
    HAL_Delay(10);

    // function set
    lcd_send_cmd (0x28);
    // display on, cursor off
    lcd_send_cmd (0x0C);
    // clear
    lcd_send_cmd (0x01);
    HAL_Delay(2);
    // entry mode
    lcd_send_cmd (0x06);
}

void lcd_send_string (char *str)
{
    while (*str) lcd_send_data (*str++);
}
