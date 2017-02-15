#include <string>
#include <iostream>
#include <fstream>
#include <cerrno>

#include <stdio.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <libxml/xpath.h>

#include <string.h>
#include <arpa/inet.h>

#include <boost/regex.hpp>
#include <boost/algorithm/string/trim.hpp>

#include <hnode-rest.hpp>

#include "HNodeSEPManager.hpp"

SensorDefinition::SensorDefinition()
{
    sensorIndex = -1;
}

SensorDefinition::~SensorDefinition()
{

}

void
SensorDefinition::setIndex( uint32_t index )
{
    sensorIndex = index;
}

uint32_t 
SensorDefinition::getIndex()
{
    return sensorIndex;
}

void 
SensorDefinition::setID( std::string value )
{
    sensorID = value;
}

std::string 
SensorDefinition::getID()
{
    return sensorID;
}

void 
SensorDefinition::setName( std::string value )
{
    sensorName = value;
}

std::string 
SensorDefinition::getName()
{
    return sensorName;
}

void 
SensorDefinition::setDesc( std::string value )
{
    sensorDesc = value;
}

std::string 
SensorDefinition::getDesc()
{
    return sensorDesc;
}

SensorMeasurements::SensorMeasurements()
{

}

SensorMeasurements::~SensorMeasurements()
{

}

void 
SensorMeasurements::addNewMeasurement( HNodeSensorMeasurement &measurement )
{
    history.push_front( measurement );

    if( history.size() > 2880 )
    {
        history.pop_back();
    }
}


bool 
SensorMeasurements::getCurrentMeasurement( HNodeSensorMeasurement &measurement )
{
    if( history.size() == 0 )
        return false;

    measurement = history.front();

    return true;
}

HNodeSEPManager::HNodeSEPManager()
{
    cfgPath         = "/etc/hnode";  

    // Start up with bad health, 
    // until we have seen good health.
    localOK  = false;
    localMsg ="Starting Up";

    healthOK = false;
    healthMsg ="Starting Up";
}

HNodeSEPManager::~HNodeSEPManager()
{

}

void
HNodeSEPManager::parseSensor( xmlDocPtr doc, xmlNodePtr objNode )
{
    xmlNode          *curElem;
    xmlChar          *tmpStr;
    SensorDefinition  sensorDef;
    uint32_t          index;
    bool              indexSet = false;

    // Find the address element
    for( curElem = objNode->children; curElem; curElem = curElem->next ) 
    {

        if( ( curElem->type == XML_ELEMENT_NODE ) && ( xmlChildElementCount( curElem ) == 0 ) ) 
        {
            if( "index" == (const char *)curElem->name )
            {
                tmpStr = xmlNodeGetContent( curElem );
                index = strtol( (char*)tmpStr, NULL, 0 );
                sensorDef.setIndex( index );
                indexSet = true;
                xmlFree( tmpStr );
            }
            else if( "id" == (const char *)curElem->name )
            {
                tmpStr = xmlNodeGetContent( curElem );
                sensorDef.setID( (char*)tmpStr );
                xmlFree( tmpStr );
            }
            else if( "name" == (const char *)curElem->name )
            {
                tmpStr = xmlNodeGetContent( curElem );
                sensorDef.setName( (char*)tmpStr );
                xmlFree( tmpStr );
            }
            else if( "desc" == (const char *)curElem->name )
            {
                tmpStr = xmlNodeGetContent( curElem );
                sensorDef.setDesc( (char*)tmpStr );
                xmlFree( tmpStr );
            }
        }
    }

    if( indexSet == true )
    {
        definitions.insert( std::pair< uint32_t, SensorDefinition >( index, sensorDef ) );
    }
}

