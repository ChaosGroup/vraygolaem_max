#include "vraygolaem.h"
#include "instance.h"
#include "pb2template_generator.h"
#include "defrayserver.h"
#include "vrender_unicode.h"
#include "hash_map.h"
#include "pluginenumcallbacks.h"
#include "resource.h"

#if GET_MAX_RELEASE(VERSION_3DSMAX) >= 9000
#include "IPathConfigMgr.h"
#endif
#if GET_MAX_RELEASE(VERSION_3DSMAX) >= 11900
#include "IFileResolutionManager.h"
#pragma comment(lib, "assetmanagement.lib")
#endif

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
// Static variables
//************************************************************

IObjParam* VRayGolaem::ip = NULL;
const float iconSize=40.0f;
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

static int ctrlID=100;

int nextID(void) { return ctrlID++; }

static ParamBlockDesc2 param_blk(params, STR_DLGTITLE,  0, &vrayGolaemClassDesc, P_AUTO_CONSTRUCT+P_AUTO_UI, REFNO_PBLOCK,
	IDD_VRAYGOLAEM, IDS_VRAYGOLAEM_PARAMS, 0, 0, &vrayGolaemDlgProc,
	// Params
	pb_file, _T("cache_file"), TYPE_FILENAME, 0, 0,
		p_ui, TYPE_EDITBOX, ed_golaemVrscene,
#if GET_MAX_RELEASE(VERSION_3DSMAX) >= 11900
		p_assetTypeID, MaxSDK::AssetManagement::AssetType::kExternalLink,
#endif
	PB_END,
	pb_shaders_file, _T("shaders_file"), TYPE_FILENAME, 0, 0,
		p_ui, TYPE_EDITBOX, ed_shadersVrscene,
#if GET_MAX_RELEASE(VERSION_3DSMAX) >= 11900
		p_assetTypeID, MaxSDK::AssetManagement::AssetType::kExternalLink,
#endif
	PB_END,
PB_END
);

//************************************************************
// VRayGolaem implementation
//************************************************************

VRayGolaem::VRayGolaem() {
	static int pblockDesc_inited=false;
	if (!pblockDesc_inited) {
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

//**************************************************************************
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
					// sphere->pblock2->SetValue(pb_radius, 0, 0.0f);
					// return CREATE_CONTINUE;
				case 1:
					/*
					float r=Length(np-mat.GetTrans());
					if (r<1e-3f) return CREATE_ABORT;

					// sphere->pblock2->SetValue(pb_radius, 0, r);
					*/
					return CREATE_STOP;
			}
			return CREATE_CONTINUE;

		case MOUSE_MOVE:
			/*
			if (point==1) {
				float r=Length(np-mat.GetTrans());
				sphere->pblock2->SetValue(pb_radius, 0, r);
			}*/
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
	extDispFlags = flags;
}

#define ICON_RADIUS 10

