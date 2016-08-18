/*	Golaem Crowd IO API - v0.00 - Copyright (C) Golaem SA.  All Rights Reserved.

	The implementation of this library is in the glmCrowdIO dynamic library redistributed with Golaem Crowd

	QUICK NOTES:
	Primarily of interest to pipeline developers to generate Golaem Crowd entities geometry

	Brief documentation under "DOCUMENTATION" below.

	Revision 0.00 (2015-11-09) release notes:

	- Functions and structures for creation, read/write and destruction of Golaem Crowd entities geometry
	*/

#ifndef GLM_CROWD_IO_INCLUDE_H
#define GLM_CROWD_IO_INCLUDE_H

#include "glm_crowd.h"
#include <math.h>
#include <float.h>
#include <vector>
#include <stdlib.h>
#include <string.h>

//#define GLM_DEVKIT_SKIP_FBX_TERRAIN // declare this before including this file, to disable fbx terrain feature, and get rid of fbx dependency

// DOCUMENTATION
//
// ===========================================================================
// Golaem Crowd entities geometry generation
//
// Depends on glm_crowd.h to read Golaem Crowd simulation caches
// Also needs the gcha, caa and fbx files
//
// Usage example:
//
//		glmInitCrowdIO();
//		GlmGeometryGenerationContext context;
//		memset(&context, 0, sizeof(GlmGeometryGenerationContext));
//		context._frame = 10;
//		context._crowdFieldCount = 2;
//		context._crowdFieldNames = (char(*)[GIO_NAME_LENGTH])GLMC_MALLOC(context._crowdFieldCount * GIO_NAME_LENGTH * sizeof(char));
//		strcpy(context._crowdFieldNames[0], "crowdField1");
//		strcpy(context._crowdFieldNames[1], "crowdField2");
//		strcpy(context._cacheName, "stadium");
//		context._cacheDirectoryCount = 1;
//		context._cacheDirectories = (char(*)[GIO_NAME_LENGTH])GLMC_MALLOC(context._cacheDirectoryCount * GIO_NAME_LENGTH * sizeof(char));
//		strcpy(context._cacheDirectories[0], "/home/username/maya/projects/default/export/stadium/cache");
//		context._characterFilesDirectoryCount = 1;
//		context._characterFilesDirectories = (char(*)[GIO_NAME_LENGTH])GLMC_MALLOC(context._characterFilesDirectoryCount * GIO_NAME_LENGTH * sizeof(char));
//		strcpy(context._characterFilesDirectories[0], "/home/username/Golaem/GolaemCrowdCharacterPack-4.2.0.1/crowd/characters/crowdMan_light.gcha");
//		context._excludedEntityCount = 0;
//		...
//		glmBeginGeometryGeneration(&context);
//		GlmEntityGeometry* geometries = (GlmEntityGeometry*)GLMC_MALLOC(context._entityCount * sizeof(GlmEntityGeometry));
//		for (unsigned int iEntity = 0; iEntity < context._entityCount; ++iEntity)
//		{
//			glmCreateEntityGeometry(&geometries[iEntity], &context, &context._entityBBoxes[iEntity]);
//		}
//		...
//		for (unsigned int iEntity = 0; iEntity < context._entityCount; ++iEntity)
//		{
//			GlmEntityBoundingBox* bbox = &context._entityBBoxes[iEntity];
//			printf("Entity ID: %u \n", bbox->_entityId);
//			GlmEntityGeometry* geometry = &geometries[iEntity];
//			printf("Number of meshes: %u \n", geometry->_meshCount);
//			for (unsigned int iMesh = 0; iMesh < geometry->_meshCount; ++iMesh)
//			{
//				GlmMesh* mesh = &geometry->_meshes[iMesh];
//				printf("Mesh name: %s \n", mesh->_name);
//				printf("Mesh vertice count: %u \n", mesh->_verticeCount);
//				for (unsigned int iFrame = 0; iFrame < context._frameToProcessCounts[bbox->_crowdFieldIndex]; ++iFrame)
//				{
//					printf("For frame: %u \n", context._framesToProcess[bbox->_crowdFieldIndex][iFrame]);
//					printf("First vertice: [%f, %f, %f] \n", mesh->_vertices[iFrame][0][0], mesh->_vertices[iFrame][0][1], mesh->_vertices[iFrame][0][2]);
//				}
//			}
//		}
//		...
//		for (unsigned int iEntity = 0; iEntity < context._entityCount; ++iEntity)
//		{
//			glmDestroyEntityGeometry(&geometries[iEntity]);
//		}
//		GLMC_FREE(geometries);
//		glmEndGeometryGeneration(&context);
//		GLMC_FREE(context._crowdFieldNames);
//		GLMC_FREE(context._cacheDirectories);
//		GLMC_FREE(context._characterFilesDirectories);
//		GLMC_FREE(context._excludedEntities);
//		glmFinishCrowdIO();
//

#ifdef _MSC_VER
#if defined GLM_CROWDIO_EXPORTS
#define GIO_API __declspec(dllexport)
#else
#define GIO_API __declspec(dllimport)
#endif
#else
#define GIO_API __attribute__ ((visibility ("default")))
#endif

//////////////////////////////////////////////////////////////////////////////
//
// Golaem Crowd entities geometry generation API
//

#define GIO_NAME_LENGTH 2048
#define GIO_TRACE_LENGTH 1024
#define GIO_MAX_POLYS 2000000
#define GIO_MAX_MESHES 10000
#define GIO_MAX_FRAMES 10000
#define GIO_MAX_INSTANCE_MESHES 10000
#define GIO_MAX_INSTANCE_MATRIX_PER_ENTITY 1000
#define GIO_NO_SHADER_GROUP_IDX UINT16_MAX

#define GCG_MAGIC_NUMBER 0x6C60
#define GTG_MAGIC_NUMBER 0x6760

namespace CrowdTerrain
{
	struct Mesh;
}

