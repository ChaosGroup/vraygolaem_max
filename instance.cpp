#include "instance.h"
#include "tomax.h"
#include "misc_ray.h"
#include "plugman.h"
#include "factory.h"
#include "vrayplugins.h"

using namespace VR;

PluginManager *golaemPlugman=NULL; // We need this to store the instance of the Golaem plugin
Factory *golaemFactory=NULL; // Factory to hold plugin parameters

VRayGolaemInstanceBase::VRayGolaemInstanceBase(VRayGolaem *_vrayGolaem, INode *node, VRayCore *vray, int renderID) {
	VRenderInstance::init(_vrayGolaem, node, vray, renderID);
}

void VRayGolaemInstanceBase::freeMem(void) {
}

VRayGolaemInstanceBase::~VRayGolaemInstanceBase(void) {
	freeMem();
}

void VRayGolaemInstanceBase::renderBegin(TimeValue t, VRayCore *vray) {
	VRenderInstance::renderBegin(t, vray);
}

void VRayGolaemInstanceBase::renderEnd(VRayCore *vray) {
	VRenderInstance::renderEnd(vray);
}

void VRayGolaemInstanceBase::frameBegin(TimeValue t, VRayCore *vray) {
	VRenderInstance::frameBegin(t, vray);
	mesh=&dummyMesh;
}

void VRayGolaemInstanceBase::frameEnd(VRayCore *vray) {
	VRenderInstance::frameEnd(vray);
	if (!renderObject) return;
	static_cast<VRayGolaem*>(renderObject)->clearGeometry(vray);
}

void VRayGolaemInstanceBase::compileGeometry(VR::VRayCore *vray) {
	if (!renderObject) return;
	static_cast<VRayGolaem*>(renderObject)->compileGeometry(vray);
}

