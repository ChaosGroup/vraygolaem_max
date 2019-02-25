/***************************************************************************
*                                                                          *
*  Copyright (C) Chaos Group & Golaem S.A. - All Rights Reserved.          *
*                                                                          *
***************************************************************************/

#include "vraygolaemHSL.h"

#include <algorithm>

#include "resource.h"

#include "vraygolaemHSL_impl.h"

#pragma warning(push)
#pragma warning(disable : 4535)

#if MAX_RELEASE < 13900
#include "maxscrpt/maxscrpt.h"
#include "maxscrpt/value.h"
#else
#include "maxscript/maxscript.h"
#include "maxscript/kernel/value.h"
#endif

#if GET_MAX_RELEASE(VERSION_3DSMAX) >= 6000
#include "IMtlRender_Compatibility.h"
#endif
#if GET_MAX_RELEASE(VERSION_3DSMAX) >= 13900
#include <IMaterialBrowserEntryInfo.h>
#endif

#pragma warning(pop)

/*
Implementation
*/

inline int intCompare(const void* a, const void* b)
{
    const int& ia = *(const int*)a;
    const int& ib = *(const int*)b;
    return ia - ib;
}

struct stExcludeIDList
{
    stExcludeIDList(){};
    ~stExcludeIDList()
    {
        IDList.freeMem();
    }

private:
    VR::Table<int> IDList;
    int excludeType;

public:
    void Add(int ID)
    {
        IDList += ID;
    }
    void Init(int iCount, int iType = 0) //"exclude" default type
    {
        IDList.setCount(iCount);
        IDList.setCount(0);
        excludeType = iType;
    }
    void Sort()
    {
        qsort(&IDList[0], IDList.count(), sizeof(int), intCompare);
    }

    int Find(int renderID)
    {
        if (IDList.count() == 0)
            return false;

        int b[2];
        for (b[0] = -1, b[1] = IDList.count(); b[1] - b[0] > 1;)
        {
            int middle = (b[0] + b[1]) / 2;
            b[renderID < IDList[middle]] = middle;
        }
        if (excludeType == 0)
            return (renderID == IDList[b[0]]);
        else
            return !(renderID == IDList[b[0]]);
    }

    bool Empty() const
    {
        return IDList.count() == 0;
    }
};

#pragma warning(push)
#pragma warning(disable : 4512)

// The occlusion sampler.
struct VRayGolaemHSLSampler : VR::AdaptiveColorSampler
{
private:
    float radius, distribution, falloff;
    int sameObjectOnly, ignoreSelfOcclusion;
    float cosMult;
    int workWithTransparency;
    VR::Vector startPoint;
    bool bAllTranspLevels;
    int mode, sampleEnvironment;

    // Indicates if the intersected, but not affecting the result objects are treated as transparent.
    // If not, no further intersections after these objects are processed.
    int excludedObjectsTransparent;

    float finalOcclusion;

    VR::Color occludedColor, unoccludedColor;

    VR::Matrix nm; // A matrix for transforming from local surface space to world space.

    stExcludeIDList& renderIDs;
    stExcludeIDList& affectresultIDs;
    int affectInclusive;

public:
    VRayGolaemHSLSampler(stExcludeIDList& exclList, stExcludeIDList& affectList, int affectListInclusive = 0)
        : renderIDs(exclList)
        , affectresultIDs(affectList)
        , affectInclusive(affectListInclusive)
    {
        if (affectresultIDs.Empty())
            affectInclusive = 0;
    }
    ~VRayGolaemHSLSampler()
    {
    }

