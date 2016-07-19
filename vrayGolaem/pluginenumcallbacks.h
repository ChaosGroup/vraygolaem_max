/***************************************************************************
*                                                                          *
*  Copyright (C) Chaos Group & Golaem S.A. - All Rights Reserved.          *
*                                                                          *
***************************************************************************/

#ifndef __PLUGIN_ENUM_CALLBACKS_H__
#define __PLUGIN_ENUM_CALLBACKS_H__

#include "plugman.h"
#include "vrayplugins.h"

// A callback that enumerates V-Ray plugins by making sure that plugin dependencies are evaluated first.
struct EnumPluginsCB: EnumPluginCallback {
	EnumPluginsCB(VR::VRayCore *vray, double t):renderer(vray), time(t) {}
	
	int process(Plugin *plugin) VRAY_OVERRIDE {
		return processBase(static_cast<PluginBase*>(plugin));
	}

	int processBase(PluginBase *plugin) {
		// Check if this is a V-Ray plugin
		VR::VRayPlugin *vrayPlugin=static_cast<VR::VRayPlugin*>(GET_INTERFACE(plugin, EXT_VRAY_PLUGIN));
		if (!vrayPlugin) return true;

		// Check if this plugin is already processed
		InstanceID instanceID=vrayPlugin->getInstanceID();
		if (processedPlugins.find(instanceID)!=processedPlugins.end()) return true;
		processedPlugins.insert(instanceID, false);

		// First process any other plugins used by this one
		VR::VRayParameterListDesc *paramDesc=vrayPlugin->pluginDesc->parameters;

		int numParams=vrayPlugin->paramList->getNumParams();
		for (int i=0; i<numParams; i++) {
			VR::VRayPluginParameter *param=vrayPlugin->paramList->getParam(i);
			if (!param) continue;

			VR::VRayParameterType paramType=paramDesc->params[i].type;
			if (paramType < VR::paramtype_object || paramType > VR::paramtype_texture_transform) continue;

			int count=param->getCount(time);
			if (count<0) {
				PluginBase *p=param->getObject(0, time);
				if (p) processBase(p);
			} else {
				for (int i=0; i<count; i++) {
					PluginBase *p=param->getObject(i, time);
					if (p) processBase(p);
				}
			}
		}

		// And finally, process the plugin
		proc(vrayPlugin);
		return true;
	}

	virtual void proc(VR::VRayPlugin *plugin)=0;
protected:
	VR::VRayCore *renderer;
	double time;
private:
	VR::HashSet<InstanceID> processedPlugins;
};

struct PreRenderBeginCB: EnumPluginCallback {
	PreRenderBeginCB(VR::VRayCore *vray):renderer(vray) {}
	int process(Plugin *plugin) VRAY_OVERRIDE {
		VR::VRaySceneModifierInterface *sceneMod=static_cast<VR::VRaySceneModifierInterface*>(GET_INTERFACE(plugin, EXT_SCENE_MODIFIER));
		if (sceneMod) {
			try {
				sceneMod->preRenderBegin(static_cast<VR::VRayRenderer*>(renderer));
			}
			catch (...) {}
		}
		return true;
	}
protected:
	VR::VRayCore *renderer;
};

struct PostRenderEndCB: EnumPluginCallback {
	PostRenderEndCB(VR::VRayCore *vray):renderer(vray) {}
	int process(Plugin *plugin) VRAY_OVERRIDE {
		VR::VRaySceneModifierInterface *sceneMod=static_cast<VR::VRaySceneModifierInterface*>(GET_INTERFACE(plugin, EXT_SCENE_MODIFIER));
		if (sceneMod) {
			try {
				sceneMod->postRenderEnd(static_cast<VR::VRayRenderer*>(renderer));
			}
			catch (...) {}
		}
		return true;
	}
protected:
	VR::VRayCore *renderer;
};

