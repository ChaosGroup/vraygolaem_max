/***************************************************************************
*                                                                          *
*  Copyright (C) Chaos Group & Golaem S.A. - All Rights Reserved.          *
*                                                                          *
***************************************************************************/

#pragma once

#pragma warning( push )
#pragma warning( disable : 4840)

#include "max.h"

#pragma warning ( pop )

#pragma warning( push )
#pragma warning( disable : 4100 4251 4275 4996 4512 4201 4244 4189 4389 4245 4127 4458 4457)

#include "utils.h"
#include "rayserver.h"
#include "plugman.h"
#include "vrayplugins.h"
#include "vraygeom.h"

#include <vrender_plugin_renderer_brdf_wrapper.h>

#include "vraysceneplugman.h"
#include <vrender_plugin_renderer_interface.h>
#include "pb2template_generator.h"

#pragma warning ( pop )

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
enum param_list{
	pb_file,
	pb_shaders_file,
	pb_use_node_attributes,		// Not used anymore but kept for retrocomp
	// display
	pb_enable_display,
	pb_display_percent,			// Not used anymore but kept for retrocomp
	pb_display_entity_ids,
	// cache
	pb_crowd_fields,
	pb_cache_name,
	pb_cache_dir,
	pb_character_files,
	// motion blur				
	pb_motion_blur_enable,		// Not used anymore but kept for retrocomp
	pb_motion_blur_start,		// Not used anymore but kept for retrocomp
	pb_motion_blur_window_size,	// Not used anymore but kept for retrocomp
	pb_motion_blur_samples,		// Not used anymore but kept for retrocomp
	// culling
	pb_frustum_enable,
	pb_frustum_margin,
	pb_camera_margin,
	// vray 
	pb_frame_offset,
	pb_scale_transform,			// Not used anymore but kept for retrocomp
	pb_object_id_base,			// Not used anymore but kept for retrocomp
	pb_primary_visibility,		// Not used anymore but kept for retrocomp
	pb_casts_shadows,			// Not used anymore but kept for retrocomp
	pb_visible_in_reflections,	// Not used anymore but kept for retrocomp
	pb_visible_in_refractions,	// Not used anymore but kept for retrocomp
	// output
	pb_temp_vrscene_file_dir,
	pb_override_node_properties,// Not used anymore but kept for retrocomp
	pb_excluded_entities,		// Not used anymore but kept for retrocomp
	pb_object_id_mode,
	pb_default_material,
	pb_display_percentage,
	pb_instancing_enable,
	// layout
	pb_layout_enable,
	pb_layout_file,
	pb_layout_name,				// Not used anymore but kept for retrocomp
	pb_layout_dir,				// Not used anymore but kept for retrocomp
	pb_terrain_file,
};

//************************************************************
// FindPluginOfTypeCallback
//************************************************************

class FindPluginOfTypeCallback : public EnumPluginCallback
{
	public:
	FindPluginOfTypeCallback(PluginID pluginType=0)
		: _pluginType(pluginType)
	{}
	virtual ~FindPluginOfTypeCallback(){}

	int process(::Plugin* plugin) VRAY_OVERRIDE
	{
		if (plugin->getPluginID() == _pluginType)
			_foundPlugins.append((VR::VRayPlugin*)plugin);
		return 1; // continue enumeration
	}

	PluginID _pluginType;
	MaxSDK::Array<VR::VRayPlugin*> _foundPlugins;
};

class VRayGolaem;

//************************************************************
// The VRayGolaem 3dsmax object
//************************************************************

// predeclaration
struct GlmSimulationData_v0;
typedef GlmSimulationData_v0 GlmSimulationData;
struct GlmFrameData_v0;
typedef GlmFrameData_v0 GlmFrameData;

