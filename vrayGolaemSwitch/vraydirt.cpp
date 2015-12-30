
#pragma warning( push )
#pragma warning( disable : 4100 4251 4273 4275 4996 )

#include "max.h"
#include "imtl.h"
#include "texutil.h"
#include "iparamm2.h"
#if GET_MAX_RELEASE(VERSION_3DSMAX) >= 6000
#include "IMtlRender_Compatibility.h"
#endif
#if GET_MAX_RELEASE(VERSION_3DSMAX) >= 13900
#include <IMaterialBrowserEntryInfo.h>
#endif

#include "pb2template_generator.h"

#include "vrayinterface.h"
#include "shadedata_new.h"
#include "vrayrenderer.h"
#include "vraydmcsampler.h"
#include "vrayplugins.h"
#include "vraytexutils.h"
#include "tomax.h"
#include "vrender_unicode.h"

#include "resource.h"

#include "vraydirt_impl.h"
//Class_ID and paramblock enum moved to that file ... 
#include "vraydirt.h"


#if MAX_RELEASE<13900
#include "maxscrpt/maxscrpt.h"
#include "maxscrpt/value.h"
#else
#include "maxscript/maxscript.h"
#include "maxscript/kernel/value.h"
#endif

using namespace VRayGolaemSwitch;

// no param block script access for VRay free
#ifdef _FREE_
#define _FT(X) _T("")
#define IS_PUBLIC 0
#else
#define _FT(X) _T(X)
#define IS_PUBLIC 1
#endif // _FREE_

/*===========================================================================*\
|	Class Descriptor
\*===========================================================================*/

class SkeletonTexmapClassDesc:public ClassDesc2
#if GET_MAX_RELEASE(VERSION_3DSMAX) >= 6000
	, public IMtlRender_Compatibility_MtlBase 
#endif
#if GET_MAX_RELEASE(VERSION_3DSMAX) >= 13900
	, public IMaterialBrowserEntryInfo
#endif
{
#if GET_MAX_RELEASE(VERSION_3DSMAX) >= 6000
	HIMAGELIST imageList;
#endif

public:
	int IsPublic() { return IS_PUBLIC; }
	void* Create(BOOL loading) { return new SkeletonTexmap; }
	const TCHAR* ClassName() { return STR_CLASSNAME; }
	SClass_ID SuperClassID() { return TEXMAP_CLASS_ID; }
	Class_ID ClassID() { return VRAYDIRT_CLASS_ID; }
	const TCHAR* Category() { return _T(""); }

	// Hardwired name, used by MAX Script as unique identifier
	const TCHAR* InternalName() { return _T("SkeletonTexmap"); }
	HINSTANCE HInstance() { return hInstance; }

#if GET_MAX_RELEASE(VERSION_3DSMAX) >= 6000
	SkeletonTexmapClassDesc() {
		imageList=NULL;
		IMtlRender_Compatibility_MtlBase::Init(*this);
	}

	~SkeletonTexmapClassDesc() {
		if (imageList) ImageList_Destroy(imageList);
		imageList=NULL;
	}

	// From IMtlRender_Compatibility_MtlBase
	bool IsCompatibleWithRenderer(ClassDesc& rendererClassDesc) {
		if (rendererClassDesc.ClassID()!=VRENDER_CLASS_ID) return false;
		return true;
	}

	bool GetCustomMtlBrowserIcon(HIMAGELIST& hImageList, int& inactiveIndex, int& activeIndex, int& disabledIndex) {
		if (!imageList) {
			HBITMAP bmp=LoadBitmap(hInstance, MAKEINTRESOURCE(bm_TEXICO));
			HBITMAP mask=LoadBitmap(hInstance, MAKEINTRESOURCE(bm_TEXICOMASK));

			imageList=ImageList_Create(11, 11, ILC_COLOR24 | ILC_MASK, 5, 0);
			int index=ImageList_Add(imageList, bmp, mask);
			if (index==-1) return false;
		}

		if (!imageList) return false;

		hImageList=imageList;
		inactiveIndex=0;
		activeIndex=1;
		disabledIndex=2;
		return true;
	}
#endif

#if GET_MAX_RELEASE(VERSION_3DSMAX) >= 13900
	FPInterface* GetInterface(Interface_ID id) {
		if (IMATERIAL_BROWSER_ENTRY_INFO_INTERFACE==id) {
			return static_cast<IMaterialBrowserEntryInfo*>(this);
		}
		return ClassDesc2::GetInterface(id);
	}

	// From IMaterialBrowserEntryInfo
	const MCHAR* GetEntryName() const { return NULL; }
	const MCHAR* GetEntryCategory() const {
#if GET_MAX_RELEASE(VERSION_3DSMAX) >= 14900
		HINSTANCE hInst=GetModuleHandle(_T("sme.gup"));
		if (hInst) {
			static MSTR category(MaxSDK::GetResourceStringAsMSTR(hInst, IDS_3DSMAX_SME_MAPS_CATLABEL).Append(_T("\\V-Ray")));
			return category.data();
		}
#endif
		return _T("Maps\\V-Ray");
	}
	Bitmap* GetEntryThumbnail() const { return NULL; }
#endif
};

