/***************************************************************************
*                                                                          *
*  Copyright (C) Chaos Group & Golaem S.A. - All Rights Reserved.          *
*                                                                          *
***************************************************************************/

#include "vraygolaem.h"
#include "instance.h"
#include "pb2template_generator.h"
#include "defrayserver.h"
#include "vrender_unicode.h"
#include "hash_map.h"
#include "pluginenumcallbacks.h"
#include "resource.h"
#include "maxscript/maxscript.h"

#include <fstream>	// std::ofstream
#include <sstream>	// std::stringstream

#if GET_MAX_RELEASE(VERSION_3DSMAX) >= 9000
#include "IPathConfigMgr.h"
#endif
#if GET_MAX_RELEASE(VERSION_3DSMAX) >= 11900
#include "IFileResolutionManager.h"
#pragma comment(lib, "assetmanagement.lib")
#endif

#define GLMC_IMPLEMENTATION
#define GLMC_NOT_INCLUDE_MINIZ
#include "glm_crowd.h"	// golaem cache reader

// no param block script access for VRay free
#ifdef _FREE_
#define _FT(X) _T("")
#define IS_PUBLIC 0
#else
#define _FT(X) _T(X)
#define IS_PUBLIC 1
#endif // _FREE_

//************************************************************
// Class descriptor
//************************************************************

class VRayGolaemClassDesc: public ClassDesc2 {
public:
	int IsPublic(void) { return IS_PUBLIC; }
	void *Create(BOOL loading) { return new VRayGolaem; }
	const TCHAR *ClassName(void) { return STR_CLASSNAME; }
	SClass_ID SuperClassID(void) { return GEOMOBJECT_CLASS_ID; }
	Class_ID ClassID(void) { return PLUGIN_CLASSID; }
	const TCHAR* Category(void) { return _T("VRay");  }

	// Hardwired name, used by MAX Script as unique identifier
	const TCHAR* InternalName(void) { return STR_INTERNALNAME; }
	HINSTANCE HInstance(void) { return hInstance; }
};

//************************************************************
// Static / Define variables
//************************************************************

#define BIGFLOAT	float(999999) // from bendmod sample
#define BIGINT		int(999999)
#define ICON_RADIUS 10
#define CROWDVRAYPLUGINID PluginID(LARGE_CONST(2011070866)) // from glmCrowdVRayPlugin.h

TCHAR *iconText=_T("VRayGolaem");
static VRayGolaemClassDesc vrayGolaemClassDesc;

//************************************************************
// DLL stuff
//************************************************************

HINSTANCE hInstance;
int controlsInit=FALSE;

BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) {
	hInstance=hinstDLL;

	if (!controlsInit) {
		controlsInit=TRUE;
#if MAX_RELEASE<13900
		InitCustomControls(hInstance);
#endif
		InitCommonControls();
	}

	return(TRUE);
}

__declspec( dllexport ) const TCHAR* LibDescription(void) { return STR_LIBDESC; }
__declspec( dllexport ) int LibNumberClasses(void) { return 1; }

__declspec( dllexport ) ClassDesc* LibClassDesc(int i) {
	switch(i) { case 0: return &vrayGolaemClassDesc; }
	return NULL;
}

__declspec( dllexport ) ULONG LibVersion(void) { return VERSION_3DSMAX; }

__declspec( dllexport ) int LibInitialize(void) { return TRUE; }

__declspec( dllexport ) int LibShutdown(void) {
	if (golaemPlugman) {
		golaemPlugman->deleteAll();
		golaemPlugman->unloadAll();
		deleteDefaultPluginManager(golaemPlugman);
		golaemPlugman=NULL;
	}
	return TRUE;
}

class VRayGolaemDlgProc: public ParamMap2UserDlgProc {
	void chooseFileName(IParamBlock2 *pblock2, ParamID paramID, const TCHAR *title);
public:
	VRayGolaemDlgProc() {}

	INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis() { }

	void SetThing(ReferenceTarget *m) {}
};

static VRayGolaemDlgProc vrayGolaemDlgProc;

//************************************************************
// Parameter block
//************************************************************

// Paramblock2 name
enum { params, }; 