    void init(const VR::VRayContext& rc, VR::Vector& normal, float rad, float distr, int sameOnly, float fall, float csm,
              int workWithTransparencyParam, const VR::Vector& p, const VR::Color& _occludedColor, const VR::Color& _unoccludedColor,
              bool AllTranspLevels = true, int _ignoreSelfOcclusion = false, int _mode = 0, int _sampleEnvironment = false, int _excludedObjectsTransparent = false)
    {
        radius = rad;
        distribution = distr;
        sameObjectOnly = sameOnly;
        falloff = fall;
        mode = _mode;
        if (_mode != 1)
        {
            VR::makeNormalMatrix(normal, nm);
        }
        else
        {
            VR::Vector reflectDir = VR::getReflectDir(rc.rayparams.viewDir, normal);
            VR::makeNormalMatrix(reflectDir, nm);
        }
        cosMult = csm;
        this->workWithTransparency = workWithTransparencyParam;
        startPoint = p;
        bAllTranspLevels = AllTranspLevels;
        ignoreSelfOcclusion = _ignoreSelfOcclusion;
        occludedColor = _occludedColor;
        unoccludedColor = _unoccludedColor;
        sampleEnvironment = _sampleEnvironment;
        excludedObjectsTransparent = _excludedObjectsTransparent;
        startPoint += nm[2] * rc.vray->getSequenceData().params.options.ray_bias;

        finalOcclusion = 0.0f;
    }

    VR::Vector getSpecularDir(float pu, float pv, float pn)
    {
        float thetaSin;
        if (pn >= 0.0f)
        {
            thetaSin = powf(pu, 1.0f / (pn + 1.0f));
        }
        else
        {
            thetaSin = 1.0f - powf(1.0f - pu, 1.0f / (1.0f - pn));
        }
        float thetaCos = sqrtf(VR::Max(0.0f, 1.0f - thetaSin * thetaSin));
        float phi = 2.0f * VR::pi() * pv;
        return VR::Vector(cosf(phi) * thetaCos, sinf(phi) * thetaCos, thetaSin);
    }

