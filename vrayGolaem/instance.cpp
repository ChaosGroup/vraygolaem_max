//
// Copyright (C) Chaos Group & Golaem S.A. - All Rights Reserved.
//

#include "instance.h"
#include "vraygolaem.h"

#include <tomax.h>
#include <defparams.h>
#include <vray_plugins_ids.h>

using namespace VUtils;

static const PluginID GolaemMeshInstance_PluginID(LARGE_CONST(2011070866));
static Interval validForever(FOREVER);

VRayGolaemInstance::VRayGolaemInstance(VRayGolaem &vrayGolaem, INode *node, VRayCore *vray, int renderID)
	: vrayGolaem(vrayGolaem)
	, paramTransform(NULL)
	, paramFrameOffset(NULL)
{
	init(&vrayGolaem, node, vray, renderID);

	// Set dummy mesh
	mesh = &dummyMesh;
}

static Transform getTransform(INode *inode, TimeValue t)
{
	vassert(inode);
	return toTransform(inode->GetObjectTM(t, &validForever) * maxToGolaem());
}

void VRayGolaemInstance::frameBegin(TimeValue t, VRayCore *vray)
{
	VRenderInstance::frameBegin(t, vray);

	// Could be NULL if plugin DSO was not found.
	if (!paramTransform || !paramFrameOffset)
		return;

	vrayGolaem.updateVRayParams(t);

	TimeConversionRAII timeConversion(*vray);
	const double time = vray->getFrameData().t;

	VRaySettableParamInterface *settableTransform =
		queryInterface<VRaySettableParamInterface>(paramTransform, EXT_SETTABLE_PARAM);
	vassert(settableTransform);

	VRaySettableParamInterface *settableFrameOffset =
		queryInterface<VRaySettableParamInterface>(paramFrameOffset, EXT_SETTABLE_PARAM);
	vassert(settableFrameOffset);

	const Transform &tm = getTransform(node, t);
	settableTransform->setTransform(tm, 0, time);
	
	const float frameOffset = vrayGolaem.getCurrentFrameOffset(t);
	settableFrameOffset->setFloat(frameOffset, 0, time);
}

