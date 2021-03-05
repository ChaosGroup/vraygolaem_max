/***************************************************************************
*                                                                          *
*  Copyright (C) Chaos Group & Golaem S.A. - All Rights Reserved.          *
*                                                                          *
***************************************************************************/

#pragma once

#pragma warning(push)
#pragma warning(disable : 4840 4458 )
#include "max.h"
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable : 4100 4189 4127 4201 4244 4251 4324 4389 4456 4457 4458 4512 4505 4535 4996)
#include "vraygeom.h"
#include <vrender_plugin_renderer_interface.h>
#pragma warning(pop)

class VRayGolaem;

struct VRayGolaemInstance
    : VUtils::VRenderInstance
{
    VRayGolaemInstance(VRayGolaem& vrayGolaem, INode* node, VUtils::VRayCore* vray, int renderID);
    virtual ~VRayGolaemInstance()
    {
    }

    /// Adds new GolaemCrowd plugin instance.
    /// @param vray V-Ray renderer instance for getting EXT_VRENDER_PLUGIN_RENDERER.
    void newVRayPlugin(VUtils::VRayCore& vray);

    // From VRenderInstance
    void frameBegin(TimeValue t, VR::VRayCore* vray) VRAY_OVERRIDE;
    void compileGeometry(VR::VRayCore*) VRAY_OVERRIDE
    {
    }

    // From RenderInstance
    Interval MeshValidity() VRAY_OVERRIDE
    {
        return FOREVER;
    }
    Point3 GetFaceNormal(int /*faceNum*/) VRAY_OVERRIDE
    {
        return Point3(0, 0, 1);
    }
    Point3 GetFaceVertNormal(int /*faceNum*/, int /*vertNum*/) VRAY_OVERRIDE
    {
        return Point3(0, 0, 1);
    }
    void GetFaceVertNormals(int /*faceNum*/, Point3 /*n*/[3]) VRAY_OVERRIDE
    {
    }
    Point3 GetCamVert(int /*vertNum*/) VRAY_OVERRIDE
    {
        return Point3(0, 0, 0);
    }
    void GetObjVerts(int /*fnum*/, Point3 /*obp*/[3]) VRAY_OVERRIDE
    {
    }
    void GetCamVerts(int /*fnum*/, Point3 /*cp*/[3]) VRAY_OVERRIDE
    {
    }

private:
    /// Object ref.
    VRayGolaem& vrayGolaem;

    /// Plugin parameters for update in frameBegin()
    VUtils::VRayPluginParameter* paramTransform;
    VUtils::VRayPluginParameter* paramFrameOffset;

    /// Dummy 3dsmax mesh.
    Mesh dummyMesh;
};
