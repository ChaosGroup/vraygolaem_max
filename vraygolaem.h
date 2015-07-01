#ifndef __VRAYGOLAEM_H__
#define __VRAYGOLAEM_H__

#pragma warning( push )
#pragma warning( disable : 4100 4251 4275 )

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
	pb_use_node_attributes,
	// cache
	pb_crowd_fields,
	pb_cache_name,
	pb_cache_dir,
	pb_character_files,
	// motion blur
	pb_motion_blur_enable,
	pb_motion_blur_start,
	pb_motion_blur_window_size,
	pb_motion_blur_samples,
	// culling
	pb_frustum_enable,
	pb_frustum_margin,
	pb_camera_margin,
	// vray 
	pb_frame_offset,
	pb_scale_transform,
	pb_object_id_base,
	pb_primary_visibility,
	pb_casts_shadows,
	pb_visible_in_reflections,
	pb_visible_in_refractions,
	// output
	pb_temp_vrscene_file_dir,
};

//************************************************************
// The VRayGolaem 3dsmax object
//************************************************************

class VRayGolaem: public GeomObject, public VR::VRenderObject, public VR::VRayPluginRendererInterface {
	// An empty dummy mesh returned from GetRenderMesh()
	Mesh _mesh;
	VR::VRayScene *_vrayScene; // The loaded .vrscene file
	VR::CharString _vrsceneFile, _shadersFile;
	bool _useNodeAttributes;  // if true, uses the attributes below to generate the vrscene, else use the loaded vrscene

	// Cache attributes
	VR::CharString _crowdFields;
	VR::CharString _cacheName;
	VR::CharString _cacheDir;
	VR::CharString _characterFiles;

	// MoBlur attributes
	bool _mBlurEnable;
	float _mBlurStart;
	float _mBlurWindowSize;
	int _mBlurSamples;

	// Culling attributes
	bool _frustumEnable;
	float _frustumMargin;
	float _cameraMargin;

	// Vray attributes
	int _frameOffset;
	int _objectIDBase;
	float _scaleTransform;
	bool _primaryVisibility;
	bool _castsShadows;
	bool _visibleInReflections;
	bool _visibleInRefractions;

	// Output attributes
	VR::CharString _tempVRSceneFileDir;

	void callRenderBegin(VR::VRayCore *vray);
	void callRenderEnd(VR::VRayCore *vray);

	void callFrameBegin(VR::VRayCore *vray);
	void callFrameEnd(VR::VRayCore *vray);

	void compileGeometry(VR::VRayCore *vray);
	void clearGeometry(VR::VRayCore *vray);

	void updateVRayParams(TimeValue t, VR::VRayCore *vray);

	friend class VRayGolaemInstanceBase;
	friend class VRayGolaemDlgProc;

protected:
	bool readCrowdVRScene(const VR::CharString& file);
	bool writeCrowdVRScene(const VR::CharString& file);

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
	void drawEntityPositions(GraphicsWindow *gw, TimeValue t);

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

bool isCharInvalidVrscene(tchar c);
void convertToValidVrsceneName(const VR::CharString& strIn, VR::CharString& strOut);

#endif // __VRAYGOLAEM_H__