    VR::Color sampleColor(const VR::VRayContext& rc, VR::VRayContext& nrc, float uc, VR::ValidType& valid)
    {
        // Compute a sampling direction.
        VR::Vector dir;
        if (mode == 0)
        {
            // Ambient occlusion
            dir = VR::normalize0(nm * this->getSpecularDir(uc, getDMCParam(nrc, 1), distribution));
        }
        else if (distribution < 0.0f)
        {
            // Mirror
            dir = VR::getReflectDir(rc.rayparams.viewDir, rc.rayresult.normal);
        }
        else
        {
            // Glossy AO
            switch (mode)
            {
            case 1:
                // Ambient occlusion/Phong reflection
                dir = VR::normalize0(nm * VR::getSpecularDir(uc, getDMCParam(nrc, 1), distribution));
                break;
            case 2:
            {
                // Blinn reflection
                VR::Vector nrm = VR::normalize0(nm * VR::getSpecularDir(uc, getDMCParam(nrc, 1), distribution));
                dir = VR::getReflectDir(rc.rayparams.viewDir, nrm);
                break;
            }
            case 3:
            {
                // Ward reflection
                float k = -logf(1.0f - uc);
                if (k < 0.0f)
                    k = 0.0f;

                float roughness = 1.0f / (distribution + 1.0f);
                VR::real thetaCos = sqrtf(1.0f / (roughness * k + 1.0f));
                VR::Vector hn = VR::getSphereDir(thetaCos, getDMCParam(nrc, 1));
                VR::Vector nrm = VR::normalize0(nm * hn);

                dir = VR::getReflectDir(rc.rayparams.viewDir, nrm);
                break;
            }
            }
        }

        // Check if the direction is below the geometric normal - if yes, do nothing.
        float n = dotf(dir, rc.rayresult.gnormal) * cosMult;
        if (n < 0.0f)
        {
            valid = false;
            return Vlado::Color(0, 0, 0);
        }

        // Set up the ray context for tracing in the given direction.
        nrc.rayparams.viewDir = dir;
        nrc.rayparams.tracedRay.p = nrc.rayparams.rayOrigin = startPoint;
        nrc.rayparams.tracedRay.dir = dir;
        nrc.rayparams.skipTag = rc.rayresult.skipTag;
        nrc.rayparams.mint = 0.0f;
        nrc.rayparams.maxt = radius;

        float occlusion = 0.0f;
        float transp = 1.0f;
        float step = 0.0f;
        //bool bGetTexture = false;

        int maxLevels = bAllTranspLevels ? nrc.vray->getSequenceData().params.options.mtl_transpMaxLevels : 1;
        for (int i = 0; i < maxLevels; i++)
        {
            // Find the first intersection without shading it.
            VR::IntersectionData isData;
            int res = nrc.vray->findIntersection(nrc, &isData);
            if (bAllTranspLevels && res && fabs(isData.wpointCoeff - nrc.rayparams.mint) < 1e-6)
            {
                step += 1e-6f + step;
                nrc.rayparams.mint += step;
                continue;
            }
            step = 0.0f;

            // When using Tex with VRayScatter with overriden getSurfaceRenderID()
            // there was a crash with incorrect rayresult.primitive pointer
            //
            nrc.rayresult.primitive = isData.primitive;

            // If we have intersected something, increase the occlusion, otherwise - keep it the same.
            if (res)
            {
                float currentTransp = 0.0f;
                float k = 0.0f;

                // Indicates if the intersected object is excluded from the calculations
                bool excluded = false;

                VR::DefaultVRayShadeData* sd = (VR::DefaultVRayShadeData*)GET_INTERFACE(isData.sd, EXT_DEFAULT_SHADE_DATA);
                VR::DefaultVRayShadeData* sd2 = (VR::DefaultVRayShadeData*)GET_INTERFACE(rc.rayresult.sd, EXT_DEFAULT_SHADE_DATA);

                if (((!sameObjectOnly && (sd && !(affectInclusive ^ affectresultIDs.Find(sd->getSurfaceRenderID(nrc))))) ||
                     (sameObjectOnly && isData.sb == rc.rayresult.sb)) &&
                    (!ignoreSelfOcclusion || isData.sb != rc.rayresult.sb) &&
                    (sd2 && !renderIDs.Find(sd2->getSurfaceRenderID(nrc))) &&
                    (!isData.surfaceProps || !isData.surfaceProps->getFlag(VR::surfPropFlag_excludeInAO)))
                    k = (falloff > 1e-6f) ? powf(1.0f - (float)isData.wpointCoeff / radius, falloff) : 1.0f;
                else
                    excluded = true;

                // Evaluate the transparency of the material
                if (workWithTransparency || (excluded && excludedObjectsTransparent))
                {
                    if (isData.wpointCoeff >= radius)
                        break;

                    nrc.setRayResult(res, &isData);
                    if (excluded && excludedObjectsTransparent)
                    {
                        // Treat excluded objects as completely transparent
                        currentTransp = 1.0f;
                    }
                    else if (nrc.rayresult.sb)
                    {
                        nrc.mtlresult.clear();
                        nrc.rayresult.sb->shade(nrc);
                        currentTransp = nrc.mtlresult.transp.intensity();
                    }
                }

                occlusion += k * (1.0f - currentTransp) * transp;

                transp *= currentTransp;
                if (transp < 1e-6f)
                    break;
            }
            else
            {
                break;
            }

            if (bAllTranspLevels)
            {
                nrc.rayparams.skipTag = nrc.rayresult.skipTag;
                nrc.rayparams.rayOrigin = nrc.rayresult.wpoint;
                nrc.rayparams.mint = nrc.rayresult.wpointCoeff;
            }
        }

        finalOcclusion += occlusion;

        VR::Color envFilter;
        if (!sampleEnvironment || occlusion > 1.0f - 1e-6f)
            envFilter.makeWhite();
        else
        {
            nrc.rayresult.shadeID = nrc.threadData->getShadeID(); // We need a new shade ID otherwise the env. texture will just return the same result
            nrc.rayresult.normal = -dir;                          // Needed if we use TexSampler for bent normals
            nrc.vray->computeEnvironment(nrc);
            envFilter = nrc.mtlresult.color;
        }

        envFilter *= (1.0f - occlusion);

        return occludedColor * occlusion + unoccludedColor * envFilter;

        /*
		if (!sampleEnvironment || occlusion>1.0f-1e-6f) {
			rawColor+=VR::Color(1.0f, 1.0f, 1.0f);
			return occludedColor*occlusion+unoccludedColor*(1.0f-occlusion);
		} else {
			nrc.vray->computeEnvironment(nrc);
			rawColor+=nrc.mtlresult.color;
			return occludedColor*occlusion+(unoccludedColor*nrc.mtlresult.color)*(1.0f-occlusion);
		}
		*/
    }

