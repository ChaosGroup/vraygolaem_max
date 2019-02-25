/***************************************************************************
*                                                                          *
*  Copyright (C) Chaos Group & Golaem S.A. - All Rights Reserved.          *
*                                                                          *
***************************************************************************/

#include "instance.h"

#include "vraygolaem.h"

using namespace VR;

VRayGolaemInstanceBase::VRayGolaemInstanceBase(VRayGolaem* _vrayGolaem, INode* node, VRayCore* vray, int renderID)
{
    VRenderInstance::init(_vrayGolaem, node, vray, renderID);
}

void VRayGolaemInstanceBase::freeMem(void)
{
}

VRayGolaemInstanceBase::~VRayGolaemInstanceBase(void)
{
    freeMem();
}

void VRayGolaemInstanceBase::renderBegin(TimeValue t, VRayCore* vray)
{
    VRenderInstance::renderBegin(t, vray);
}

void VRayGolaemInstanceBase::renderEnd(VRayCore* vray)
{
    VRenderInstance::renderEnd(vray);
}

void VRayGolaemInstanceBase::frameBegin(TimeValue t, VRayCore* vray)
{
    VRenderInstance::frameBegin(t, vray);
    mesh = &dummyMesh;
}

void VRayGolaemInstanceBase::frameEnd(VRayCore* vray)
{
    VRenderInstance::frameEnd(vray);
    if (!renderObject)
        return;
}

void VRayGolaemInstanceBase::compileGeometry(VR::VRayCore* /*vray*/)
{
    if (!renderObject)
        return;
}