static SkeletonTexmapClassDesc SkelTexmapCD;

/*===========================================================================*\
|	DLL stuff
\*===========================================================================*/

HINSTANCE hInstance;
int controlsInit=false;

bool WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) {
	hInstance = hinstDLL;

	if (!controlsInit) {
		controlsInit = TRUE;
#if MAX_RELEASE<13900
		InitCustomControls(hInstance);
#endif
		InitCommonControls();
	}

	switch(fdwReason) {
	case DLL_PROCESS_ATTACH:
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return(TRUE);
}

__declspec(dllexport) const TCHAR *LibDescription() { return STR_LIBDESC; }
__declspec(dllexport) int LibNumberClasses() { return 1; }

__declspec(dllexport) ClassDesc* LibClassDesc(int i) {
	switch(i) {
	case 0: return &SkelTexmapCD;
	default: return 0;
	}
}

__declspec(dllexport) ULONG LibVersion() { return VERSION_3DSMAX; }

/*===========================================================================*\
|	Parameter block
\*===========================================================================*/

// Paramblock2 name
//moved to pb2vraydirt.h
//enum { tex_params, }; 

static int numID=100;
int ctrlID(void) { return numID++; }

static ParamBlockDesc2 stex_param_blk ( 
	tex_params, _T("SkeletonTexmap parameters"),  0, &SkelTexmapCD, P_AUTO_CONSTRUCT + P_AUTO_UI, 0, 
	//rollout
	0, 0, 0, 0, NULL,
	// params
	pb_map_selector, _FT("texmap_selector"), TYPE_TEXMAP, 0, 0,
	p_subtexno, 0,
	p_ui, TYPE_TEXMAPBUTTON, ctrlID(),
	PB_END,
	pb_start_offset, _FT("startOffset"), TYPE_INT, P_ANIMATABLE, 0,
	p_default, 0,
	p_ui, TYPE_SPINNER, EDITTYPE_POS_INT, ctrlID(), ctrlID(), 1,
	PB_END,
	pb_map_default, _FT("texmap_default"), TYPE_TEXMAP, 0, 0,
	p_subtexno, 1,
	p_ui, TYPE_TEXMAPBUTTON, ctrlID(),
	PB_END,
	pb_map_shader0, _FT("texmap_shader0"), TYPE_TEXMAP, 0, 0,
	p_subtexno, 2,
	p_ui, TYPE_TEXMAPBUTTON, ctrlID(),
	PB_END,
	pb_map_shader1, _FT("texmap_shader1"), TYPE_TEXMAP, 0, 0,
	p_subtexno, 3,
	p_ui, TYPE_TEXMAPBUTTON, ctrlID(),
	PB_END,
	pb_map_shader2, _FT("texmap_shader2"), TYPE_TEXMAP, 0, 0,
	p_subtexno, 4,
	p_ui, TYPE_TEXMAPBUTTON, ctrlID(),
	PB_END,
	pb_map_shader3, _FT("texmap_shader3"), TYPE_TEXMAP, 0, 0,
	p_subtexno, 5,
	p_ui, TYPE_TEXMAPBUTTON, ctrlID(),
	PB_END,
	pb_map_shader4, _FT("texmap_shader4"), TYPE_TEXMAP, 0, 0,
	p_subtexno, 6,
	p_ui, TYPE_TEXMAPBUTTON, ctrlID(),
	PB_END,
	pb_map_shader5, _FT("texmap_shader5"), TYPE_TEXMAP, 0, 0,
	p_subtexno, 7,
	p_ui, TYPE_TEXMAPBUTTON, ctrlID(),
	PB_END,
	pb_map_shader6, _FT("texmap_shader6"), TYPE_TEXMAP, 0, 0,
	p_subtexno, 8,
	p_ui, TYPE_TEXMAPBUTTON, ctrlID(),
	PB_END,
	pb_map_shader7, _FT("texmap_shader7"), TYPE_TEXMAP, 0, 0,
	p_subtexno, 9,
	p_ui, TYPE_TEXMAPBUTTON, ctrlID(),
	PB_END,
	pb_map_shader8, _FT("texmap_shader8"), TYPE_TEXMAP, 0, 0,
	p_subtexno, 10,
	p_ui, TYPE_TEXMAPBUTTON, ctrlID(),
	PB_END,
	pb_map_shader9, _FT("texmap_shader9"), TYPE_TEXMAP, 0, 0,
	p_subtexno, 11,
	p_ui, TYPE_TEXMAPBUTTON, ctrlID(),
	PB_END,
PB_END
);

void SkeletonTexmap::greyDlgControls(IParamMap2 *map) {
	if (!map) return;
}

/*===========================================================================*\
|	Constructor and Reset systems
|  Ask the ClassDesc2 to make the AUTO_CONSTRUCT paramblocks and wire them in
\*===========================================================================*/

void SkeletonTexmap::Reset() {
	_ivalid.SetEmpty();
	SkelTexmapCD.Reset(this);
}

SkeletonTexmap::SkeletonTexmap()
	: Texmap() 
{
	static int pblockDesc_inited=false;
	if (!pblockDesc_inited) {
		initPBlockDesc(stex_param_blk);
		pblockDesc_inited=true;
	}

	pblock=NULL;
	_ivalid.SetEmpty();
	SkelTexmapCD.MakeAutoParamBlocks(this);	// make and intialize paramblock2
	InitializeCriticalSection(&_csect);
}

SkeletonTexmap::~SkeletonTexmap() {
	DeleteCriticalSection(&_csect);
}

ParamDlg* SkeletonTexmap::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) {
	return new SkelTexParamDlg(this, hwMtlEdit, imp);
	//IAutoMParamDlg* masterDlg = SkelTexmapCD.CreateParamDlgs(hwMtlEdit, imp, this);
	//return masterDlg;
}