class VRayGolaem
	: public GeomObject
	, public VR::VRenderObject
	, public ObjectIDWrapperInterface
{
	friend class VRayGolaemInstanceBase;
	friend class VRayGolaemDlgProc;

	//////////////////////////////////////////
	// Data Members
	//////////////////////////////////////////

	Mesh _mesh;						//!< An empty dummy mesh returned from GetRenderMesh()
	VR::VRayScene *_vrayScene;		//!< The loaded .vrscene file
	VR::CharString _vrsceneFile;	//!< 
	VR::CharString _shadersFile;	//!<

	// Cache attributes
	CStr _crowdFields;
	CStr _cacheName;
	CStr _cacheDir;
	CStr _characterFiles;

	// layout attributes
	bool _layoutEnable;
	CStr _layoutFile;
	CStr _terrainFile;

	// MoBlur attributes
	bool _mBlurEnable;
	bool _overMBlurWindowSize;
	float _mBlurWindowSize;
	bool _overMBlurSamples;
	int _mBlurSamples;

	// Culling attributes
	bool _frustumEnable;
	float _frustumMargin;
	float _cameraMargin;

	// Vray attributes
	int _frameOffset;
	int _objectIDBase;
	short _objectIDMode;
	float _displayPercent;
	bool _instancingEnable;
	bool _primaryVisibility;
	bool _castsShadows;
	bool _visibleInReflections;
	bool _visibleInRefractions;
	CStr _defaultMaterial;

	// Output attributes
	CStr _tempVRSceneFileDir;

	// Internal attributes
	MaxSDK::Array<GlmSimulationData*> _simulationData;
	MaxSDK::Array<GlmFrameData*> _frameData;
	bool _updateCacheData;
	Box3 _nodeBbox;					//!< Node bbox

public:
	IParamBlock2 *pblock2;
	BOOL suspendSnap;				//!< Snap suspension flag (TRUE during creation only)

public:
	//////////////////////////////////////////
	// Constructor / Destructor
	//////////////////////////////////////////
	VRayGolaem(void);
	~VRayGolaem(void);
	
	//////////////////////////////////////////
	// From BaseObject
	//////////////////////////////////////////
	int IsRenderable() { return true; }
	int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt);
	void Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt);
	void SetExtendedDisplay(int flags);
	int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
	CreateMouseCallBack* GetCreateMouseCallBack();

	void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);
	void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
	void InvalidateUI();

	//////////////////////////////////////////
	// From Object
	//////////////////////////////////////////
	ObjectState Eval(TimeValue time);

	void InitNodeName(TSTR& s) { s=STR_CLASSNAME; }
	ObjectHandle ApplyTransform(Matrix3& /*matrix*/) { return this; }
	Interval ObjectValidity(TimeValue t);

	// We don't convert to anything
	int CanConvertToType(Class_ID /*obtype*/) { return FALSE; }
	Object* ConvertToType(TimeValue /*t*/, Class_ID /*obtype*/) { assert(0);return NULL; }
	
	void GetWorldBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box);
	void GetLocalBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box);
	void GetDeformBBox(TimeValue t, Box3 &b, Matrix3 *tm, BOOL useSel);
	int DoOwnSelectHilite()	{ return 1; }

	//////////////////////////////////////////
	// From GeomObject
	//////////////////////////////////////////
	Mesh* GetRenderMesh(TimeValue t, INode *inode, View& view, BOOL& needDelete);

	//////////////////////////////////////////
	// Animatable methods
	//////////////////////////////////////////
	void DeleteThis() { delete this; }
	Class_ID ClassID() { return PLUGIN_CLASSID; }
	void GetClassName(TSTR& s) { s=_T("VRayGolaem"); }
	int IsKeyable() { return 0; }
	void* GetInterface(ULONG id);
	void ReleaseInterface(ULONG id, void *ip);
	
	//////////////////////////////////////////
	// Direct paramblock access
	//////////////////////////////////////////
	int	NumParamBlocks() { return 1; }	
	IParamBlock2* GetParamBlock(int /*i*/) { return pblock2; }
	IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock2->ID() == id) ? pblock2 : NULL; }

	int NumSubs() { return 1; }  
	Animatable* SubAnim(int i);
	TSTR SubAnimName(int i);

	//////////////////////////////////////////
	// From ref
	//////////////////////////////////////////
 	int NumRefs() { return 1; }
	RefTargetHandle GetReference(int i);
	void SetReference(int i, RefTargetHandle rtarg);

	RefTargetHandle Clone(RemapDir& remap);
	RefTargetHandle Clone();

	RefResult NotifyRefChanged(NOTIFY_REF_CHANGED_ARGS);

	//////////////////////////////////////////
	// Draw
	//////////////////////////////////////////
	void readGolaemCache(TimeValue t);
	void draw(TimeValue t, INode *node, ViewExp *vpt);
	void drawEntities(GraphicsWindow *gw, const Matrix3& transform, TimeValue t);

	//////////////////////////////////////////
	// read/write vrscene
	//////////////////////////////////////////
