#include <string>
#include <iostream>
#include <fstream>
#include <cerrno>

#include <hnode-rest.hpp>
#include "HNodeSEPManager.hpp"
#include "HNodeSEPResource.hpp"

HNodeSEPHealthResource::HNodeSEPHealthResource( HNodeSEPManager &mgr )
:RESTResourceRESTStatusProvider( "/health", mgr, WXRSRC_STATID_HEALTH )
{

}

HNodeSEPHealthResource::~HNodeSEPHealthResource()
{

}

HNodeSEPDefinitionResource::HNodeSEPDefinitionResource( HNodeSEPManager &mgr )
:RESTResourceRESTStatusProvider( "/sensors/definitions", mgr, WXRSRC_STATID_SENSOR_DEFINITIONS )
{

}

HNodeSEPDefinitionResource::~HNodeSEPDefinitionResource()
{

}

HNodeSEPCurrentResource::HNodeSEPCurrentResource( HNodeSEPManager &mgr )
:RESTResourceRESTStatusProvider( "/sensors/current", mgr, WXRSRC_STATID_CURRENT_READING )
{

}

HNodeSEPCurrentResource::~HNodeSEPCurrentResource()
{

}