static ParamBlockDesc2 param_blk(params, STR_DLGTITLE,  0, &vrayGolaemClassDesc, P_AUTO_CONSTRUCT+P_AUTO_UI, REFNO_PBLOCK,
	IDD_VRAYGOLAEM, IDS_VRAYGOLAEM_PARAMS, 0, 0, &vrayGolaemDlgProc,
	// Params
	pb_file, _T("cache_file"), TYPE_FILENAME, 0, 0,
		p_ui, TYPE_EDITBOX, ED_GOLAEMVRSCENE,
#if GET_MAX_RELEASE(VERSION_3DSMAX) >= 11900
		p_assetTypeID, MaxSDK::AssetManagement::AssetType::kExternalLink,
#endif
	PB_END,
	pb_shaders_file, _T("shaders_file"), TYPE_FILENAME, 0, 0,
		p_ui, TYPE_EDITBOX, ED_SHADERSVRSCENE,
#if GET_MAX_RELEASE(VERSION_3DSMAX) >= 11900
		p_assetTypeID, MaxSDK::AssetManagement::AssetType::kExternalLink,
#endif
	PB_END,
	pb_use_node_attributes, _T("use_node_attributes"), TYPE_BOOL, 0, 0,
	p_default, TRUE,
	p_ui, TYPE_SINGLECHEKBOX, ED_USERNODEATTRIBUTES,
	PB_END,

	// display attributes
	pb_enable_display, _T("enable_display"), TYPE_BOOL, 0, 0,
	p_default, TRUE,
	p_ui, TYPE_SINGLECHEKBOX, ED_DISPLAYENABLE,
	PB_END,
	pb_display_percent, _T("display_percent"), TYPE_INT, 0, 0,
	p_default, 100,
	p_range, 0, 100, 
	p_ui, TYPE_SPINNER,  EDITTYPE_POS_INT, ED_DISPLAYPERCENT, ED_DISPLAYPERCENTSPIN, 1,
	PB_END,
	pb_display_entity_ids, _T("display_entity_ids"), TYPE_BOOL, 0, 0,
	p_default, TRUE,
	p_ui, TYPE_SINGLECHEKBOX, ED_DISPLAYENTITYIDS,
	PB_END,

	// cache attributes
	pb_crowd_fields, _T("crowd_fields"), TYPE_STRING, 0, 0,
	p_ui, TYPE_EDITBOX, ED_CROWDFIELDS,
	PB_END,
	pb_cache_name, _T("cache_name"), TYPE_STRING, 0, 0,
	p_ui, TYPE_EDITBOX, ED_CACHENAME,
	PB_END,
	pb_cache_dir, _T("cache_dir"), TYPE_STRING, 0, 0,
	p_ui, TYPE_EDITBOX, ED_CACHEDIR,
	PB_END,
	pb_character_files, _T("character_files"), TYPE_STRING, 0, 0,
	p_ui, TYPE_EDITBOX, ED_CHARACTERFILES,
	PB_END,

	// motion blur attributes
	pb_motion_blur_enable, _T("motion_blur_enable"), TYPE_BOOL, 0, 0,
	p_default, FALSE,
	p_ui, TYPE_SINGLECHEKBOX, ED_MBLURENABLE,
	PB_END,
	pb_motion_blur_start, _T("motion_blur_start"), TYPE_FLOAT, 0, 0,
	p_default, -0.5f,
	p_range, -BIGFLOAT, BIGFLOAT, 
	p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, ED_MBLURSTART, ED_MBLURSTARTSPIN, 0.50f,
	PB_END,
	pb_motion_blur_window_size, _T("motion_blur_window_size"), TYPE_FLOAT, 0, 0,
	p_default, 1.f,
	p_range, 0.f, BIGFLOAT, 
	p_ui, TYPE_SPINNER, EDITTYPE_POS_FLOAT, ED_MBLURWINDOWSIZE, ED_MBLURWINDOWSIZESPIN, 0.50f,
	PB_END,
	pb_motion_blur_samples, _T("motion_blur_samples"), TYPE_INT, 0, 0,
	p_default, 1,
	p_range, 0, BIGINT, 
	p_ui, TYPE_SPINNER,  EDITTYPE_POS_INT, ED_MBLURSAMPLES, ED_MBLURSAMPLESSPIN, 1,
	PB_END,

	// culling attributes
	pb_frustum_enable, _T("frustum_enable"), TYPE_BOOL, 0, 0,
	p_default, FALSE,
	p_ui, TYPE_SINGLECHEKBOX, ED_FRUSTUMENABLE,
	PB_END,
	pb_frustum_margin, _T("frustum_margin"), TYPE_FLOAT, 0, 0,
	p_default, 10.f,
	p_range, -BIGFLOAT, BIGFLOAT, 
	p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, ED_FRUSTUMMARGIN, ED_FRUSTUMMARGINSPIN, 1.f,
	PB_END,
	pb_camera_margin, _T("camera_margin"), TYPE_FLOAT, 0, 0,
	p_default, 10.f,
	p_range, -BIGFLOAT, BIGFLOAT, 
	p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, ED_CAMERAMARGIN, ED_CAMERAMARGINSPIN, 1.f,
	PB_END,

	// vray attributes
	pb_frame_offset, _T("frame_offset"), TYPE_INT, 0, 0,
	p_default, 0,
	p_range, -BIGINT, BIGINT, 
	p_ui, TYPE_SPINNER,  EDITTYPE_INT, ED_FRAMEOFFSET, ED_FRAMEOFFSETSPIN, 1,
	PB_END,
	pb_scale_transform, _T("scale_transform"), TYPE_FLOAT, 0, 0,
	p_default, 10.f,
	p_range, -BIGFLOAT, BIGFLOAT, 
	p_ui, TYPE_SPINNER, EDITTYPE_FLOAT, ED_SCALETRANSFORM, ED_SCALETRANSFORMSPIN, 1.f,
	PB_END,
	pb_object_id_base, _T("object_id_base"), TYPE_INT, 0, 0,
	p_default, 0,
	p_range, 0, BIGINT, 
	p_ui, TYPE_SPINNER,  EDITTYPE_POS_INT, ED_OBJECTIDBASE, ED_OBJECTIDBASESPIN, 1,
	PB_END,
	pb_primary_visibility, _T("primary_visibility"), TYPE_BOOL, 0, 0,
	p_default, TRUE,
	p_ui, TYPE_SINGLECHEKBOX, ED_PRIMARYVISIBILITY,
	PB_END,
	pb_casts_shadows, _T("casts_shadows"), TYPE_BOOL, 0, 0,
	p_default, TRUE,
	p_ui, TYPE_SINGLECHEKBOX, ED_CASTSSHADOWS,
	PB_END,
	pb_visible_in_reflections, _T("visible_in_reflections"), TYPE_BOOL, 0, 0,
	p_default, TRUE,
	p_ui, TYPE_SINGLECHEKBOX, ED_VISIBLEINREFLECTIONS,
	PB_END,
	pb_visible_in_refractions, _T("visible_in_refractions"), TYPE_BOOL, 0, 0,
	p_default, TRUE,
	p_ui, TYPE_SINGLECHEKBOX, ED_VISIBLEINREFRACTIONS,
	PB_END,
	pb_temp_vrscene_file_dir, _T("temp_vrscene_file_dir"), TYPE_STRING, 0, 0,
	p_default, _T("TEMP"),
	p_ui, TYPE_EDITBOX, ED_TEMPVRSCENEFILEDIR,
	PB_END,

PB_END
);

//************************************************************
// VRayGolaem implementation
//************************************************************

//------------------------------------------------------------
// VRayGolaem
//------------------------------------------------------------
VRayGolaem::VRayGolaem() 
	: _simulationData(NULL), _frameData(NULL), _updateCacheData(true)
{
	static int pblockDesc_inited=false;
	if (!pblockDesc_inited) 
	{
		initPBlockDesc(param_blk);
		pblockDesc_inited=true;
	}
	pblock2=NULL;
	vrayGolaemClassDesc.MakeAutoParamBlocks(this);
	assert(pblock2);
	suspendSnap=FALSE;
}

VRayGolaem::~VRayGolaem() {
}

//------------------------------------------------------------
// Misc
//------------------------------------------------------------
void VRayGolaem::InvalidateUI() {
	param_blk.InvalidateUI(pblock2->LastNotifyParamID());
}

static Pb2TemplateGenerator templateGenerator;

void VRayGolaem::BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev) {
	vrayGolaemClassDesc.BeginEditParams(ip, this, flags, prev);
}

void VRayGolaem::EndEditParams(IObjParam *ip, ULONG flags, Animatable *next) {
	vrayGolaemClassDesc.EndEditParams(ip, this, flags, next);
}

RefTargetHandle VRayGolaem::Clone(RemapDir& remap) {
	VRayGolaem* newob=new VRayGolaem();	
	BaseClone(this, newob, remap);
	newob->ReplaceReference(0, pblock2->Clone(remap));
	return newob;
}

Animatable* VRayGolaem::SubAnim(int i) {
	switch (i) {
		case 0: return pblock2;
		default: return NULL;
	}
}

TSTR VRayGolaem::SubAnimName(int i) {
	switch (i) {
		case 0: return STR_DLGTITLE;
		default: return _T("");
	}
}

RefTargetHandle VRayGolaem::GetReference(int i) {
	switch (i) {
		case REFNO_PBLOCK: return pblock2;
		default: return NULL;
	}
}

void VRayGolaem::SetReference(int i, RefTargetHandle rtarg) {
	switch (i) {
		case REFNO_PBLOCK: pblock2 = (IParamBlock2*)rtarg; break;
	}
}

RefResult VRayGolaem::NotifyRefChanged(NOTIFY_REF_CHANGED_ARGS) {
	switch (message) {
		case REFMSG_CHANGE:
			if (hTarget==pblock2) {
				// if (pblock2->LastNotifyParamID()==VRayGolaem_fileName) readPreview();
				param_blk.InvalidateUI();
			}
			break;
	}
	return REF_SUCCEED;
}

Interval VRayGolaem::ObjectValidity(TimeValue t) {
	return Interval(t,t);
}

//------------------------------------------------------------
// proc
//------------------------------------------------------------
int VRayGolaemCreateCallBack::proc(ViewExp *vpt, int msg, int point, int flags, IPoint2 m, Matrix3& mat) {
	if (!sphere) return CREATE_ABORT;

	Point3 np=vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);

	switch (msg) {
		case MOUSE_POINT:
			switch (point) {
				case 0:
					sphere->suspendSnap=TRUE;				
					sp0=m;
					p0=vpt->SnapPoint(m, m, NULL, SNAP_IN_3D);
					mat.SetTrans(p0);
				case 1:
					return CREATE_STOP;
			}
			return CREATE_CONTINUE;

		case MOUSE_MOVE:
			return CREATE_CONTINUE;

		case MOUSE_ABORT:
			return CREATE_ABORT;
	}

	return CREATE_CONTINUE;
}

static VRayGolaemCreateCallBack createCB;

CreateMouseCallBack* VRayGolaem::GetCreateMouseCallBack() {
	createCB.SetObj(this);
	return &createCB;
}

void VRayGolaem::SetExtendedDisplay(int flags) {
}

