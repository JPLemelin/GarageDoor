#include "config.h"
#include "Eeprom.h"

#include <avr/eeprom.h>

//
// According to the Atmel data sheet the lifespan is 100,000 write/erase cycles.
//
// Speed test of writing value to eeprom
// Results:
//  32 bits => 12.922 ms
//  16 bits =>  6.433 ms
//   8 bits =>  3.168 ms
//
//    StatsAverage stat;
//    stat.reset();
//    for(int i = 0; i < 100; ++i)
//    {
//        static uint32_t value = 0;
//        unsigned long time_stamp = micros();
//        eeprom_write_dword((uint32_t*)2, value++);
//        stat.addValue(micros() - time_stamp);
//    }
//    log_logln(stat.getAverage()); // 12922.96
//
//    stat.reset();
//    for(int i = 0; i < 100; ++i)
//    {
//        static uint16_t value = 0;
//        unsigned long time_stamp = micros();
//        eeprom_write_word((uint16_t*)2, value++);
//        stat.addValue(micros() - time_stamp);
//    }
//    log_logln(stat.getAverage()); // 6433.80
//
//    stat.reset();
//    for(int i = 0; i < 100; ++i)
//    {
//        static uint8_t value = 0;
//        unsigned long time_stamp = micros();
//        eeprom_write_byte((uint8_t*)2, value++);
//        stat.addValue(micros() - time_stamp);
//    }
//    log_logln(stat.getAverage()); // 3168.40


#define EEPROM_VERSION_MAJOR 0
#define EEPROM_VERISON_MINOR 0

#define ADDRESS_VERSION_MAJOR 0x0000
#define ADDRESS_VERSION_MINOR 0x0001

#define ADDRESS_BALL_COUNT    0x0002
#define ADDRESS_SETTING       0x0004

Eeprom::Eeprom()
{	
    uint8_t version_major = eeprom_read_byte((uint8_t*) ADDRESS_VERSION_MAJOR);
    uint8_t version_minor = eeprom_read_byte((uint8_t*) ADDRESS_VERSION_MINOR);

    _is_setting_compatible_ = version_major == EEPROM_VERSION_MAJOR && version_minor <= ADDRESS_VERSION_MINOR;

    log_log("Eeprom version: ");
    log_log(version_major);
    log_log(".");
    log_logln(version_minor);

    // Read ball count
    if (true == _is_setting_compatible_)
    {
        ball_count_ = eeprom_read_word((uint16_t*) ADDRESS_BALL_COUNT);
    }
    else
    {
        // reset ball count
        eeprom_write_word((uint16_t*)ADDRESS_BALL_COUNT, 0);
        ball_count_ = 0;
    }

    log_log("Ball count: ");
    log_logln(ball_count_);

}

bool Eeprom::isVersionCompatible() const
{
    return _is_setting_compatible_;
}

void Eeprom::readSetting(void* setting, size_t size) const
{
    //TODO: add size of setting in eeprom
    eeprom_read_block(setting, (void*)ADDRESS_SETTING, size);
}

void Eeprom::writeSetting(const void* setting, size_t size) const
{
    // Write version
    eeprom_write_byte((uint8_t*)ADDRESS_VERSION_MAJOR, EEPROM_VERSION_MAJOR);
    eeprom_write_byte((uint8_t*)ADDRESS_VERSION_MINOR, EEPROM_VERISON_MINOR);
    // Write setting
    eeprom_write_block(setting, (void*)ADDRESS_SETTING, size);
}
    
uint16_t Eeprom::getBallCount() const
{
    return ball_count_;
}

uint16_t Eeprom::incrementBallCount()
{
    ball_count_++;

    // Save it
    eeprom_write_word((uint16_t*)ADDRESS_BALL_COUNT, ball_count_);
    
    return ball_count_;
}