/*===========================================================================*\
|	Subanim & References support
\*===========================================================================*/

RefTargetHandle SkeletonTexmap::GetReference(int i) {
	return pblock;
}

void SkeletonTexmap::SetReference(int i, RefTargetHandle rtarg) {
	pblock=(IParamBlock2*) rtarg;
}

TSTR SkeletonTexmap::SubAnimName(int i) {
	return STR_DLGTITLE;
}

Animatable* SkeletonTexmap::SubAnim(int i) {
	return pblock;
}

RefResult SkeletonTexmap::NotifyRefChanged(NOTIFY_REF_CHANGED_ARGS) 
{
	switch (message) {
	case REFMSG_CHANGE:
		_ivalid.SetEmpty();
		if (hTarget == pblock) {
			ParamID changing_param = pblock->LastNotifyParamID();
			IParamMap2 *map=pblock->GetMap();
			if (map) {
				map->Invalidate(changing_param);
			}
		}
		break;
	case REFMSG_NODE_HANDLE_CHANGED: 
		{
			//IMergeManager* imm = (IMergeManager*) partID;
			//if (imm) {}
			return REF_STOP;
		}
	}
	return(REF_SUCCEED);
}

/*===========================================================================*\
|	Updating and cloning
\*===========================================================================*/

RefTargetHandle SkeletonTexmap::Clone(RemapDir &remap) {
	SkeletonTexmap *mnew = new SkeletonTexmap();
	*((MtlBase*)mnew) = *((MtlBase*)this);  // copy superclass stuff
	BaseClone(this, mnew, remap);

	mnew->ReplaceReference(0, remap.CloneRef(pblock));

	mnew->_ivalid.SetEmpty();	
	return (RefTargetHandle)mnew;
}