void VRayGolaem::GetLocalBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box) {
	box.Init();
	float radius=ICON_RADIUS; // pblock2->GetFloat(pb_radius, t);
	box+=Point3(-radius, -radius, -radius);
	box+=Point3(radius, radius, radius);
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

inline void drawLine(GraphicsWindow *gw, Point3 &p0, Point3 &p1) {
	Point3 p[3]={ p0, p1 };
	gw->segment(p, TRUE);
}

inline void drawTriangle(GraphicsWindow *gw, Point3 &p0, Point3 &p1, Point3 &p2) {
	Point3 n=Normalize((p1-p0)^(p2-p0));
	Point3 p[4]={ p0, p1, p2 };
	Point3 uvw[4]={ Point3(0,0,0), Point3(0,0,0), Point3(0,0,0) };
	Point3 nrm[4]={ n, n, n };
	gw->triangleN(p, nrm, uvw);
}

inline void drawBBox(GraphicsWindow *gw, Box3 &b) {
	gw->setTransform(Matrix3(1));
	Point3 p[8];
	for (int i=0; i<8; i++) p[i]=b[i];
	gw->startSegments();

	drawLine(gw, p[0], p[1]);
	drawLine(gw, p[0], p[2]);
	drawLine(gw, p[0], p[4]);

	/*
	drawLine(gw, p[7], p[6]);
	drawLine(gw, p[7], p[5]);
	drawLine(gw, p[7], p[3]);
	*/
	gw->endSegments();
}

const float size=40.0f;

void VRayGolaem::draw(TimeValue t, INode *node, ViewExp *vpt) {
	GraphicsWindow *gw=vpt->getGW();

	Matrix3 tm=node->GetObjectTM(t);
	gw->setTransform(tm);

	Color color=Color(node->GetWireColor());
	if (node->IsFrozen()) color=GetUIColor(COLOR_FREEZE);
	else if (node->Selected()) color=GetUIColor(COLOR_SELECTION);
	gw->setColor(LINE_COLOR, color);

	float radius=ICON_RADIUS; // pblock2->GetFloat(pb_radius, t);
	int nsegs=30;
	float u0=radius, v0=0.0f;
	Point3 pt[3];

	gw->startSegments();

	for (int i=0; i<nsegs; i++) {
		float a=2.0f*pi*float(i+1)/float(nsegs);

		float u1=radius*cosf(a);
		float v1=radius*sinf(a);

		pt[0]=Point3(u0, v0, 0.0f);
		pt[1]=Point3(u1, v1, 0.0f);
		gw->segment(pt, true);

		pt[0]=Point3(0.0f, u0, v0);
		pt[1]=Point3(0.0f, u1, v1);
		gw->segment(pt, true);

		pt[0]=Point3(u0, 0.0f, v0);
		pt[1]=Point3(u1, 0.0f, v1);
		gw->segment(pt, true);

		u0=u1;
		v0=v1;
	}

	gw->endSegments();

	tm.NoScale();
	float scaleFactor=vpt->NonScalingObjectSize()*vpt->GetVPWorldWidth(tm.GetTrans())/(float)360.0;
	tm.Scale(Point3(scaleFactor,scaleFactor,scaleFactor));
	gw->setTransform(tm);

	IPoint3 ipt;
	Point3 org(0,0,0);
	gw->wTransPoint(&org, &ipt);

	SIZE sp;
	gw->getTextExtents(iconText, &sp);

	ipt.x-=sp.cx/2;
	ipt.y-=sp.cy/2;

	gw->setColor(TEXT_COLOR, 0.0f, 0.0f, 0.0f);
	gw->wText(&ipt, iconText);

	ipt.x--;
	ipt.y--;
	gw->setColor(TEXT_COLOR, 1.0f, 1.0f, 1.0f);
	gw->wText(&ipt, iconText);
}

int VRayGolaem::Display(TimeValue t, INode* node, ViewExp *vpt, int flags) {
	draw(t, node, vpt);
	return 0;
}

ObjectState VRayGolaem::Eval(TimeValue time) {
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
	return &mesh;
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
				if (ctrlID==bn_golaemBrowse && vrayGolaem) {
					chooseFileName(pblock, pb_file, _T("Choose Golaem .vrscene file"));
				}
				if (ctrlID==bn_shadersBrowse && vrayGolaem) {
					chooseFileName(pblock, pb_shaders_file, _T("Choose shaders .vrscene file"));
				}
			}
			break;
		}
	}

	return FALSE;
}

static const TCHAR *vrsceneExtList=_T("V-Ray scene file (*.vrscene)\0*.vrscene\0All files(*.*)\0*.*\0\0");
static const TCHAR *vrsceneDefExt=_T("vrscene");

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

//**************************************************************
// VRenderObject
int VRayGolaem::init(const ObjectState &os, INode *node, VR::VRayCore *vray) {
	VRenderObject::init(os, node, vray);
	return true;
}

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