void VRayGolaem::GetLocalBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box) 
{
	float radius=ICON_RADIUS; 
	_nodeBbox+=Point3(-radius, -radius, -radius);
	_nodeBbox+=Point3(radius, radius, radius);
	box = _nodeBbox;
}

void VRayGolaem::GetWorldBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box) {
	if (!inode) return;
	Box3 localBox;
	GetLocalBoundBox(t, inode, vpt, localBox);
	box=localBox*(inode->GetObjectTM(t));
}

void VRayGolaem::GetDeformBBox(TimeValue t, Box3 &b, Matrix3 *tm, BOOL useSel) {
	if (!tm) GetLocalBoundBox(t, NULL, NULL, b);
	else {
		Box3 bbox;
		GetLocalBoundBox(t, NULL, NULL, bbox);
		b.Init();
		for (int i=0; i<8; i++) b+=(*tm)*bbox[i];
	}
}

int VRayGolaem::HitTest(TimeValue t, INode *node, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt) {
	static HitRegion hitRegion;
	DWORD	savedLimits;

	GraphicsWindow *gw=vpt->getGW();	
	Material *mtl=gw->getMaterial();
	MakeHitRegion(hitRegion, type, crossing, 4, p);

	gw->setRndLimits(((savedLimits = gw->getRndLimits())|GW_PICK)&~GW_ILLUM);
	gw->setHitRegion(&hitRegion);
	gw->clearHitCode();

	draw(t, node, vpt);

	gw->setRndLimits(savedLimits);
	
	if((hitRegion.type != POINT_RGN) && !hitRegion.crossing) return TRUE;
	return gw->checkHitCode();
}

void VRayGolaem::Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt) {
	if (suspendSnap) return;
}

//------------------------------------------------------------
// Display
//------------------------------------------------------------
int VRayGolaem::Display(TimeValue t, INode* node, ViewExp *vpt, int flags) {
	draw(t, node, vpt);
	return 0;
}

ObjectState VRayGolaem::Eval(TimeValue time) 
{
	_updateCacheData = true; // time has changed, we should re-read the cache
	return ObjectState(this);
}

void* VRayGolaem::GetInterface(ULONG id) {
	if (id==I_VRAYGEOMETRY) return (VR::VRenderObject*) this;
	return GeomObject::GetInterface(id);
}

void VRayGolaem::ReleaseInterface(ULONG id, void *ip) {
	if (id==I_VRAYGEOMETRY) return;
	GeomObject::ReleaseInterface(id, ip);
}

Mesh* VRayGolaem::GetRenderMesh(TimeValue t, INode *inode, View& view, BOOL& needDelete) {
	needDelete=false;
	return &_mesh;
}

INT_PTR VRayGolaemDlgProc::DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	int id=LOWORD(wParam);

	IParamBlock2 *pblock=NULL;
	VRayGolaem *vrayGolaem=NULL;

	if (map) pblock=map->GetParamBlock();
	if (pblock) vrayGolaem= static_cast<VRayGolaem*>(pblock->GetOwner());

	switch (msg) {
		case WM_INITDIALOG: {
			break;
		}
		case WM_DESTROY:
			break;
		case WM_COMMAND: {
			int ctrlID=LOWORD(wParam);
			int notifyCode=HIWORD(wParam);
			HWND ctrlHWnd=(HWND) lParam;

			if (notifyCode==BN_CLICKED) {
				if (ctrlID==BN_GOLAEMBROWSE && vrayGolaem) {
					chooseFileName(pblock, pb_file, _T("Choose Golaem .vrscene file"));
					// if the vrscene has been loaded, fill the node attributes
					const TCHAR *fname_wstr=pblock->GetStr(pb_file, t);
					if (fname_wstr)
					{
						GET_MBCS(fname_wstr, fname_mbcs);
						vrayGolaem->readCrowdVRScene(fname_mbcs);
					}
				}
				if (ctrlID==BN_SHADERSBROWSE && vrayGolaem) {
					chooseFileName(pblock, pb_shaders_file, _T("Choose shaders .vrscene file"));
				}
			}
			break;
		}
	}

	return FALSE;
}

//************************************************************
// Browse
//************************************************************

static const TCHAR *vrsceneExtList=_T("V-Ray scene file (*.vrscene)\0*.vrscene\0All files(*.*)\0*.*\0\0");
static const TCHAR *vrsceneDefExt=_T("vrscene");

//------------------------------------------------------------
// chooseFileName
//------------------------------------------------------------
void VRayGolaemDlgProc::chooseFileName(IParamBlock2 *pblock2, ParamID paramID, const TCHAR *title) {
	TCHAR fname[512]=_T("");
	fname[0]='\0';

	const TCHAR *storedName=pblock2->GetStr(paramID);
	if (storedName) vutils_strcpy_n(fname, storedName, COUNT_OF(fname));

	OPENFILENAME fn;
	fn.lStructSize=sizeof(fn);
	fn.hwndOwner=GetCOREInterface()->GetMAXHWnd();
	fn.hInstance=hInstance;
	fn.lpstrFilter=vrsceneExtList;
	fn.lpstrCustomFilter=NULL;
	fn.nMaxCustFilter=0;
	fn.nFilterIndex=1;
	fn.lpstrFile=fname;
	fn.nMaxFile=512;
	fn.lpstrFileTitle=NULL;
	fn.nMaxFileTitle=0;
	fn.lpstrInitialDir=NULL;
	fn.lpstrTitle=title;
	fn.Flags=0;
	fn.lpstrDefExt=vrsceneDefExt;
	fn.lCustData=NULL;
	fn.lpfnHook=NULL;
	fn.lpTemplateName=NULL;

	BOOL res=GetOpenFileName(&fn);

	const TCHAR *fullFname=NULL;
#if GET_MAX_RELEASE(VERSION_3DSMAX) >= 9000
	MaxSDK::Util::Path fpath(fname);
#if GET_MAX_RELEASE(VERSION_3DSMAX) < 11900
	IPathConfigMgr::GetPathConfigMgr()->NormalizePathAccordingToSettings(fpath);
	fullFname=fpath.GetCStr();
#else
	TSTR mstrfname(fname);
	IFileResolutionManager::GetInstance()->DoGetUniversalFileName(mstrfname);
	fullFname=mstrfname.data();
#endif
#else
	fullFname = fname;
#endif

	if (res) {
		pblock2->SetValue(paramID, 0, const_cast<TCHAR*>(fullFname));
		IParamMap2 *map=pblock2->GetMap();
		if (map) map->Invalidate(paramID);
	}
}

//************************************************************
// Draw
//************************************************************

//------------------------------------------------------------
// readGolaemCache
//------------------------------------------------------------
void VRayGolaem::readGolaemCache(TimeValue t)
{
	if (!_updateCacheData) return;
	
	// clean previous data
	for (size_t iData=0, nbData=_simulationData.length(); iData<nbData; ++iData)
	{
		glmDestroyFrameData(&_frameData[iData], _simulationData[iData]);
		glmDestroySimulationData(&_simulationData[iData]);
	}
	_simulationData.removeAll();
	_frameData.removeAll();

	// update params
	updateVRayParams(t);
	_updateCacheData = false;

	// read caches
	MaxSDK::Array<CStr> crowdFields;
	splitStr(_crowdFields, ';', crowdFields);
	if (_cacheName.length() != 0 && _cacheDir.length() != 0)
	{
		for (size_t iCf=0, nbCf=crowdFields.length(); iCf<nbCf; ++iCf)
		{
			int currentFrame = (int)((float)t / (float)TIME_TICKSPERSEC * (float)GetFrameRate()) + _frameOffset; 
			CStr currentFrameStr; currentFrameStr.printf("%i", currentFrame);
			CStr gscsFileStr(_cacheDir + "/" + _cacheName + "." + crowdFields[iCf] + ".gscs");
			CStr gscfFileStr(_cacheDir + "/" + _cacheName + "." + crowdFields[iCf] + "." + currentFrameStr + ".gscf");

			GlmSimulationCacheStatus status;
			GlmSimulationData* simulationData(NULL);
			GlmFrameData* frameData(NULL);
			status = glmCreateAndReadSimulationData(&simulationData, gscsFileStr);
			if (status == GSC_SUCCESS)
			{
				glmCreateFrameData(&frameData, simulationData);
				status = glmReadFrameData(frameData, simulationData, gscfFileStr);
				if (status == GSC_SUCCESS)
				{
					_simulationData.append(simulationData);
					_frameData.append(frameData);
				}
				else
				{
					glmDestroySimulationData(&simulationData);
					DebugPrint(_T("VRayGolaem: Error loading .gscf file \"%s\""), gscfFileStr);
				}
			}
			else
			{
				DebugPrint(_T("VRayGolaem: Error loading .gscs file \"%s\""), gscsFileStr);
			}
		}
	}
}