void SkeletonTexmap::NotifyChanged() {
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}

void SkeletonTexmap::Update(TimeValue t, Interval& valid) {		
	if (!_ivalid.InInterval(t)) 
	{
		_ivalid.SetInfinite();
		pblock->GetValue(pb_map_selector, t, _texmapSelector, _ivalid);
		pblock->GetValue(pb_start_offset, t, _startOffset, _ivalid);

		pblock->GetValue(pb_map_default, t, _texmapDefault, _ivalid);
		pblock->GetValue(pb_map_shader0, t, _texmapShader0, _ivalid);
		pblock->GetValue(pb_map_shader1, t, _texmapShader1, _ivalid);
		pblock->GetValue(pb_map_shader2, t, _texmapShader2, _ivalid);
		pblock->GetValue(pb_map_shader3, t, _texmapShader3, _ivalid);
		pblock->GetValue(pb_map_shader4, t, _texmapShader4, _ivalid);
		pblock->GetValue(pb_map_shader5, t, _texmapShader5, _ivalid);
		pblock->GetValue(pb_map_shader6, t, _texmapShader6, _ivalid);
		pblock->GetValue(pb_map_shader7, t, _texmapShader7, _ivalid);
		pblock->GetValue(pb_map_shader8, t, _texmapShader8, _ivalid);
		pblock->GetValue(pb_map_shader9, t, _texmapShader9, _ivalid);

		if (_texmapSelector) _texmapSelector->Update(t, _ivalid);
		if (_texmapDefault) _texmapDefault->Update(t, _ivalid);
		if (_texmapShader0) _texmapShader0->Update(t, _ivalid);
		if (_texmapShader1) _texmapShader1->Update(t, _ivalid);
		if (_texmapShader2) _texmapShader2->Update(t, _ivalid);
		if (_texmapShader3) _texmapShader3->Update(t, _ivalid);
		if (_texmapShader4) _texmapShader4->Update(t, _ivalid);
		if (_texmapShader5) _texmapShader5->Update(t, _ivalid);
		if (_texmapShader6) _texmapShader6->Update(t, _ivalid);
		if (_texmapShader7) _texmapShader7->Update(t, _ivalid);
		if (_texmapShader8) _texmapShader8->Update(t, _ivalid);
		if (_texmapShader9) _texmapShader9->Update(t, _ivalid);
	}
	_shadeCache.renderEnd(NULL);
	_cacheInit=false;
	valid &= _ivalid;
}

/*===========================================================================*\
|	Dlg Definition
\*===========================================================================*/

INT_PTR SkelTexDlgProc::DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	int id=LOWORD(wParam);
	switch (msg) {
	case WM_INITDIALOG: {
		SkeletonTexmap *texmap=(SkeletonTexmap*) (map->GetParamBlock()->GetOwner());
		texmap->greyDlgControls(map);
		break;
						}
	case WM_DESTROY:
		break;
	case WM_COMMAND:
		break;
	}
	return FALSE;
}

