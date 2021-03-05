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

namespace VRayGolaemSwitch
{
#define VRAYGOLAEMSWITCH_CLASS_ID Class_ID(0x3ae8c64c, 0xe585959)
#define STR_CLASSNAME _T("VRayGolaemSwitch")
#define STR_LIBDESC _T("VRayGolaemSwitch plugin")
#define STR_SELECTORNAME _T("Selector")
#define STR_DEFAULTNAME _T("Default")
#define STR_SHADER0NAME _T("Shader0")
#define STR_SHADER1NAME _T("Shader1")
#define STR_SHADER2NAME _T("Shader2")
#define STR_SHADER3NAME _T("Shader3")
#define STR_SHADER4NAME _T("Shader4")
#define STR_SHADER5NAME _T("Shader5")
#define STR_SHADER6NAME _T("Shader6")
#define STR_SHADER7NAME _T("Shader7")
#define STR_SHADER8NAME _T("Shader8")
#define STR_SHADER9NAME _T("Shader9")
#define STR_DLGTITLE _T("VRayGolaemSwitch Parameters")

    // Paramblock2 name
    enum
    {
        tex_params,
    };

    // Paramblock2 parameter list
    enum
    {
        pb_map_selector,
        pb_start_offset,
        pb_map_default,
        pb_map_shader0,
        pb_map_shader1,
        pb_map_shader2,
        pb_map_shader3,
        pb_map_shader4,
        pb_map_shader5,
        pb_map_shader6,
        pb_map_shader7,
        pb_map_shader8,
        pb_map_shader9
    };
} // namespace VRayGolaemSwitch

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
    Texmap* _texmapSelector;
    int _startOffset;
    Texmap* _texmapDefault;
    Texmap* _texmapShader0;
    Texmap* _texmapShader1;
    Texmap* _texmapShader2;
    Texmap* _texmapShader3;
    Texmap* _texmapShader4;
    Texmap* _texmapShader5;
    Texmap* _texmapShader6;
    Texmap* _texmapShader7;
    Texmap* _texmapShader8;
    Texmap* _texmapShader9;

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
        return VRAYGOLAEMSWITCH_CLASS_ID;
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
        return VRAYGOLAEMSWITCH_CLASS_ID;
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