struct RenderBeginCB: EnumPluginsCB {
	RenderBeginCB(VR::VRayCore *vray, double t):EnumPluginsCB(vray, t) {}
	void proc(VR::VRayPlugin *plugin) VRAY_OVERRIDE {
		try { plugin->renderBegin(static_cast<VR::VRayRenderer*>(renderer)); }
		catch (...) {}
	}
};

struct RenderEndCB: EnumPluginsCB {
	RenderEndCB(VR::VRayCore *vray, double t):EnumPluginsCB(vray, t) {}
	void proc(VR::VRayPlugin *plugin) VRAY_OVERRIDE {
		try { plugin->renderEnd(static_cast<VR::VRayRenderer*>(renderer)); }
		catch (...) {}
	}
};

struct FrameBeginCB: EnumPluginsCB {
	FrameBeginCB(VR::VRayCore *vray, double t):EnumPluginsCB(vray, t) {}

	void proc(VR::VRayPlugin *plugin) VRAY_OVERRIDE {
		try { plugin->frameBegin(static_cast<VR::VRayRenderer*>(renderer)); }
		catch (...) {}
	}
};

struct PreFrameBeginCB: EnumPluginCallback {
	PreFrameBeginCB(VR::VRayCore *vray):renderer(vray) {}
	int process(Plugin *plugin) VRAY_OVERRIDE {
		VR::VRaySceneModifierInterface *sceneMod=static_cast<VR::VRaySceneModifierInterface*>(GET_INTERFACE(plugin, EXT_SCENE_MODIFIER));
		if (sceneMod) {
			try { sceneMod->preFrameBegin(static_cast<VR::VRayRenderer*>(renderer)); }
			catch (...) {}
		}
		return true;
	}
protected:
	VR::VRayCore *renderer;
};

struct PostFrameEndCB: EnumPluginCallback {
	PostFrameEndCB(VR::VRayCore *vray):renderer(vray) {}
	int process(Plugin *plugin) VRAY_OVERRIDE {
		VR::VRaySceneModifierInterface *sceneMod=static_cast<VR::VRaySceneModifierInterface*>(GET_INTERFACE(plugin, EXT_SCENE_MODIFIER));
		if (sceneMod) sceneMod->postFrameEnd(static_cast<VR::VRayRenderer*>(renderer));
		return true;
	}
protected:
	VR::VRayCore *renderer;
};

struct FrameEndCB: EnumPluginsCB {
	FrameEndCB(VR::VRayCore *vray, double t):EnumPluginsCB(vray, t) {}
	void proc(VUtils::VRayPlugin *plugin) VRAY_OVERRIDE {
		plugin->frameEnd(static_cast<VR::VRayRenderer*>(renderer));
	}
};

struct CompileGeometryCB: EnumPluginCallback {
	VR::VRayCore *renderer;

	CompileGeometryCB(VR::VRayCore *vray):renderer(vray) {}
	
	int process(Plugin *plugin) VRAY_OVERRIDE {
		VR::GeomGenInterface *geom=static_cast<VR::GeomGenInterface*>(GET_INTERFACE(plugin, EXT_GEOM_GEN));
		if (geom) {
			try {
				const VR::VRaySequenceData &sdata=renderer->getSequenceData();
				if (sdata.progress) {
					VR::VRayPlugin *vrayPlugin=queryInterface<VR::VRayPlugin>(plugin, EXT_VRAY_PLUGIN);
					if (vrayPlugin) {
						const tchar *name=vrayPlugin->getPluginName();
						if (name) {
							sdata.progress->debug("VRayGolaem: Compiling geometry for plugin \"%s\"", name);
						}
					}
				}
				geom->compileGeometry(static_cast<VR::VRayRenderer*>(renderer));
			}
			catch (...) {
				// The .vrscene parser may throw exceptions here if there are invalid parameters in the .vrscene
			}
		}	
		return true;
	}
};

