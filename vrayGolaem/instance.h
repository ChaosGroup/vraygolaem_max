/***************************************************************************
*                                                                          *
*  Copyright (C) Chaos Group & Golaem S.A. - All Rights Reserved.          *
*                                                                          *
***************************************************************************/

#ifndef __INSTANCE_H__
#define __INSTANCE_H__

#include "vraygolaem.h"

class VRayGolaemInstanceBase: public VR::VRenderInstance {
protected:
	Mesh dummyMesh; // Dummy 3dsmax mesh
	void freeMem(void);
public:
	VRayGolaemInstanceBase(VRayGolaem *vrayGolaem, INode *node, VR::VRayCore *vray, int renderID);
	virtual ~VRayGolaemInstanceBase(void);

	// From VRenderInstance
	void renderBegin(TimeValue t, VR::VRayCore *vray);
	void frameBegin(TimeValue t, VR::VRayCore *vray);
	void frameEnd(VR::VRayCore *vray);
	void renderEnd(VR::VRayCore *vray);
	void compileGeometry(VR::VRayCore *vray);

	// From RenderInstance
	Interval MeshValidity(void) { return FOREVER; }
	Point3 GetFaceNormal(int faceNum) { return Point3(0,0,1); }
	Point3 GetFaceVertNormal(int faceNum, int vertNum) { return Point3(0,0,1); }
	void GetFaceVertNormals(int faceNum, Point3 n[3]) {}
	Point3 GetCamVert(int vertNum) { return Point3(0,0,0); }
	void GetObjVerts(int fnum, Point3 obp[3]) {}
	void GetCamVerts(int fnum, Point3 cp[3]) {}
};

#endif