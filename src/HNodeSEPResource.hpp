#ifndef __HNODE_SEP_RESOURCE_H__
#define __HNODE_SEP_RESOURCE_H__

#include <hnode-rest.hpp>
#include "HNodeSEPManager.hpp"

class WeatherRootResource : public RESTResourceRESTStatusProvider
{
    private:

    public:
        WeatherRootResource( Acurite5N1Manager &mgr );
       ~WeatherRootResource();
};

#endif //__HNODE_SEP_RESOURCE_H__