struct ClearGeometryCB: EnumPluginCallback {
	VR::VRayCore *renderer;
	ClearGeometryCB(VR::VRayCore *vray):renderer(vray) {}
	int process(Plugin *plugin) VRAY_OVERRIDE {
		VR::GeomGenInterface *geom=static_cast<VR::GeomGenInterface*>(GET_INTERFACE(plugin, EXT_GEOM_GEN));
		if (geom) geom->clearGeometry(static_cast<VR::VRayRenderer*>(renderer));
		return true;
	}
};

class VRayGolaem;

/// A helper class that temporarily adds a VRayGolaem plugin as VRayPluginRendererInterface to the given VRayCore object. This is because
/// in 3ds Max, the VRayCore does not implement this interface (for now, will be implemented later on), so we need to provide one ourselves.
/// The interface is added in the constructor and removed in the destructor.
struct PluginRendererInterfaceRAII {
	PluginRendererInterfaceRAII(VR::VRayCore *vrayCore, VRayGolaem *glm):vray(vrayCore), vrayGolaem(glm), addedInterface(false) {
		VR::VRayPluginRendererInterface *pluginRenderer=static_cast<VR::VRayPluginRendererInterface*>(GET_INTERFACE(vray, EXT_PLUGIN_RENDERER));
		if (!pluginRenderer) {
			// There is no plugin renderer; we need to add ourselves
			vray->addInterface(static_cast<VR::VRayPluginRendererInterface*>(vrayGolaem), false);
			addedInterface=true;
		}
	}

	~PluginRendererInterfaceRAII(void) {
		if (addedInterface) {
			vray->removeInterface(static_cast<VR::VRayPluginRendererInterface*>(vrayGolaem));
		}
	}
protected:
	int addedInterface;
	VR::VRayCore *vray;
	VRayGolaem *vrayGolaem;
};

/// A helper class that temporarily modifies the current time/frame start/frame end in the frame data of the renderer.
/// This is needed because in 3ds Max the values are in 3ds Max ticks (there are 4800 ticks for one second of animation),
/// whereas V-Ray for Maya assumes that the values are in frames. The values are converted in the constructor and restored
/// in the destructor.
struct TimeConversionRAII {
	TimeConversionRAII(VR::VRayCore *vrayCore):vray(vrayCore) {
		const VR::VRayFrameData &fdata=vray->getFrameData();
		const VR::VRaySequenceData &sdata=vray->getSequenceData();

		VR::VRayFrameData &cfdata=const_cast<VR::VRayFrameData&>(fdata);
		VR::VRaySequenceData &csdata=const_cast<VR::VRaySequenceData&>(sdata);

		maxTime=cfdata.t;
		maxFrameStart=cfdata.frameStart;
		maxFrameEnd=cfdata.frameEnd;
		maxDuration=csdata.params.moblur.duration;
		maxCenter=csdata.params.moblur.intervalCenter;

		double ticksPerFrame=(double) GetTicksPerFrame();

		cfdata.t=maxTime/ticksPerFrame;
		cfdata.frameStart=maxFrameStart/ticksPerFrame;
		cfdata.frameEnd=maxFrameEnd/ticksPerFrame;

		csdata.params.moblur.duration=(float)(cfdata.frameEnd-cfdata.frameStart);
		csdata.params.moblur.intervalCenter=(float)((cfdata.frameStart+cfdata.frameEnd)*0.5f-cfdata.t);
	}

	~TimeConversionRAII(void) {
		const VR::VRayFrameData &fdata=vray->getFrameData();
		const VR::VRaySequenceData &sdata=vray->getSequenceData();

		VR::VRayFrameData &cfdata=const_cast<VR::VRayFrameData&>(fdata);
		VR::VRaySequenceData &csdata=const_cast<VR::VRaySequenceData&>(sdata);

		cfdata.t=maxTime;
		cfdata.frameStart=maxFrameStart;
		cfdata.frameEnd=maxFrameEnd;

		csdata.params.moblur.duration=(float)maxDuration;
		csdata.params.moblur.intervalCenter=(float)maxCenter;
	}
protected:
	double maxTime, maxFrameStart, maxFrameEnd, maxDuration, maxCenter;
	VR::VRayCore *vray;
};

#endif