static Pb2TemplateGenerator templateGenerator;

SkelTexParamDlg::SkelTexParamDlg(SkeletonTexmap *m, HWND hWnd, IMtlParams *i) {
	texmap=m;
	ip=i;

	DLGTEMPLATE* tmp=templateGenerator.GenerateTemplateEx(texmap->pblock, STR_DLGTITLE, 217, NULL, 0);
	pmap=CreateMParamMap2(texmap->pblock, ip, hInstance, hWnd, NULL, NULL, tmp, STR_DLGTITLE, 0, &dlgProc);
	templateGenerator.ReleaseDlgTemplate(tmp);
}

void SkelTexParamDlg::DeleteThis(void) {
	if (pmap) DestroyMParamMap2(pmap);
	pmap=NULL;
	delete this;
}

/*===========================================================================*\
|	Actual shading takes place
\*===========================================================================*/

static AColor white(1.0f,1.0f,1.0f,1.0f);

void SkeletonTexmap::writeElements(VR::VRayContext &rc, const TexmapCache &res) 
{
	//if (!affectElements || !rc.mtlresult.fragment || mode==0) return;

	rc.mtlresult.fragment->setChannelDataByAlias(REG_CHAN_VFB_REFLECT, &res.reflection);
	rc.mtlresult.fragment->setChannelDataByAlias(REG_CHAN_VFB_REFLECTION_FILTER, &res.reflectionFilter);
	rc.mtlresult.fragment->setChannelDataByAlias(REG_CHAN_VFB_RAW_REFLECTION, &res.reflectionRaw);
}

AColor SkeletonTexmap::EvalColor(ShadeContext& sc) 
{
	// If the shade context is not generated by VRay - do nothing.
	if (!sc.globContext || sc.ClassID()!=VRAYCONTEXT_CLASS_ID) return AColor(0.0f, 0.0f, 0.0f, 0.0f);
	
	// Convert the shade context to the VRay-for-3dsmax context.
	VR::VRayInterface &rc=(VR::VRayInterface&) sc;
	// Check if the cache has been initialized.
	if (!_cacheInit) 
	{
		EnterCriticalSection(&_csect);
		if (!_cacheInit) 
		{
			_shadeCache.renderBegin((VR::VRayRenderer*) rc.vray);
			_cacheInit=true;
		}
		LeaveCriticalSection(&_csect);
	}
	
	// already in cache?
	TexmapCache res;
	res.color=AColor(1.0f, 0.47f, 0.0f, 0.0f);
	if (_shadeCache.getCache(rc, res)) 
		return res.color;

	// invalid selector
	//int newSelector(getSelectorValue(rc)-_startOffset);
	int newSelector(-_startOffset);
	if (_texmapSelector) newSelector+= (int)_texmapSelector->EvalMono(sc);
	if ((newSelector < 0) || (newSelector >= 10))
	{
		Texmap* map (GetSubTexmap(0));
		if (map) res.color = map->EvalColor(sc);
	}
	// valid
	else
	{
		Texmap* map (GetSubTexmap(newSelector+2)); // +2 as the first subTexMaps are the selector & the defaultShader
		if (map) res.color = map->EvalColor(sc);
	}

	_shadeCache.putCache(rc, res);
	return res.color;
	
	/*if (gbufID) sc.SetGBufferID(gbufID);



	// Convert the shade context to the VRay-for-3dsmax context.
	VR::VRayInterface &rc=(VR::VRayInterface&) sc;

	// Ignore for shadow rays
	// We cannot ignore a texture for shadow rays, as it may be cached at that point, so when we come to the
	// actual evaluation we will return the black cached result.
	// if (sc.mode==SCMODE_SHADOW || (rc.rayparams.rayType & VR::RT_SHADOW)) return res;
	if (rc.rayparams.totalLevel>=150) return AColor(0.0f, 0.0f, 0.0f, 0.0f);

	}*/

	//TexmapCache res;
	//res.color=AColor(1.0f, 0.47f, 0.0f, 0.0f);

	// If the surface point is already in the cache - just return the cached value.
	/*if (shadeCache.getCache(rc, res)) {
	writeElements(rc, res);
	return res.color;
	}*/


	//Color occluded=_combineTex(sc, occludedColor, texmapOccluded, texmapOccludedMult);

	// TODO

	//shadeCache.putCache(rc, res);

	//writeElements(rc, res);
	//return res.color;
}

