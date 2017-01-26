#include <string>
#include <iostream>
#include <fstream>
#include <cerrno>

#include <hnode-rest.hpp>
#include "HNodeSEPManager.hpp"
#include "HNodeSEPResource.hpp"

WeatherRootResource::WeatherRootResource( Acurite5N1Manager &mgr )
:RESTResourceRESTStatusProvider( "/measurements/current", mgr, WXRSRC_STATID_CURRENT_READING )
{

}

WeatherRootResource::~WeatherRootResource()
{

}