protected:
	bool readCrowdVRScene(const VR::CharString& file);
	bool writeCrowdVRScene(const VR::CharString& file);

public:
	//////////////////////////////////////////
	// From VRayPluginRendererInterface
	//////////////////////////////////////////
	PluginBase* getPlugin(void) { return NULL; }

	//////////////////////////////////////////
	// From VRenderObject
	//////////////////////////////////////////
	int init(const ObjectState &os, INode *node, VR::VRayCore *vray);
	VR::VRenderInstance* newRenderInstance(INode *node, VR::VRayCore *vray, int renderID);
	void deleteRenderInstance(VR::VRenderInstance *ri);
	void renderBegin(TimeValue t, VR::VRayCore *vray);
	void renderEnd(VR::VRayCore *vray);
	void frameBegin(TimeValue t, VR::VRayCore *vray);
	void frameEnd(VR::VRayCore *vray);

	// From ObjectIDWrapperInterface
	int getObjectID() VRAY_OVERRIDE { return _objectIDBase; }

private:	
	void updateVRayParams(TimeValue t);

	// Enable or disable some UI controls based on the settings.
	void grayDlgControls(void);

	// Create V-Ray plugins for the materials attached to the node that
	// references the VRayGolaem object.
	void createMaterials(VR::VRayCore *vray);

	// Enumerate the sub-materials for the given 3ds Max materials and create
	// wrappers for it.
	void enumMaterials(VUtils::VRayCore *vray, Mtl *mtl);

	// Create a wrapper material in the plugin manager for this 3ds Max material.
	// Only V-Ray compatible materials are supported.
	void wrapMaterial(VUtils::VRayCore *vray, Mtl *mtl);
};

//************************************************************
// VRayGolaemCreateCallBack
//************************************************************

class VRayGolaemCreateCallBack: public CreateMouseCallBack {
	VRayGolaem *sphere;
	IPoint2 sp0;
	Point3 p0;
public:
	int proc(ViewExp *vpt, int msg, int point, int flags, IPoint2 m, Matrix3& mat);
	void SetObj(VRayGolaem *obj) { sphere=obj; }
};

//************************************************************
// Inline
//************************************************************

bool isCharInvalidVrscene(char c);
void convertToValidVrsceneName(const CStr& strIn, CStr& strOut);
void splitStr(const CStr& input, char delim, MaxSDK::Array<CStr> & result);

void drawLine(GraphicsWindow *gw, const Point3 &p0, const Point3 &p1);
void drawBBox(GraphicsWindow *gw, const Box3 &b);
void drawSphere(GraphicsWindow *gw, const Point3 &pos, float radius, int nsegs);
void drawText(GraphicsWindow *gw, const MCHAR*  text, const Point3& pos);

Matrix3 golaemToMax();
Matrix3 maxToGolaem();

INode* FindNodeRef(ReferenceTarget *rt );
INode* GetNodeRef(ReferenceMaker *rm);
