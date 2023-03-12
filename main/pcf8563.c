#include <string.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "pcf8563.h"

#define CHECK_ARG(ARG) do { if (!ARG) return ESP_ERR_INVALID_ARG; } while (0)

uint8_t bcd2dec(uint8_t val)
{
    return (val >> 4) * 10 + (val & 0x0f);
}

uint8_t dec2bcd(uint8_t val)
{
    return ((val / 10) << 4) + (val % 10);
}


esp_err_t pcf8563_init_desc(i2c_dev_t *dev, i2c_port_t port, gpio_num_t sda_gpio, gpio_num_t scl_gpio)
{
    CHECK_ARG(dev);

    dev->port = port;
    dev->addr = PCF8563_ADDR;
    dev->sda_io_num = sda_gpio;
    dev->scl_io_num = scl_gpio;
    dev->clk_speed = I2C_FREQ_HZ;
    return i2c_master_init(port, sda_gpio, scl_gpio, dev->clk_speed);
}

esp_err_t pcf8563_reset(i2c_dev_t *dev)
{
	CHECK_ARG(dev);

	uint8_t data[2];
	data[0] = 0;
	data[1] = 0;
	
	return i2c_dev_write_reg(dev, PCF8563_ADDR_STATUS1, data, 2);
}

esp_err_t pcf8563_set_time(i2c_dev_t *dev, struct tm *time)
{
    CHECK_ARG(dev);
    CHECK_ARG(time);

    uint8_t data[7];

    /* time/date data */
    data[0] = dec2bcd(time->tm_sec);
    data[1] = dec2bcd(time->tm_min);
    data[2] = dec2bcd(time->tm_hour);
    data[3] = dec2bcd(time->tm_mday);
    data[4] = dec2bcd(time->tm_wday);		// tm_wday is 0 to 6
    data[5] = dec2bcd(time->tm_mon + 1);	// tm_mon is 0 to 11
    data[6] = dec2bcd(time->tm_year - 2000);

    return i2c_dev_write_reg(dev, PCF8563_SEC_REG, data, 7);
}

esp_err_t pcf8563_get_time(i2c_dev_t *dev, struct tm *time)
{
    CHECK_ARG(dev);
    CHECK_ARG(time);

    uint8_t data[7];

    /* read time */
    esp_err_t res = i2c_dev_read_reg(dev, PCF8563_SEC_REG, data, 7);
        if (res != ESP_OK) return res;

    /* convert to unix time structure */
    ESP_LOGD("pcf8563_get_time", "data=%02x %02x %02x %02x %02x %02x %02x",
                 data[0],data[1],data[2],data[3],data[4],data[5],data[6]);
    time->tm_sec = bcd2dec(data[0] & 0x7F);
    time->tm_min = bcd2dec(data[1] & 0x7F);
    time->tm_hour = bcd2dec(data[2] & 0x3F);
    time->tm_mday = bcd2dec(data[3] & 0x3F);
    time->tm_wday = bcd2dec(data[4] & 0x07);		// tm_wday is 0 to 6
    time->tm_mon  = bcd2dec(data[5] & 0x1F) - 1;	// tm_mon is 0 to 11
    time->tm_year = bcd2dec(data[6]) + 2000;
    time->tm_isdst = 0;

    return ESP_OK;
}

uint8_t check_err(esp_err_t res, uint8_t * data, uint16_t size, char * op) {
    if (res != ESP_OK) {
        for (int r=0; r < size;r++) {
            printf("%d", data[r]);
        }
        printf("\n%s %s\n", op, esp_err_to_name(res));   
    }
    return res;
}

uint8_t pcf8563_get_flags(i2c_dev_t *dev) {
    uint8_t flags[1];
    esp_err_t res = i2c_dev_read_reg(dev, PCF8563_ADDR_STATUS2, &flags, 1);
    check_err(res, flags, 1, "pcf8563_get_flags read_reg ADDR_STATUS2 failed");
	
    // & 0b00010011;	    //Mask only configuration bits
    flags[0] = flags[0] & 0b00010011;
    res = i2c_dev_write_reg(dev, PCF8563_ADDR_STATUS2, flags, 1);
    check_err(res, flags, 1, "pcf8563_get_flags write_reg ADDR_STATUS2 failed");

	return flags[0] & 0x0C;	//Mask unnecessary bits
}

// Based on https://github.com/HausnerR/AVR-PCF8563/blob/master/PCF8563.c
// Set timer masks the input freq and will also enable timer
void pcf8563_set_timer(i2c_dev_t *dev, uint8_t freq, uint8_t count) {
    uint8_t timer1[1];
    uint8_t timer2[1];
    freq &= 0b10000011;
    timer1[0] = freq;
    timer2[0] = count;

    esp_err_t res = i2c_dev_write_reg(dev, PCF8563_TIMER1_REG,  &timer1, 1);
    check_err(res, timer1, 1, "pcf8563_set_timer write_reg 0x0E failed");
    res = i2c_dev_write_reg(dev, PCF8563_TIMER2_REG,  &timer2, 1);
    check_err(res, timer2, 1, "pcf8563_set_timer write_reg 0x0F failed");
}

void pcf8563_enable_timer(i2c_dev_t *dev) {
    uint8_t _data[1];
    esp_err_t res = i2c_dev_read_reg(dev, PCF8563_ADDR_STATUS2, &_data, 1);
    check_err(res, _data, 1, "pcf8563_enable_timer read_reg 0x01 failed");

    uint8_t timer1[1];
    res = i2c_dev_read_reg(dev, PCF8563_TIMER1_REG, &timer1, 1);
    check_err(res, timer1, 1, "pcf8563_enable_timer read_reg 0x0E failed");

    _data[0] &= ~PCF8563_TIMER_TF;
    _data[0] |= (PCF8563_ALARM_AF | PCF8563_TIMER_TIE);
    timer1[0] |= PCF8563_TIMER_TE;
    res = i2c_dev_write_reg(dev, PCF8563_ADDR_STATUS2,  &_data, 1);
    check_err(res, _data, 1, "pcf8563_enable_timer write_reg 0x01 failed");
    res = i2c_dev_write_reg(dev, PCF8563_TIMER1_REG,  &timer1, 1);
    check_err(res, timer1, 1, "pcf8563_enable_timer write_reg 0x0E failed");
}

