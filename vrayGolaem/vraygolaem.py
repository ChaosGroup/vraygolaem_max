# /***************************************************************************
# *                                                                          *
# *  Copyright (C) Golaem S.A. - All Rights Reserved.                        *
# *                                                                          *
# ***************************************************************************/

import MaxPlus

# Function called when a VRayGolaem node is created
def glmVRayGolaemPostCreationCallback(proxyName):
    # get node
    node = MaxPlus.INode.GetINodeByName(proxyName)
    defaultMatName = node.BaseObject.ParameterBlock.default_material.GetValue()
    if not defaultMatName:
        defaultMatName = "crowdProxyDefaultShader"
        	
    # create VRayMtl (used as default)
    mat = MaxPlus.Factory.CreateMaterial(MaxPlus.Class_ID(0x37bf3f2f, 0x7034695c))
    mat.Diffuse = MaxPlus.Color(1, 0.47, 0)
    mat.SetName(MaxPlus.WStr(defaultMatName + "@"))

	# create multi material
    mmat = MaxPlus.Factory.CreateDefaultMultiMtl()
    mmat.SetName(MaxPlus.WStr(proxyName + "Mtl"))
    mmat.SetSubMtl(0, mat)
    
    # apply multi material to node
    node.Material = mmat
    print ("VRayGolaem: " + proxyName + "Mtl MultiMaterial has been successfully assigned to " + proxyName )