//------------------------------------------------------------
// drawEntities
//------------------------------------------------------------
void VRayGolaem::drawEntities(GraphicsWindow *gw, TimeValue t)
{
	// get display attributes
	bool displayEnable = pblock2->GetInt(pb_enable_display, t) == 1;
	int displayPercent = pblock2->GetInt(pb_display_percent, t);
	bool displayEntityIds = pblock2->GetInt(pb_display_entity_ids, t) == 1;
	if (!displayEnable) return;

	// update cache if required
	readGolaemCache(t);
	if (_simulationData.length() == 0 || _frameData.length() == 0 || _simulationData.length() != _frameData.length()) return;

	// draw
	_nodeBbox.Init();
	
	for (size_t iData=0, nbData=_simulationData.length(); iData<nbData; ++iData)
	{
		int maxDisplayedEntity = _simulationData[iData]->_entityCount * displayPercent / 100;
		for (size_t iEntity=0, entityCount = maxDisplayedEntity; iEntity<entityCount; ++iEntity)
		{
			unsigned int entityType = _simulationData[iData]->_entityTypes[iEntity];
			float entityRadius = _simulationData[iData]->_entityRadius[iEntity];
			float entityHeight = _simulationData[iData]->_entityHeight[iEntity];
			if(_simulationData[iData]->_boneCount[entityType])
			{
				// draw bbox
				unsigned int iBoneIndex = _simulationData[iData]->_iBoneOffsetPerEntityType[entityType] + _simulationData[iData]->_indexInEntityType[iEntity] * _simulationData[iData]->_boneCount[entityType];
				Point3 entityPosition(_frameData[iData]->_bonePositions[iBoneIndex][0], _frameData[iData]->_bonePositions[iBoneIndex][1], _frameData[iData]->_bonePositions[iBoneIndex][2]);
				// axis transformation for max
				Matrix3 axisTranform = RotateXMatrix(pi/2);
				entityPosition = axisTranform * entityPosition;
				Box3 entityBbox(Point3(entityPosition[0]-entityRadius, entityPosition[1]-entityRadius, entityPosition[2]), Point3(entityPosition[0]+entityRadius, entityPosition[1]+entityRadius, entityPosition[2]+entityHeight));
				entityPosition *= _scaleTransform;
				entityBbox.pmin *= _scaleTransform;
				entityBbox.pmax *= _scaleTransform;
				drawBBox(gw, entityBbox); // update node bbox
				_nodeBbox += entityBbox;

				// draw EntityID
				if (displayEntityIds)
				{
					CStr entityIdStrs; entityIdStrs.printf("%i", _simulationData[iData]->_entityIds[iEntity]);
					drawText(gw, entityIdStrs.ToMCHAR(), entityPosition);
				}
			}
		}
	}
}

//------------------------------------------------------------
// draw
//------------------------------------------------------------
void VRayGolaem::draw(TimeValue t, INode *node, ViewExp *vpt) 
{
	GraphicsWindow *gw=vpt->getGW();
	Matrix3 tm=node->GetObjectTM(t);
	gw->setTransform(tm);

	Color color=Color(node->GetWireColor());
	if (node->IsFrozen()) color=GetUIColor(COLOR_FREEZE);
	else if (node->Selected()) color=GetUIColor(COLOR_SELECTION);
	gw->setColor(LINE_COLOR, color);

	// locator
	drawSphere(gw, Point3::Origin, ICON_RADIUS, 30);

	// entities	
	drawEntities(gw, t);

	// text
	tm.NoScale();
	float scaleFactor=vpt->NonScalingObjectSize()*vpt->GetVPWorldWidth(tm.GetTrans())/(float)360.0;
	tm.Scale(Point3(scaleFactor,scaleFactor,scaleFactor));
	gw->setTransform(tm);
	drawText(gw, iconText, Point3::Origin);
}

//************************************************************
// VRenderObject
//************************************************************

#if MAX_RELEASE >= 6000 && MAX_RELEASE < 8900
	#define VRAYRT_MAIN    "VRAY30_RT_FOR_3DSMAX60_MAIN"
	#define VRAYRT_PLUGINS "VRAY30_RT_FOR_3DSMAX60_PLUGINS"
#elif MAX_RELEASE >= 8900 && MAX_RELEASE < 10900
	#define VRAYRT_MAIN    "VRAY30_RT_FOR_3DSMAX90_MAIN"
	#define VRAYRT_PLUGINS "VRAY30_RT_FOR_3DSMAX90_PLUGINS"
#elif MAX_RELEASE >= 10900 && MAX_RELEASE < 11900
	#define VRAYRT_MAIN    "VRAY30_RT_FOR_3DSMAX2009_MAIN"
	#define VRAYRT_PLUGINS "VRAY30_RT_FOR_3DSMAX2009_PLUGINS"
#elif MAX_RELEASE >= 11900 && MAX_RELEASE < 12900
	#define VRAYRT_MAIN    "VRAY30_RT_FOR_3DSMAX2010_MAIN"
	#define VRAYRT_PLUGINS "VRAY30_RT_FOR_3DSMAX2010_PLUGINS"
#elif MAX_RELEASE >= 12900 && MAX_RELEASE < 13900
	#define VRAYRT_MAIN    "VRAY30_RT_FOR_3DSMAX2011_MAIN"
	#define VRAYRT_PLUGINS "VRAY30_RT_FOR_3DSMAX2011_PLUGINS"
#elif MAX_RELEASE >= 13900 && MAX_RELEASE < 14900
	#define VRAYRT_MAIN    "VRAY30_RT_FOR_3DSMAX2012_MAIN"
	#define VRAYRT_PLUGINS "VRAY30_RT_FOR_3DSMAX2012_PLUGINS"
#elif MAX_RELEASE >= 14850 && MAX_RELEASE < 15900
	#define VRAYRT_MAIN    "VRAY30_RT_FOR_3DSMAX2013_MAIN"
	#define VRAYRT_PLUGINS "VRAY30_RT_FOR_3DSMAX2013_PLUGINS"
#elif MAX_RELEASE >= 15850 && MAX_RELEASE < 16900
	#define VRAYRT_MAIN    "VRAY30_RT_FOR_3DSMAX2014_MAIN"
	#define VRAYRT_PLUGINS "VRAY30_RT_FOR_3DSMAX2014_PLUGINS"
#elif MAX_RELEASE >= 16850 && MAX_RELEASE < 17900
	#define VRAYRT_MAIN    "VRAY30_RT_FOR_3DSMAX2015_MAIN"
	#define VRAYRT_PLUGINS "VRAY30_RT_FOR_3DSMAX2015_PLUGINS"
#else
#error Unsupported version of 3ds Max API
#endif