bool
HNodeSEPManager::loadConfiguration()
{
    xmlDocPtr           doc;
    xmlNode            *rootElem;
    xmlNode            *curElem;
    xmlXPathContextPtr  xpathCtx; 
    xmlXPathObjectPtr   xpathObj; 
    std::string filePath;

    filePath = cfgPath + "/sep/sep_config.xml";

    // Clear out any existing items.
    clear();

    doc = xmlReadFile( filePath.c_str(), NULL, 0 );
    if (doc == NULL) 
    {
        fprintf( stderr, "Failed to parse %s\n", filePath.c_str() );
        setLocalHealthError( "Missing or unparsable configuration file: " + filePath );
	    return true;
    }

    std::cout << "HNodeSEPManager::loadConfiguration" << std::endl;

    // Get the root element for the document
    rootElem = xmlDocGetRootElement( doc );

    // Create xpath evaluation context 
    xpathCtx = xmlXPathNewContext( doc );
    if( xpathCtx == NULL ) 
    {
        fprintf( stderr,"Error: unable to create new XPath context\n" );
        return true;
    }

    // Process all of the sensor definitions
    std::string nodeXPath = "/hnode-sep-config/stationid";

    xpathObj = xmlXPathEvalExpression( (const xmlChar *) nodeXPath.c_str(), xpathCtx );
    if( xpathObj == NULL ) 
    {
        fprintf( stderr,"Error: unable to evaluate xpath expression \"%s\"\n", nodeXPath.c_str() );
        return true;
    }
    
    if( ( xpathObj->nodesetval != NULL ) && ( xpathObj->nodesetval->nodeNr > 0 ) )
    {    
        if( xpathObj->nodesetval->nodeTab[0]->type == XML_ELEMENT_NODE )
        {
            xmlChar *tmpStr = xmlNodeGetContent( xpathObj->nodesetval->nodeTab[0] );
            stationID = (char*)tmpStr;
            xmlFree( tmpStr );
        }
    }

    // Free things.
    xmlXPathFreeObject(xpathObj);

    // Process all of the sensor definitions
    nodeXPath = "/hnode-sep-config/sensor-list/sensor";

    xpathObj = xmlXPathEvalExpression( (const xmlChar *) nodeXPath.c_str(), xpathCtx );
    if( xpathObj == NULL ) 
    {
        fprintf( stderr,"Error: unable to evaluate xpath expression \"%s\"\n", nodeXPath.c_str() );
        return true;
    }
    
    if( xpathObj->nodesetval != NULL )
    {    
        int nodeCnt = xpathObj->nodesetval->nodeNr;

        if( nodeCnt != 0 )
        {
            std::cout << "Result (%d nodes): " << nodeCnt <<std::endl;
            for( unsigned int nodeIndx = 0; nodeIndx < nodeCnt; ++nodeIndx ) 
            {  
                curElem = xpathObj->nodesetval->nodeTab[ nodeIndx ];	

                if( curElem->type == XML_ELEMENT_NODE ) 
                {
                    std::cout << "object Elem: " << curElem->name <<std::endl;
                    parseSensor( doc, curElem );
	            } 
            }
        }
    }

    // Free things.
    xmlXPathFreeObject(xpathObj);

    // Free things
    xmlXPathFreeContext(xpathCtx); 
    xmlFreeDoc(doc);

    return false;
}

bool
HNodeSEPManager::saveConfiguration()
{
    return false;
}

void 
HNodeSEPManager::processCurrentEvents( uint32_t &curTime )
{
    //printf( "processCurrentEvents: %s\n", curTime.getSimpleString().c_str() );
    //printf( "processCurrentEvents: %s\n", curTime.getISOString().c_str() );
    //printf( "hour: %d\n", curTime.getHour() );
    //printf( "minutes: %d\n", curTime.getMinute() );
    //printf( "seconds: %d\n", curTime.getSecond() );

}