    void multResult(float m)
    {
        finalOcclusion *= m;
    }

    float getOcclusion(void)
    {
        return finalOcclusion;
    }
};

#pragma warning(pop)

using namespace VRayGolaemHSL;

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

class SkeletonTexmapClassDesc : public ClassDesc2
#if GET_MAX_RELEASE(VERSION_3DSMAX) >= 6000,
                                public IMtlRender_Compatibility_MtlBase
#endif
#if GET_MAX_RELEASE(VERSION_3DSMAX) >= 13900,
                                public IMaterialBrowserEntryInfo
#endif
{
#if GET_MAX_RELEASE(VERSION_3DSMAX) >= 6000
    HIMAGELIST imageList;
#endif

public:
    int IsPublic()
    {
        return IS_PUBLIC;
    }
    void* Create(BOOL /*loading*/)
    {
        return new SkeletonTexmap;
    }
    const TCHAR* ClassName()
    {
        return STR_CLASSNAME;
    }
    SClass_ID SuperClassID()
    {
        return TEXMAP_CLASS_ID;
    }
    Class_ID ClassID()
    {
        return VRAYGOLAEMHSL_CLASS_ID;
    }
    const TCHAR* Category()
    {
        return _T("");
    }

    // Hardwired name, used by MAX Script as unique identifier
    const TCHAR* InternalName()
    {
        return _T("SkeletonTexmap");
    }
    HINSTANCE HInstance()
    {
        return hInstance;
    }

#if GET_MAX_RELEASE(VERSION_3DSMAX) >= 6000
    SkeletonTexmapClassDesc()
    {
        imageList = NULL;
        IMtlRender_Compatibility_MtlBase::Init(*this);
    }

    ~SkeletonTexmapClassDesc()
    {
        if (imageList)
            ImageList_Destroy(imageList);
        imageList = NULL;
    }

    // From IMtlRender_Compatibility_MtlBase
    bool IsCompatibleWithRenderer(ClassDesc& rendererClassDesc)
    {
        if (rendererClassDesc.ClassID() != VRENDER_CLASS_ID)
            return false;
        return true;
    }

    bool GetCustomMtlBrowserIcon(HIMAGELIST& hImageList, int& inactiveIndex, int& activeIndex, int& disabledIndex)
    {
        if (!imageList)
        {
            HBITMAP bmp = LoadBitmap(hInstance, MAKEINTRESOURCE(bm_TEXICO));
            HBITMAP mask = LoadBitmap(hInstance, MAKEINTRESOURCE(bm_TEXICOMASK));

            imageList = ImageList_Create(11, 11, ILC_COLOR24 | ILC_MASK, 5, 0);
            int index = ImageList_Add(imageList, bmp, mask);
            if (index == -1)
                return false;
        }

        if (!imageList)
            return false;

        hImageList = imageList;
        inactiveIndex = 0;
        activeIndex = 1;
        disabledIndex = 2;
        return true;
    }
#endif

#if GET_MAX_RELEASE(VERSION_3DSMAX) >= 13900
    FPInterface* GetInterface(Interface_ID id)
    {
        if (IMATERIAL_BROWSER_ENTRY_INFO_INTERFACE == id)
        {
            return static_cast<IMaterialBrowserEntryInfo*>(this);
        }
        return ClassDesc2::GetInterface(id);
    }

    // From IMaterialBrowserEntryInfo
    const MCHAR* GetEntryName() const
    {
        return NULL;
    }
    const MCHAR* GetEntryCategory() const
    {
#if GET_MAX_RELEASE(VERSION_3DSMAX) >= 14900
        HINSTANCE hInst = GetModuleHandle(_T("sme.gup"));
        if (hInst)
        {
            static MSTR category(MaxSDK::GetResourceStringAsMSTR(hInst, IDS_3DSMAX_SME_MAPS_CATLABEL).Append(_T("\\V-Ray")));
            return category.data();
        }
#endif
        return _T("Maps\\V-Ray");
    }
    Bitmap* GetEntryThumbnail() const
    {
        return NULL;
    }
#endif
};

static SkeletonTexmapClassDesc SkelTexmapCD;

