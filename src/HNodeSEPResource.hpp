#ifndef __HNODE_SEP_RESOURCE_H__
#define __HNODE_SEP_RESOURCE_H__

#include <hnode-rest.hpp>
#include "HNodeSEPManager.hpp"

class HNodeSEPHealthResource : public RESTResourceRESTStatusProvider
{
    private:

    public:
        HNodeSEPHealthResource( HNodeSEPManager &mgr );
       ~HNodeSEPHealthResource();
};

class HNodeSEPDefinitionResource : public RESTResourceRESTStatusProvider
{
    private:

    public:
        HNodeSEPDefinitionResource( HNodeSEPManager &mgr );
       ~HNodeSEPDefinitionResource();
};

class HNodeSEPCurrentResource : public RESTResourceRESTStatusProvider
{
    private:

    public:
        HNodeSEPCurrentResource( HNodeSEPManager &mgr );
       ~HNodeSEPCurrentResource();
};

#endif //__HNODE_SEP_RESOURCE_H__