void 
HNodeSEPManager::populateContentNodeFromStatusProvider( unsigned int id, RESTContentNode *outNode, std::map< std::string, std::string > paramMap )
{

    switch( id )
    {
        case WXRSRC_STATID_HEALTH:
        {
            char tmpStr[128];

            // Give the root element a tag name
            outNode->setAsObject( "hnode-sep-health" );

            if( localOK == false )
            {
                outNode->setField( "state", "failed" );
                outNode->setField( "msg", localMsg );
            }
            else if( healthOK == false )
            {
                outNode->setField( "state", "failed" );
                outNode->setField( "msg", healthMsg );
            }
            else
            {
                outNode->setField( "state", "running" );
            }

            sprintf( tmpStr, "%lu", healthStatusTime.tv_sec ); 
            outNode->setField( "status-ts-sec", tmpStr );

            sprintf( tmpStr, "%lu", healthStatusTime.tv_usec ); 
            outNode->setField( "status-ts-usec", tmpStr );

            sprintf( tmpStr, "%lu", healthLastMeasurementTime.tv_sec ); 
            outNode->setField( "last-measurement-ts-sec", tmpStr );

            sprintf( tmpStr, "%lu", healthLastMeasurementTime.tv_usec ); 
            outNode->setField( "last-measurement-ts-usec", tmpStr );

            sprintf( tmpStr, "%u", healthMeasurementCount ); 
            outNode->setField( "measurement-count", tmpStr );
        }
        break;

        case WXRSRC_STATID_SENSOR_DEFINITIONS:
        {
            // Give the root element a tag name
            outNode->setAsObject( "hnode-sep-sensor-definitions" );

            // Add a list for any active zones
            RESTContentNode *mList = RESTContentHelperFactory::newContentNode();

            mList->setAsArray( "sensor-definition-list" );
            outNode->addChild( mList );

            // Should we indicate this somehow??
            if( localOK == false )
            {
                break;
            }

            for( std::map< uint32_t, SensorDefinition >::iterator dit = definitions.begin(); dit != definitions.end(); dit++ )
            {
                RESTContentNode         *curNode;
                char                     tmpStr[128];

                curNode = RESTContentHelperFactory::newContentNode();

                curNode->setAsObject( "sensor-definition" );

                curNode->setField( "station-id", stationID );
                curNode->setField( "sensor-id", dit->second.getID() );
                curNode->setField( "name", dit->second.getName() );
                curNode->setField( "desc", dit->second.getDesc() );

                mList->addChild( curNode );
            }
        }
        break;

        case WXRSRC_STATID_CURRENT_READING:
        {

            // Give the root element a tag name
            outNode->setAsObject( "current-measurements" );

            // Add a list for any active zones
            RESTContentNode *mList = RESTContentHelperFactory::newContentNode();

            mList->setAsArray( "measurement-list" );
            outNode->addChild( mList );

            // Should we indicate this somehow??
            if( ( localOK == false ) || ( healthOK == false ) )
            {
                break;
            }

            for( std::map< uint32_t, SensorDefinition >::iterator dit = definitions.begin(); dit != definitions.end(); dit++ )
            {
                RESTContentNode         *curNode;
                HNodeSensorMeasurement   curM;
                struct timeval           tstamp;
                char                     tmpStr[128];

                std::map< uint32_t, SensorMeasurements >::iterator it = measurements.find( dit->first );

                if( it == measurements.end() )
                {
                    continue;
                }
                     
                if( it->second.getCurrentMeasurement( curM ) == false )
                {
                    continue;
                }

                curNode = RESTContentHelperFactory::newContentNode();

                curNode->setAsObject( "measurement" );

                curNode->setField( "station-id", stationID );
                curNode->setField( "sensor-id", dit->second.getID() );

                curNode->setField( "type", curM.getTypeAsStr() );
                curNode->setField( "units", curM.getUnitsAsStr() );

                sprintf( tmpStr, "%d", curM.getCount() ); 
                curNode->setField( "epcount", tmpStr );

                sprintf( tmpStr, "%lf", curM.getReading() ); 
                curNode->setField( "reading", tmpStr );

                curM.getTimestamp( tstamp );

                sprintf( tmpStr, "%lu", tstamp.tv_sec ); 
                curNode->setField( "tstamp-sec", tmpStr );

                sprintf( tmpStr, "%lu", tstamp.tv_usec ); 
                curNode->setField( "tstamp-usec", tmpStr );

                mList->addChild( curNode );
            }

        }
        break;
#if 0
        case SCHRSRC_STATID_EVENTLOG:
        {
            eventLog.populateContentNode( outNode );
        }
        break;

        case SCHRSRC_STATID_CALENDAR:
        {
            ScheduleEventList *eventList;
            ScheduleDateTime   startTime;
            ScheduleDateTime   endTime;

            std::map< std::string, std::string >::iterator it = paramMap.find( "startTime" );
 
            if( it == paramMap.end() )
            { 
                startTime.getCurrentTime();
            }
            else
            {
                startTime.setTimeFromISOString( it->second );
            }

            it = paramMap.find( "endTime" );
 
            if( it == paramMap.end() )
            { 
                endTime.setTime( startTime );
                endTime.addHours( 2 );
            }
            else
            {
                endTime.setTimeFromISOString( it->second );
            }

            eventList = getPotentialEventsForPeriod( startTime, endTime );

            // Give the root element a tag name
            outNode->setAsObject( "schedule-event-calendar" );

            outNode->setField( "period-start", startTime.getISOString() );
            outNode->setField( "period-end", endTime.getISOString() );

            RESTContentNode *evList = RESTContentHelperFactory::newContentNode();

            evList->setAsArray( "event-list" );
            outNode->addChild( evList );

            // Do processing for active rules
            for( unsigned int index = 0; index < eventList->getEventCount(); ++index )
            {
                ScheduleEvent *event = eventList->getEvent( index );

                ScheduleDateTime evStart;
                ScheduleDateTime evEnd;

                event->getStartTime( evStart );
                event->getEndTime( evEnd );

                std::cout << "Event Entry -- ID: " << event->getId() << " Title: " << event->getDescription() << " Start: " << evStart.getISOString() << " End: " << evEnd.getISOString() << std::endl;

                RESTContentNode *curNode = RESTContentHelperFactory::newContentNode();

                curNode->setAsObject( "event" );

                curNode->setField( "id", event->getId() );
                curNode->setField( "start-time", evStart.getISOString() );
                curNode->setField( "end-time", evEnd.getISOString() );
                curNode->setField( "zone-name", event->getZoneRecord().getZoneName() );
                curNode->setField( "trigger-name", event->getTriggerRecord().getTriggerName() );
                curNode->setField( "duration", event->getDurationStr() );  
                curNode->setField( "erID", event->getTriggerRecord().getERID() );
                curNode->setField( "zgID", event->getZoneRecord().getGroupID() );
                curNode->setField( "zrID", event->getZoneRecord().getRuleID() );
                curNode->setField( "tgID", event->getTriggerRecord().getGroupID() );
                curNode->setField( "trID", event->getTriggerRecord().getRuleID() );
          
                evList->addChild( curNode );
            } 

            freeScheduleEventList( eventList );

        }
        break;
#endif

        default:
            std::cerr << "ERROR: Undefined status provider" << std::endl;
            return;
        break;
    }

}