// Get the path to the V-Ray plugins; use the V-Ray RT environment variable for this.
const tchar* getVRayPluginPath() {
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

void VRayGolaem::updateVRayParams(TimeValue t, VR::VRayCore *vray) {
	const TCHAR *fname_wstr=pblock2->GetStr(pb_file, t);
	if (!fname_wstr) vrsceneFile="";
	else {
		GET_MBCS(fname_wstr, fname_mbcs);
		vrsceneFile=fname_mbcs;
	}

	const TCHAR *shadersName_wstr=pblock2->GetStr(pb_shaders_file, t);
	if (!shadersName_wstr) shadersFile="";
	else {
		GET_MBCS(shadersName_wstr, shadersName_mbcs);
		shadersFile=shadersName_mbcs;
	}
}

void VRayGolaem::renderBegin(TimeValue t, VR::VRayCore *_vray) {
	VR::VRayRenderer *vray=static_cast<VR::VRayRenderer*>(_vray);
	VRenderObject::renderBegin(t, vray);

	updateVRayParams(t, vray);

	const VR::VRaySequenceData &sdata=vray->getSequenceData();

	if (!golaemPlugman) {
		golaemPlugman=newDefaultPluginManager();
		const tchar *vrayPluginPath = getVRayPluginPath();
		sdata.progress->info("VRayGolaem: Loading V-Ray plugins from \"%s\"", getVRayPluginPath());
		golaemPlugman->loadLibraryFromPathCollection(vrayPluginPath, "/vray_*.dll", NULL, vray->getSequenceData().progress);
	}

	// Load the .vrscene into the plugin manager
	vrayScene=new VR::VRayScene(golaemPlugman);

	if (vrsceneFile.empty()) {
		sdata.progress->warning("VRayGolaem: No .vrscene file specified");
	} else {
		VR::ErrorCode errCode=vrayScene->readFile(vrsceneFile.ptr());
		if (errCode.error()) {
			VR::CharString errMsg=errCode.getErrorString();
			sdata.progress->warning("VRayGolaem: Error loading .vrscene file \"%s\": %s", vrsceneFile.ptr(), errMsg.ptr());
		} else {
			sdata.progress->info("VRayGolaem: Scene file \"%s\" loaded successfully", vrsceneFile.ptr());
		}
	}

	if (shadersFile.empty()) {
		sdata.progress->warning("VRayGolaem: No shaders .vrscene file specified");
	} else {
		VR::ErrorCode errCode=vrayScene->readFile(shadersFile.ptr());
		if (errCode.error()) {
			VR::CharString errMsg=errCode.getErrorString();
			sdata.progress->warning("VRayGolaem: Error loading shaders .vrscene file \"%s\": %s", shadersFile.ptr(), errMsg.ptr());
		} else {
			sdata.progress->info("VRayGolaem: Shaders file \"%s\" loaded successfully", shadersFile.ptr());
		}
	}

	callRenderBegin(vray);
}

void VRayGolaem::renderEnd(VR::VRayCore *_vray) {
	VR::VRayRenderer *vray=static_cast<VR::VRayRenderer*>(_vray);
	VRenderObject::renderEnd(vray);
	
	callRenderEnd(vray);
	if (vrayScene) {
		vrayScene->freeMem();
		delete vrayScene;
		vrayScene=NULL;
	}
}

void VRayGolaem::frameBegin(TimeValue t, VR::VRayCore *_vray) {
	VR::VRayRenderer *vray=static_cast<VR::VRayRenderer*>(_vray);
	VRenderObject::frameBegin(t, vray);

	callFrameBegin(vray);
}

void VRayGolaem::frameEnd(VR::VRayCore *_vray) {
	VR::VRayRenderer *vray=static_cast<VR::VRayRenderer*>(_vray);
	VRenderObject::frameEnd(vray);

	callFrameEnd(vray);
}

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