float SkeletonTexmap::EvalMono(ShadeContext& sc) {
	if (gbufID) sc.SetGBufferID(gbufID);
	Color c=EvalColor(sc);
	return Intens(c);
}

Point3 SkeletonTexmap::EvalNormalPerturb(ShadeContext& sc) 
{
	if (gbufID) sc.SetGBufferID(gbufID);
	return Point3(0,0,0);
}

int SkeletonTexmap::NumSubTexmaps() { return 12; }

Texmap* SkeletonTexmap::GetSubTexmap(int i) 
{
	if (i==0) return pblock->GetTexmap(pb_map_selector);
	else if (i==1) return pblock->GetTexmap(pb_map_default);
	else if (i==2) return pblock->GetTexmap(pb_map_shader0);
	else if (i==3) return pblock->GetTexmap(pb_map_shader1);
	else if (i==4) return pblock->GetTexmap(pb_map_shader2);
	else if (i==5) return pblock->GetTexmap(pb_map_shader3);
	else if (i==6) return pblock->GetTexmap(pb_map_shader4);
	else if (i==7) return pblock->GetTexmap(pb_map_shader5);
	else if (i==8) return pblock->GetTexmap(pb_map_shader6);
	else if (i==9) return pblock->GetTexmap(pb_map_shader7);
	else if (i==10) return pblock->GetTexmap(pb_map_shader8);
	else if (i==11) return pblock->GetTexmap(pb_map_shader9);
	return NULL;
}
void SkeletonTexmap::SetSubTexmap(int i, Texmap *m) 
{
	if (i==0) pblock->SetValue(pb_map_selector, 0, m);	
	else if (i==1) pblock->SetValue(pb_map_default, 0, m);
	else if (i==2) pblock->SetValue(pb_map_shader0, 0, m);
	else if (i==3) pblock->SetValue(pb_map_shader1, 0, m);
	else if (i==4) pblock->SetValue(pb_map_shader2, 0, m);
	else if (i==5) pblock->SetValue(pb_map_shader3, 0, m);
	else if (i==6) pblock->SetValue(pb_map_shader4, 0, m);
	else if (i==7) pblock->SetValue(pb_map_shader5, 0, m);
	else if (i==8) pblock->SetValue(pb_map_shader6, 0, m);
	else if (i==9) pblock->SetValue(pb_map_shader7, 0, m);
	else if (i==10) pblock->SetValue(pb_map_shader8, 0, m);
	else if (i==11) pblock->SetValue(pb_map_shader9, 0, m);
}
TSTR SkeletonTexmap::GetSubTexmapSlotName(int i) 
{
	if (i==0) return TSTR(STR_SELECTORNAME);
	if (i==1) return TSTR(STR_DEFAULTNAME);
	if (i==2) return TSTR(STR_SHADER0NAME);
	if (i==3) return TSTR(STR_SHADER1NAME);
	if (i==4) return TSTR(STR_SHADER2NAME);
	if (i==5) return TSTR(STR_SHADER3NAME);
	if (i==6) return TSTR(STR_SHADER4NAME);
	if (i==7) return TSTR(STR_SHADER5NAME);
	if (i==8) return TSTR(STR_SHADER6NAME);
	if (i==9) return TSTR(STR_SHADER7NAME);
	if (i==10) return TSTR(STR_SHADER8NAME);
	if (i==11) return TSTR(STR_SHADER9NAME);
	return TSTR(_T(""));
}