//------------------------------------------------------------
// init
//------------------------------------------------------------
int VRayGolaem::init(const ObjectState &os, INode *node, VR::VRayCore *vray) 
{
	VRenderObject::init(os, node, vray);
	return true;
}

//------------------------------------------------------------
// Get the path to the V-Ray plugins; use the V-Ray RT environment variable for this
//------------------------------------------------------------
const tchar* getVRayPluginPath() 
{
	char pluginsDirVar[512];
	vutils_strcpy(pluginsDirVar, VRAYRT_PLUGINS);
	vutils_strcat(pluginsDirVar, "_");
	vutils_strcat(pluginsDirVar, PROCESSOR_ARCHITECTURE);

	const char *s = getenv(pluginsDirVar);
	if (s == NULL) {
		tchar str[512];
		sprintf(str, "Could not read V-Ray environment variable \"%s\"\n", pluginsDirVar);
		VUtils::debug(str);
		return ".";
	}
	return s;
}

//------------------------------------------------------------
// updateVRayParams
//------------------------------------------------------------
void VRayGolaem::updateVRayParams(TimeValue t) 
{
	// vrscene attributes
	const TCHAR *fname_wstr=pblock2->GetStr(pb_file, t);
	if (!fname_wstr) _vrsceneFile="";
	else {
		GET_MBCS(fname_wstr, fname_mbcs);
		_vrsceneFile=fname_mbcs;
	}

	const TCHAR *shadersName_wstr=pblock2->GetStr(pb_shaders_file, t);
	if (!shadersName_wstr) _shadersFile="";
	else {
		GET_MBCS(shadersName_wstr, shadersName_mbcs);
		_shadersFile=shadersName_mbcs;
	}
	_useNodeAttributes = pblock2->GetInt(pb_use_node_attributes, t) == 1;

	// cache attributes
	const TCHAR *crowdFields_wstr=pblock2->GetStr(pb_crowd_fields, t);
	if (!crowdFields_wstr) _crowdFields="";
	else {
		GET_MBCS(crowdFields_wstr, crowdFields_mbcs);
		_crowdFields=crowdFields_mbcs;
	}

	const TCHAR *cacheName_wstr=pblock2->GetStr(pb_cache_name, t);
	if (!cacheName_wstr) _cacheName="";
	else {
		GET_MBCS(cacheName_wstr, cacheName_mbcs);
		_cacheName=cacheName_mbcs;
	}

	const TCHAR *cacheDir_wstr=pblock2->GetStr(pb_cache_dir, t);
	if (!cacheDir_wstr) _cacheDir="";
	else {
		GET_MBCS(cacheDir_wstr, cacheDir_mbcs);
		_cacheDir=cacheDir_mbcs;
	}

	const TCHAR *characterFiles_wstr=pblock2->GetStr(pb_character_files, t);
	if (!characterFiles_wstr) _characterFiles="";
	else {
		GET_MBCS(characterFiles_wstr, characterFiles_mbcs);
		_characterFiles=characterFiles_mbcs;
	}

	// motion blur attributes
	_mBlurEnable = pblock2->GetInt(pb_motion_blur_enable, t) == 1;
	_mBlurStart = pblock2->GetFloat(pb_motion_blur_start, t);
	_mBlurWindowSize = pblock2->GetFloat(pb_motion_blur_window_size, t);
	_mBlurSamples = pblock2->GetInt(pb_motion_blur_samples, t);

	// culling attributes
	_frustumEnable = pblock2->GetInt(pb_frustum_enable, t) == 1;
	_frustumMargin = pblock2->GetFloat(pb_frustum_margin, t);
	_cameraMargin = pblock2->GetFloat(pb_camera_margin, t);

	// vray
	_scaleTransform = pblock2->GetFloat(pb_scale_transform, t);
	_frameOffset = pblock2->GetInt(pb_frame_offset, t);
	_objectIDBase = pblock2->GetInt(pb_object_id_base, t);
	_primaryVisibility = pblock2->GetInt(pb_primary_visibility, t) == 1;
	_castsShadows = pblock2->GetInt(pb_casts_shadows, t) == 1;
	_visibleInReflections = pblock2->GetInt(pb_visible_in_reflections, t) == 1;
	_visibleInRefractions = pblock2->GetInt(pb_visible_in_refractions, t) == 1;

	// output
	const TCHAR *tempVrScene_wstr=pblock2->GetStr(pb_temp_vrscene_file_dir, t);
	if (!tempVrScene_wstr) _tempVRSceneFileDir="TEMP";
	else {
		GET_MBCS(tempVrScene_wstr, tempVrScene_mbcs);
		_tempVRSceneFileDir=tempVrScene_mbcs;
	}
}

INode* FindNodeRef(ReferenceTarget *rt )
{
   DependentIterator di( rt );
   ReferenceMaker *rm;
   INode *nd = NULL;
   rm = di.Next( );
   while ( rm )
   {   
      nd = GetNodeRef( rm );
      if (nd) return nd;
	  rm = di.Next( );
   }
   return NULL;
}  

INode* GetNodeRef(ReferenceMaker *rm)
{
   if (rm->SuperClassID()==BASENODE_CLASS_ID)
      return (INode *)rm;
   else
      return rm->IsRefTarget() ? FindNodeRef ( ( ReferenceTarget * ) rm) : NULL;
}

//------------------------------------------------------------
// renderBegin / renderEnd
//------------------------------------------------------------
void VRayGolaem::renderBegin(TimeValue t, VR::VRayCore *_vray) 
{
	VR::VRayRenderer *vray=static_cast<VR::VRayRenderer*>(_vray);
	VRenderObject::renderBegin(t, vray);

	updateVRayParams(t);

	const VR::VRaySequenceData &sdata=vray->getSequenceData();

	if (!golaemPlugman) {
		golaemPlugman=newDefaultPluginManager();
		const tchar *vrayPluginPath = getVRayPluginPath();
		sdata.progress->info("VRayGolaem: Loading V-Ray plugins from \"%s\"", getVRayPluginPath());
		golaemPlugman->loadLibraryFromPathCollection(vrayPluginPath, "/vray_*.dll", NULL, vray->getSequenceData().progress);
	}

	// Creates the crowd .vrscene file on the fly if required
	VR::CharString vrSceneFileToLoad(_vrsceneFile);
	if (_useNodeAttributes)
	{
		CStr outputDir(getenv (_tempVRSceneFileDir));
		if (outputDir!=NULL) 
		{
			MaxSDK::Array<CStr> crowdFields;
			splitStr(_crowdFields, ';', crowdFields);
			if (outputDir.Length() != 0 && _cacheName.Length() != 0 && crowdFields.length() != 0)
			{
				CStr outputPathStr(outputDir + "/" + _cacheName + "." + crowdFields[0] + ".vrscene");
				VR::CharString vrSceneExportPath(outputPathStr); // TODO
				if (!writeCrowdVRScene(vrSceneExportPath)) 
				{
					sdata.progress->warning("VRayGolaem: Error writing .vrscene file \"%s\"", vrSceneExportPath.ptr());
				}
				else 
				{
					sdata.progress->info("VRayGolaem: Writing .vrscene file \"%s\"", vrSceneExportPath.ptr());
					vrSceneFileToLoad = vrSceneExportPath;
				}
			}
			else
			{
				sdata.progress->warning("VRayGolaem: Node attributes invalid (CrowdFields, Cache Name or Cache Dir is empty)");
			}
		}
		else
		{
			sdata.progress->warning("VRayGolaem: Error finding environment variable for .vrscene output \"%s\"", _tempVRSceneFileDir);
		}
	}

	// Load the .vrscene into the plugin manager
	_vrayScene=new VR::VRayScene(golaemPlugman);

	if (vrSceneFileToLoad.empty()) {
		sdata.progress->warning("VRayGolaem: No .vrscene file specified");
	} else {
		VR::ErrorCode errCode=_vrayScene->readFile(vrSceneFileToLoad.ptr());
		if (errCode.error()) {
			VR::CharString errMsg=errCode.getErrorString();
			sdata.progress->warning("VRayGolaem: Error loading .vrscene file \"%s\": %s", vrSceneFileToLoad.ptr(), errMsg.ptr());
		} else {
			sdata.progress->info("VRayGolaem: Scene file \"%s\" loaded successfully", vrSceneFileToLoad.ptr());
		}
	}

	if (_shadersFile.empty()) {
		sdata.progress->warning("VRayGolaem: No shaders .vrscene file specified");
	} else {
		VR::ErrorCode errCode=_vrayScene->readFile(_shadersFile.ptr());
		if (errCode.error()) {
			VR::CharString errMsg=errCode.getErrorString();
			sdata.progress->warning("VRayGolaem: Error loading shaders .vrscene file \"%s\": %s", _shadersFile.ptr(), errMsg.ptr());
		} else {
			sdata.progress->info("VRayGolaem: Shaders file \"%s\" loaded successfully", _shadersFile.ptr());
		}
	}

	callRenderBegin(vray);
}

