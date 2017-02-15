#ifndef __HNODE_SEP_MANAGER_H__
#define __HNODE_SEP_MANAGER_H__

#include <string>
#include <list>
#include <vector>
#include <map>
#include <exception>

#include "boost/date_time/gregorian/gregorian.hpp"
#include "boost/date_time/posix_time/posix_time.hpp" 
#include "boost/date_time/local_time/local_time.hpp"

#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>

#include <hnode-rest.hpp>

#include "HNodeSensorMeasurement.hpp"

class Acurite5N1Exception : public std::exception
{
    private:
        unsigned long eCode;
        std::string eMsg;

    public:
        Acurite5N1Exception( unsigned long errCode, std::string errMsg )
        {
            eCode = errCode;
            eMsg  = errMsg;
        }

       ~Acurite5N1Exception() throw() {};

        virtual const char* what() const throw()
        {
            return eMsg.c_str();
        }

        unsigned long getErrorCode() const throw()
        {
            return eCode;
        }

        std::string getErrorMsg() const throw()
        {
            return eMsg;
        }
};

class SensorDefinition
{
    private:
        uint32_t sensorIndex;
        std::string sensorID;
        std::string sensorName;
        std::string sensorDesc;

    public:

        SensorDefinition();
       ~SensorDefinition();

        void setIndex( uint32_t index );
        uint32_t getIndex();

        void setID( std::string value );
        std::string getID();

        void setName( std::string value );
        std::string getName();

        void setDesc( std::string value );
        std::string getDesc();
};

class SensorMeasurements
{
    private:
        std::list< HNodeSensorMeasurement > history;

    public:

        SensorMeasurements();
       ~SensorMeasurements();

        void addNewMeasurement(  HNodeSensorMeasurement &measurement );

        bool getCurrentMeasurement( HNodeSensorMeasurement &measurement );
};

typedef enum Acurite5N1StatusRequestIDs
{
    WXRSRC_STATID_CURRENT_READING    = 1,
    WXRSRC_STATID_HEALTH             = 2,
    WXRSRC_STATID_SENSOR_DEFINITIONS = 3
}WXRSRC_STATID_T;

class HNodeSEPManager : public RESTContentManager
{
    private:
        std::string cfgPath;

        std::string stationID;

        bool           localOK;
        std::string    localMsg;

        bool           healthOK;
        struct timeval healthStatusTime;
        struct timeval healthLastMeasurementTime;
        uint32_t       healthMeasurementCount;
        std::string    healthMsg;

        std::map< uint32_t, SensorDefinition >   definitions;
        std::map< uint32_t, SensorMeasurements > measurements;

        void parseSensor( xmlDocPtr doc, xmlNodePtr objNode );

        virtual void populateContentNodeFromStatusProvider( unsigned int id, RESTContentNode *outNode, std::map< std::string, std::string > paramMap );

    public:
        HNodeSEPManager();
       ~HNodeSEPManager();

        void setConfigurationPath( std::string cfgPath );
        bool loadConfiguration();
        bool saveConfiguration();

        void setLocalHealthError( std::string errMsg );
        void setLocalHealthOK();

        void start();

        void processCurrentEvents( uint32_t &curTime );

        void addNewMeasurement( uint32_t sensorIndex, HNodeSensorMeasurement &measurement );

        void updateHealthStatus( bool okflag, struct timeval *statusTime, struct timeval *lastMeasurementTime, uint32_t measurmentCount );
       
        void updateHealthMsg( std::string value );

        virtual RESTContentNode* newObject( unsigned int type );
        virtual void freeObject( RESTContentNode *objPtr );
        virtual unsigned int getTypeFromObjectElementName( std::string name );
        virtual RESTContentTemplate *getContentTemplateForType( unsigned int type );
};

#endif // __HNODE_SEP_MANAGER_H__