void VRayGolaemInstance::newVRayPlugin(VRayCore &vray)
{
	const VRaySequenceData &sdata = vray.getSequenceData();

	VRenderPluginRendererInterface *pluginRenderer =
		queryInterface<VRenderPluginRendererInterface>(vray, EXT_VRENDER_PLUGIN_RENDERER);
	vassert(pluginRenderer);

	const TimeValue t = GetCOREInterface()->GetTime();

	GET_MBCS(node->GetName(), nodeName);

	// Correct the name of the shader to call. When exporting a scene from Maya with Vray,
	// some shader name special characters are replaced with not parsable character (":" => "__")
	// to be able to find the correct shader name to call, we need to apply the same conversion
	// to the shader names contained in the cam file
	CStr correctedCacheName(vrayGolaem._cacheName);
	convertToValidVrsceneName(vrayGolaem._cacheName, correctedCacheName);

	VRayPlugin *vrayGolaemPlugin =
		pluginRenderer->newPlugin(GolaemMeshInstance_PluginID, correctedCacheName.data());
	if (!vrayGolaemPlugin) {
		if (sdata.progress) {
			sdata.progress->error("VRayGolaemInstance: Failed to create GolaemCrowd plugin instance!");
		}
		return;
	}

	VRayPlugin *nodePlugin =
		pluginRenderer->newPlugin(PluginId::Node, nodeName);
	vassert(nodePlugin);

	// XXX: Some dummy material may require.
	// nodePlugin->setParameter(pluginRenderer->newPluginParam("material", NULL));
	nodePlugin->setParameter(pluginRenderer->newBoolParam("visible", true));
	nodePlugin->setParameter(pluginRenderer->newTransformParam("transform", TraceTransform(1)));
	nodePlugin->setParameter(pluginRenderer->newPluginParam("geometry", vrayGolaemPlugin));

	// Transform could be updated in frameBegin().
	const Transform &tm = getTransform(node, t);
	paramTransform = pluginRenderer->newTransformParam("proxyMatrix", TraceTransform(tm));
	vrayGolaemPlugin->setParameter(paramTransform);

	const float frameOffset = vrayGolaem.getCurrentFrameOffset(t);
	paramFrameOffset = pluginRenderer->newFloatParam("frameOffset", frameOffset);
	vrayGolaemPlugin->setParameter(paramFrameOffset);

	vrayGolaemPlugin->setParameter(pluginRenderer->newBoolParam("cameraVisibility", vrayGolaem._primaryVisibility));
	vrayGolaemPlugin->setParameter(pluginRenderer->newBoolParam("dccPackage", true));
	vrayGolaemPlugin->setParameter(pluginRenderer->newBoolParam("frustumCullingEnable", vrayGolaem._frustumEnable));
	vrayGolaemPlugin->setParameter(pluginRenderer->newBoolParam("instancingEnable", vrayGolaem._instancingEnable));
	vrayGolaemPlugin->setParameter(pluginRenderer->newBoolParam("layoutEnable", vrayGolaem._layoutEnable));
	vrayGolaemPlugin->setParameter(pluginRenderer->newBoolParam("motionBlurEnable", vrayGolaem._mBlurEnable));
	vrayGolaemPlugin->setParameter(pluginRenderer->newBoolParam("reflectionsVisibility", vrayGolaem._visibleInReflections));
	vrayGolaemPlugin->setParameter(pluginRenderer->newBoolParam("refractionsVisibility", vrayGolaem._visibleInRefractions));
	vrayGolaemPlugin->setParameter(pluginRenderer->newBoolParam("shadowsVisibility", vrayGolaem._castsShadows));

	vrayGolaemPlugin->setParameter(pluginRenderer->newIntParam("geometryTag", vrayGolaem._geometryTag));
	vrayGolaemPlugin->setParameter(pluginRenderer->newIntParam("objectIdBase", vrayGolaem._objectIDBase));
	vrayGolaemPlugin->setParameter(pluginRenderer->newIntParam("objectIdMode", vrayGolaem._objectIDMode));

	vrayGolaemPlugin->setParameter(pluginRenderer->newFloatParam("cameraMargin", vrayGolaem._cameraMargin));
	vrayGolaemPlugin->setParameter(pluginRenderer->newFloatParam("frustumMargin", vrayGolaem._frustumMargin));
	vrayGolaemPlugin->setParameter(pluginRenderer->newFloatParam("renderPercent", vrayGolaem._displayPercent));

	vrayGolaemPlugin->setParameter(pluginRenderer->newStringParam("cacheFileDir", vrayGolaem._cacheDir.data()));
	vrayGolaemPlugin->setParameter(pluginRenderer->newStringParam("cacheName", vrayGolaem._cacheName.data()));
	vrayGolaemPlugin->setParameter(pluginRenderer->newStringParam("characterFiles", vrayGolaem._characterFiles.data()));
	vrayGolaemPlugin->setParameter(pluginRenderer->newStringParam("crowdField", vrayGolaem._crowdFields.data()));
	vrayGolaemPlugin->setParameter(pluginRenderer->newStringParam("defaultMaterial", vrayGolaem._defaultMaterial.data()));
	vrayGolaemPlugin->setParameter(pluginRenderer->newStringParam("layoutFile", vrayGolaem._layoutFile.data()));
	vrayGolaemPlugin->setParameter(pluginRenderer->newStringParam("proxyName", nodeName));
	vrayGolaemPlugin->setParameter(pluginRenderer->newStringParam("terrainFile", vrayGolaem._terrainFile.data()));

	if (vrayGolaem._overMBlurWindowSize) {
		vrayGolaemPlugin->setParameter(pluginRenderer->newBoolParam("motionBlurWindowSize", vrayGolaem._mBlurWindowSize));
	}
	if (vrayGolaem._overMBlurSamples) {
		vrayGolaemPlugin->setParameter(pluginRenderer->newBoolParam("motionBlurSamples", vrayGolaem._mBlurSamples));
	}
}