void VRayGolaem::renderEnd(VR::VRayCore *_vray) 
{
	VR::VRayRenderer *vray=static_cast<VR::VRayRenderer*>(_vray);
	VRenderObject::renderEnd(vray);
	
	callRenderEnd(vray);
	if (_vrayScene) {
		_vrayScene->freeMem();
		delete _vrayScene;
		_vrayScene=NULL;
	}
}

//------------------------------------------------------------
// frameBegin / frameEnd
//------------------------------------------------------------
void VRayGolaem::frameBegin(TimeValue t, VR::VRayCore *_vray) 
{
	VR::VRayRenderer *vray=static_cast<VR::VRayRenderer*>(_vray);
	VRenderObject::frameBegin(t, vray);
	callFrameBegin(vray);
}

void VRayGolaem::frameEnd(VR::VRayCore *_vray) 
{
	VR::VRayRenderer *vray=static_cast<VR::VRayRenderer*>(_vray);
	VRenderObject::frameEnd(vray);
	callFrameEnd(vray);
}

//------------------------------------------------------------
// newRenderInstance / deleteRenderInstance
//------------------------------------------------------------
VR::VRenderInstance* VRayGolaem::newRenderInstance(INode *node, VR::VRayCore *vray, int renderID) {
	if (vray) {
		const VR::VRaySequenceData &sdata=vray->getSequenceData();
		if (sdata.progress) {
			const TCHAR *nodeName=node? node->GetName() : _T("");
			GET_MBCS(nodeName, nodeName_mbcs);
			sdata.progress->debug("VRayGolaem: newRenderInstance() for node \"%s\"", nodeName_mbcs);
		}
	}
	VRayGolaemInstanceBase *golaemInstance=new VRayGolaemInstanceBase(this, node, vray, renderID);
	return golaemInstance;
}

void VRayGolaem::deleteRenderInstance(VR::VRenderInstance *ri) {
	delete static_cast<VRayGolaemInstanceBase*>(ri);
}

//------------------------------------------------------------
// callRenderBegin / callFrameBegin / callRenderEnd / callFrameEnd
//-----------------------------------------------------------
void VRayGolaem::callRenderBegin(VR::VRayCore *vray) {
	PluginRendererInterfaceRAII plgInterface(vray, this);

	PreRenderBeginCB preRenderBeginCb(vray);
	golaemPlugman->enumPlugins(&preRenderBeginCb);

	RenderBeginCB renderBeginCb(vray, 0.0f);
	golaemPlugman->enumPlugins(&renderBeginCb);
}

void VRayGolaem::callFrameBegin(VR::VRayCore *vray) {
	PluginRendererInterfaceRAII plgInterface(vray, this);
	TimeConversionRAII timeConversion(vray);

	PreFrameBeginCB preFrameBeginCb(vray);
	golaemPlugman->enumPlugins(&preFrameBeginCb);

	FrameBeginCB frameBeginCb(vray, vray->getFrameData().t);
	golaemPlugman->enumPlugins(&frameBeginCb);
}

void VRayGolaem::callRenderEnd(VR::VRayCore *vray) {
	PluginRendererInterfaceRAII plgInterface(vray, this);

	RenderEndCB renderEndCb(vray, 0.0f);
	golaemPlugman->enumPlugins(&renderEndCb);

	PostRenderEndCB postRenderEndCb(vray);
	golaemPlugman->enumPlugins(&postRenderEndCb);
}

void VRayGolaem::callFrameEnd(VR::VRayCore *vray) {
	PluginRendererInterfaceRAII plgInterface(vray, this);
	TimeConversionRAII timeConversion(vray);

	FrameEndCB frameEndCb(vray, vray->getFrameData().t);
	golaemPlugman->enumPlugins(&frameEndCb);

	PostFrameEndCB postFrameEndCb(vray);
	golaemPlugman->enumPlugins(&postFrameEndCb);
}

//------------------------------------------------------------
// compileGeometry / clearGeometry
//-----------------------------------------------------------
void VRayGolaem::compileGeometry(VR::VRayCore *vray) {
	TimeConversionRAII timeConversion(vray);

	const VR::VRaySequenceData &sdata=vray->getSequenceData();
	if (sdata.progress)
		sdata.progress->debug("VRayGolaem: Compiling geometry");

	CompileGeometryCB compileGeometryCb(vray);
	int res=golaemPlugman->enumPlugins(&compileGeometryCb);

	if (sdata.progress)
		sdata.progress->debug("VRayGolaem: %i plugins enumerated for compileGeometry()", res);
}

void VRayGolaem::clearGeometry(VR::VRayCore *vray) {
	TimeConversionRAII timeConversion(vray);

	const VR::VRaySequenceData &sdata=vray->getSequenceData();
	if (sdata.progress)
		sdata.progress->debug("VRayGolaem: Clearing geometry");

	ClearGeometryCB clearGeometryCb(vray);
	int res=golaemPlugman->enumPlugins(&clearGeometryCb);

	if (sdata.progress)
		sdata.progress->debug("VRayGolaem: %i plugins enumerated for clearGeometry()", res);
}

PluginManager* VRayGolaem::getPluginManager(void) {
	return golaemPlugman;
}

//************************************************************
// Read / Write VRScene
//************************************************************