uint8_t pcf8563_get_timer(i2c_dev_t *dev) {
    uint8_t _data[1];
    esp_err_t res = i2c_dev_read_reg(dev, PCF8563_TIMER2_REG, &_data, 1);
    check_err(res, _data, 1, "pcf8563_get_timer read_reg 0x0F failed");
    return _data[0];
}

// Return true if timer is active
bool pcf8563_is_timer_active(i2c_dev_t *dev) {
    uint8_t _data[1];
    esp_err_t res = i2c_dev_read_reg(dev, PCF8563_ADDR_STATUS2, &_data, 1);
    check_err(res, _data, 1, "pcf8563_is_timer_active read_reg 0x01 failed");
    return (bool) _data[0] & PCF8563_TIMER_TF;
}

// Set and enable clock out
esp_err_t pcf8563_set_clock_out(i2c_dev_t *dev, uint8_t freq) {
    freq &= 0b10000011;
    uint8_t _data[1];
    _data[0] = freq;
    esp_err_t res = i2c_dev_write_reg(dev, PCF8563_SQW_REG,  &_data, 1);
    check_err(res, _data, 1, "pcf8563_set_clock_out write_reg 0x0D failed");
    return res;
}

/**
 * @deprecated use  pcf8563_set_clock_out this will not pass QA
 * 
 * @param dev 
 * @param freq 
 * @return esp_err_t 
 */
esp_err_t pcf8563_set_clock_out2(i2c_dev_t *dev, uint8_t freq) {
    if (freq > PCF8563_CLK_1_div_60HZ) return false;
    uint8_t _data[1];
    _data[0] = freq | PCF8563_CLK_ENABLE;

    esp_err_t res = i2c_dev_write_reg(dev, PCF8563_SQW_REG,  &_data, 1);
    check_err(res, _data, 1, "pcf8563_enable_clock write_reg 0x0D failed");
    return res;
}

esp_err_t pcf8563_set_alarm(i2c_dev_t *dev, struct tm *time)
{
    CHECK_ARG(dev);
    CHECK_ARG(time);
    // Minimal validation time struct
    if (time->tm_hour > 23 || time->tm_min > 60 || time->tm_sec > 60) {
        ESP_LOGE("pcf8563_set_alarm", "time struct not valid HH:%d MM:%d SEC:%d",
        time->tm_hour, time->tm_min, time->tm_sec);
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t data[7];
    /* time/date data */
    data[0] = dec2bcd(time->tm_sec);
    data[1] = dec2bcd(time->tm_min);
    data[2] = dec2bcd(time->tm_hour);
    data[3] = dec2bcd(time->tm_mday);
    data[4] = dec2bcd(time->tm_wday);		// tm_wday is 0 to 6
    data[5] = dec2bcd(time->tm_mon + 1);	// tm_mon is 0 to 11
    data[6] = dec2bcd(time->tm_year - 2000);

    esp_err_t res = i2c_dev_write_reg(dev, PCF8563_ALRM_MIN_REG, data, 7);
    check_err(res, data, 7, "pcf8563_set_alarm write_reg 0x09 failed");
    return res;
}

esp_err_t pcf8563_enable_alarm(i2c_dev_t *dev) {
    uint8_t _data[1];
    esp_err_t res = i2c_dev_read_reg(dev, PCF8563_ADDR_STATUS2, &_data, 1);
    check_err(res, _data, 1, "pcf8563_enable_alarm read_reg 0x01 failed");

    _data[0] &= ~PCF8563_ALARM_AF;
    _data[0] |= (PCF8563_TIMER_TF | PCF8563_ALARM_AIE);

    res = i2c_dev_write_reg(dev, PCF8563_ADDR_STATUS2, _data, 1);
    check_err(res, _data, 1, "pcf8563_set_alarm write_reg 0x01 failed");
    return res;
}

esp_err_t pcf8563_disable_alarm(i2c_dev_t *dev) {
    uint8_t _data[1];
    esp_err_t res = i2c_dev_read_reg(dev, PCF8563_ADDR_STATUS2, &_data, 1);
    check_err(res, _data, 1, "pcf8563_enable_alarm read_reg 0x01 failed");

    _data[0] &= ~(PCF8563_ALARM_AF | PCF8563_ALARM_AIE);
    _data[0] |= PCF8563_TIMER_TF;

    res = i2c_dev_write_reg(dev, PCF8563_ADDR_STATUS2, _data, 1);
    check_err(res, _data, 1, "pcf8563_set_alarm write_reg 0x01 failed");
    return res;
}

esp_err_t pcf8563_reset_alarm(i2c_dev_t *dev) {
    uint8_t _data[1];
    esp_err_t res = i2c_dev_read_reg(dev, PCF8563_ADDR_STATUS2, &_data, 1);
    check_err(res, _data, 1, "pcf8563_enable_alarm read_reg 0x01 failed");

    _data[0] &= ~(PCF8563_ALARM_AF);
    _data[0] |= PCF8563_TIMER_TF;

    res = i2c_dev_write_reg(dev, PCF8563_ADDR_STATUS2, _data, 1);
    check_err(res, _data, 1, "pcf8563_set_alarm write_reg 0x01 failed");
    return res;
}