#ifdef __cplusplus
extern "C"
{
#endif

	// Structs--------------------------------------------------------------------

	// Entity ID and bounding box data, created by glmBeginGeometryGeneration
	struct GlmEntityBoundingBox_0
	{
		uint8_t _isVisible; // is the character visible? Used if _enableFrustumCulling is on in the GlmProcessCharactersParameters
		uint32_t _entityId;	// entity id of the character in the simulation
		uint32_t _entityIndex; // entity index of the character in the simulation cache
		uint32_t _entityAssetIndex; // entity index of the character in the caa
		uint8_t _crowdFieldIndex; // index of the crowdField this character is a part of
		uint8_t _characterIndex; // index of the Golaem Crowd Character Type
		float _boundingBoxHalfExtents[3]; // bounding box distances from the origin
		float _entityOrigin[3]; // origin of the entity for this frame
		float _entityScale; // used in Arnold to adapt halfExtents, as the meshes vertices are checked before bein transformed by isntance matrix (else fails silently and may crash !)
	};
	typedef GlmEntityBoundingBox_0 GlmEntityBoundingBox;

	// Shader name and category, attributes/context are stored in a ShaderGroup to mutualize attributes
	struct GlmShader_0
	{
		char _name[GIO_NAME_LENGTH]; // name of the shader
		char _category[GIO_NAME_LENGTH]; // category of the shader: surface, displace, etc..
	};
	typedef GlmShader_0 GlmShader;

	// Shader group contains an array of shaders and an array of attributes names, values are stored per mesh in GlmMesh 
	struct GlmShaderGroup_0
	{
		char _name[GIO_NAME_LENGTH];
		uint16_t _intShaderAttributeCount;
		char(*_intShaderAttributeNames)[GIO_NAME_LENGTH]; // array size = _intShaderAttributeCount
		uint16_t* _intShaderAttributeIndexes; // array size = _intShaderAttributeCount

		uint16_t _floatShaderAttributeCount;
		char(*_floatShaderAttributeNames)[GIO_NAME_LENGTH]; // array size = _floatShaderAttributeCount
		uint16_t* _floatShaderAttributeIndexes; // array size = _floatShaderAttributeCount

		uint16_t _fileShaderAttributeCount;
		char(*_fileShaderAttributeNames)[GIO_NAME_LENGTH]; // array size = _fileShaderAttributeCount
		uint16_t* _fileShaderAttributeIndexes; // array size = _fileShaderAttributeCount

		uint16_t _vectorShaderAttributeCount;
		char(*_vectorShaderAttributeNames)[GIO_NAME_LENGTH]; // array size = _vectorShaderAttributeCount
		uint16_t* _vectorShaderAttributeIndexes; // array size = _vectorShaderAttributeCount

		uint16_t _shaderCount;
		GlmShader* _shaders;
	};
	typedef GlmShaderGroup_0 GlmShaderGroup;

	// Input and ouput parameters passed as a context between functions declared below
	struct GlmPPAttributes_0
	{
		uint8_t _floatAttributeCount; // number of Float attributes
		char(*_floatAttributeNames)[GIO_NAME_LENGTH]; // name of the float per particle attribute, array size first dimension = _floatAttributeCount, second dimension = _entityCountPerCrowdField
		float** _floatAttributeData; // float data per particle attributes, array size first dimension = _floatAttributeCount
		uint8_t _vectorAttributeCount; // number of Vector attributes
		char(*_vectorAttributeNames)[GIO_NAME_LENGTH]; // name of the vector per particle attribute, array size first dimension = _vectorAttributeCount, second dimension = _entityCountPerCrowdField
		float(**_vectorAttributeData)[3]; // vector data per particle attributes, array size = _vectorAttributeCount
	};
	typedef GlmPPAttributes_0 GlmPPAttributes;

	struct GlmFileString
	{
		uint16_t	_allocSize;
		char*		_string;
	};

	// -------------------------------------------------------
	// -------------------------------------------------------
	struct GlmBlendTarget_0
	{
		GlmFileString		_targetName;
		uint32_t			_controlPointsCount;
		uint32_t*			_controlPointsIndices; // size = _controlPointsCount
		float(*_controlPointsPosition)[3]; // size = _controlPointsCount, target additive deformation at full deform		
	};
	typedef GlmBlendTarget_0 GlmBlendTarget;

	// -------------------------------------------------------
	struct GlmBlendShapeChannel_0
	{
		float			_deformPercent;			// runtime deform percent, in between should be handled on this basis
		uint16_t		_blendTargetCount;
		GlmBlendTarget*	_blendTargets;			//_blendTargetCount size
		float*			_fullDeformPercents;	// each target fulldeform match a certain percent, in between are blended according to deformPercent (logically located between two _fullDeformPercents)
	};
	typedef GlmBlendShapeChannel_0 GlmBlendShapeChannel;

	// -------------------------------------------------------
	struct GlmBlendShape_0
	{
		char					_blendShapeName[GIO_NAME_LENGTH]; // name of the blendshape for blindData update
		uint16_t				_blendShapeChannelCount;
		GlmBlendShapeChannel*	_blendShapeChannels;
	};
	typedef GlmBlendShape_0 GlmBlendShape;

	typedef enum
	{
		GLM_SKIN_RIGID,
		GLM_SKIN_LINEAR,
		GLM_SKIN_DUALQ,
		GLM_SKIN_BLEND
	} GlmSkinningType;

	struct GlmFileMeshVertex_0
	{
		float _position[3];
		float _normal[3];
		float _u;
		float _v;

		// skinning
		float		_skinInfluenceBlendWeight;	// 0.f for linear, 1.f for dual Q, between for blend
		float*		_skinInfluenceWeights;		// size = for (i in _vertexCount) Sum(_influenceCountPerVertex[i])
		uint16_t*	_skinInfluenceBoneId;		// size = for (i in _vertexCount) Sum(_influenceCountPerVertex[i])
		uint8_t		_skinVertexInfluenceCount;	// size = _vertexCount. read inline all influences to get first vertex n1 influence(s), then vertex second n2 influence(s), etc
	};
	typedef GlmFileMeshVertex_0 GlmFileMeshVertex;

	struct GlmFileMesh_0
	{
		GlmFileString _name; // name of the mesh

		uint8_t		_skinningType; // 0 : rigid, 1 : linear, 2 : dualQ, 3:blend
		uint16_t	_rigidSkinningBoneId;

		uint8_t		_hasUVs;

		float		_defaultLocalToWorldMatrix[16];

		// vertices
		uint32_t	_vertexCount; // number of GlmFileMeshVertex in this mesh, post optimization/merge
		GlmFileMeshVertex* _vertices; // _vertexCount * GlmFileMeshVertex

		// polygons
		uint32_t	_triangleCount; // number of polygons per mesh
		uint32_t(*_vertexIndicesPerTriangle)[3]; // id of vertices by polygon, array size = _polygonCount * 3

		// "geometry" transform animation
		uint32_t	_animTransformCount; // can be 0	
		float(*_animationLocalToWorldMatrix)[16]; // one per animation transform count
		double		_animStartTime;
		double		_animStopTime;

		// vertex cache
		uint32_t	_vertexCacheFrameCount;
		double		_vertexCacheStartTime;
		double		_vertexCacheStopTime;
		float(*_vertexCachePositions)[3]; // _original_ vertexCount * frameCount

		// blendshapes (not used for rendering)
		uint16_t		_blendShapeCount;
		GlmBlendShape*	_blendShapes;
	};
	typedef GlmFileMesh_0 GlmFileMesh;

	// Render data for an entity: meshes
	struct GlmGeometryFile_0
	{
		uint16_t _meshCount;
		GlmFileMesh* _meshes;

		uint16_t _boneCount;
		GlmFileString* _boneNames;				// name of the bones
		uint16_t* _boneParenting;
		float(*_boneOffsetPositions)[3];		// Bindpose positions (translate component of inverse rest matrix), used for skinning
		float(*_boneOffsetOrientations)[4];		// Bindpose orientations (orientation component of inverse rest matrix), used for skinning
		float(*_boneJointOrientations)[4];		// can't rebuild a maya skeleton without pre rotation information, which is joint orientation. boneOffsetOrientations is inverse of R*JO
	};
	typedef GlmGeometryFile_0 GlmGeometryFile;

	// Render data for a mesh: vertices, polygons, normals, UVs and shader attributes values
	struct GlmMesh_0
	{
		int32_t _instanceGroup;
		int32_t _instanceIndex;
		int16_t _instanceFirstMatrixIndex;

		char _name[GIO_NAME_LENGTH]; // name of the mesh
		// shaders
		uint16_t _shaderGroupIdx;
		// vertices
		uint32_t _verticeCount; // number of vertices per mesh
		float(**_vertices)[3]; // vertex positions, array size first dimension = _frameToProcessCounts, second dimension = _verticeCount
		// polygons
		uint32_t _polygonCount;	// number of polygons per mesh
		uint32_t* _verticePerPolygonCount; // number of vertices by polygon, array size = _polygonCount
		uint32_t* _vertexIndicesPerPolygon; // id of vertices by polygon, array size = _polygonCount * _verticePerPolygonCount
		// normals
		uint8_t _hasNormals; // 1 if the character has normals / control point. Normals are always extracted as mapped per polygon vertex
		float(**_normals)[3]; // normals per polygon vertex, array size first dimension = _frameToProcessCounts, second dimension = _polygonCount * _verticePerPolygonCount
		// tangents
		uint8_t _hasTangents; // 1 if the character has tangents / mesh
		uint8_t _tangentsByControlPoint;
		float(*_tangents)[6]; // tangents and binormals. If _tangentsByControlPoint == 1 then array size == _verticeCount, else array size = _polygonCount * _verticePerPolygonCount
		// UVs
		uint8_t _hasUVs; // 1 if the character has UV coordinates / mesh
		uint8_t _uvsByControlPoint; // 1 if the UVs are mapped by control point / mesh
		float(*_Us); // U coordinates. If _uvsByControlPoint == 1 then array size == _verticeCount, else array size = _polygonCount * _verticePerPolygonCount
		float(*_Vs); // U coordinates. If _uvsByControlPoint == 1 then array size == _verticeCount, else array size = _polygonCount * _verticePerPolygonCount
		// object id
		uint32_t _objectId;
	};
	typedef GlmMesh_0 GlmMesh;

	// Render data for an entity: meshes
	struct GlmEntityGeometry_0
	{
		uint16_t _meshCount;
		GlmMesh* _meshes;

		int32_t* _intShaderAttributeValues; // use GlmShaderGroup::_intShaderAttributeCount and GlmShaderGroup::_intShaderAttributeIndexes to access it
		float* _floatShaderAttributeValues; // use GlmShaderGroup::_floatShaderAttributeCount and GlmShaderGroup::_floatShaderAttributeIndexes to access it
		char(*_fileShaderAttributeValues)[GIO_NAME_LENGTH]; // use GlmShaderGroup::_fileShaderAttributeCount and GlmShaderGroup::_fileShaderAttributeIndexes to access it
		float(*_vectorShaderAttributeValues)[3]; // use GlmShaderGroup::_vectorShaderAttributeCount and GlmShaderGroup::_vectorShaderAttributeIndexes to access it
	};
	typedef GlmEntityGeometry_0 GlmEntityGeometry;
	
	typedef float GlmMeshInstanceMatrix[4][4];

	// instanced meshes
	struct GlmMeshInstanceGroup_0
	{
		int32_t		_hashKey; 
		GlmMesh*	_referenceMesh; // points on first geometry instanciated
		uint32_t	_instanceCount; // crowdField wide, used to set instanceIndex, used by renderer to know if it makes the reference mesh or an instance of it
		void*		_rendererSpecificData;
		uint8_t		_definitionComplete; // says that the mesh can be used in renderer. Could occur that a thread is preparing the reference mesh, while another is already further on same mesh and wants to use the reference.
	};
	typedef GlmMeshInstanceGroup_0 GlmMeshInstanceGroup;	
	
	// helper to find right mesh instance group according to hashkey then name. This will be sorted
	struct GlmMeshInstanceGroupSortEntry_0
	{
		uint32_t				_hashKey;
		const char*				_name;
		int32_t					_instanceGroup;
	};
	typedef GlmMeshInstanceGroupSortEntry_0 GlmMeshInstanceGroupSortEntry;

	// static allocation of instanceMatrices to avoid reallocating (and thus locking) matrices
	struct GlmEntityInstanceMatrices
	{
		GlmMeshInstanceMatrix _instanceMatrices[GIO_MAX_INSTANCE_MATRIX_PER_ENTITY]; // by default 64 ko = 16 float * 1k
		uint16_t _instanceMatrixUsedCount;
	};

	// instances per crowdField, to be able to diemnsion it according to frame count
	struct GlmMeshInstanceGroupPerCrowdField_0
	{
		uint32_t _meshInstanceGroupCount; // number of shader groups used in the current frame
		GlmMeshInstanceGroup _meshInstanceGroups[GIO_MAX_INSTANCE_MESHES]; // all meshInstanceGroups, one instance group per character file / per rigid mesh
		void* _meshInstanceGroupSet; // for internal use
	};
	typedef GlmMeshInstanceGroupPerCrowdField_0 GlmMeshInstanceGroupPerCrowdField;

	// Input and ouput parameters passed as a context between functions declared below
	// Input paramters must be allocated/deallocated and set by the user
	// Output parameters are allocated/deallocated by glmBeginGeometryGeneration and glmFinishProcessEntityRenderData
	struct GlmGeometryGenerationContext_0
	{
		// input context
		float _frame; // the frame to process
		// crowd fields
		uint8_t _crowdFieldCount; // number of crowd fields
		char(*_crowdFieldNames)[GIO_NAME_LENGTH]; // crowdFields names, array size = _crowdFieldCount
		// simulation directories
		char _cacheName[GIO_NAME_LENGTH]; // maya scene name
		uint8_t _cacheDirectoryCount; // number of directories to look for the golaem simulation cache files, .gscs and .gscf
		char(*_cacheDirectories)[GIO_NAME_LENGTH]; // directories to look for the golaem simulation cache files, .gscs and .gscf, array size = _cacheDirectoryCount
		uint8_t _characterFilesDirectoryCount; // number of directories to look for the golaem character files, .gcha
		char(*_characterFilesDirectories)[GIO_NAME_LENGTH]; // directories to look for the golaem character files, .gcha
		uint8_t _enableLayout; // 1 if layout is enabled
		char _layoutDirectory[GIO_NAME_LENGTH]; // layout directory
		char _layoutName[GIO_NAME_LENGTH]; // layout name
		char _terrainFile[GIO_NAME_LENGTH]; // destination terrain file
		// excluded entities
		uint32_t _excludedEntityCount; // number of entities excluded from rendering
		uint32_t* _excludedEntities; // entities excluded from rendering, array size = _excludedEntityCount
		// flip texture UVs
		uint8_t _flipUs; // 1 if texture U axis needs to be flipped
		uint8_t _flipVs; // 1 if texture V axis needs to be flipped
		uint8_t _triangulate; // 1 if the geometry need to be triangulated by glmCreateEntityGeometry
		uint8_t _generateTangents;
		// motion blur
		uint8_t _motionBlurEnabled; // 1 if motion blur is enabled
		float _motionBlurStartFrame; // motion blur start frame
		float _motionBlurWindowSize; // motion blur window size (in frames)
		uint32_t _motionBlurSamples; // number of motion blur samples
		// frustum culling
		uint8_t _enableFrustumCulling; // 1 if frustum culling is used
		float _frustumMargin; // grow the view frustum
		float _cameraMargin; // radius around camera eye where there is be no frustum culling
		float _cameraWorldPosition[3]; // world-position of the camera, Y-up, used to compute frustum culling
		float _viewMatrix[4][4]; // view matrix, column-major, used to compute frustum culling
		float _projectionMatrix[4][4]; // projection matrix, column-major, used to compute frustum culling
		// object id
		uint32_t _objectIdBase; // Start id for the ObjectId pass
		uint8_t _objectIdMode; // ObjectId pass mode (0: per mesh, 1: per shader, 2: per entity, 3: per crowdField, 4: all)
		// render percent
		float _renderPercent; // Render Percent
		// dirmap rules
		char _dirmap[GIO_NAME_LENGTH]; // dirmaps , array size = _dirMapCount
		// proxy matrix
		float _proxyMatrix[4][4];
		float _proxyMatrixInverse[4][4];

		uint8_t _instancingEnabled;

		// output context
		uint32_t _entityCount; // number of entities for the current frame
		uint32_t* _entityCountPerCrowdField; // number of entities per crowd field for the current frame, array size = _crowdFieldCount
		GlmEntityBoundingBox* _entityBBoxes; // entities bounding boxes and id information, array size = _entityCount
		GlmEntityGeometry* _entityGeometry; // entity geometry, array size = _entityCount
		uint8_t* _frameToProcessCounts; // number of frames computed for motion blur, array size = _crowdFieldCount
		double** _framesToProcess; // frames computed for motion blur, array size first dimension = _crowdFieldCount, second dimension = _frameToProcessCounts
		GlmPPAttributes* _ppAttributes; // user per-particle attributes, array size = _crowdFieldCount
		uint16_t _shaderGroupCount; // number of shader groups used in the current frame
		GlmShaderGroup* _shaderGroups; // shader groups used in the current frame, array size = _shaderGroupCount

		GlmMeshInstanceGroupPerCrowdField *_meshInstances;
	};
	typedef GlmGeometryGenerationContext_0 GlmGeometryGenerationContext;

	// Enums-------------------------------------------------------------------
	typedef enum
	{
		GIO_SUCCESS,
		GIO_GSC_FILE_OPEN_FAILED, // simulation cache files open failed
		GIO_CAA_FILE_OPEN_FAILED, // crowd association (.caa) file open failed
		GIO_CHARACTER_NO_RENDERING_TYPE, // a character has not been assigned a Rendering Type from the Rendering Attributes panel
		GIO_CHARACTER_INVALID_RENDERING_TYPE, // a character has an invalid Rendering Type. Using the first Rendering Type found
		GIO_CHARACTER_NO_MESH_ASSET, // a character has no mesh asset
		GIO_FRAME_NO_DATA, // insufficient data in simulation cache for the frame context->_frame
		GIO_FBX_FILE_OPEN_FAILED, // failed to load the Fbx file of a Character
		GIO_FBX_FILE_MESH_NOT_FOUND, // failed to find a mesh in the Fbx file of a Character
		GIO_GCG_FILE_OPEN_FAILED, // failed to load the gcg file of a Character
		GIO_GCG_FILE_BAD_FORMAT, // failed to load the gcg file of a Character
		GIO_GCG_BAD_CACHE, // the input cache has wrong number of points
		GIO_GCG_FILE_NO_MESH_FOUND,
		GIO_GCG_FILE_MESH_NOT_FOUND, // failed to find a mesh in the gcg file of a Character
		GIO_GCG_ROOT_NOT_FOUND, // could not find root by name in input fbx while converting
		GIO_GCG_BONE_NOT_FOUND, // could not find root by name in input fbx while converting
		GIO_MESH_NO_SHADER_GROUP, // failed to find a shader group for a mesh
		GIO_MESH_NO_NORMAL, // the input mesh has no normals when generating a geometry file, must generate them
		GIO_INVALID_CONTEXT, // context passed as a parameter is invalid, either it has not yet been created or destroyed
		GIO_INVALID_BONE_COUNT // bone count mismatch between cache and GCHA
	} GlmGeometryGenerationStatus;

	typedef enum
	{
		GLM_CREATE_ALL,
		GLM_CREATE_NAMES_AND_SHADERS,
		GLM_CREATE_GEOMETRY
	} GlmCreateGeometryMode;

	typedef void(*glmLogCallback)(const char* msg, void* userData);

	// Functions------------------------------------------------------------------

	// initialize the CrowdIO library, to be called first, once
	extern GIO_API void glmInitCrowdIO();

	// initialize the entities geometry generation for a frame, to be called before glmCreateEntityGeometry
	// context input parameters must be allocated and set before calling
	// context output parameters will be allocated and set by this function
	extern GIO_API GlmGeometryGenerationStatus glmBeginGeometryGeneration(GlmGeometryGenerationContext* context);

	// create an entity geometry, allocates geometry
	// context output parameter _shaderGroups will be modified by this function
	// mode parameter is used to either generate mesh and shader names, geometry or both of them
	extern GIO_API GlmGeometryGenerationStatus glmCreateEntityGeometry(GlmEntityGeometry* geometry, GlmGeometryGenerationContext* context, const GlmEntityBoundingBox* bbox, GlmEntityInstanceMatrices& instanceMatrices, GlmCreateGeometryMode mode = GLM_CREATE_ALL);

	// destroy an entity geometry, deallocates geometry
	extern GIO_API void glmDestroyEntityGeometry(GlmEntityGeometry* geometry, const GlmGeometryGenerationContext* context, const GlmEntityBoundingBox* bbox);

	// deallocate the context output parameters, context input parameters must be deallocated by the user
	extern GIO_API GlmGeometryGenerationStatus glmEndGeometryGeneration(GlmGeometryGenerationContext* context);

	// deinitialize the CrowdIO library, to be called last, once
	extern GIO_API void glmFinishCrowdIO();

	// convert a status in an error message
	extern GIO_API const char* glmConvertGeometryGenerationStatus(GlmGeometryGenerationStatus status);

	// register a log callback to get more information
	extern GIO_API void glmRegisterLogCallback(glmLogCallback callback, void* userData);

	// write/read a GolaemCharacterGeoemtry file
	extern GIO_API void glmClearGeometryFile(GlmGeometryFile& geometry);
	extern GIO_API GlmGeometryGenerationStatus glmWriteGeometryFile(const char* filename, GlmGeometryFile& geometry);
	extern GIO_API GlmGeometryGenerationStatus glmReadGeometryFile(const char* filename, GlmGeometryFile& geometry);
	extern GIO_API void glmFileSetString(GlmFileString& string, const char* value);

	extern GIO_API GlmGeometryGenerationStatus glmWriteTerrainFile(const char* filename, const uint16_t meshCount, const GlmFileMesh* fileMeshes);
	extern GIO_API GlmGeometryGenerationStatus glmReadTerrainFile(const char* filename, uint16_t& meshCount, GlmFileMesh*& fileMeshes);

	extern GIO_API GlmSimulationCacheStatus glmImportFrameCacheWithHistory(GlmGeometryGenerationContext* context, const char* filePath, const char* filePathModel, const char* cachePath, const char* layoutPath, GlmSimulationData*& simulationData, GlmFrameData*& frameCache, int currentFrame, CrowdTerrain::Mesh* terrainMeshSource, CrowdTerrain::Mesh* terrainMeshDestination, bool enableLayout);

#ifdef __cplusplus
}
#endif