//------------------------------------------------------------
// readCrowdVRScene: parse the imported crowd .vrscene to fill the node attributes
//------------------------------------------------------------
bool VRayGolaem::readCrowdVRScene(const VR::CharString& file) 
{	
	// create a Vray context
	PluginManager* tempPlugMan(golaemPlugman);
	bool deletePlugMan(false);
	if (tempPlugMan == NULL)
	{
		tempPlugMan=newDefaultPluginManager();
		const tchar *vrayPluginPath = getVRayPluginPath();
		tempPlugMan->loadLibraryFromPathCollection(vrayPluginPath, "/vray_*.dll", NULL, NULL);
		deletePlugMan=true;
	}
	VR::VRayScene* tmpVrayScene=new VR::VRayScene(tempPlugMan);
	VR::ErrorCode errCode=tmpVrayScene->readFile(file.ptr());
	if (!errCode.error())
	{		
		// find the nodes
		FindPluginOfTypeCallback pluginCallback(CROWDVRAYPLUGINID);
		tempPlugMan->enumPlugins(&pluginCallback);

		// read attributes
		if (pluginCallback._foundPlugins.length())
		{
			VR::VRayPlugin* plugin (pluginCallback._foundPlugins[0]);
			VR::VRayPluginParameter* currentParam = NULL;
			CStr crowdFields;
			
			// cache attributes
			currentParam = plugin->getParameter("glmCrowdField");
			if (currentParam)
			{
				crowdFields = currentParam->getString();
			}
			currentParam = plugin->getParameter("glmCacheName");
			if (currentParam)
			{
				GET_WSTR(currentParam->getString(), currentParamMbcs)
				pblock2->SetValue(pb_cache_name, 0, currentParamMbcs, 0);
			}
			currentParam = plugin->getParameter("glmCacheFileDir");
			if (currentParam)
			{
				GET_WSTR(currentParam->getString(), currentParamMbcs)
				pblock2->SetValue(pb_cache_dir, 0, currentParamMbcs, 0);
			}
			currentParam = plugin->getParameter("glmCharacterFiles");
			if (currentParam)
			{
				GET_WSTR(currentParam->getString(), currentParamMbcs)
				pblock2->SetValue(pb_character_files, 0, currentParamMbcs, 0);
			}

			// motion blur
			currentParam = plugin->getParameter("glmMBlurEnabled");
			if (currentParam) pblock2->SetValue(pb_motion_blur_enable, 0, currentParam->getBool());
			currentParam = plugin->getParameter("glmMBlurStart");
			if (currentParam) pblock2->SetValue(pb_motion_blur_start, 0, (float)currentParam->getDouble());
			currentParam = plugin->getParameter("glmMBlurWindowSize");
			if (currentParam) pblock2->SetValue(pb_motion_blur_window_size, 0, (float)currentParam->getDouble());
			currentParam = plugin->getParameter("glmMBlurSamples");
			if (currentParam) pblock2->SetValue(pb_motion_blur_samples, 0, currentParam->getInt());

			// frustum culling
			currentParam = plugin->getParameter("glmEnableFrustumCulling");
			if (currentParam) pblock2->SetValue(pb_frustum_enable, 0, currentParam->getBool());
			currentParam = plugin->getParameter("glmFrustumMargin");
			if (currentParam) pblock2->SetValue(pb_frustum_margin, 0, (float)currentParam->getDouble());
			currentParam = plugin->getParameter("glmCameraMargin");
			if (currentParam) pblock2->SetValue(pb_camera_margin, 0, (float)currentParam->getDouble());

			// vray
			currentParam = plugin->getParameter("glmFrameOffset");
			if (currentParam) pblock2->SetValue(pb_frame_offset, 0, currentParam->getInt());
			currentParam = plugin->getParameter("glmTransform");
			if (currentParam) pblock2->SetValue(pb_scale_transform, 0, (float)currentParam->getTransform().m[0].length());
			currentParam = plugin->getParameter("glmObjectIDBase");
			if (currentParam) pblock2->SetValue(pb_object_id_base, 0, currentParam->getInt());
			currentParam = plugin->getParameter("glmCameraVisibility");
			if (currentParam) pblock2->SetValue(pb_primary_visibility, 0, currentParam->getBool());
			currentParam = plugin->getParameter("glmShadowsVisibility");
			if (currentParam) pblock2->SetValue(pb_casts_shadows, 0, currentParam->getBool());
			currentParam = plugin->getParameter("glmReflectionsVisibility");
			if (currentParam) pblock2->SetValue(pb_visible_in_reflections, 0, currentParam->getBool());
			currentParam = plugin->getParameter("glmRefractionsVisibility");
			if (currentParam) pblock2->SetValue(pb_visible_in_refractions, 0, currentParam->getBool());

			// other crowdFields?
			for (size_t iPlugin=1; iPlugin<pluginCallback._foundPlugins.length(); ++iPlugin)
			{
				plugin = pluginCallback._foundPlugins[iPlugin];
				currentParam = plugin->getParameter("glmCrowdField");
				if (currentParam)
					crowdFields += (CStr(";") + CStr(currentParam->getString()));
			}
			GET_WSTR(crowdFields, currentParamMbcs)
			pblock2->SetValue(pb_crowd_fields, 0, currentParamMbcs, 0);

			// ok, vray_glmCrowdVRayPlugin.dll is loaded and all params are filled
			CStr logMessage = CStr("VRayGolaem: Success loading .vrscene file \"") + CStr(file.ptr()) + CStr("\" \n");
			mprintf(logMessage.ToBSTR());
		}
		else
		{
			// CROWDVRAYPLUGINID not found = not loaded or env not configured
			CStr vrayEnvVar = CStr(VRAYRT_PLUGINS) + CStr("_") + CStr(PROCESSOR_ARCHITECTURE);
			CStr logMessage = CStr("VRayGolaem: Error loading .vrscene file \"") + CStr(file.ptr()) + CStr("\". vray_glmCrowdVRayPlugin.dll plugin was not found in environment variable \"") + vrayEnvVar + CStr("\" (")+ CStr(getVRayPluginPath()) + CStr(").\n");
			mprintf(logMessage.ToBSTR());
		}
		
	}
	else
	{
		CStr logMessage = CStr("VRayGolaem: Success loading .vrscene file \"") + CStr(file.ptr()) + CStr("\". Vrscene file is invalid.\n");
		mprintf(logMessage.ToBSTR());
	}
	
	// delete the Vray context
	tmpVrayScene->freeMem();
	delete tmpVrayScene;

	if (deletePlugMan)
	{
		tempPlugMan->deleteAll();
		tempPlugMan->unloadAll();
		deleteDefaultPluginManager(tempPlugMan);
	}

	return true;
}

