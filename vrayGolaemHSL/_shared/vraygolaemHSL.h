/***************************************************************************
*                                                                          *
*  Copyright (C) Chaos Group & Golaem S.A. - All Rights Reserved.          *
*                                                                          *
***************************************************************************/

#pragma once

#pragma warning(push)
#pragma warning(disable : 4840 4458)

#include "max.h"

#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable : 4201 4100 4996 4244 4512 4389 4189 4127 4456 4458 4505 4535)

#include "utils.h"
#include "vraydmcsampler.h"
#include "vrayplugins.h"
#include "vrenderdll.h"
#include "vraytexutils.h"
#include "vrender_unicode.h"
#include "iparamm2.h"

#include "shadedata_new.h"
#include "pb2template_generator.h"

#pragma warning(pop)

//************************************************************
// #defines
//************************************************************

namespace VRayGolaemHSL
{
#define VRAYGOLAEMHSL_CLASS_ID Class_ID(0x5ac3205c, 0x5f3c5801)
#define STR_CLASSNAME _T("VRayGolaemHSL")
#define STR_LIBDESC _T("VRayGolaemHSL plugin")
#define STR_INPNAME _T("Input")
#define STR_HUENAME _T("Hue")
#define STR_SATNAME _T("Saturation")
#define STR_LITNAME _T("Lightness")
#define STR_DLGTITLE _T("VRayGolaemHSL Parameters")

    // Paramblock2 name
    enum
    {
        tex_params,
    };

    // Paramblock2 parameter list
    enum
    {
        pb_map_input,
        pb_map_h,
        pb_map_s,
        pb_map_l
    };
} // namespace VRayGolaemHSL

//************************************************************
// SkeletonTexmap
//************************************************************

struct TexmapCache
{
    AColor color;
    VR::Color reflection, reflectionRaw, reflectionFilter;
};

class SkeletonTexmap : public Texmap
{
    void writeElements(VR::VRayContext& rc, const TexmapCache& res);

public:
    // Validity interval
    Interval _ivalid;

    // Various variables
    Texmap* _texmapInput;
    Texmap* _texmapH;
    Texmap* _texmapS;
    Texmap* _texmapL;

    int _cacheInit;
    CRITICAL_SECTION _csect;
    VR::ShadeCache<TexmapCache, true, true> _shadeCache;

    // Constructor/destructor
    SkeletonTexmap();
    ~SkeletonTexmap();

    // Parameter and UI management
    IParamBlock2* pblock;
    ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams* imp);
    void Update(TimeValue t, Interval& valid);

    void Reset();
    Interval Validity(TimeValue t)
    {
        Interval v;
        Update(t, v);
        return _ivalid;
    }

    void NotifyChanged();

    // Evaluate the color of map for the context
    AColor EvalColor(ShadeContext& sc);
    float EvalMono(ShadeContext& sc);
    AColor EvalFunction(ShadeContext& sc, float u, float v, float du, float dv);

    // For Bump mapping, need a perturbation to apply to a normal
    Point3 EvalNormalPerturb(ShadeContext& sc);

    // Methods to access texture maps of material
    int NumSubTexmaps();
    Texmap* GetSubTexmap(int i);
    void SetSubTexmap(int i, Texmap* m);
    TSTR GetSubTexmapSlotName(int i);

    Class_ID ClassID()
    {
        return VRAYGOLAEMHSL_CLASS_ID;
    }
    SClass_ID SuperClassID()
    {
        return TEXMAP_CLASS_ID;
    }
    void GetClassName(TSTR& s)
    {
        s = STR_CLASSNAME;
    }
    void DeleteThis()
    {
        delete this;
    }

    // Sub-anims
    int NumSubs()
    {
        return 1;
    }
    Animatable* SubAnim(int i);
    TSTR SubAnimName(int i);
    int SubNumToRefNum(int subNum)
    {
        return subNum;
    }

    // References
    int NumRefs()
    {
        return 1;
    }
    RefTargetHandle GetReference(int i);
    void SetReference(int i, RefTargetHandle rtarg);

    RefTargetHandle Clone(RemapDir& remap);
    RefTargetHandle Clone();

    RefResult NotifyRefChanged(NOTIFY_REF_CHANGED_ARGS);

    // Parameter blocks
    int NumParamBlocks()
    {
        return 1;
    }
    IParamBlock2* GetParamBlock(int /*i*/)
    {
        return pblock;
    }
    IParamBlock2* GetParamBlockByID(BlockID id)
    {
        return (pblock->ID() == id) ? pblock : NULL;
    }

    // From Animatable
    int RenderBegin(TimeValue /*t*/, ULONG /*flags*/)
    {
        _cacheInit = false;
        return true;
    }
    int RenderEnd(TimeValue /*t*/)
    {
        _cacheInit = false;
        return true;
    }

    // Other methods
    void greyDlgControls(IParamMap2* map);
};

//************************************************************
// SkelTexDlgProc
//************************************************************

class SkelTexDlgProc : public ParamMap2UserDlgProc
{
public:
    SkelTexDlgProc(void)
    {
    }
    INT_PTR DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void DeleteThis()
    {
    }
};

static SkelTexDlgProc dlgProc;

class SkelTexParamDlg : public ParamDlg
{
public:
    SkeletonTexmap* texmap;
    IMtlParams* ip;
    IParamMap2* pmap;

    SkelTexParamDlg(SkeletonTexmap* m, HWND hWnd, IMtlParams* i);

    Class_ID ClassID(void)
    {
        return VRAYGOLAEMHSL_CLASS_ID;
    }
    void SetThing(ReferenceTarget* m)
    {
        texmap = (SkeletonTexmap*)m;
        pmap->SetParamBlock(texmap->pblock);
    }
    ReferenceTarget* GetThing(void)
    {
        return texmap;
    }
    void SetTime(TimeValue /*t*/)
    {
    }
    void ReloadDialog(void)
    {
    }
    void ActivateDlg(BOOL /*onOff*/)
    {
    }
    void DeleteThis(void);
};