void 
HNodeSEPManager::setLocalHealthError( std::string errMsg )
{
    localOK  = false;
    localMsg = errMsg;
}

void 
HNodeSEPManager::setLocalHealthOK()
{
    localOK = true;
    localMsg.clear();
}

void
HNodeSEPManager::start()
{

}

void 
HNodeSEPManager::addNewMeasurement( uint32_t sensorIndex, HNodeSensorMeasurement &measurement )
{
    std::map< uint32_t, SensorMeasurements >::iterator it;

    it = measurements.find( sensorIndex );

    if( it == measurements.end() )
    {
        SensorMeasurements mtmp;

        measurements.insert( std::pair< uint32_t, SensorMeasurements >( sensorIndex, mtmp ) );

        it = measurements.find( sensorIndex );
    }
    
    it->second.addNewMeasurement( measurement );
}

void 
HNodeSEPManager::updateHealthStatus( bool okflag, struct timeval *statusTime, struct timeval *lastMeasurementTime, uint32_t measurementCount )
{
    healthOK = okflag;

    healthStatusTime.tv_sec  = statusTime->tv_sec;
    healthStatusTime.tv_usec = statusTime->tv_usec;

    healthLastMeasurementTime.tv_sec  = lastMeasurementTime->tv_sec;
    healthLastMeasurementTime.tv_usec = lastMeasurementTime->tv_usec;

    healthMeasurementCount = measurementCount;
}
       
void 
HNodeSEPManager::updateHealthMsg( std::string value )
{
    healthMsg = value;
}

RESTContentNode* 
HNodeSEPManager::newObject( unsigned int type )
{
    return NULL;
}

void 
HNodeSEPManager::freeObject( RESTContentNode *objPtr )
{

}

unsigned int 
HNodeSEPManager::getTypeFromObjectElementName( std::string name )
{
    return 0;
}

RESTContentTemplate *
HNodeSEPManager::getContentTemplateForType( unsigned int type )
{
    return NULL;
}