/*===========================================================================*\
|	DLL stuff
\*===========================================================================*/

HINSTANCE hInstance;
int controlsInit = false;

bool WINAPI DllMain(HINSTANCE hinstDLL, ULONG fdwReason, LPVOID /*lpvReserved*/)
{
    hInstance = hinstDLL;

    if (!controlsInit)
    {
        controlsInit = TRUE;
#if MAX_RELEASE < 13900
        InitCustomControls(hInstance);
#endif
        InitCommonControls();
    }

    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return (TRUE);
}

__declspec(dllexport) const TCHAR* LibDescription()
{
    return STR_LIBDESC;
}
__declspec(dllexport) int LibNumberClasses()
{
    return 1;
}

__declspec(dllexport) ClassDesc* LibClassDesc(int i)
{
    switch (i)
    {
    case 0:
        return &SkelTexmapCD;
    default:
        return 0;
    }
}

__declspec(dllexport) ULONG LibVersion()
{
    return VERSION_3DSMAX;
}

/*===========================================================================*\
|	Parameter block
\*===========================================================================*/

static int numID = 100;
int ctrlID(void)
{
    return numID++;
}

static ParamBlockDesc2 stex_param_blk(
    tex_params, _T("SkeletonTexmap parameters"), 0, &SkelTexmapCD, P_AUTO_CONSTRUCT + P_AUTO_UI, 0,
    //rollout
    0, 0, 0, 0, NULL,
    // params
    pb_map_input, _FT("texmap_input"), TYPE_TEXMAP, 0, 0,
    p_subtexno, 0,
    p_ui, TYPE_TEXMAPBUTTON, ctrlID(),
    PB_END,
    pb_map_h, _FT("texmap_h"), TYPE_TEXMAP, 0, 0,
    p_subtexno, 1,
    p_ui, TYPE_TEXMAPBUTTON, ctrlID(),
    PB_END,
    pb_map_s, _FT("texmap_s"), TYPE_TEXMAP, 0, 0,
    p_subtexno, 2,
    p_ui, TYPE_TEXMAPBUTTON, ctrlID(),
    PB_END,
    pb_map_l, _FT("texmap_l"), TYPE_TEXMAP, 0, 0,
    p_subtexno, 3,
    p_ui, TYPE_TEXMAPBUTTON, ctrlID(),
    PB_END,
    PB_END);

void SkeletonTexmap::greyDlgControls(IParamMap2* map)
{
    if (!map)
        return;
}

/*===========================================================================*\
|	Constructor and Reset systems
|  Ask the ClassDesc2 to make the AUTO_CONSTRUCT paramblocks and wire them in
\*===========================================================================*/

void SkeletonTexmap::Reset()
{
    _ivalid.SetEmpty();
    SkelTexmapCD.Reset(this);
}

SkeletonTexmap::SkeletonTexmap()
    : Texmap()
{
    static int pblockDesc_inited = false;
    if (!pblockDesc_inited)
    {
        initPBlockDesc(stex_param_blk);
        pblockDesc_inited = true;
    }

    pblock = NULL;
    _ivalid.SetEmpty();
    SkelTexmapCD.MakeAutoParamBlocks(this); // make and intialize paramblock2
    InitializeCriticalSection(&_csect);
}

SkeletonTexmap::~SkeletonTexmap()
{
    DeleteCriticalSection(&_csect);
}

ParamDlg* SkeletonTexmap::CreateParamDlg(HWND hwMtlEdit, IMtlParams* imp)
{
    return new SkelTexParamDlg(this, hwMtlEdit, imp);
    //IAutoMParamDlg* masterDlg = SkelTexmapCD.CreateParamDlgs(hwMtlEdit, imp, this);
    //return masterDlg;
}

/*===========================================================================*\
|	Subanim & References support
\*===========================================================================*/

RefTargetHandle SkeletonTexmap::GetReference(int /*i*/)
{
    return pblock;
}

void SkeletonTexmap::SetReference(int /*i*/, RefTargetHandle rtarg)
{
    pblock = (IParamBlock2*)rtarg;
}

TSTR SkeletonTexmap::SubAnimName(int /*i*/)
{
    return STR_DLGTITLE;
}