namespace CrowdTerrain
{
#ifdef __cplusplus
	extern "C"
	{
#endif
		extern GIO_API Mesh* loadTerrainAsset(const char *terrainCompletePath);
		extern GIO_API void closeTerrainAsset(const Mesh* terrainMesh);
#ifdef __cplusplus
	}
#endif
	
	Mesh* loadTerrainFBX(const char *terrainCompletePath); // declared empty if GLM_DEVKIT_SKIP_FBX_TERRAIN is declared
	Mesh* loadTerrainGTG(const char *terrainCompletePath);

	struct Vec3
	{
		Vec3()
		{
		}
		Vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z)
		{
		}
		void normalizeSelf()
		{
			float d = 1.f / (sqrtf(x * x + y * y + z * z + 0.0001f));
			x *= d;
			y *= d;
			z *= d;
		}
		void setValues(float _x, float _y, float _z)
		{
			x = _x; y = _y; z = _z;
		}
		void setValues(float* values)
		{
			x = values[0]; y = values[1]; z = values[2];
		}
		const float& operator[] (unsigned int index) const
		{
			return ((float*)&x)[index];
		}
		float& operator[] (unsigned int index)
		{
			return ((float*)&x)[index];
		}

		float x, y, z;
	};

	struct Matrix4
	{
		Matrix4()
		{
		}

		Matrix4(const Matrix4& source)
		{
			*this = source;
		}

		Matrix4(const double *values)
		{
			for (int i = 0; i < 16; i++)
				_matrix[i] = (float)values[i];
		}

		Matrix4& operator = (const Matrix4 & source)
		{
			memcpy(_matrix, source._matrix, sizeof(float) * 16);
			return *this;
		}

		Vec3 transformVector(const Vec3& src) const
		{
			Vec3 out;

			out.x = src.x * _matrix44[0][0] + src.y * _matrix44[1][0] + src.z * _matrix44[2][0];
			out.y = src.x * _matrix44[0][1] + src.y * _matrix44[1][1] + src.z * _matrix44[2][1];
			out.z = src.x * _matrix44[0][2] + src.y * _matrix44[1][2] + src.z * _matrix44[2][2];

			return out;
		}

		Vec3 transformPoint(const Vec3& src) const
		{
			Vec3 out;

			out.x = src.x * _matrix44[0][0] + src.y * _matrix44[1][0] + src.z * _matrix44[2][0] + _matrix44[3][0];
			out.y = src.x * _matrix44[0][1] + src.y * _matrix44[1][1] + src.z * _matrix44[2][1] + _matrix44[3][1];
			out.z = src.x * _matrix44[0][2] + src.y * _matrix44[1][2] + src.z * _matrix44[2][2] + _matrix44[3][2];

			return out;
		}

		void setToIdentity()
		{
			static const float matrixValues[16] = {
				1.f, 0.f, 0.f, 0.f,
				0.f, 1.f, 0.f, 0.f,
				0.f, 0.f, 1.f, 0.f,
				0.f, 0.f, 0.f, 1.f };
			memcpy(_matrix, matrixValues, sizeof(matrixValues));
		}

		// inverse matrix (generic case)
		void inverse(const Matrix4& srcMatrix)
		{
			float det = 0.f;

			// transpose matrix
			float src[16];
			for (int i = 0; i<4; ++i)
			{
				src[i] = srcMatrix._matrix44[i][0];
				src[i + 4] = srcMatrix._matrix44[i][1];
				src[i + 8] = srcMatrix._matrix44[i][2];
				src[i + 12] = srcMatrix._matrix44[i][3];
			}

			// calculate pairs for first 8 elements (cofactors)
			float tmp[12]; // temp array for pairs
			tmp[0] = src[10] * src[15];
			tmp[1] = src[11] * src[14];
			tmp[2] = src[9] * src[15];
			tmp[3] = src[11] * src[13];
			tmp[4] = src[9] * src[14];
			tmp[5] = src[10] * src[13];
			tmp[6] = src[8] * src[15];
			tmp[7] = src[11] * src[12];
			tmp[8] = src[8] * src[14];
			tmp[9] = src[10] * src[12];
			tmp[10] = src[8] * src[13];
			tmp[11] = src[9] * src[12];

			float *m16 = &_matrix[0];
			// calculate first 8 elements (cofactors)
			m16[0] = (tmp[0] * src[5] + tmp[3] * src[6] + tmp[4] * src[7]) - (tmp[1] * src[5] + tmp[2] * src[6] + tmp[5] * src[7]);
			m16[1] = (tmp[1] * src[4] + tmp[6] * src[6] + tmp[9] * src[7]) - (tmp[0] * src[4] + tmp[7] * src[6] + tmp[8] * src[7]);
			m16[2] = (tmp[2] * src[4] + tmp[7] * src[5] + tmp[10] * src[7]) - (tmp[3] * src[4] + tmp[6] * src[5] + tmp[11] * src[7]);
			m16[3] = (tmp[5] * src[4] + tmp[8] * src[5] + tmp[11] * src[6]) - (tmp[4] * src[4] + tmp[9] * src[5] + tmp[10] * src[6]);
			m16[4] = (tmp[1] * src[1] + tmp[2] * src[2] + tmp[5] * src[3]) - (tmp[0] * src[1] + tmp[3] * src[2] + tmp[4] * src[3]);
			m16[5] = (tmp[0] * src[0] + tmp[7] * src[2] + tmp[8] * src[3]) - (tmp[1] * src[0] + tmp[6] * src[2] + tmp[9] * src[3]);
			m16[6] = (tmp[3] * src[0] + tmp[6] * src[1] + tmp[11] * src[3]) - (tmp[2] * src[0] + tmp[7] * src[1] + tmp[10] * src[3]);
			m16[7] = (tmp[4] * src[0] + tmp[9] * src[1] + tmp[10] * src[2]) - (tmp[5] * src[0] + tmp[8] * src[1] + tmp[11] * src[2]);

			// calculate pairs for second 8 elements (cofactors)
			tmp[0] = src[2] * src[7];
			tmp[1] = src[3] * src[6];
			tmp[2] = src[1] * src[7];
			tmp[3] = src[3] * src[5];
			tmp[4] = src[1] * src[6];
			tmp[5] = src[2] * src[5];
			tmp[6] = src[0] * src[7];
			tmp[7] = src[3] * src[4];
			tmp[8] = src[0] * src[6];
			tmp[9] = src[2] * src[4];
			tmp[10] = src[0] * src[5];
			tmp[11] = src[1] * src[4];

			// calculate second 8 elements (cofactors)
			m16[8] = (tmp[0] * src[13] + tmp[3] * src[14] + tmp[4] * src[15]) - (tmp[1] * src[13] + tmp[2] * src[14] + tmp[5] * src[15]);
			m16[9] = (tmp[1] * src[12] + tmp[6] * src[14] + tmp[9] * src[15]) - (tmp[0] * src[12] + tmp[7] * src[14] + tmp[8] * src[15]);
			m16[10] = (tmp[2] * src[12] + tmp[7] * src[13] + tmp[10] * src[15]) - (tmp[3] * src[12] + tmp[6] * src[13] + tmp[11] * src[15]);
			m16[11] = (tmp[5] * src[12] + tmp[8] * src[13] + tmp[11] * src[14]) - (tmp[4] * src[12] + tmp[9] * src[13] + tmp[10] * src[14]);
			m16[12] = (tmp[2] * src[10] + tmp[5] * src[11] + tmp[1] * src[9]) - (tmp[4] * src[11] + tmp[0] * src[9] + tmp[3] * src[10]);
			m16[13] = (tmp[8] * src[11] + tmp[0] * src[8] + tmp[7] * src[10]) - (tmp[6] * src[10] + tmp[9] * src[11] + tmp[1] * src[8]);
			m16[14] = (tmp[6] * src[9] + tmp[11] * src[11] + tmp[3] * src[8]) - (tmp[10] * src[11] + tmp[2] * src[8] + tmp[7] * src[9]);
			m16[15] = (tmp[10] * src[10] + tmp[4] * src[8] + tmp[9] * src[9]) - (tmp[8] * src[9] + tmp[11] * src[10] + tmp[5] * src[8]);

			// calculate determinant
			det = src[0] * m16[0] + src[1] * m16[1] + src[2] * m16[2] + src[3] * m16[3];

			// calculate matrix inverse
			float invdet = 1 / det;

			for (int j = 0; j < 16; ++j)
			{
				m16[j] *= invdet;
			}
		}

		union
		{
			float _matrix44[4][4];
			float _matrix[16];
		};
	};

	template<typename pointT> struct AABB
	{
		AABB() : _left(-1), _right(-1), _firstIndex(0), _indexCount(0)
		{
		}
		pointT _AABBmin, _AABBmax;
		int _left, _right;
		int _firstIndex, _indexCount;  // _indexCount == 0 if none
	};

	inline Vec3 operator- (const Vec3& vector1, const Vec3& vector2)
	{
		return Vec3(vector1.x - vector2.x, vector1.y - vector2.y, vector1.z - vector2.z);
	}
	inline Vec3 operator+ (const Vec3& vector1, const Vec3& vector2)
	{
		return Vec3(vector1.x + vector2.x, vector1.y + vector2.y, vector1.z + vector2.z);
	}
	inline Vec3 operator^ (const Vec3& vector1, const Vec3& vector2)
	{
		return Vec3(vector1.y*vector2.z - vector1.z*vector2.y, vector1.z*vector2.x - vector1.x*vector2.z, vector1.x*vector2.y - vector1.y*vector2.x);
	}
	inline Vec3 operator* (const Vec3& vector, const float scale)
	{
		return Vec3(vector.x*scale, vector.y*scale, vector.z*scale);
	}
	inline float operator* (const Vec3& vector1, const Vec3& vector2)
	{
		return vector1.x*vector2.x + vector1.y*vector2.y + vector1.z - vector2.z;
	}

	struct GIO_API SubMesh
	{
		SubMesh() :
			_vertices(0)
			, _normals(0)
			, _indices(0)
			, _indiceCount(0)
			, _vertexCount(0)
			, _userNode(NULL)
		{
		}

		~SubMesh()
		{
			delete[] _vertices;
			delete[] _normals;
			delete[] _indices;
		}

		void init(Vec3 * lVertices, Vec3 * lNormals, unsigned int lVertexCount, unsigned int *lIndices, unsigned int lIndiceCount);
		void merge(Vec3 * lVertices, Vec3 * lNormals, unsigned int lVertexCount, unsigned int *lIndices, unsigned int lIndiceCount);
		void setFrame(int frame);
		bool raycastClosest(const Vec3& rayOrigin, const Vec3& rayEnd, Vec3 *collisionPoint, Vec3 *collisionNormal, const Matrix4* proxyMatrix, const Matrix4* proxyMatrixInverse) const;

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		Vec3 *_vertices;
		Vec3 *_normals;

		unsigned int * _indices;
		unsigned int _indiceCount;
		unsigned int _vertexCount;

		std::vector<AABB<Vec3> > _AABB;

		Matrix4 _worldToLocal;
		Matrix4 _localToWorld;

		std::vector<Matrix4> _animation; // one matrix per frame. world to local
		std::vector<Matrix4> _animationInverse;

		void* _userNode;
	};

	struct GIO_API Mesh
	{
		Mesh() : _startFrame(0)
		{
			_proxyMatrix.setToIdentity();
			_proxyMatrixInverse.setToIdentity();
		}

		~Mesh()
		{
			for (unsigned int i = 0; i < _subMeshes.size(); i++)
			{
				delete _subMeshes[i];
			}
		}

		bool raycastClosest(const Vec3& rayOrigin, const Vec3& rayEnd, Vec3 *collisionPoint, Vec3 *collisionNormal, bool useProxyMatrix) const;
		void setFrame(int frame)
		{
			frame -= _startFrame;
			for (unsigned int i = 0; i < _subMeshes.size(); i++)
				_subMeshes[i]->setFrame(frame);
		}
		std::vector<SubMesh*> _subMeshes;
		int _startFrame;
		void setProxyMatrices(const Matrix4* proxyMatrix, const Matrix4* proxyMatrixInverse)
		{
			if (proxyMatrix)
				_proxyMatrix = *proxyMatrix;
			else
				_proxyMatrix.setToIdentity();
			if (proxyMatrixInverse)
				_proxyMatrixInverse = *proxyMatrixInverse;
			else
				_proxyMatrixInverse.setToIdentity();
		}

	protected:
		Matrix4 _proxyMatrix;
		Matrix4 _proxyMatrixInverse;
	};

	template<typename pointT> pointT computeBarycentric(const pointT& normal, const pointT& a, const pointT& b, const pointT& c, const pointT& P)
	{
		pointT bary;

		// The area of a triangle is 
		float areaABC = normal * ((b - a) ^ (c - a));
		float areaPBC = normal * ((b - P) ^ (c - P));
		float areaPCA = normal * ((c - P) ^ (a - P));

		bary.x = areaPBC / areaABC; // alpha
		bary.y = areaPCA / areaABC; // beta
		bary.z = 1.0f - bary.x - bary.y; // gamma

		return bary;
	}

	inline Vec3 barycentricLerp(Vec3 a, Vec3 b, Vec3 c, float r, float s, float t)
	{
		return a * r + b * s + c * t;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// raycasts volumes hierarchy
	template<typename vertexT, typename pointT> void computeAABBBounds(vertexT *vts, int *indices, int indexCount, pointT& AABBmin, pointT& AABBmax)
	{
		AABBmin.setValues(FLT_MAX, FLT_MAX, FLT_MAX);
		AABBmax.setValues(-FLT_MAX, -FLT_MAX, -FLT_MAX);

		for (int i = 0; i < indexCount; i++)
		{
			float* v = (float*)&vts[(*indices++)];
			for (int j = 0; j < 3; j++)
			{
				AABBmin[j] = (AABBmin[j] < v[j]) ? AABBmin[j] : v[j];
				AABBmax[j] = (AABBmax[j] > v[j]) ? AABBmax[j] : v[j];
			}
		}
	}

	template<typename vertexT, typename pointT> int computeBestAxis(vertexT *vts, int *indices, int indexCount, const pointT& AABBmin, const pointT& AABBmax)
	{
		if (indexCount < 16 * 3)
			return -1;

		int score[3];
		for (int i = 0; i < 3; i++)
		{
			float axisMid = (AABBmin[i] + AABBmax[i]) * 0.5f;
			int left(0), right(0);

			for (int k = 0; k < indexCount; k += 3)
			{
				float faceMiddle = ((*(pointT*)&vts[indices[k + 0]])[i] + (*(pointT*)&vts[indices[k + 1]])[i] + (*(pointT*)&vts[indices[k + 2]])[i]) / 3.f;
				if (faceMiddle < axisMid)
					left++;
				else
					right++;
			}
			score[i] = abs(left - right);
		}
		int best(0);
		for (int i = 1; i < 3; i++)
			if (score[i] < score[best])
				best = i;

		return best;
	}

	// reorder indices in the array. low part is before split axis(on the left). high part, on the right.
	template<typename vertexT, typename pointT> int splitIndices(vertexT *vts, int *indices, int indexCount, int axis, const pointT& AABBmin, const pointT& AABBmax)
	{
		float axisMid = (AABBmin[axis] + AABBmax[axis]) * 0.5f;
		int *ind = indices;
		int tailIndex = indexCount - 3;
		for (int k = 0; k < indexCount;)
		{
			float faceMiddle = ((*(pointT*)&vts[ind[k]])[axis] + (*(pointT*)&vts[ind[k + 1]])[axis] + (*(pointT*)&vts[ind[k + 2]])[axis]) / 3.f;
			if (faceMiddle < axisMid)
			{
				k += 3;
			}
			else
			{
				std::swap(ind[k], ind[tailIndex]);
				std::swap(ind[k + 1], ind[tailIndex + 1]);
				std::swap(ind[k + 2], ind[tailIndex + 2]);
				tailIndex -= 3;
			}
			if (tailIndex <= k)
				break;
		}
		return tailIndex;
	}

	template<typename vertexT, typename pointT, typename AABContainer> int computeAABBHierarchy(vertexT *vts, int *indices, int firstIndex, int indexCount, int level, AABContainer& container)
	{
		if (indexCount <= 0)
			return -1;

		AABB<pointT> aabb;

		computeAABBBounds<vertexT, pointT>(vts, indices + firstIndex, indexCount, aabb._AABBmin, aabb._AABBmax);
		int bestAxis = computeBestAxis<vertexT, pointT>(vts, indices + firstIndex, indexCount, aabb._AABBmin, aabb._AABBmax);
		if (level < 20 && bestAxis >= 0)
		{
			int indicesInLeft(splitIndices(vts, indices + firstIndex, indexCount, bestAxis, aabb._AABBmin, aabb._AABBmax));

			aabb._left = computeAABBHierarchy<vertexT, pointT, AABContainer>(vts, indices, firstIndex, indicesInLeft, level + 1, container);
			aabb._right = computeAABBHierarchy<vertexT, pointT, AABContainer>(vts, indices, firstIndex + indicesInLeft, indexCount - indicesInLeft, level + 1, container);
		}
		else
		{
			aabb._firstIndex = firstIndex;
			aabb._indexCount = indexCount;
		}
		container.push_back(aabb);
		return (int)(container.size() - 1);
	}

#if !defined(RayTriangleEPSILON)

#define RayTriangleEPSILON 0.000001
#define RayTriangleCROSS(dest,v1,v2) \
    dest[0]=v1[1]*v2[2]-v1[2]*v2[1]; \
    dest[1]=v1[2]*v2[0]-v1[0]*v2[2]; \
    dest[2]=v1[0]*v2[1]-v1[1]*v2[0];
#define RayTriangleDOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])
#define RayTriangleSUB(dest,v1,v2) \
    dest[0]=v1[0]-v2[0]; \
    dest[1]=v1[1]-v2[1]; \
    dest[2]=v1[2]-v2[2];

#endif

	inline int rayTriangle(float orig[3], float dir[3],
		float vert0[3], float vert1[3], float vert2[3],
		float *t, float *u, float *v)
	{
		float edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
		float det, inv_det;

		/* find vectors for two edges sharing vert0 */
		RayTriangleSUB(edge1, vert1, vert0);
		RayTriangleSUB(edge2, vert2, vert0);

		/* begin calculating determinant - also used to calculate U parameter */
		RayTriangleCROSS(pvec, dir, edge2);

		/* if determinant is near zero, ray lies in plane of triangle */
		det = RayTriangleDOT(edge1, pvec);

#ifdef TEST_CULL           /* define TEST_CULL if culling is desired */
		if (det < RayTriangleEPSILON)
			return 0;

		/* calculate distance from vert0 to ray origin */
		RayTriangleSUB(tvec, orig, vert0);

		/* calculate U parameter and test bounds */
		*u = RayTriangleDOT(tvec, pvec);
		if (*u < 0.0 || *u > det)
			return 0;

		/* prepare to test V parameter */
		RayTriangleCROSS(qvec, tvec, edge1);

		/* calculate V parameter and test bounds */
		*v = RayTriangleDOT(dir, qvec);
		if (*v < 0.0 || *u + *v > det)
			return 0;

		/* calculate t, scale parameters, ray intersects triangle */
		*t = DOT(edge2, qvec);
		inv_det = 1.0 / det;
		*t *= inv_det;
		*u *= inv_det;
		*v *= inv_det;
#else                    /* the non-culling branch */
		if (det > -RayTriangleEPSILON && det < RayTriangleEPSILON)
			return 0;
		inv_det = 1.0f / det;

		/* calculate distance from vert0 to ray origin */
		RayTriangleSUB(tvec, orig, vert0);

		/* calculate U parameter and test bounds */
		*u = RayTriangleDOT(tvec, pvec) * inv_det;
		if (*u < 0.0 || *u > 1.0)
			return 0;

		/* prepare to test V parameter */
		RayTriangleCROSS(qvec, tvec, edge1);

		/* calculate V parameter and test bounds */
		*v = RayTriangleDOT(dir, qvec) * inv_det;
		if (*v < 0.0 || *u + *v > 1.0)
			return 0;

		/* calculate t, ray intersects triangle */
		*t = RayTriangleDOT(edge2, qvec) * inv_det;
#endif
		return 1;
	}

	//-----------------------------------------------------------------------------
	// intersection of a ray and an AABox
	//-----------------------------------------------------------------------------
	template<typename pointT> bool intersectRayAABox(const pointT & rO, const pointT & rV, const pointT& min, const pointT& max, float &tnear, float &tfar)
	{
		pointT T_1, T_2; // vectors to hold the T-values for every direction
		float t_near = -FLT_MAX;
		float t_far = FLT_MAX;

		for (int i = 0; i < 3; i++)
		{
			//we test slabs in every direction
			if (fabsf(rV[i]) <= FLT_EPSILON)
			{
				// ray parallel to planes in this direction
				if ((rO[i] < min[i]) || (rO[i] > max[i]))
				{
					return false; // parallel AND outside box : no intersection possible
				}
			}
			else
			{
				// ray not parallel to planes in this direction
				T_1[i] = (min[i] - rO[i]) / rV[i];
				T_2[i] = (max[i] - rO[i]) / rV[i];

				if (T_1[i] > T_2[i])
				{
					// we want T_1 to hold values for intersection with near plane
					//swap(T_1,T_2);
					float temp = T_1[i];
					T_1[i] = T_2[i];
					T_2[i] = temp;
				}
				if (T_1[i] > t_near)
				{
					t_near = T_1[i];
				}
				if (T_2[i] < t_far)
				{
					t_far = T_2[i];
				}
				if ((t_near > t_far) || (t_far < 0.f))
				{
					return false;
				}
			}
		}
		tnear = t_near; tfar = t_far; // put return values in place
		return true; // if we made it here, there was an intersection - YAY
	}

	template<typename vertexT, typename pointT> bool hierarchicalRaycast(const pointT& sourcePt, const pointT &end, int &triIndex, float& tt, int aabbIndex, const AABB<pointT>* aabbArray, const vertexT* vertices, const int* indices)
	{
		const AABB<pointT> & aabb(aabbArray[aabbIndex]);

		float tnear, tfar;
		if (intersectRayAABox<pointT>(sourcePt, end, aabb._AABBmin, aabb._AABBmax, tnear, tfar))
		{
			bool hasHit(false);

			if (aabb._left != -1)
				hasHit |= hierarchicalRaycast<vertexT, pointT>(sourcePt, end, triIndex, tt, aabb._left, aabbArray, vertices, indices);

			if (aabb._right != -1)
				hasHit |= hierarchicalRaycast<vertexT, pointT>(sourcePt, end, triIndex, tt, aabb._right, aabbArray, vertices, indices);

			for (int i = aabb._firstIndex; i < (aabb._firstIndex + aabb._indexCount); i += 3)
			{
				float tu, tv, t;
				float* v1 = (float*)&vertices[indices[i]];
				float* v2 = (float*)&vertices[indices[i + 1]];
				float* v3 = (float*)&vertices[indices[i + 2]];
				if (rayTriangle((float*)&sourcePt.x, (float*)&end.x,
					v1, v2, v3,
					&t, &tu, &tv))
				{
					if ((t<tt) && (t>-0.001f) && (t < 1.f))
					{
						triIndex = i;
						tt = t;
						hasHit = true;
					}
				}
			}
			return hasHit;
		}
		return false;
	}
};

#endif // GLM_CROWD_IO_INCLUDE_H
