#ifndef __HNODE_SENSOR_MEASUREMENT_H__
#define __HNODE_SENSOR_MEASUREMENT_H__

#include <stdint.h>
#include <time.h>
#include <sys/time.h>

#include <string>

typedef enum HNodeWeatherMeasurementType
{
    HNWM_TYPE_NOT_SET            = 0,
    HNWM_TYPE_WIND_SPEED         = 1,
    HNWM_TYPE_TEMPERATURE        = 2,
    HNWM_TYPE_RELATIVE_HUMIDITY  = 3,
    HNWM_TYPE_WIND_DIRECTION     = 4,
    HNWM_TYPE_RAINFALL           = 5
}HNWM_TYPE_T;

typedef enum HNodeWeatherMeasurementUnits
{
    HNWM_UNITS_NOT_SET           = 0,
    HNWM_UNITS_KPH               = 1,
    HNWM_UNITS_MPH               = 2,
    HNWM_UNITS_CELSIUS           = 3,
    HNWM_UNITS_FAHRENHEIT        = 4,
    HNWM_UNITS_PERCENT           = 5,
    HNWM_UNITS_DEGREES           = 6,
    HNWM_UNITS_INCHES            = 7
}HNWM_UNITS_T;

class HNodeWeatherMeasurement
{
    private:
        HNWM_TYPE_T     type;  
        HNWM_UNITS_T    units;
        uint32_t        count;
        struct timeval  tstamp;
        double          reading;

    public:
        HNodeWeatherMeasurement();
       ~HNodeWeatherMeasurement();

        void setType( HNWM_TYPE_T value );
        HNWM_TYPE_T getType();
        void setTypeFromStr( std::string value );
        std::string getTypeAsStr();
  
        void setUnits( HNWM_UNITS_T value );
        HNWM_UNITS_T getUnits();
        void setUnitsFromStr( std::string value );
        std::string getUnitsAsStr();

        void setCount( uint32_t value );
        uint32_t getCount();

        void setReading( double value );
        double getReading();

        void setTimestamp( struct timeval &value );
        void getTimestamp( struct timeval &value );
        void updateTimestamp();

        std::string getAsStr();

        void buildPacketData( uint8_t *bufPtr, uint32_t &length );
        void parsePacketData( uint8_t *bufPtr, uint32_t length );
};

#endif // __HNODE_SENSOR_MEASUREMENT_H__
