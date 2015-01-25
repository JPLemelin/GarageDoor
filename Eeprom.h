#ifndef _EEPROM_h
#define _EEPROM_h

#include <Arduino.h>

class Eeprom
{
public:

    Eeprom();

    void readSetting(void* setting, size_t size) const;

    void writeSetting(const void* setting, size_t size) const;

    uint16_t getBallCount() const;

    uint16_t incrementBallCount();

    bool isVersionCompatible() const;
    
private:

    bool _is_setting_compatible_;

    uint16_t ball_count_;

};

#endif