//------------------------------------------------------------
// writeCrowdVRScene: get the node attributes to create a crowd .vrscene
//------------------------------------------------------------
bool VRayGolaem::writeCrowdVRScene(const VR::CharString& file) 
{
	// check file path
	std::stringstream outputStr;
	std::ofstream outputFileStream(file.ptr());
	if (!outputFileStream.is_open()) return false;

	// correct the name of the shader to call. When exporting a scene from Maya with Vray, some shader name special characters are replaced with not parsable character (":" => "__")
	// to be able to find the correct shader name to call, we need to apply the same conversion to the shader names contained in the cam file
	CStr correctedCacheName(_cacheName);
	convertToValidVrsceneName(_cacheName, correctedCacheName);

	MaxSDK::Array<CStr> crowdFields;
	splitStr(_crowdFields, ';', crowdFields);

	for (size_t iCf = 0, nbCf = crowdFields.length(); iCf<nbCf; ++iCf)
	{
		// crowd material
		outputStr << "CrowdCharacterShader " << correctedCacheName << crowdFields[iCf] << "Mtl@material" << std::endl;
		outputStr << "{" << std::endl;
		outputStr << "}" << std::endl;
		outputStr << std::endl;

		// render stats
		outputStr << "MtlRenderStats " << correctedCacheName << crowdFields[iCf] << "Mtl@renderStats" << std::endl;
		outputStr << "{" << std::endl;
		outputStr << "\t" << "base_mtl=" << correctedCacheName << crowdFields[iCf] << "Mtl@material;" << std::endl;
		outputStr << "}" << std::endl;
		outputStr << std::endl;

		// node
		outputStr << "Node " << correctedCacheName << crowdFields[iCf] << "@node" << std::endl;
		outputStr << "{" << std::endl;
		outputStr << "\t" << "transform=Transform(Matrix(Vector(1, 0, 0), Vector(0, 1, 0), Vector(0, 0, 1)), Vector(0, 0, 0));" << std::endl;
		outputStr << "\t" << "geometry=" << correctedCacheName << crowdFields[iCf] << "@mesh1;" << std::endl;
		outputStr << "\t" << "material=" << correctedCacheName << crowdFields[iCf] << "Mtl@renderStats;" << std::endl;
		int nSamples = 1;
		if (_mBlurEnable) nSamples = _mBlurSamples;
		outputStr << "\t" << "nsamples=" << nSamples << ";" << std::endl;
		outputStr << "\t" << "visible=1;" << std::endl;
		outputStr << "}" << std::endl;
		outputStr << std::endl;

		outputStr << "GolaemCrowd " << correctedCacheName << crowdFields[iCf] << "@mesh1" << std::endl;
		outputStr << "{" << std::endl;
		outputStr << "\t" << "glmTransform=Transform(Matrix(Vector("<< _scaleTransform <<", 0, 0), Vector(0, "<< _scaleTransform <<", 0), Vector(0, 0, "<< _scaleTransform <<")), Vector(0, 0, 0));" << std::endl;
		outputStr << "\t" << "glmFrameOffset="<< _frameOffset <<";" << std::endl;
		outputStr << "\t" << "glmCrowdField=\"" << crowdFields[iCf] << "\";" << std::endl;
		outputStr << "\t" << "glmCacheName=\"" << _cacheName << "\";" << std::endl;
		outputStr << "\t" << "glmCacheFileDir=\"" << _cacheDir << "\";" << std::endl;
		outputStr << "\t" << "glmCharacterFiles=\"" << _characterFiles << "\";" << std::endl;
		outputStr << "\t" << "glmExcludedEntities=\"\";" << std::endl;
		// moblur
		outputStr << "\t" << "glmMBlurEnabled=" << _mBlurEnable << ";" << std::endl;
		outputStr << "\t" << "glmMBlurStart=" << _mBlurStart << ";" << std::endl;
		outputStr << "\t" << "glmMBlurWindowSize=" << _mBlurWindowSize << ";" << std::endl;
		outputStr << "\t" << "glmMBlurSamples=" << _mBlurSamples << ";" << std::endl;
		// frustum culling
		outputStr << "\t" << "glmEnableFrustumCulling=" << _frustumEnable << ";" << std::endl;
		outputStr << "\t" << "glmFrustumMargin=" << _frustumMargin << ";" << std::endl;
		outputStr << "\t" << "glmCameraMargin=" << _cameraMargin << ";" << std::endl;
		// vray
		outputStr << "\t" << "glmObjectIDBase=" << _objectIDBase << ";" << std::endl;
		outputStr << "\t" << "glmCameraVisibility=" << _primaryVisibility << ";" << std::endl;
		outputStr << "\t" << "glmShadowsVisibility=" << _castsShadows << ";" << std::endl;
		outputStr << "\t" << "glmReflectionsVisibility=" << _visibleInReflections << ";" << std::endl;
		outputStr << "\t" << "glmRefractionsVisibility=" << _visibleInRefractions << ";" << std::endl;

		outputStr << "\t" << "glmDccPackage=1;" << std::endl;

		outputStr << "}" << std::endl;
		outputStr << std::endl;
	}

	// write in file
	outputFileStream << outputStr.str();
	outputFileStream.close();
	return true;
}

//************************************************************
// Inline utility functions
//************************************************************

//------------------------------------------------------------
// isCharInvalidVrscene
//------------------------------------------------------------
bool isCharInvalidVrscene(char c)
{
	if (c == '|' || c == '@') return false;
	if (c >= 'a' && c <= 'z') return false;
	if (c >= 'A' && c <= 'Z') return false;
	if (c >= '0' && c <= '9') return false;
	return true;
}

//------------------------------------------------------------
// convertToValidVrsceneName
//------------------------------------------------------------
void convertToValidVrsceneName(const CStr& strIn, CStr& strOut)
{
	int strSize = int(strIn.length());
	if (strSize == 0)
	{
		strOut.Resize(0);
		return;
	}
	strOut.Resize(strSize * 2);

	// If the first character is a digit, convert that to a letter
	int pos(0), i(0);
	strOut.dataForWrite()[0] = strIn[0];
	if (strIn[0] >= '0' && strIn[0] <= '9')
	{
		strOut.dataForWrite()[0] = 'a' + (strIn[0] - '0');
		pos++; 
		i++;
	}

	while (i < strSize)
	{
		if (isCharInvalidVrscene(strIn[i]))
		{
			strOut.dataForWrite()[pos++] = '_';
			if (strIn[i] == ':')
			{
				strOut.dataForWrite()[pos++] = '_';
			}
		}
		else strOut.dataForWrite()[pos++] = strIn[i];
		i++;
	}

	strOut.Resize(pos);
}

void splitStr(const CStr& input, char delim, MaxSDK::Array<CStr> & result)
{
	int startPos(0);
	if (input.length() == 0) return;
	
	// first character is delim
	if (input[0]==delim)
	{
		result.append("");
		startPos=1;
	}

	for (int iChar=1, nbChars=input.length(); iChar < nbChars; ++iChar)
	{
		if (input[iChar] == delim)
		{
			CStr tmpStr = input.Substr(startPos, iChar-startPos);
			result.append(tmpStr);
			startPos = iChar+1;
		}
	}

	if (startPos != input.length())
	{
		CStr tmpStr = input.Substr(startPos, input.length()-startPos);
		result.append(tmpStr);
	}
}


//************************************************************
// Inline draw functions
//************************************************************

inline void drawLine(GraphicsWindow *gw, const Point3 &p0, const Point3 &p1) 
{
	Point3 p[3]={ p0, p1 };
	gw->segment(p, TRUE);
}

inline void drawBBox(GraphicsWindow *gw, const Box3 &b) 
{
	gw->setTransform(Matrix3(1));
	Point3 p[8];
	for (int i=0; i<8; i++) p[i]=b[i];
	gw->startSegments();
	drawLine(gw, p[0], p[1]);
	drawLine(gw, p[0], p[2]);
	drawLine(gw, p[3], p[1]);
	drawLine(gw, p[3], p[2]);

	drawLine(gw, p[7], p[6]);
	drawLine(gw, p[7], p[5]);
	drawLine(gw, p[4], p[5]);
	drawLine(gw, p[4], p[6]);

	drawLine(gw, p[0], p[4]);
	drawLine(gw, p[1], p[5]);
	drawLine(gw, p[2], p[6]);
	drawLine(gw, p[3], p[7]);
	gw->endSegments();
}

inline void drawSphere(GraphicsWindow *gw, const Point3 &pos, float radius, int nsegs)
{
	float u0=radius, v0=0.0f;
	Point3 pt[3];

	// draw locator sphere
	gw->startSegments();
	for (int i=0; i<nsegs; i++) 
	{
		float a=2.0f*pi*float(i+1)/float(nsegs);
		float u1=radius*cosf(a);
		float v1=radius*sinf(a);

		pt[0]=Point3(u0, v0, 0.0f) + pos;
		pt[1]=Point3(u1, v1, 0.0f) + pos;
		gw->segment(pt, true);

		pt[0]=Point3(0.0f, u0, v0) + pos;
		pt[1]=Point3(0.0f, u1, v1) + pos;
		gw->segment(pt, true);

		pt[0]=Point3(u0, 0.0f, v0) + pos;
		pt[1]=Point3(u1, 0.0f, v1) + pos;
		gw->segment(pt, true);

		u0=u1;
		v0=v1;
	}
	gw->endSegments();
}

inline void drawText(GraphicsWindow *gw, const MCHAR* text, const Point3& pos) 
{
	IPoint3 ipt;
	gw->wTransPoint(&pos, &ipt);

	// text position
	SIZE sp;
	gw->getTextExtents(text, &sp);

	// draw shadow text
	ipt.x-=sp.cx/2;
	ipt.y-=sp.cy/2;
	gw->setColor(TEXT_COLOR, 0.0f, 0.0f, 0.0f);
	gw->wText(&ipt, text);

	// draw white text
	ipt.x--;
	ipt.y--;
	gw->setColor(TEXT_COLOR, 1.0f, 1.0f, 1.0f);
	gw->wText(&ipt, text);
}



