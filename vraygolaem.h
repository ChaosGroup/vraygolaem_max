#ifndef __INFINITEPLANE_H__
#define __INFINITEPLANE_H__

#include "max.h"
#include <bmmlib.h>
#include "iparamm2.h"
#include "render.h"  
#include "texutil.h"
#include "gizmo.h"
#include "gizmoimp.h"
#include "istdplug.h"

#include "utils.h"
#include "vraygeom.h"
#include "rayserver.h"
#include "plugman.h"
#include "vrayplugins.h"
#include "defparams.h"
#include "factory.h"
#include "vraysceneplugman.h"
#include "vrender_unicode.h"

//************************************************************
// #defines
//************************************************************

#define	PLUGIN_CLASSID Class_ID(0xece3ed9, 0x1256b63)

#define STR_CLASSNAME _T("VRayGolaem")
#define STR_INTERNALNAME _T("VRayGolaem")
#define STR_LIBDESC _T("VRay-Golaem example")
#define STR_DLGTITLE _T("VRayGolaem Parameters")

#define REFNO_PBLOCK 0

// Paramblock2 parameter list
enum {
	pb_file,
	pb_shaders_file,
};

//************************************************************
// The VRayGolaem 3dsmax object
//************************************************************

class VRayGolaem: public GeomObject, public VR::VRenderObject, public VR::VRayPluginRendererInterface {
	// An empty dummy mesh returned from GetRenderMesh()
	Mesh mesh;
	VR::VRayScene *vrayScene; // The loaded .vrscene file
	VR::CharString vrsceneFile, shadersFile;

	void callRenderBegin(VR::VRayCore *vray);
	void callRenderEnd(VR::VRayCore *vray);

	void callFrameBegin(VR::VRayCore *vray);
	void callFrameEnd(VR::VRayCore *vray);

	void compileGeometry(VR::VRayCore *vray);
	void clearGeometry(VR::VRayCore *vray);

	void updateVRayParams(TimeValue t, VR::VRayCore *vray);

	friend class VRayGolaemInstanceBase;
public:
	IParamBlock2 *pblock2;
	IParamMap2 *pmap;
	static IObjParam *ip;

	// Snap suspension flag (TRUE during creation only)
	BOOL suspendSnap;
				
	float simple;
 	int extDispFlags;

	VRayGolaem(void);
	~VRayGolaem(void);
	
	// From BaseObject
	int IsRenderable() { return true; }
	int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt);
	void Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt);
	void SetExtendedDisplay(int flags);
	int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
	CreateMouseCallBack* GetCreateMouseCallBack();

	void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);
	void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
	void InvalidateUI();

	// From Object
	ObjectState Eval(TimeValue time);

	void InitNodeName(TSTR& s) { s=STR_CLASSNAME; }
	ObjectHandle ApplyTransform(Matrix3& matrix) { return this; }
	Interval ObjectValidity(TimeValue t);

	// We don't convert to anything
	int CanConvertToType(Class_ID obtype) { return FALSE; }
	Object* ConvertToType(TimeValue t, Class_ID obtype) { assert(0);return NULL; }
	
	void GetWorldBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box);
	void GetLocalBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box);
	void GetDeformBBox(TimeValue t, Box3 &b, Matrix3 *tm, BOOL useSel);
	int DoOwnSelectHilite()	{ return 1; }

	// From GeomObject
	Mesh* GetRenderMesh(TimeValue t, INode *inode, View& view, BOOL& needDelete);

	// Animatable methods
	void DeleteThis() { delete this; }
	Class_ID ClassID() { return PLUGIN_CLASSID; }
	void GetClassName(TSTR& s) { s=_T("VRayGolaem"); }
	int IsKeyable() { return 0; }
	void* GetInterface(ULONG id);
	void ReleaseInterface(ULONG id, void *ip);
	
	// Direct paramblock access
	int	NumParamBlocks() { return 1; }	
	IParamBlock2* GetParamBlock(int i) { return pblock2; }
	IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock2->ID() == id) ? pblock2 : NULL; }

	int NumSubs() { return 1; }  
	Animatable* SubAnim(int i);
	TSTR SubAnimName(int i);

	// From ref
 	int NumRefs() { return 1; }
	RefTargetHandle GetReference(int i);
	void SetReference(int i, RefTargetHandle rtarg);

#if GET_MAX_RELEASE(VERSION_3DSMAX) < 8900
	RefTargetHandle Clone(RemapDir& remap=NoRemap());
#else
	RefTargetHandle Clone(RemapDir& remap=DefaultRemapDir());
#endif
	RefResult NotifyRefChanged(NOTIFY_REF_CHANGED_ARGS);

	// From VRenderObject
	int init(const ObjectState &os, INode *node, VR::VRayCore *vray);
	VR::VRenderInstance* newRenderInstance(INode *node, VR::VRayCore *vray, int renderID);
	void deleteRenderInstance(VR::VRenderInstance *ri);
	void renderBegin(TimeValue t, VR::VRayCore *vray);
	void renderEnd(VR::VRayCore *vray);
	void frameBegin(TimeValue t, VR::VRayCore *vray);
	void frameEnd(VR::VRayCore *vray);

	// Other methods
	void draw(TimeValue t, INode *node, ViewExp *vpt);

	// From VRayPluginRendererInterface
	PluginManager* getPluginManager(void);
	PluginBase* getPlugin(void) { return NULL; }
};

class VRayGolaemCreateCallBack: public CreateMouseCallBack {
	VRayGolaem *sphere;
	IPoint2 sp0;
	Point3 p0;
public:
	int proc(ViewExp *vpt, int msg, int point, int flags, IPoint2 m, Matrix3& mat);
	void SetObj(VRayGolaem *obj) { sphere=obj; }
};

extern PluginManager *golaemPlugman; // We need this to store the instance of the Golaem plugin

#endif