Animatable* SkeletonTexmap::SubAnim(int /*i*/)
{
    return pblock;
}

RefResult SkeletonTexmap::NotifyRefChanged(NOTIFY_REF_CHANGED_ARGS)
{
    (void)changeInt;
    (void)partID;
    (void)propagate;
    switch (message)
    {
    case REFMSG_CHANGE:
        _ivalid.SetEmpty();
        if (hTarget == pblock)
        {
            ParamID changing_param = pblock->LastNotifyParamID();
            IParamMap2* map = pblock->GetMap();
            if (map)
            {
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
    return (REF_SUCCEED);
}

/*===========================================================================*\
|	Updating and cloning
\*===========================================================================*/

RefTargetHandle SkeletonTexmap::Clone()
{
#if GET_MAX_RELEASE(VERSION_3DSMAX) < 8900
    NoRemap defaultRemap;
#else
    DefaultRemapDir defaultRemap;
#endif
    RemapDir& remap = defaultRemap;
    return Clone(remap);
}

RefTargetHandle SkeletonTexmap::Clone(RemapDir& remap)
{
    SkeletonTexmap* mnew = new SkeletonTexmap();
    *((MtlBase*)mnew) = *((MtlBase*)this); // copy superclass stuff
    BaseClone(this, mnew, remap);

    mnew->ReplaceReference(0, remap.CloneRef(pblock));

    mnew->_ivalid.SetEmpty();
    return (RefTargetHandle)mnew;
}

void SkeletonTexmap::NotifyChanged()
{
    NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}

void SkeletonTexmap::Update(TimeValue t, Interval& valid)
{
    if (!_ivalid.InInterval(t))
    {
        _ivalid.SetInfinite();
        pblock->GetValue(pb_map_input, t, _texmapInput, _ivalid);
        pblock->GetValue(pb_map_h, t, _texmapH, _ivalid);
        pblock->GetValue(pb_map_s, t, _texmapS, _ivalid);
        pblock->GetValue(pb_map_l, t, _texmapL, _ivalid);

        if (_texmapInput)
            _texmapInput->Update(t, _ivalid);
        if (_texmapH)
            _texmapH->Update(t, _ivalid);
        if (_texmapS)
            _texmapS->Update(t, _ivalid);
        if (_texmapL)
            _texmapL->Update(t, _ivalid);
    }
    _shadeCache.renderEnd(NULL);
    _cacheInit = false;
    valid &= _ivalid;
}

/*===========================================================================*\
|	Dlg Definition
\*===========================================================================*/

INT_PTR SkelTexDlgProc::DlgProc(TimeValue /*t*/, IParamMap2* map, HWND /*hWnd*/, UINT msg, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    //int id=LOWORD(wParam);
    switch (msg)
    {
    case WM_INITDIALOG:
    {
        SkeletonTexmap* texmap = (SkeletonTexmap*)(map->GetParamBlock()->GetOwner());
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

SkelTexParamDlg::SkelTexParamDlg(SkeletonTexmap* m, HWND hWnd, IMtlParams* i)
{
    texmap = m;
    ip = i;

    DLGTEMPLATE* tmp = templateGenerator.GenerateTemplateEx(texmap->pblock, STR_DLGTITLE, 217, NULL, 0);
    pmap = CreateMParamMap2(texmap->pblock, ip, hInstance, hWnd, NULL, NULL, tmp, STR_DLGTITLE, 0, &dlgProc);
    templateGenerator.ReleaseDlgTemplate(tmp);
}

void SkelTexParamDlg::DeleteThis(void)
{
    if (pmap)
        DestroyMParamMap2(pmap);
    pmap = NULL;
    delete this;
}

/*===========================================================================*\
|	Actual shading takes place
\*===========================================================================*/

//************************************************************
/*! @name Color Utilities
*/
//*********************************************************
//@{
//------------------------------------------------------------
//! Convert RGB to HSL
/*	\param rgbValue RGB color in the set [0, 1]
\note From http://mjijackson.com/2008/02/rgb-to-hsl-and-rgb-to-hsv-color-model-conversion-algorithms-in-javascript
*/
//---------------------------------------------------------
void rbgToHsl(const AColor& inColor, Point3& outHSL)
{
    float maxC(VR::Max(inColor.r, (VR::Max(inColor.g, inColor.b))));
    float minC(VR::Min(inColor.r, (VR::Min(inColor.g, inColor.b))));

    outHSL.x = outHSL.y = outHSL.z = (maxC + minC) / 2.f;

    if (maxC == minC)
    {
        outHSL.x = outHSL.y = 0.f; // achromatic
    }
    else
    {
        // saturation
        float diff = maxC - minC;
        outHSL.y = (outHSL.z > 0.5) ? (diff / (2.f - maxC - minC)) : (diff / (maxC + minC));

        // hue
        if (maxC == inColor.r)
            outHSL.x = (inColor.g - inColor.b) / diff + (inColor.g < inColor.b ? 6.f : 0.f);
        else if (maxC == inColor.g)
            outHSL.x = (inColor.b - inColor.r) / diff + 2.f;
        else if (maxC == inColor.b)
            outHSL.x = (inColor.r - inColor.g) / diff + 4.f;

        outHSL.x /= 6.f;
    }
}

//------------------------------------------------------------
//! Convert hue component to rgb (used by hslToRgb)
/*	\return a scalar in the set [0, 1]
\note From http://mjijackson.com/2008/02/rgb-to-hsl-and-rgb-to-hsv-color-model-conversion-algorithms-in-javascript
*/
//---------------------------------------------------------
float hueToRgb(float p, float q, float t)
{
    if (t < 0)
        t += 1.f;
    if (t > 1)
        t -= 1.f;
    if (t < (1.f / 6.f))
        return (p + (q - p) * 6.f * t);
    if (t < (1.f / 2.f))
        return q;
    if (t < (2.f / 3.f))
        return (p + (q - p) * (2.f / 3.f - t) * 6.f);
    return p;
}

//------------------------------------------------------------
//! Convert HSL to RBG
/*	\param hslValue Assumes h, s, and l are contained in the set [0, 1] and
\return corresponding RGB color in the set [0, 1]
\note From http://mjijackson.com/2008/02/rgb-to-hsl-and-rgb-to-hsv-color-model-conversion-algorithms-in-javascript
*/
//---------------------------------------------------------
void hslToRgb(const Point3& inHSL, AColor& outColor)
{
    if (inHSL.y == 0.)
    {
        outColor.r = outColor.g = outColor.b = inHSL.z; // achromatic
    }
    else
    {
        float q((inHSL.z < 0.5) ? (inHSL.z * (1 + inHSL.y)) : (inHSL.z + inHSL.y - inHSL.z * inHSL.y));
        float p(2.f * inHSL.z - q);
        outColor.r = hueToRgb(p, q, (inHSL.x + 1.f / 3.f));
        outColor.g = hueToRgb(p, q, inHSL.x);
        outColor.b = hueToRgb(p, q, (inHSL.x - 1.f / 3.f));
    }
}

//------------------------------------------------------------
//! Modify a color from some hsl parameters
/*	\param inColor source color
	\param h hue delta in the set [-1, 1]
	\param s sat delta in the set [-1, 1]
	\param l lit delta in the set [-1, 1]
	\param outColor corresponding RGB color in the set [0, 1]
*/
//---------------------------------------------------------
void changeColorHSL(const AColor& inColor, float h, float s, float l, AColor& outColor)
{
    // go to hsl
    Point3 inColorAsHsl;
    rbgToHsl(inColor, inColorAsHsl);

    // bound hsl values
    h = VR::clamp(h, -1.f, 1.f);
    s = VR::clamp(s, -1.f, 1.f);
    l = VR::clamp(l, -1.f, 1.f);

    // circular hue
    h /= 2;
    inColorAsHsl[0] = fmod(inColorAsHsl[0] + h, 1.f);
    if (inColorAsHsl[0] < 0)
        inColorAsHsl[0] = 1.f + inColorAsHsl[0];

    // saturation
    inColorAsHsl[1] = (s > 0) ? (inColorAsHsl[1] + (1 - inColorAsHsl[1]) * s) : (inColorAsHsl[1] + (inColorAsHsl[1]) * s);

    // lightness
    inColorAsHsl[2] = (l > 0) ? (inColorAsHsl[2] + (1 - inColorAsHsl[2]) * l) : (inColorAsHsl[2] + (inColorAsHsl[2]) * l);

    // back to rgb
    hslToRgb(inColorAsHsl, outColor);
}

static AColor white(1.0f, 1.0f, 1.0f, 1.0f);

void SkeletonTexmap::writeElements(VR::VRayContext& rc, const TexmapCache& res)
{
    //if (!affectElements || !rc.mtlresult.fragment || mode==0) return;

    rc.mtlresult.fragment->setChannelDataByAlias(REG_CHAN_VFB_REFLECT, &res.reflection);
    rc.mtlresult.fragment->setChannelDataByAlias(REG_CHAN_VFB_REFLECTION_FILTER, &res.reflectionFilter);
    rc.mtlresult.fragment->setChannelDataByAlias(REG_CHAN_VFB_RAW_REFLECTION, &res.reflectionRaw);
}

AColor SkeletonTexmap::EvalColor(ShadeContext& sc)
{
    // If the shade context is not generated by VRay - do nothing.
    if (!sc.globContext || sc.ClassID() != VRAYCONTEXT_CLASS_ID)
        return AColor(0.0f, 0.0f, 0.0f, 0.0f);

    // Convert the shade context to the VRay-for-3dsmax context.
    VR::VRayInterface& rc = (VR::VRayInterface&)sc;
    // Check if the cache has been initialized.
    if (!_cacheInit)
    {
        EnterCriticalSection(&_csect);
        if (!_cacheInit)
        {
            _shadeCache.renderBegin((VR::VRayRenderer*)rc.vray);
            _cacheInit = true;
        }
        LeaveCriticalSection(&_csect);
    }

    // already in cache?
    TexmapCache res;
    res.color = AColor(1.0f, 0.47f, 0.0f, 0.0f);
    if (_shadeCache.getCache(rc, res))
        return res.color;

    float h(0.f), s(0.f), l(0.f);
    if (_texmapInput)
        res.color = _texmapInput->EvalColor(sc);
    if (_texmapH)
        h = _texmapH->EvalMono(sc);
    if (_texmapS)
        s = _texmapS->EvalMono(sc);
    if (_texmapL)
        l = _texmapL->EvalMono(sc);
    changeColorHSL(res.color, h, s, l, res.color);

    _shadeCache.putCache(rc, res);
    return res.color;
}

float SkeletonTexmap::EvalMono(ShadeContext& sc)
{
    if (gbufID)
        sc.SetGBufferID(gbufID);
    Color c = EvalColor(sc);
    return Intens(c);
}

Point3 SkeletonTexmap::EvalNormalPerturb(ShadeContext& sc)
{
    if (gbufID)
        sc.SetGBufferID(gbufID);
    return Point3(0, 0, 0);
}

int SkeletonTexmap::NumSubTexmaps()
{
    return 12;
}

Texmap* SkeletonTexmap::GetSubTexmap(int i)
{
    if (i == 0)
        return pblock->GetTexmap(pb_map_input);
    else if (i == 1)
        return pblock->GetTexmap(pb_map_h);
    else if (i == 2)
        return pblock->GetTexmap(pb_map_s);
    else if (i == 3)
        return pblock->GetTexmap(pb_map_l);
    return NULL;
}
void SkeletonTexmap::SetSubTexmap(int i, Texmap* m)
{
    if (i == 0)
        pblock->SetValue(pb_map_input, 0, m);
    else if (i == 1)
        pblock->SetValue(pb_map_h, 0, m);
    else if (i == 2)
        pblock->SetValue(pb_map_s, 0, m);
    else if (i == 3)
        pblock->SetValue(pb_map_l, 0, m);
}
TSTR SkeletonTexmap::GetSubTexmapSlotName(int i)
{
    if (i == 0)
        return TSTR(STR_INPNAME);
    if (i == 1)
        return TSTR(STR_HUENAME);
    if (i == 2)
        return TSTR(STR_SATNAME);
    if (i == 3)
        return TSTR(STR_LITNAME);
    return TSTR(_T(""));
}
