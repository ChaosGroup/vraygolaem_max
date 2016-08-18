/*	Golaem Crowd API - v0.00 - Copyright (C) Golaem SA.  All Rights Reserved.

	Do this:
	#define GLMC_IMPLEMENTATION
	before you include this file in *one* C or C++ file to create the implementation.

	// i.e. it should look like this:
	#include ...
	#include ...
	#include ...
	#define GLMC_IMPLEMENTATION
	#include "glm_crowd.h"

	You can #define GLMC_ASSERT(expression) before the #include to avoid using system assert.
	And #define GLMC_MALLOC(size), GLMC_REALLOC(pointer, size), and GLMC_FREE(pointer, size) to avoid using malloc, realloc, and free

	QUICK NOTES:
	Primarily of interest to pipeline developers to integrate Golaem Crowd simulation cache

	Brief documentation under "DOCUMENTATION" below.

	Revision 0.00 (2015-03-12) release notes:

	- Functions and structures for creation, read/write and destruction of Golaem Simulation Caches
*/

#ifndef GLM_CROWD_INCLUDE_H
#define GLM_CROWD_INCLUDE_H

#include "stdint.h"
#include "stdio.h"

// DOCUMENTATION
//
// ===========================================================================
// Simulation Cache
//
// .gscs contains a SimulationData struct, simulation common data for all frames
// .gscf contains a FrameData struct, frame-specific data
//
// Basic read frames-data usage:
//		GlmSimulationCacheStatus status;
//		GlmSimulationData* simulationData;
//		status = glmCreateAndReadSimulationData(&simulationData, gscsFileName);
//		if (status != GSC_SUCCESS) ...
//		GlmFrameData* frameData;
//		glmCreateFrameData(&frameData, simulationData);
//		status = glmReadFrameData(frameData, simulationData, gscfFileName);
//		if (status != GSC_SUCCESS) ...
//		...
//		status = glmReadFrameData(frameData, simulationData, gscfFileName);
//		if (status != GSC_SUCCESS) ...
//		...
//		glmDestroyFrameData(&frameData);
//		glmDestroySimulationData(&simulationData);
//

//////////////////////////////////////////////////////////////////////////////
//
// Simulation Cache API
//

#ifdef __cplusplus
extern "C"
{
#endif
	extern const char golaemFrameExtension[];
	extern const char golaemSimulationExtension[];
	extern const char golaemTransformHistoryExtension[];
	extern const char golaemAssetAssociationExtension[];

	// Simulation cache format----------------------------------------------------
	typedef enum
	{
		GSC_O128_P96, // Golaem Simulation Cache, orientations and positions not quantized
		GSC_O64_P96, // Golaem Simulation Cache, orientations quantized on 64bit, positions not quantized
		GSC_O32_P96, // Golaem Simulation Cache, orientations quantized on 32bit, positions not quantized
		GSC_O128_P48, // Golaem Simulation Cache, orientations not quantized, positions quantized on 48bit
		GSC_O64_P48, // Golaem Simulation Cache, orientations quantized on 64bit, positions quantized on 48bit
		GSC_O32_P48, // Golaem Simulation Cache, orientations quantized on 32bit, positions quantized on 48bit
	} GlmSimulationCacheFormat;

	// Simulation cache data------------------------------------------------------

	// per-particle attributes types
	typedef enum
	{
		GSC_PP_FLOAT = 1,
		GSC_PP_VECTOR = 2
	} GlmPPAttributeType;

#define GSC_PP_MAX_NAME_LENGTH 256

	// per-simulation data
	typedef struct GlmSimulationData_v0
	{
		uint8_t		_version;
		uint32_t	_contentHashKey; // to check if simulation matches frame file

		// entity. 
		uint32_t _entityCount; // number of entities in the simulation cache
		int64_t* _entityIds; // entity Ids for all entities, array size = _entityCount
		uint16_t* _entityTypes; // entity types for all entities, array size = _entityCount
		uint32_t* _indexInEntityType; // entity index in its entityType, array size = _entityCount, entity if _entityId[i] is at index _indexInEntityType[i] of _entityType[i], useful to retrieve a specific position/orientation/etc.
		float* _scales; // scales for all entities, array size = _entityCount
		float* _entityRadius; // entity indexes for all entities, array size = _entityCount
		float* _entityHeight; // entity indexes for all entities, array size = _entityCount

		// entityType
		uint16_t _entityTypeCount; // number of different entity types
		uint32_t* _entityCountPerEntityType; // number of entities for this EntityType, array size = _entityTypeCount
		uint16_t* _boneCount; // number of bones for this EntityType, array size = _entityTypeCount
		uint32_t* _iBoneOffsetPerEntityType; // indexes of the first bone of the first entity of an EntityType, for _bonePositions and _boneOrientations, array size = _entityTypeCount
		float* _maxBonesHierarchyLength; // max length of a skeleton bones hierarchy, array size = _entityTypeCount
		uint16_t* _blindDataCount; // number of blindData for this EntityType, array size = _entityTypeCount
		uint32_t* _iBlindDataOffsetPerEntityType; // indexes of the first blindData of the first entity of an EntityType, for _blindData, array size = _entityTypeCount
		uint8_t* _hasGeoBehavior; // does this entity type have a geometry behavior?, array size = _entityTypeCount
		uint32_t* _iGeoBehaviorOffsetPerEntityType; // indexes of the first geometry behavior of the first entity of an EntityType, for _geoBehaviorGeometryIds, _geoBehaviorBlendModes and _geoBehaviorBlendModes, array size = _entityTypeCount
		uint16_t* _snsCountPerEntityType;	// count of sns information per entityType (defined by gskm/gch)
		uint32_t* _snsOffsetPerEntityType;	// offset per entityType = sum of previous (_snsCountPerEntityType * entityCountPerEntityType)

		// ppAttribute
		uint8_t _ppFloatAttributeCount; // number of Float attributes
		char(*_ppFloatAttributeNames)[GSC_PP_MAX_NAME_LENGTH]; // name of the float per particle attribute, array size first dimension = _floatAttributeCount, second dimension = _entityCountPerCrowdField
		uint8_t _ppVectorAttributeCount; // number of Vector attributes
		char(*_ppVectorAttributeNames)[GSC_PP_MAX_NAME_LENGTH]; // name of the vector per particle attribute, array size first dimension = _vectorAttributeCount, second dimension = _entityCountPerCrowdField
		uint8_t* _backwardCompatPPAttributeTypes;

		float _proxyMatrix[16];
		float _proxyMatrixInverse[16];

	} GlmSimulationData_v0;
	typedef GlmSimulationData_v0 GlmSimulationData;

	// per-frame data
	typedef struct GlmFrameData_v0
	{
		uint8_t		_cacheFormat;				// read cache format is stored to be able to rewrite this cache data with same format
		uint32_t	_simulationContentHashKey;  // to check if simulation matches frame file
		uint8_t		_hasSquashAndStretch; // if true at write time, cacheFormat will be forced to unquantized positions (P96) if P48 was chosen.

		// EntityType
		// /!\ Order is NOT the same as entity Arrays : it is ordered per EntityType, then per entity in its entityType : ET0.entity0, ET0.entity1, ET0.entity2, ..., ET1.entity0, ET1.entity1, ...
		float(*_bonePositions)[3]; // bones positions per Entity per EntityType, array size = _boneCount * _entityCountPerEntityType * _entityTypeCount
		float(*_boneOrientations)[4]; // bones orientations per Entity per EntityType, array size = _boneCount * _entityCountPerEntityType * _entityTypeCount
		float(*_snsValues)[4]; // Squatch and stretch values per EntityType, array size = _snsCountPerEntityType * entityCountPerEntityType* entityTypeCount
		float* _blindData; // blind data per Entity per EntityType, array size = _blindDataCount * _entityCountPerEntityType * _entityTypeCount : [ET0-E0-0blindDataCount][ET0-E1-0blindDataCount][ET1-E0-0blindDataCount]etc..
		uint16_t* _geoBehaviorGeometryIds; // geometry behavior geometry id, array size = _entityCountPerEntityType * _entityTypeCount
		float(*_geoBehaviorAnimFrameInfo)[3]; // geometry behavior animation frame info per Entity of an Entitype, array size =  _entityCountPerEntityType * _entityTypeCount, NULL if no geometry behavior. Infos are current, start and stop frames
		uint8_t* _geoBehaviorBlendModes; // geometry behavior blend mode per Entity of an Entitype, array size =  _entityCountPerEntityType * _entityTypeCount, NULL if no geometry behavior - stores a boolean as 0 or 1

		// allocation data for cloth, to avoid reallocation each frame if memory space is sufficient (not serialized)
		uint32_t _clothAllocatedEntities;
		uint32_t _clothAllocatedIndices;
		uint32_t _clothAllocatedVertices;

		// cloth :
		// entity uses cloth if _clothEntityCount != 0 && _entityClothIndex[entityIndex] is not -1
		// one cloth entity may have several cloth meshes.
		
		// serialization helpers
		uint32_t _clothEntityCount; // if 0, skip reading of everything else cloth-related (let to NULL)
		uint32_t _clothTotalMeshIndices; // shortcut for reading
		uint32_t _clothTotalVertices; // shortcut for reading

		// is entity using cloth ? if so, which cloth index to use if cloth arrays
		int32_t* _entityClothIndex; // helper : matching cloth entity index, or -1 if not used, size = _entityCount beware it is SIGNED
		
		// one entity have _clothEntityMeshCount[clothEntityIndex] meshes
		uint32_t* _clothEntityMeshCount; // count of cloth mesh indices for this cloth entity, size = _clothEntityCount, read in entity order

		// one clothIndex has _clothEntityMeshCount[clothEntityIndex] meshes
		// mesh indices in geometry starts at _clothEntityFirstAssetMeshIndex. 
		// there vertices starts at _clothEntityFirstMeshVertex (sequential read of all meshes)
		// the vertices are quantized according to _clothEntityQuantizationReference & _clothEntityQuantizationMaxExtent
		uint32_t* _clothEntityFirstAssetMeshIndex;	// helper : mesh indices for a given clothEntity starts at this index
		uint32_t* _clothEntityFirstMeshVertex;	// vertices for a given clothEntity starts at this index. Add all meshes vertices count to shift to right vertex for a given mesh
		float(*_clothEntityQuantizationReference)[3]; // _clothEntityCount elements, gives reference vertex for quantization = this entity vertices (min+max)/2 in all dimensions
		float* _clothEntityQuantizationMaxExtent; //_clothEntityCount elements, gives maxExtent of cloth for quantization max of (_clothEntityQuantizationReference - min value) in all dimensions

		// for each mesh, from _clothEntityFirstMeshVertex[clothIndex] to _clothEntityFirstMeshVertex[clothIndex] + _clothEntityMeshCount[clothIndex], we must read _clothMeshVertexCount

		// cloth data per mesh
		uint32_t* _clothMeshIndicesInCharAssets; // mesh indices, in original geometry, for cloth, in serial, entity0 indices, then entity 1 indices, etc
		uint32_t* _clothMeshVertexCount; // number of vertices to read for a given mesh index

		// cloth vertex positions array		
		float(*_clothVertices)[3]; // vertices position : clothEntity 0 mesh (0 to n1) vertices, clothEntity 1 mesh (0-n2), ..., up to last clothEntity last mesh vertices

		// ppAttribute
		float** _ppFloatAttributeData; // float data per particle attributes, array size first dimension = _floatAttributeCount
		float(**_ppVectorAttributeData)[3]; // vector data per particle attributes, array size = _vectorAttributeCount
	} GlmFrameData_v0;
	typedef GlmFrameData_v0 GlmFrameData;

	// per tranform layer
	typedef struct GlmHistory_v0
	{
		uint32_t _transformCount;
		uint8_t* _transformTypes; // values of SimulationCacheTransformType, size = _transformCount
		uint8_t* _active; // on/off, size = _transformCount
		uint32_t* _boneIndex; // bone to apply transformation on, size = _transformCount
		uint32_t* _renderingTypeIdx; // renderingTypeIdx if needed, size = _transformCount

		uint32_t _options; // options. see GlmHistoryOptions

		uint32_t _transformGroupCount;
		uint8_t* _transformGroupActive; //size = _transformGroupCount
		char(*_transformGroupName)[GSC_PP_MAX_NAME_LENGTH]; // name of the transform Group, array size first dimension = _transformGroupCount, second dimension = GSC_PP_MAX_NAME_LENGTH
		uint32_t(* _transformGroupBoundaries)[2]; // group boundaries, size = _transformGroupCount


		float(*_transformRotate)[4]; // transform Quaternion parameter, size = _transformCount
		float(*_transformTranslate)[3]; // transform translation parameter, size = _transformCount
		float(*_transformPivot)[3]; // transform pivot world position, size = _transformCount
		float *_scale; // uniform scale

		uint32_t* _clothIndice; // Cloth indice if needed, size = _transformCount
		uint32_t* _enableCloth; // Enable/Disable cloth if needed, size = _transformCount

		uint32_t _entityCount; // array size of all entities operations
		int64_t* _entityIds; // entities list. size = _entityCount
		uint32_t* _entityArrayStartIndex; // index of first entity used for operation in _entityIds array. size = _transformCount
		uint32_t* _entityArrayCount; // count of entities transformed per operation. size = _transformCount

		// entity ids for duplicated entities
		// work the same way entityIds work
		uint32_t _duplicatedEntityCount;
		int64_t* _duplicatedEntityIds;
		uint32_t* _duplicatedEntityArrayStartIndex; // size = _transformCount
		uint32_t* _duplicatedEntityArrayCount; // size = _transformCount

		// mesh asset override
		// Override is not visible in GlmEntityTransform because it's handled at rendering time with the .CAA
		uint32_t _expandCount;
		float(*_expands)[3];
		uint32_t* _expandArrayStartIndex; // size = _transformCount
		uint32_t* _expandArrayCount; // size = _transformCount

		// trajectory/.. per frame translation&orientation
		uint32_t _perFramePosOriCount;
		float(*_frameCurvePos)[3];
		float(*_frameCachePos)[3];
		float(*_framePos)[3];
		float(*_frameOri)[4];
		uint32_t* _perFramePosOriArrayStartIndex; // size = _transformCount
		uint32_t* _perFramePosOriArrayCount; // size = _transformCount

		// additional scale
		// work the same way entityIds work
		uint32_t _scaleRangeCount;
		float*_scaleRanges;
		uint32_t* _scaleRangeArrayStartIndex; // size = _transformCount
		uint32_t* _scaleRangeArrayCount; // size = _transformCount
		// it's present here for storage and maniupulation (enable/disable)
		// mesh asset override
		// Override is not visible in GlmEntityTransform because it's handled at rendering time with the .CAA
		// it's present here for storage and maniupulation (enable/disable)
		uint32_t _meshAssetsOverrideTotalCount;	// total count of mesh asset for all entities 
		uint32_t* _meshAssetsOverride;			// list all the mesh asset override. size = _meshAssetsOverrideTotalCount
		uint32_t* _meshAssetsOverrideStartIndex; // index of first mesh assets override in _meshAssetsOverride. size = _entityCount
		uint32_t* _meshAssetsOverrideCount;		// count of mesh assets override in _meshAssetsOverride. size = _entityCount
		
		// entity type hierarchical bones. use _iBoneOffsetPerEntityType from simulation data to retrieve local orientation/position
		uint32_t _localBoneCount;
		float(*_localBoneOrientation)[4];	// size = _localBoneCount
		float(*_localBonePosition)[3];		// size = _localBoneCount
		uint32_t *_localBoneParent;			// size = _localBoneCount

		uint32_t _localBoneOffsetCount; // == _entityTypeCount
		uint32_t *_localBoneOffset; // per entityType size = _entityTypeCount

		// postures editing
		uint32_t _postureCount;
		uint32_t _postureTotalBoneCount; // = _postureTotalBoneCount * count of bones in the posture. posture bone count =  _totalPostureBoneCount / _totalPostureCount
		uint32_t *_posturesFrameCount; // count of uint32_t. Array size = _transformCount
		uint32_t *_posturesFrameStart; // index in _posturesFrames. Array size = _transformCount
		
		uint32_t *_posturesFrames; // size = sum of all _posturesFrameCount == _totalPosturesCount

		float(*_posturesPositions)[3]; 
		float(*_posturesOrientations)[4];

		// frame offsets
		uint32_t _frameOffsetCount;
		float* _frameOffsets;
		uint32_t* _frameOffsetArrayStartIndex; // size = _transformCount
		uint32_t* _frameOffsetArrayCount; // size = _transformCount

		// frame scales
		uint32_t _frameWarpCount;
		float* _frameWarps;
		uint32_t* _frameWarpArrayStartIndex; // size = _transformCount
		uint32_t* _frameWarpArrayCount; // size = _transformCount

		// frame transform paramters
		float *_frameOffsetMin; // size = _transformCount
		float *_frameOffsetMax; // size = _transformCount
		float *_frameWarpMin; // size = _transformCount
		float *_frameWarpMax; // size = _transformCount

		// scale range min/max
		float *_scaleRangeMin; // size = _transformCount
		float *_scaleRangeMax; // size = _transformCount

		// frame start/count for per frame pos/ori
		uint32_t *_startFrame; // size = _transformCount
		uint32_t *_frameCount; // size = _transformCount
		uint32_t *_trajectoryMode; // size = _transformCount
		uint32_t *_trajectorySteps; // size = _transformCount
		uint32_t *_smoothIterationCount; // size = _transformCount
		float *_smoothFrontBackRatio; // size = _transformCount
		float(*_smoothComponents)[3]; // size = _transformCount

		char(*_snapToTarget)[GSC_PP_MAX_NAME_LENGTH]; // name of the target object, array size first dimension = _transformCount, second dimension = GSC_PP_MAX_NAME_LENGTH
		uint32_t _snapToTotalCount;	// total count of snapTo Target for all entities 
		uint32_t* _snapToStartIndex; // size = _transformCount
		uint32_t* _snapToCount; // size = _transformCount
		float(*_snapToPositions)[3]; // size = _snapToTotalCount
		float(*_snapToRotations)[4]; // size = _snapToTotalCount

		char(*_shaderAttribute)[GSC_PP_MAX_NAME_LENGTH]; // name of the target object, array size first dimension = _transformCount, second dimension = GSC_PP_MAX_NAME_LENGTH
		void *_terrainMeshSource; // used at rendering time. not serialized
		void *_terrainMeshDestination; // used at rendering time. not serialized
	} GlmHistory_v0;
	typedef GlmHistory_v0 GlmHistory;

	// transformation per entity for whole cache. *Not saved.* Only used for modification on cache

	typedef struct GlmFrameOffset_V0
	{
		int _entityIndex;
		// frame offset
		float _frameOffset;
		float _frameWarp;

		int _frameIndex; // global frame index
		int _framesLoadedIndex; // index in frames loaded
		float _fraction;
	} GlmFrameOffset;

	// transformation per entity for whole cache. *Not saved.* Only used for modification on cache
	typedef struct GlmEntityTransform_v0
	{
		int64_t _entityId;
		float _matrix[16];
		float (_orientation)[4];
		float _matrixBase[16];
		float (_orientationBase)[4];

		int _sourceIndexInCrowdField; // duplicate entities have the same index as their source
		float _scale; 

		char _useCloth;
		char _killed;
		// pivot position used for scaling
		float (_scalePivot)[3];

		// ground adaptation
		float (_groundAdaptOffset)[3];
		float (_groundAdaptOrientation)[4];

		// edited posture
		uint32_t _postureCount;
		uint32_t *_postureFrames;
		
		uint32_t _postureBoneCount; // bones per posture. total bone for this entity must be multiplied by _postureFrameCount. Just a helper instead of accessing simulation data.
		float(*_posturesPositions)[3]; // transform vector4 parameter, size = _transformCount
		float(*_posturesOrientations)[4];
		
		// edited bones rest relative

		float(*_boneRestRelativeOrientation)[4];	// free if not NULL
		float(*_boneRestRelativePosition)[3];		// free if not NULL

		// 
		unsigned int _lastEditPostureHistoryIndex; // last history transform index in the transform list. So we can apply bone history transform from that index. Edit posture works like keyframe.

		// cloth
		uint32_t _clothedEntityIndex;
		float(*_clothVerticesSource)[3];
		uint32_t* _clothIndicesSource;
		uint32_t* _clothMeshVertexCountSource;
		//uint32_t* _clothMeshVertexOffsetPerClothIndexSource;

		int _clothTotalMeshIndices;
		int _clothTotalVertices;

		// work buffer allocated once for every entity -> transformation/frame modification is not threadsafe
		GlmFrameOffset _frameOffset;
		int _outOfCache; // when no frame can be loaded for that entity because of time offset/time warp

		// per frame pos/or
		int _perFramePosOriIndex;
		int _perFramePosOriArrayCount;

		// work buffer allocated once for every entity -> transformation/frame modification is not threadsafe
		float(*_sortedBonesWorldOri)[4];
		float(*_sortedBonesWorldPos)[3];
		float(*_sortedBonesScale)[4];
		float(*_restRelativeOri)[4];
	} GlmEntityTransform_v0;
	typedef GlmEntityTransform_v0 GlmEntityTransform;

	// history options
	typedef enum
	{
		OptionsGroundAdaptOrient = 1<<0,
		OptionsGroundAdaptUseTerrain = 1 << 1,
	} GlmHistoryOptions;

	// Transformation Types----------------------------------------------
	typedef struct GlmFrameToLoad_V0
	{
		int _frameIndex;
		GlmFrameData* _frame;
	} GlmFrameToLoad;

	// Transformation Types----------------------------------------------
	typedef enum 
	{
		SimulationCacheNoop,
		SimulationCacheRotate,
		SimulationCacheScale,
		SimulationCacheTranslate,
		SimulationCacheDuplicate,
		SimulationCacheKill,
		SimulationCacheUnkill,
		SimulationCachePosture,
		SimulationCachePostureBoneEdit,
		SimulationCacheSetMeshAssets,
		SimulationCacheEnableClothMeshAssets,
		SimulationCacheExpand,
		SimulationCacheFrameOffset,
		SimulationCacheFrameWarp,
		SimulationCacheScaleRange,
		SimulationCacheTrajectoryEdit,
		SimulationCacheSnapTo,
		SimulationCacheSetShaderAttribute,
	} GlmSimulationCacheTransformType;

	// Simulation cache status codes----------------------------------------------
	typedef enum
	{
		GSC_SUCCESS,
		GSC_FILE_OPEN_FAILED,
		GSC_FILE_MAGIC_NUMBER_ERROR, // file header does not begin by 0x65CF or 0x65CF, this is not a Golaem Simulation Cache
		GSC_FILE_VERSION_ERROR, // incorrect version, could be a newer version of the Golaem Simulation Cache
		GSC_FILE_FORMAT_ERROR, // incorrect format, could be a newer version of the Golaem Simulation Cache
		GSC_SIMULATION_FILE_DOES_NOT_MATCH, // used Golaem Simulation Cache simulation file does not match this frame file
		GSC_SIMULATION_NO_FRAMES_FOUND, // used when no frame could be loaded from the cache
	} GlmSimulationCacheStatus;

	// convert a simulation cache status in an error message
	const char* glmConvertSimulationCacheStatus(GlmSimulationCacheStatus status);

	// Simulation cache functions-------------------------------------------------

	// allocate *simulationData
	void glmCreateSimulationData(
		GlmSimulationData** simulationData, // *simulationData will be allocated by this function
		uint32_t entityCount, // number of entities, range = 0..4,294,967,295
		uint16_t entityTypeCount, // number of different entity types, range = 1..65,535
		uint8_t ppFloatAttributeCount, // number of float per particle attributes, range = 0..255
		uint8_t ppVectorAttributeCount); // number of vector per particle attributes, range = 0..255

	// allocate and initialize *simulationData from a .gscs file
	// return GSC_SUCCESS || GSC_FILE_OPEN_FAILED || GSC_FILE_MAGIC_NUMBER_ERROR || GSC_FILE_VERSION_ERROR
	extern GlmSimulationCacheStatus glmCreateAndReadSimulationData(GlmSimulationData** simulationData, const char* file);

	// write simulationData in a .gscs file
	// return GSC_SUCCESS || GSC_FILE_OPEN_FAILED
	extern GlmSimulationCacheStatus glmWriteSimulationData(const char* file, const GlmSimulationData* simulationData);

	// deallocate *simulationData and set it to NULL
	extern void glmDestroySimulationData(GlmSimulationData** simulationData);

	// allocate *frameData
	extern void glmCreateFrameData(GlmFrameData** frameData, const GlmSimulationData* simulationData);

	// allocate cloth data for frame data (except entityInUse), not in glmCreateFrameData because cloth count can change at every frame
	uint64_t glmComputeSimulationDataSize(const GlmSimulationData* data);

	// Compute the size in bytes allocated for a GlmFrameData
	uint64_t glmComputeFrameDataSize(const GlmFrameData* frameData, const GlmSimulationData* simulationData);

	// allocate cloth data for frame data (except entityInUse), not in glmCreateFrameData because cloth count can change at every frame
	extern void glmCreateClothData(const GlmSimulationData* simuData, GlmFrameData* frameData, unsigned int clothEntityCount, unsigned int clothIndices, unsigned int clothVertices);

	// read a .gscf file in the previously allocated *frameData
	// return GSC_SUCCESS || GSC_FILE_OPEN_FAILED || GSC_FILE_MAGIC_NUMBER_ERROR || GSC_FILE_VERSION_ERROR || GSC_FILE_FORMAT_ERROR
	extern GlmSimulationCacheStatus glmReadFrameData(GlmFrameData* frameData, const GlmSimulationData* simulationData, const char* file);

	// write *frameData in a .gscf file
	// return GSC_SUCCESS || GSC_FILE_OPEN_FAILED
	extern GlmSimulationCacheStatus glmWriteFrameData(const char* file, const GlmFrameData* frameData, const GlmSimulationData* simulationData);

	// deallocate *frameData and set it to NULL
	extern void glmDestroyFrameData(GlmFrameData** frameData, const GlmSimulationData* simulationData);

	// create a transform layer
	extern void glmCreateHistory(GlmHistory** history, unsigned int transformCount, unsigned int transformGroupCount, unsigned int entityCount, unsigned int totalPostureCount, unsigned int totalPostureBoneCount, unsigned int totalEntityTypesBoneCount, unsigned int totalMeshAssetsOverride, unsigned int duplicatedEntityCount, unsigned int totalEntityTypeCount, unsigned int expandCount, unsigned int totalFrameOffsetCount, unsigned int totalFrameWarpCount, unsigned int scaleRangeCount, unsigned int perFramePosOriCount, unsigned int snapToTotalCount);

	// deallocate history and set it to NULL
	extern void glmDestroyHistory(GlmHistory** history);

	// perform a raycast on terrain mesh and return true if hit. collision point is the closest to the rayorigin
	extern int (*glmRaycastClosest)(void *terrain, const float* rayOrigin, const float* rayEnd, float *collisionPoint, float *collisionNormal, float *proxyMatrix, float *proxyMatrixInverse);

	extern void(*glmTerrainSetFrame)(void *terrainSource, void *terrainDestination, int frame);
#ifndef GLMC_NO_JSON
	// write *frameData in a JSON .gscla file
	// return GSC_SUCCESS || GSC_FILE_OPEN_FAILED
	extern GlmSimulationCacheStatus glmWriteHistoryJSON(const char* file, GlmHistory* history);

	// allocate and initialize *history from a JSON .gscla file
	// return GSC_SUCCESS || GSC_FILE_OPEN_FAILED || GSC_FILE_MAGIC_NUMBER_ERROR || GSC_FILE_VERSION_ERROR
	extern GlmSimulationCacheStatus glmCreateAndReadHistoryJSON(GlmHistory** history, const char* file);
#endif
	// create an entityTransform array from TransformHistory and GlmSimulationData
	// computes frames needed when using time offset and time scale
	unsigned int glmComputeEntityFrameOffsets(GlmFrameOffset *frameOffsets, int entityCount, int currentFrame, GlmFrameToLoad *frames);

	// create an entityTransform array from TransformHistory and GlmSimulationData
	void glmCreateEntityTransforms(GlmSimulationData* simulationData, GlmHistory* history, GlmEntityTransform** entityTransforms, int *entityTransformCount);

	// deallocate a GlmEntityTransform
	void glmDestroyEntityTransforms(GlmEntityTransform** entityTransforms, unsigned int entityTransformCount);

	// Create a new GlmSimulationData pointed by simulationDataDestination made of simulationDataSource and modifications. User is responsible of simulationDataDestination deletion
	void glmCreateModifiedSimulationData(GlmSimulationData* simulationDataSource, GlmEntityTransform* entityTransforms, unsigned int entityTransformCount, GlmSimulationData** simulationDataDestination);

	// Create a new GlmFrameData pointed by frameDataDestination made of frameDataSource and modifications. User is responsible of frameDataDestination deletion
	GlmSimulationCacheStatus glmCreateModifiedFrameData(GlmSimulationData* simulationDataIn, GlmFrameData* frameDataIn, GlmEntityTransform* entityTransforms, unsigned int entityTransformCount, GlmHistory* history, GlmSimulationData* simulationDataOut, GlmFrameData** frameDataOut, int currentFrame, const char * filePathModel, const char * cacheDirectory);

	void glmInterpolateFrameData(const GlmSimulationData* simulationData, const GlmFrameData* frameData1, const GlmFrameData* frameData2, float ratio, GlmFrameData* result); // ratio must be between 0 (full frame 1) and 1 (full frame 2), frames must be of same simulationData

	uint32_t getClothEntityMeshCount(const GlmFrameData* frameData, int clothEntityIndex);
	uint32_t getClothEntityIMeshVertexCount(const GlmFrameData* frameData, int clothEntityIndex, int iMesh); // a clothEntity has "meshCount" meshes. Get each of its index in all cloth entities meshes cache via this.
	void getClothEntityIMeshVerticesPtr(const GlmFrameData* frameData, int clothEntityIndex, int iMesh, float(**outFirstVertexPtr)[3]); // a clothEntity has "meshCount" meshes. Get each of its index in all cloth entities meshes cache via this.

	// compression interface (usde in crowd_io)
	extern void glmFileWrite(const void* data, unsigned long elementSize, unsigned long count, FILE* fp);
	extern void glmFileWriteUInt16(const uint16_t* data, unsigned int count, FILE* fp);
	extern void glmFileWriteUInt32(const uint32_t* data, unsigned int count, FILE* fp);
	extern void glmFileWriteUInt64(const uint64_t* data, unsigned int count, FILE* fp);
	
	extern void glmFileRead(void* data, unsigned long elementSize, unsigned long count, FILE* fp);
	extern void glmFileReadUInt16(uint16_t* data, unsigned int count, FILE* fp);
	extern void glmFileReadUInt32(uint32_t* data, unsigned int count, FILE* fp);
	extern void glmFileReadUInt64(uint64_t* data, unsigned int count, FILE* fp);

#ifdef __cplusplus
}
#endif

//
//
////   end header file   /////////////////////////////////////////////////////
#endif // GLM_CROWD_INCLUDE_H

//////////////////////////////////////////////////////////////////////////////
//
// Implementation
//
#ifdef GLMC_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <limits.h>

#ifdef _MSC_VER
#include <direct.h>
#include <io.h>
#else
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#endif

#ifndef GLMC_ASSERT
#include <assert.h>
#ifdef NDEBUG
#define GLMC_ASSERT(x) (void)(x)
#else
#define GLMC_ASSERT(x) assert(x)
#endif
#endif

#ifndef GLMC_NOT_INCLUDE_MINIZ
#include "miniz.c"
#endif

#ifndef GLMC_NO_JSON
#include "json.h"
#endif

#ifndef UINT8_MAX
#define UINT8_MAX 0xff
#endif
#ifndef INT8_MAX
#define INT8_MAX 0x7f
#endif

#ifndef UINT16_MAX
#define UINT16_MAX 0xffff
#endif
#ifndef INT16_MAX
#define INT16_MAX 0x7fff
#endif

#ifndef UINT32_MAX
#define UINT32_MAX (0xffffffffUL)
#endif
#ifndef INT32_MAX
#define INT32_MAX (0x7fffffffL)
#endif

#ifndef INT32_MIN
#define INT32_MIN (-INT32_MAX-1)
#endif

#if defined(GLMC_MALLOC) && defined(GLMC_FREE) && defined(GLMC_REALLOC)
// ok
#elif !defined(GLMC_MALLOC) && !defined(GLMC_FREE) && !defined(GLMC_REALLOC)
// ok
#else
#error "Must define all or none of GLMC_MALLOC, GLMC_FREE, and GLMC_REALLOC."
#endif

#ifndef GLMC_MALLOC
#define GLMC_MALLOC(sz)    malloc(sz)
#define GLMC_REALLOC(p, sz) realloc(p, sz)
#define GLMC_FREE(p)       free(p)
#endif

#define GLMC_EPSILON 0.001f
#define GLMC_PI 3.14159265358979323846f
#define GLMC_PI_DIV_2 1.57079632679489661923f
#define GLMC_1_DIV_SQRT_2 0.7071067811865475f

int(*glmRaycastClosest)(void *terrain, const float* rayOrigin, const float* rayEnd, float *collisionPoint, float *collisionNormal, float *proxyMatrix, float *proxyMatrixInverse) = NULL;
void(*glmTerrainSetFrame)(void *terrainSource, void *terrainDestination, int frame) = NULL;

//////////////////////////////////////////////////////////////////////////////
//
// FNV Hash
//
// reference: http://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
// offset basis = 32 bit offset_basis = 2166136261
// FNV_prime = 32 bit FNV_prime = 224 + 28 + 0x93 = 16777619

// algorithm :
// hash = offset_basis
// for each octet_of_data to be hashed
//  hash = hash xor octet_of_data
//  hash = hash * FNV_prime
// return hash

const uint32_t FNV_prime = 16777619;
const uint32_t FNV_offset_basis = 2166136261;

static void glmStartHash(uint32_t *currentHashValue)
{
	*currentHashValue = FNV_offset_basis;
}

static void glmCumulativeHash8(uint8_t valueToHash, uint32_t* currentHashValue)
{
	*currentHashValue ^= valueToHash;
	*currentHashValue *= FNV_prime;
}

static void glmCumulativeHash16(uint16_t valueToHash, uint32_t* currentHashValue)
{
	glmCumulativeHash8(valueToHash & 0x0F, currentHashValue);
	glmCumulativeHash8(valueToHash >> 8, currentHashValue);
}

static void glmCumulativeHash32(uint32_t valueToHash, uint32_t* currentHashValue)
{
	glmCumulativeHash8(valueToHash & 0x000F, currentHashValue);
	glmCumulativeHash8((valueToHash & 0x00F0) >> 8, currentHashValue);
	glmCumulativeHash8((valueToHash & 0x0F00) >> 16, currentHashValue);
	glmCumulativeHash8(valueToHash >> 24, currentHashValue);
}

//////////////////////////////////////////////////////////////////////////////
//
// Endian
//

#ifdef GLMC_BIG_ENDIAN
// byte-swap the 16-bit argument
static uint16_t glmSwapByteOrder16(uint16_t value)
{
#if defined(_MSC_VER) && !defined(_DEBUG)
	return _byteswap_ushort(value);
#elif (defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3)))
	return __builtin_bswap16(value);
#else
	return ((value & (uint16_t)0x00ffU) << 8U) // byte 0
		| ((value & (uint16_t)0xff00U) >> 8U); // byte 1
#endif
}

// byte-swap the 32-bit argument
static uint32_t glmSwapByteOrder32(uint32_t value)
{
#if defined(_MSC_VER) && !defined(_DEBUG)
	return _byteswap_ulong(value);
#elif (defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3)))
	return __builtin_bswap32(value);
#else
	return ((value & (uint32_t)0x000000ffUL) << 24U) // byte 0
		| ((value & (uint32_t)0x0000ff00UL) << 8U) // byte 1
		| ((value & (uint32_t)0x00ff0000UL) >> 8U) // byte 2
		| ((value & (uint32_t)0xff000000UL) >> 24U); // byte 3
#endif
}

// byte-swap the 64-bit argument
static uint64_t glmSwapByteOrder64(uint64_t value)
{
#if defined(_MSC_VER) && !defined(_DEBUG)
	return _byteswap_uint64(value);
#elif (defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3)))
	return __builtin_bswap64(value);
#else
	return ((value & (uint64_t)0x00000000000000ffULL) << 56U) // byte 0
		| ((value & (uint64_t)0x000000000000ff00ULL) << 40U) // byte 1
		| ((value & (uint64_t)0x0000000000ff0000ULL) << 24U) // byte 2
		| ((value & (uint64_t)0x00000000ff000000ULL) << 8U) // byte 3
		| ((value & (uint64_t)0x000000ff00000000ULL) >> 8U) // byte 4
		| ((value & (uint64_t)0x0000ff0000000000ULL) >> 24U) // byte 5
		| ((value & (uint64_t)0x00ff000000000000ULL) >> 40U) // byte 6
		| ((value & (uint64_t)0xff00000000000000ULL) >> 56U); // byte 7
#endif
}
#endif

//////////////////////////////////////////////////////////////////////////////
//
// Quantization compression
//
// references:
// Jason Gregory, Game Engine Architecture book, Chapter Animation Systems / Compression Techniques / Quantization
// Niklas Frykholm, Bitsquid development blog, Low Level Animation Systems, http://bitsquid.blogspot.fr/2009/11/bitsquid-low-level-animation-system.html
// Johnb, Gamedev forum post, http://www.gamedev.net/topic/461253-compressed-quaternions/

#define GLMC_COMPRESSED_QUATERNION_LARGEST_COMPONENT 0x00000003UL

#define GLMC_COMPRESSED_QUATERNION32_COMPONENT_0 0x00000ffcUL
#define GLMC_COMPRESSED_QUATERNION32_BIT_SHIFT 10U

#define GLMC_COMPRESSED_QUATERNION64_COMPONENT_0 0x00000000003fffcULL
#define GLMC_COMPRESSED_QUATERNION64_BIT_SHIFT 20U

// map a floating-point value to an interval, during the encoding process the 
// float is rounded to the center of the center of the enclosing interval (R)
static void glmCompressFloatRL(uint32_t* quantized, float value, float min, float max, uint32_t nBits)
{
	float unitFloat = (value - min) / (max - min);

	// number of intervals based on the number of ouput bits asked to produce
	uint32_t nIntervals = 1u << nBits;

	// scale the input value from the range [0,1] into the range [0, nIntervals - 1]
	// we substract one interval because we want the largest output value to fit into nBits bits.
	float scaled = unitFloat * (float)(nIntervals - 1u);

	// finally round to the nearest interval center. We do this by adding 0.5f,
	// and then truncating to the next-lowest interval index (by casting to uint32_t)
	*quantized = (uint32_t)(scaled + 0.5f);

	// guard against invalid input values
	if (*quantized > nIntervals - 1u)
	{
		*quantized = nIntervals - 1u;
	}
}

// map an interval value to a floating-point value, during the decoding process the float is reconstructed 
// using the value of the lefthand side of the interval to which our original value was mapped (L)
static void glmUncompressFloatRL(float* value, uint32_t quantized, float min, float max, uint32_t nBits)
{
	// determine the number of intervals based on the number of bits we used when we encoded the value
	uint32_t nIntervals = 1u << nBits;

	// decode by simply converting the uint32_t to a float, and scaling by the interval size
	float intervalSize = 1.0f / (float)(nIntervals - 1u);

	float approxUnitFloat = (float)quantized * intervalSize;

	*value = min + (approxUnitFloat * (max - min));
}

// compress a quaternion on 32 bit: 2 bits for largest component position, 10 bit to store 3 smallest components.
// the largest component is computed from the other three
// measured distance precision: max = 0.0022375863 mean = 0.000730949047 standard deviation = 0.000268183067
// measured component error: max = 0.00191289186 mean = 0.000299098203
// measures done on 10000000 generated normalized quaternions, checked on sample simulation caches
static void glmCompressQuaternion32(uint32_t* outputQuaternion, float inputQuaternion[4])
{
	uint32_t bitShift = 2;
	int i;
	int iMax = 0;
	float largestComponent = 0.f;
	float component;

	// find which component has the largest absolute value
	for (i = 0; i < 4; ++i)
	{
		component = fabsf(inputQuaternion[i]);
		if (component > largestComponent)
		{
			iMax = i;
			largestComponent = component;
		}
	}

	*outputQuaternion = iMax & GLMC_COMPRESSED_QUATERNION_LARGEST_COMPONENT;

	for (i = 0; i < 4; ++i)
	{
		if (i != iMax)
		{
			uint32_t quantizedComponent;
			// inverse the sign of the components if largestComponent is negative
			if (inputQuaternion[iMax] >= 0.f)
			{
				glmCompressFloatRL(&quantizedComponent, inputQuaternion[i], -GLMC_1_DIV_SQRT_2, GLMC_1_DIV_SQRT_2, GLMC_COMPRESSED_QUATERNION32_BIT_SHIFT);
			}
			else
			{
				glmCompressFloatRL(&quantizedComponent, -inputQuaternion[i], -GLMC_1_DIV_SQRT_2, GLMC_1_DIV_SQRT_2, GLMC_COMPRESSED_QUATERNION32_BIT_SHIFT);
			}
			*outputQuaternion |= quantizedComponent << bitShift;
			bitShift += GLMC_COMPRESSED_QUATERNION32_BIT_SHIFT;
		}
	}
}

static void glmUncompressQuaternion32(float outputQuaternion[4], uint32_t inputQuaternion)
{
	int i;
	int max = inputQuaternion & GLMC_COMPRESSED_QUATERNION_LARGEST_COMPONENT;
	uint64_t mask = GLMC_COMPRESSED_QUATERNION32_COMPONENT_0;
	uint32_t bitShift = 2;
	float largestComponent2 = 1.0f;
	uint32_t component;

	for (i = 0; i < 4; ++i)
	{
		if (i != max)
		{
			component = (uint32_t)((inputQuaternion & mask) >> bitShift);
			mask = mask << GLMC_COMPRESSED_QUATERNION32_BIT_SHIFT;
			bitShift += GLMC_COMPRESSED_QUATERNION32_BIT_SHIFT;
			glmUncompressFloatRL(&outputQuaternion[i], component, -GLMC_1_DIV_SQRT_2, GLMC_1_DIV_SQRT_2, GLMC_COMPRESSED_QUATERNION32_BIT_SHIFT);
			largestComponent2 -= (outputQuaternion[i] * outputQuaternion[i]);
		}
	}
	outputQuaternion[max] = sqrtf(largestComponent2);
}

// compress a quaternion on 64 bit: 2 bit for largest component position, 20 bit to store 3 smallest components.
// the largest component is computed from the other three
// measured distance precision: max = 0.000690698624 mean = 0.0000861751032 standard deviation = 0.000153445129
// measured component error: max = 0.00000202655792 mean = 0.000000293557605
// measures done on 10000000 generated normalized quaternions
static void glmCompressQuaternion64(uint64_t* outputQuaternion, float inputQuaternion[4])
{
	uint32_t bitShift = 2;
	int i;
	int iMax = 0;
	float largestComponent = 0.f;
	float component;

	// find which component has the largest absolute value
	for (i = 0; i < 4; ++i)
	{
		component = fabsf(inputQuaternion[i]);
		if (component > largestComponent)
		{
			iMax = i;
			largestComponent = component;
		}
	}

	*outputQuaternion = iMax & GLMC_COMPRESSED_QUATERNION_LARGEST_COMPONENT;

	for (i = 0; i < 4; ++i)
	{
		if (i != iMax)
		{
			uint32_t quantizedComponent;
			// inverse the sign of the components if largestComponent is negative
			if (inputQuaternion[iMax] >= 0.f)
			{
				glmCompressFloatRL(&quantizedComponent, inputQuaternion[i], -GLMC_1_DIV_SQRT_2, GLMC_1_DIV_SQRT_2, GLMC_COMPRESSED_QUATERNION64_BIT_SHIFT);
			}
			else
			{
				glmCompressFloatRL(&quantizedComponent, -inputQuaternion[i], -GLMC_1_DIV_SQRT_2, GLMC_1_DIV_SQRT_2, GLMC_COMPRESSED_QUATERNION64_BIT_SHIFT);
			}
			*outputQuaternion |= (uint64_t)quantizedComponent << bitShift;
			bitShift += GLMC_COMPRESSED_QUATERNION64_BIT_SHIFT;
		}
	}
}

static void glmUncompressQuaternion64(float outputQuaternion[4], uint64_t inputQuaternion)
{
	int i;
	int max = inputQuaternion & GLMC_COMPRESSED_QUATERNION_LARGEST_COMPONENT;
	uint64_t mask = GLMC_COMPRESSED_QUATERNION64_COMPONENT_0;
	uint32_t bitShift = 2;
	float largestComponent2 = 1.0f;
	uint32_t component;

	for (i = 0; i < 4; ++i)
	{
		if (i != max)
		{
			component = (uint32_t)((inputQuaternion & mask) >> bitShift);
			mask = mask << GLMC_COMPRESSED_QUATERNION64_BIT_SHIFT;
			bitShift += GLMC_COMPRESSED_QUATERNION64_BIT_SHIFT;
			glmUncompressFloatRL(&outputQuaternion[i], component, -GLMC_1_DIV_SQRT_2, GLMC_1_DIV_SQRT_2, GLMC_COMPRESSED_QUATERNION64_BIT_SHIFT);
			largestComponent2 -= (outputQuaternion[i] * outputQuaternion[i]);
		}
	}
	outputQuaternion[max] = sqrtf(largestComponent2);
}


// compress a position on 96 bit: 16 bit per component, on the range -max..max
// measured component error: max < 0.002
// measures done on sample simulation caches, checked on sample simulation caches
static void glmCompressPosition48(uint16_t outputPosition[3], float inputPosition[3], float max)
{
	uint32_t component;
	glmCompressFloatRL(&component, inputPosition[0], -max, max, 16u);
	outputPosition[0] = (uint16_t)component;
	glmCompressFloatRL(&component, inputPosition[1], -max, max, 16u);
	outputPosition[1] = (uint16_t)component;
	glmCompressFloatRL(&component, inputPosition[2], -max, max, 16u);
	outputPosition[2] = (uint16_t)component;
}

static void glmUncompressPosition48(float outputPosition[3], uint16_t inputPosition[3], float max)
{
	glmUncompressFloatRL(&outputPosition[0], (uint32_t)inputPosition[0], -max, max, 16u);
	glmUncompressFloatRL(&outputPosition[1], (uint32_t)inputPosition[1], -max, max, 16u);
	glmUncompressFloatRL(&outputPosition[2], (uint32_t)inputPosition[2], -max, max, 16u);
}

//////////////////////////////////////////////////////////////////////////////
//
// Compressed file read/write
//
// Golaem file read/write functions handling byte swapping and zlib compression

#ifdef __cplusplus
extern "C"
{
#endif
	extern int mz_compress(unsigned char *pDest, unsigned long *pDest_len, const unsigned char *pSource, unsigned long source_len);
	extern int mz_uncompress(unsigned char *pDest, unsigned long *pDest_len, const unsigned char *pSource, unsigned long source_len);
#ifdef __cplusplus
}
#endif

// handle zlib uncompress
void glmFileRead(void* data, unsigned long elementSize, unsigned long count, FILE* fp)
{
	if (count > 1)
	{
		int z_result;
		unsigned char* dataReadInCompressed;
		uint32_t sizeDataCompressed;
		unsigned long dataSize = elementSize * count;
		fread(&sizeDataCompressed, sizeof(uint32_t), 1, fp);
#ifdef GLMC_BIG_ENDIAN
		sizeDataCompressed = glmSwapByteOrder32(sizeDataCompressed);
#endif
		dataReadInCompressed = (unsigned char*)GLMC_MALLOC(sizeDataCompressed * sizeof(unsigned char));
		fread(dataReadInCompressed, sizeDataCompressed, 1, fp);

		z_result = mz_uncompress((unsigned char*)data, &dataSize, dataReadInCompressed, (unsigned long)sizeDataCompressed);
		GLMC_ASSERT((z_result == 0) && "Uncompress failed");
		GLMC_FREE(dataReadInCompressed);
	}
	else
	{
		fread(data, elementSize, count, fp);
	}
}

// handle 16-bit byte swapping for big endian machines
void glmFileReadUInt16(uint16_t* data, unsigned int count, FILE* fp)
{
#ifdef GLMC_BIG_ENDIAN
	unsigned int i;
#endif
	glmFileRead(data, sizeof(uint16_t), count, fp);
#ifdef GLMC_BIG_ENDIAN
	for (i = 0; i < count; ++i)
	{
		data[i] = glmSwapByteOrder16(data[i]);
	}
#endif
}

// handle 32-bit byte swapping for big endian machines
void glmFileReadUInt32(uint32_t* data, unsigned int count, FILE* fp)
{
#ifdef GLMC_BIG_ENDIAN
	unsigned int i;
#endif
	glmFileRead(data, sizeof(uint32_t), count, fp);
#ifdef GLMC_BIG_ENDIAN
	for (i = 0; i < count; ++i)
	{
		data[i] = glmSwapByteOrder32(data[i]);
	}
#endif
}

// handle 64-bit byte swapping for big endian machines
void glmFileReadUInt64(uint64_t* data, unsigned int count, FILE* fp)
{
#ifdef GLMC_BIG_ENDIAN
	unsigned int i;
#endif
	glmFileRead(data, sizeof(uint64_t), count, fp);
#ifdef GLMC_BIG_ENDIAN
	for (i = 0; i < count; ++i)
	{
		data[i] = glmSwapByteOrder64(data[i]);
	}
#endif
}

// handle zlib compression
void glmFileWrite(const void* data, unsigned long elementSize, unsigned long count, FILE* fp)
{
	if (count > 1)
	{
		int z_result;
		uint32_t sizeToWrite;
		unsigned long dataSize = elementSize * count;

		// destination buffer, must be at least (1.01X + 12) bytes as large as source.. we made it 1.1X + 12bytes
		unsigned long sizeDataCompressed = (unsigned long)((dataSize * 1.1) + 12); // current size of the destination buffer, when compress completes this var will be updated to contain the new size of the compressed data in bytes.
		unsigned char* dataCompressed = (unsigned char*)GLMC_MALLOC(sizeDataCompressed);

		z_result = mz_compress(dataCompressed, &sizeDataCompressed, (unsigned char*)data, dataSize);

		GLMC_ASSERT(z_result == 0 && "Compress failed");
		GLMC_ASSERT((sizeDataCompressed < UINT32_MAX) && "Simulation Cache can not compress data field bigger than 4,294,967,295 byte");
		sizeToWrite = (uint32_t)sizeDataCompressed;
#ifdef GLMC_BIG_ENDIAN
		sizeToWrite = glmSwapByteOrder32(sizeToWrite);
#endif
		fwrite(&sizeToWrite, sizeof(uint32_t), 1, fp);
		fwrite(dataCompressed, sizeDataCompressed, 1, fp);
		GLMC_FREE(dataCompressed);
	}
	else
	{
		fwrite(data, elementSize, count, fp);
	}
}

// handle 16-bit byte swapping for big endian machines
void glmFileWriteUInt16(const uint16_t* data, unsigned int count, FILE* fp)
{
#ifdef GLMC_BIG_ENDIAN
	unsigned int i;
	for (i = 0; i < count; ++i)
	{
		data[i] = glmSwapByteOrder16(data[i]);
	}
#endif
	glmFileWrite(data, sizeof(uint16_t), count, fp);
#ifdef GLMC_BIG_ENDIAN
	for (i = 0; i < count; ++i)
	{
		data[i] = glmSwapByteOrder16(data[i]);
	}
#endif
}

// handle 32-bit byte swapping for big endian machines
void glmFileWriteUInt32(const uint32_t* data, unsigned int count, FILE* fp)
{
#ifdef GLMC_BIG_ENDIAN
	unsigned int i;
	for (i = 0; i < count; ++i)
	{
		data[i] = glmSwapByteOrder32(data[i]);
	}
#endif
	glmFileWrite(data, sizeof(uint32_t), count, fp);
#ifdef GLMC_BIG_ENDIAN
	for (i = 0; i < count; ++i)
	{
		data[i] = glmSwapByteOrder32(data[i]);
	}
#endif
}

// handle 64-bit byte swapping for big endian machines
void glmFileWriteUInt64(const uint64_t* data, unsigned int count, FILE* fp)
{
#ifdef GLMC_BIG_ENDIAN
	unsigned int i;
	for (i = 0; i < count; ++i)
	{
		data[i] = glmSwapByteOrder64(data[i]);
	}
#endif
	glmFileWrite(data, sizeof(uint64_t), count, fp);
#ifdef GLMC_BIG_ENDIAN
	for (i = 0; i < count; ++i)
	{
		data[i] = glmSwapByteOrder64(data[i]);
	}
#endif
}

//////////////////////////////////////////////////////////////////////////////
//
// Simulation cache
//

// version 0x00 : original version 
// version 0x01 : Added Sns 4th value, set to 1 for backward compatibility
// version 0x02 : split ppattributes per type, generate cloth runtime data accesss helpers (vertices and indices offset)

#define GSC_VERSION 0x02
#define GSCS_MAGIC_NUMBER 0x65C5
#define GSCF_MAGIC_NUMBER 0x65CF
#define GSCL_MAGIC_NUMBER 0xB00F

const char golaemFrameExtension[] = "gscf"; // need to be declared lowercase for comparison
const char golaemSimulationExtension[] = "gscs"; // need to be declared lowercase for comparison
const char golaemTransformHistoryExtension[] = "gscl"; // need to be declared lowercase for comparison
const char golaemAssetAssociationExtension[] = "caa"; // need to be declared lowercase for comparison

void glmSetIdentityMatrix(float *matrix);

//----------------------------------------------------------------------------
const char* glmConvertSimulationCacheStatus(GlmSimulationCacheStatus status)
{
	switch (status)
	{
	case GSC_SUCCESS:
		return "Golaem simulation cache: success";
	case GSC_FILE_OPEN_FAILED:
		return "Golaem simulation cache: file open failed";
	case GSC_FILE_MAGIC_NUMBER_ERROR:
		return "Golaem simulation cache: file header does not begin by 0x65CF or 0x65CF, this is not a Golaem Simulation Cache";
	case GSC_FILE_VERSION_ERROR:
		return "Golaem simulation cache: incorrect version, could be a newer version of the Golaem Simulation Cache";
	case GSC_FILE_FORMAT_ERROR:
		return "Golaem simulation cache: incorrect format, could be a newer version of the Golaem Simulation Cache";
	case GSC_SIMULATION_FILE_DOES_NOT_MATCH:
		return "Golaem simulation cache: used Golaem Simulation Cache simulation file does not match this frame file";
	case GSC_SIMULATION_NO_FRAMES_FOUND:
		return "Golaem simulation cache: unable to find and open a valid Simulation Cache Frame";
	default:
		return "Golaem simulation cache: unkown error code";
	}
}

//----------------------------------------------------------------------------
void glmCreateSimulationData(
	GlmSimulationData** simulationData,
	uint32_t entityCount,
	uint16_t entityTypeCount,
	uint8_t ppFloatAttributeCount,
	uint8_t ppVectorAttributeCount)
{
	GlmSimulationData* data;

	// input validation before allocation
#ifndef NDEBUG
	GLMC_ASSERT((entityCount >= 1) && "entityCount limit exceeded, range = 1..4,294,967,295");
	GLMC_ASSERT((entityTypeCount >= 1) && "entityTypeCount limit exceeded, range = 1..65,535");
#endif

	// entity
	*simulationData = (GlmSimulationData*)GLMC_MALLOC(sizeof(GlmSimulationData));
	data = *simulationData;

	// entity
	data->_entityCount = entityCount;
	data->_entityIds = (int64_t*)GLMC_MALLOC(data->_entityCount * sizeof(int64_t));
	data->_entityTypes = (uint16_t*)GLMC_MALLOC(data->_entityCount * sizeof(uint16_t));
	data->_indexInEntityType = (uint32_t*)GLMC_MALLOC(data->_entityCount * sizeof(uint32_t));
	data->_scales = (float*)GLMC_MALLOC(data->_entityCount * sizeof(float));
	data->_entityRadius = (float*)GLMC_MALLOC(data->_entityCount * sizeof(float));
	data->_entityHeight = (float*)GLMC_MALLOC(data->_entityCount * sizeof(float));

	// entityType
	data->_entityTypeCount = entityTypeCount;
	data->_entityCountPerEntityType = (uint32_t*)GLMC_MALLOC(data->_entityTypeCount * sizeof(uint32_t));
	data->_boneCount = (uint16_t*)GLMC_MALLOC(data->_entityTypeCount * sizeof(uint16_t));
	data->_iBoneOffsetPerEntityType = (uint32_t*)GLMC_MALLOC(data->_entityTypeCount * sizeof(uint32_t));
	data->_maxBonesHierarchyLength = (float*)GLMC_MALLOC(data->_entityTypeCount * sizeof(float));
	data->_blindDataCount = (uint16_t*)GLMC_MALLOC(data->_entityTypeCount * sizeof(uint16_t));
	data->_iBlindDataOffsetPerEntityType = (uint32_t*)GLMC_MALLOC(data->_entityTypeCount * sizeof(uint32_t));
	data->_hasGeoBehavior = (uint8_t*)GLMC_MALLOC(data->_entityTypeCount * sizeof(uint8_t));
	data->_iGeoBehaviorOffsetPerEntityType = (uint32_t*)GLMC_MALLOC(data->_entityTypeCount * sizeof(uint32_t));
	data->_snsCountPerEntityType = (uint16_t*)GLMC_MALLOC(data->_entityTypeCount * sizeof(uint16_t));
	data->_snsOffsetPerEntityType = (uint32_t*)GLMC_MALLOC(data->_entityTypeCount * sizeof(uint32_t));

	// ppAttribute
	data->_backwardCompatPPAttributeTypes = NULL;
	data->_ppFloatAttributeCount = ppFloatAttributeCount;
	data->_ppVectorAttributeCount = ppVectorAttributeCount;
	data->_ppFloatAttributeNames = (char(*)[GSC_PP_MAX_NAME_LENGTH])GLMC_MALLOC(data->_ppFloatAttributeCount * sizeof(char[GSC_PP_MAX_NAME_LENGTH]));
	data->_ppVectorAttributeNames = (char(*)[GSC_PP_MAX_NAME_LENGTH])GLMC_MALLOC(data->_ppVectorAttributeCount * sizeof(char[GSC_PP_MAX_NAME_LENGTH]));
	//data->_ppAttributeTypes = (uint8_t*)GLMC_MALLOC(data->_ppAttributeCount * sizeof(uint8_t));

	glmSetIdentityMatrix(data->_proxyMatrix);
	glmSetIdentityMatrix(data->_proxyMatrixInverse);
}

//----------------------------------------------------------------------------
uint64_t glmComputeSimulationDataSize(const GlmSimulationData* data)
{
	uint64_t totalSize = sizeof(GlmSimulationData);

	// entity
	totalSize += data->_entityCount * sizeof(int64_t);	// data->_entityIds
	totalSize += data->_entityCount * sizeof(uint16_t); // data->_entityTypes
	totalSize += data->_entityCount * sizeof(uint32_t); // data->_indexInEntityType
	totalSize += data->_entityCount * sizeof(float);	// data->_scales
	totalSize += data->_entityCount * sizeof(float);	// data->_entityRadius
	totalSize += data->_entityCount * sizeof(float);	// data->_entityHeight

	// entityType
	totalSize += data->_entityTypeCount * sizeof(uint32_t); // data->_entityCountPerEntityType
	totalSize += data->_entityTypeCount * sizeof(uint16_t); // data->_boneCount
	totalSize += data->_entityTypeCount * sizeof(uint32_t); // data->_iBoneOffsetPerEntityType
	totalSize += data->_entityTypeCount * sizeof(float);	// data->_maxBonesHierarchyLength
	totalSize += data->_entityTypeCount * sizeof(uint16_t); // data->_blendShapeCount
	totalSize += data->_entityTypeCount * sizeof(uint32_t); // data->_iBlendShapeOffsetPerEntityType
	totalSize += data->_entityTypeCount * sizeof(uint8_t);	// data->_hasGeoBehavior
	totalSize += data->_entityTypeCount * sizeof(uint32_t); // data->_iGeoBehaviorOffsetPerEntityType
	totalSize += data->_entityTypeCount * sizeof(uint16_t); // data->_snsCountPerEntityType
	totalSize += data->_entityTypeCount * sizeof(uint32_t); // data->_snsOffsetPerEntityType

	// ppAttribute
	totalSize += data->_ppFloatAttributeCount * GSC_PP_MAX_NAME_LENGTH * sizeof(char);
	totalSize += data->_ppVectorAttributeCount * GSC_PP_MAX_NAME_LENGTH * sizeof(char);

	return totalSize;
}

//----------------------------------------------------------------------------
GlmSimulationCacheStatus glmCreateAndReadSimulationData(GlmSimulationData** simulationData, const char* file)
{
	uint16_t magicNumber;
	uint8_t version;
	uint32_t entityCount;
	uint16_t entityTypeCount;
	uint8_t ppFloatAttributeCount;
	uint8_t ppVectorAttributeCount;
	uint32_t contentHashKey;
	GlmSimulationData* data;
	int ppAttributeCount = 0;

#ifdef _MSC_VER				
	FILE* fp;
	errno_t err;
	err = fopen_s(&fp, file, "rb");
	if (err != 0) return GSC_FILE_OPEN_FAILED;
#else
	FILE* fp = fopen(file, "rb");
	if (fp == NULL) return GSC_FILE_OPEN_FAILED;
#endif

	// header
	glmFileReadUInt16(&magicNumber, 1, fp);
	if (magicNumber != GSCS_MAGIC_NUMBER)
	{
		fclose(fp);
		return GSC_FILE_MAGIC_NUMBER_ERROR;
	}
	glmFileRead(&version, sizeof(uint8_t), 1, fp);
	if (version > GSC_VERSION)
	{
		fclose(fp);
		return GSC_FILE_VERSION_ERROR;
	}

	
	glmFileReadUInt32(&contentHashKey, 1, fp);
	glmFileReadUInt32(&entityCount, 1, fp);
	glmFileReadUInt16(&entityTypeCount, 1, fp);

	if (version < 0x02)
	{
		// float attribute temporary stores all attributes count until below
		glmFileRead(&ppAttributeCount, sizeof(uint8_t), 1, fp);
		ppVectorAttributeCount = ppFloatAttributeCount = 0; // allocate no space for now, will be done in backward compatibility case below
	}
	else
	{
	glmFileRead(&ppFloatAttributeCount, sizeof(uint8_t), 1, fp);
	glmFileRead(&ppVectorAttributeCount, sizeof(uint8_t), 1, fp);
	}

	// create
	glmCreateSimulationData(simulationData, entityCount, entityTypeCount, ppFloatAttributeCount, ppVectorAttributeCount);
	data = *simulationData;

	data->_contentHashKey = contentHashKey;
	data->_version = version;

	// entity
	glmFileRead(data->_entityIds, sizeof(int64_t), data->_entityCount, fp);
	glmFileReadUInt16(data->_entityTypes, data->_entityCount, fp);
	glmFileReadUInt32(data->_indexInEntityType, data->_entityCount, fp);
	glmFileRead(data->_scales, sizeof(float), data->_entityCount, fp);
	glmFileRead(data->_entityRadius, sizeof(float), data->_entityCount, fp);
	glmFileRead(data->_entityHeight, sizeof(float), data->_entityCount, fp);

	// entityType
	glmFileReadUInt32(data->_entityCountPerEntityType, data->_entityTypeCount, fp);
	glmFileReadUInt16(data->_boneCount, data->_entityTypeCount, fp);
	glmFileReadUInt32(data->_iBoneOffsetPerEntityType, data->_entityTypeCount, fp);
	glmFileRead(data->_maxBonesHierarchyLength, sizeof(float), data->_entityTypeCount, fp);
	glmFileReadUInt16(data->_blindDataCount, data->_entityTypeCount, fp);
	glmFileReadUInt32(data->_iBlindDataOffsetPerEntityType, data->_entityTypeCount, fp);
	glmFileRead(data->_hasGeoBehavior, sizeof(uint8_t), data->_entityTypeCount, fp);
	glmFileReadUInt32(data->_iGeoBehaviorOffsetPerEntityType, data->_entityTypeCount, fp);
	glmFileReadUInt16(data->_snsCountPerEntityType, data->_entityTypeCount, fp);
	glmFileReadUInt32(data->_snsOffsetPerEntityType, data->_entityTypeCount, fp);

	// ppAttribute
	if (version < 0x02)
	{
		// read old names & types
		int iPPAttribute = 0;
		char(*ppAttributeNames)[GSC_PP_MAX_NAME_LENGTH] = (char(*)[GSC_PP_MAX_NAME_LENGTH])GLMC_MALLOC(ppAttributeCount * sizeof(char[GSC_PP_MAX_NAME_LENGTH]));
		data->_backwardCompatPPAttributeTypes = (uint8_t*)GLMC_MALLOC(ppAttributeCount * sizeof(uint8_t));
				
		glmFileRead(ppAttributeNames, sizeof(char), ppAttributeCount * GSC_PP_MAX_NAME_LENGTH, fp);
		glmFileRead(data->_backwardCompatPPAttributeTypes, sizeof(uint8_t), ppAttributeCount, fp);
		
		// set new data with old one :
		for (; iPPAttribute < ppAttributeCount; iPPAttribute++)
		{
			switch (data->_backwardCompatPPAttributeTypes[iPPAttribute])
			{
			case GSC_PP_FLOAT:
				data->_ppFloatAttributeCount++;
				break;
			case GSC_PP_VECTOR:
				data->_ppVectorAttributeCount++;
				break;
			}
		}

		// allocate new names :
		data->_ppFloatAttributeNames = (char(*)[GSC_PP_MAX_NAME_LENGTH])GLMC_MALLOC(data->_ppFloatAttributeCount * sizeof(char[GSC_PP_MAX_NAME_LENGTH]));
		data->_ppVectorAttributeNames = (char(*)[GSC_PP_MAX_NAME_LENGTH])GLMC_MALLOC(data->_ppVectorAttributeCount * sizeof(char[GSC_PP_MAX_NAME_LENGTH]));

		// set names :
		data->_ppFloatAttributeCount = 0;
		data->_ppVectorAttributeCount = 0;
		for (iPPAttribute = 0; iPPAttribute < ppAttributeCount; iPPAttribute++)
		{
			switch (data->_backwardCompatPPAttributeTypes[iPPAttribute])
			{
			case GSC_PP_FLOAT:
				memcpy(data->_ppFloatAttributeNames[data->_ppFloatAttributeCount], ppAttributeNames[iPPAttribute], sizeof(char[GSC_PP_MAX_NAME_LENGTH]));
				data->_ppFloatAttributeCount++;
				break;
			case GSC_PP_VECTOR:
				memcpy(data->_ppVectorAttributeNames[data->_ppVectorAttributeCount], ppAttributeNames[iPPAttribute], sizeof(char[GSC_PP_MAX_NAME_LENGTH]));
				data->_ppVectorAttributeCount++;
				break;
			}
		}

		// free temp data
		GLMC_FREE(ppAttributeNames);
	}
	else
	{
	glmFileRead(data->_ppFloatAttributeNames, sizeof(char), data->_ppFloatAttributeCount * GSC_PP_MAX_NAME_LENGTH, fp);
	glmFileRead(data->_ppVectorAttributeNames, sizeof(char), data->_ppVectorAttributeCount * GSC_PP_MAX_NAME_LENGTH, fp);
	}

	fclose(fp);

	return GSC_SUCCESS;
}

//----------------------------------------------------------------------------
GlmSimulationCacheStatus glmWriteSimulationData(const char* file, const GlmSimulationData* data)
{
	uint16_t magicNumber;
	uint8_t version;
	uint32_t contentHashValue;
	int i;

#ifdef _MSC_VER				
	FILE* fp;
	errno_t err;
	err = fopen_s(&fp, file, "wb");
	if (err != 0) return GSC_FILE_OPEN_FAILED;
#else
	FILE* fp = fopen(file, "wb");
	if (fp == NULL) return GSC_FILE_OPEN_FAILED;
#endif

	// header
	magicNumber = GSCS_MAGIC_NUMBER;
	glmFileWriteUInt16(&magicNumber, 1, fp);
	version = GSC_VERSION;
	glmFileWrite(&version, sizeof(uint8_t), 1, fp);

	// compute hashKey :
	glmStartHash(&contentHashValue);
	glmCumulativeHash32(data->_entityCount, &contentHashValue);
	glmCumulativeHash32(data->_entityTypeCount, &contentHashValue);
	for (i = 0; i < data->_entityTypeCount; i++)
	{
		glmCumulativeHash32(data->_entityCountPerEntityType[i], &contentHashValue);
		glmCumulativeHash16(data->_boneCount[i], &contentHashValue);
		glmCumulativeHash16(data->_blindDataCount[i], &contentHashValue);
		glmCumulativeHash16(data->_snsCountPerEntityType[i], &contentHashValue);
	}
	glmCumulativeHash8(data->_ppFloatAttributeCount, &contentHashValue);
	glmCumulativeHash8(data->_ppVectorAttributeCount, &contentHashValue);

	// overwrite const pointer to write back contentHashKey 
	((GlmSimulationData*)data)->_contentHashKey = contentHashValue;

	// finally write hash key
	glmFileWriteUInt32(&contentHashValue, 1, fp); // cannot set value on data, const , but write it

	glmFileWriteUInt32((uint32_t*)&data->_entityCount, 1, fp);
	glmFileWriteUInt16((uint16_t*)&data->_entityTypeCount, 1, fp);
	glmFileWrite(&data->_ppFloatAttributeCount, sizeof(uint8_t), 1, fp);
	glmFileWrite(&data->_ppVectorAttributeCount, sizeof(uint8_t), 1, fp);

	// entity
	glmFileWrite(data->_entityIds, sizeof(int64_t), data->_entityCount, fp);
	glmFileWriteUInt16(data->_entityTypes, data->_entityCount, fp);
	glmFileWriteUInt32(data->_indexInEntityType, data->_entityCount, fp);
	glmFileWrite(data->_scales, sizeof(float), data->_entityCount, fp);
	glmFileWrite(data->_entityRadius, sizeof(float), data->_entityCount, fp);
	glmFileWrite(data->_entityHeight, sizeof(float), data->_entityCount, fp);

	// entityType
	glmFileWriteUInt32(data->_entityCountPerEntityType, data->_entityTypeCount, fp);
	glmFileWriteUInt16(data->_boneCount, data->_entityTypeCount, fp);
	glmFileWriteUInt32(data->_iBoneOffsetPerEntityType, data->_entityTypeCount, fp);
	glmFileWrite(data->_maxBonesHierarchyLength, sizeof(float), data->_entityTypeCount, fp);
	glmFileWriteUInt16(data->_blindDataCount, data->_entityTypeCount, fp);
	glmFileWriteUInt32(data->_iBlindDataOffsetPerEntityType, data->_entityTypeCount, fp);
	glmFileWrite(data->_hasGeoBehavior, sizeof(uint8_t), data->_entityTypeCount, fp);
	glmFileWriteUInt32(data->_iGeoBehaviorOffsetPerEntityType, data->_entityTypeCount, fp);
	glmFileWriteUInt16(data->_snsCountPerEntityType, data->_entityTypeCount, fp);
	glmFileWriteUInt32(data->_snsOffsetPerEntityType, data->_entityTypeCount, fp);

	// ppAttribute
	glmFileWrite(data->_ppFloatAttributeNames, sizeof(char), data->_ppFloatAttributeCount * GSC_PP_MAX_NAME_LENGTH, fp);
	glmFileWrite(data->_ppVectorAttributeNames, sizeof(char), data->_ppVectorAttributeCount * GSC_PP_MAX_NAME_LENGTH, fp);
	
	fclose(fp);

	return GSC_SUCCESS;
}

//----------------------------------------------------------------------------
void glmDestroySimulationData(GlmSimulationData** simulationData)
{
	GlmSimulationData* data = *simulationData;
	GLMC_ASSERT((data != NULL) && "Simulation data must be created before being destroyed");
	GLMC_FREE(data->_backwardCompatPPAttributeTypes);
	GLMC_FREE(data->_ppFloatAttributeNames);
	GLMC_FREE(data->_ppVectorAttributeNames);
	GLMC_FREE(data->_snsOffsetPerEntityType);
	GLMC_FREE(data->_snsCountPerEntityType);
	GLMC_FREE(data->_iGeoBehaviorOffsetPerEntityType);
	GLMC_FREE(data->_hasGeoBehavior);
	GLMC_FREE(data->_iBlindDataOffsetPerEntityType);
	GLMC_FREE(data->_blindDataCount);
	GLMC_FREE(data->_maxBonesHierarchyLength);
	GLMC_FREE(data->_iBoneOffsetPerEntityType);
	GLMC_FREE(data->_boneCount);
	GLMC_FREE(data->_entityCountPerEntityType);
	GLMC_FREE(data->_entityRadius);
	GLMC_FREE(data->_entityHeight);
	GLMC_FREE(data->_scales);
	GLMC_FREE(data->_indexInEntityType);
	GLMC_FREE(data->_entityTypes);
	GLMC_FREE(data->_entityIds);
	GLMC_FREE(data);

	*simulationData = NULL;
}

//----------------------------------------------------------------------------
void glmCreateFrameData(GlmFrameData** frameData, const GlmSimulationData* simulationData)
{
	unsigned int i;
	unsigned int totalBoneCount = 0;
	unsigned int totalSnSCount = 0;
	unsigned int totalBlindDataCount = 0;
	unsigned int totalGeoBehaviorCount = 0;
	GlmFrameData* data;

	// input validation before allocation
#ifndef NDEBUG
	GLMC_ASSERT((simulationData->_entityCount >= 1) && "entityTypeCount limit exceeded, range = 1..65,535");
	GLMC_ASSERT((simulationData->_entityTypeCount >= 1) && "entityTypeCount limit exceeded, range = 1..65,535");
	for (i = 0; i < simulationData->_entityTypeCount; ++i)
	{
		GLMC_ASSERT(((simulationData->_boneCount[i] >= 1) || (simulationData->_entityCountPerEntityType[i] == 0)) && "boneCount limit exceeded, range = 1..65,535");
		GLMC_ASSERT(((simulationData->_hasGeoBehavior[i] == 0) || (simulationData->_hasGeoBehavior[i] == 1)) && "hasGeoBehavior value must be 0 or 1");
	}
#endif

	// for compatibility check 
	*frameData = (GlmFrameData*)GLMC_MALLOC(sizeof(GlmFrameData));
	data = *frameData;

	// for compatibility check 
	data->_simulationContentHashKey = simulationData->_contentHashKey;

	// for cache format consistancy (do not use quantization on stretched scenes)
	data->_hasSquashAndStretch = 0;

	// entityType
	for (i = 0; i < simulationData->_entityTypeCount; ++i)
	{
		totalBoneCount += simulationData->_boneCount[i] * simulationData->_entityCountPerEntityType[i];
		totalSnSCount += simulationData->_snsCountPerEntityType[i] * simulationData->_entityCountPerEntityType[i];
		totalBlindDataCount += simulationData->_blindDataCount[i] * simulationData->_entityCountPerEntityType[i];
		if (simulationData->_hasGeoBehavior[i])
		{
			totalGeoBehaviorCount += simulationData->_entityCountPerEntityType[i];
		}
	}

	data->_bonePositions = (float(*)[3])GLMC_MALLOC(totalBoneCount * sizeof(float[3]));
	data->_boneOrientations = (float(*)[4])GLMC_MALLOC(totalBoneCount * sizeof(float[4]));
	data->_snsValues = (float(*)[4])GLMC_MALLOC(totalSnSCount * sizeof(float[4]));
	data->_blindData = (float*)GLMC_MALLOC(totalBlindDataCount * sizeof(float));
	if (totalGeoBehaviorCount > 0)
	{
		data->_geoBehaviorGeometryIds = (uint16_t*)GLMC_MALLOC(totalGeoBehaviorCount * sizeof(uint16_t));
		data->_geoBehaviorAnimFrameInfo = (float(*)[3])GLMC_MALLOC(totalGeoBehaviorCount * sizeof(float[3]));
		data->_geoBehaviorBlendModes = (uint8_t*)GLMC_MALLOC(totalGeoBehaviorCount * sizeof(uint8_t));
	}
	else
	{
		data->_geoBehaviorGeometryIds = NULL;
		data->_geoBehaviorAnimFrameInfo = NULL;
		data->_geoBehaviorBlendModes = NULL;
	}

	// default is not using cloth, arrays are NULL
	data->_clothEntityCount = 0; // other stuff written / read only if != 0
	data->_clothTotalMeshIndices = 0;
	data->_clothTotalVertices = 0;
	data->_clothAllocatedEntities = 0;
	data->_clothAllocatedIndices = 0;
	data->_clothAllocatedVertices = 0;
	data->_entityClothIndex = NULL;
	data->_clothEntityFirstAssetMeshIndex = NULL;
	data->_clothEntityFirstMeshVertex = NULL;
	data->_clothEntityMeshCount = NULL;
	data->_clothEntityQuantizationReference = NULL;
	data->_clothEntityQuantizationMaxExtent = NULL;
	data->_clothMeshVertexCount = NULL;
	data->_clothMeshIndicesInCharAssets = NULL;
	data->_clothVertices = NULL;

	// ppAttribute
	data->_ppFloatAttributeData = NULL;
	data->_ppVectorAttributeData = NULL;
	if (simulationData->_ppFloatAttributeCount > 0)
	{
		data->_ppFloatAttributeData = (float**)GLMC_MALLOC(simulationData->_ppFloatAttributeCount * sizeof(float*));
		for (i = 0; i < simulationData->_ppFloatAttributeCount; ++i)
		{
			data->_ppFloatAttributeData[i] = (float*)GLMC_MALLOC(simulationData->_entityCount * sizeof(float));
		}
	}
	if (simulationData->_ppVectorAttributeCount > 0)
	{
		data->_ppVectorAttributeData = (float(**)[3])GLMC_MALLOC(simulationData->_ppVectorAttributeCount * sizeof(float*));
		for (i = 0; i < simulationData->_ppVectorAttributeCount; ++i)
		{
			data->_ppVectorAttributeData[i] = (float(*)[3])GLMC_MALLOC(simulationData->_entityCount * sizeof(float[3]));
		}
	}
}

//----------------------------------------------------------------------------
void glmFileReadOrientations(float(*bonesOrientations)[4], unsigned int totalBoneCount, FILE* fp, GlmSimulationCacheFormat format)
{
	switch (format)
	{
	case GSC_O32_P48:
	case GSC_O32_P96:
	{
		unsigned int i;
		uint32_t* compressedBoneOrientations = (uint32_t*)GLMC_MALLOC(totalBoneCount * sizeof(uint32_t));
		glmFileReadUInt32(compressedBoneOrientations, totalBoneCount, fp);
		for (i = 0; i < totalBoneCount; ++i)
		{
			glmUncompressQuaternion32(bonesOrientations[i], compressedBoneOrientations[i]);
		}
		GLMC_FREE(compressedBoneOrientations);
	}
	break;
	case GSC_O64_P48:
	case GSC_O64_P96:
	{
		unsigned int i;
		uint64_t* compressedBoneOrientations = (uint64_t*)GLMC_MALLOC(totalBoneCount * sizeof(uint64_t));
		glmFileReadUInt64(compressedBoneOrientations, totalBoneCount, fp);
		for (i = 0; i < totalBoneCount; ++i)
		{
			glmUncompressQuaternion64(bonesOrientations[i], compressedBoneOrientations[i]);
		}
		GLMC_FREE(compressedBoneOrientations);
	}
	break;
	default:
		glmFileRead(bonesOrientations, sizeof(float), totalBoneCount * 4, fp);
	}
}

//----------------------------------------------------------------------------
void glmFileReadPositions(float(*bonesPositions)[3], unsigned int totalBoneCount, const GlmSimulationData* data, FILE* fp, GlmSimulationCacheFormat format)
{
	switch (format)
	{
	case GSC_O32_P48:
	case GSC_O64_P48:
	case GSC_O128_P48:
	{
		unsigned int iEntityType;
		unsigned int iRootBone = 0;
		float localPosition[3];
		float(*rootBonePositions)[3];
		uint16_t(*compressedBonePositions)[3];

		uint32_t validEntityCount = 0;
		for (iEntityType = 0; iEntityType < data->_entityTypeCount; ++iEntityType)
		{
			validEntityCount += data->_entityCountPerEntityType[iEntityType];
		}

		rootBonePositions = (float(*)[3])GLMC_MALLOC(validEntityCount * sizeof(float[3]));
		compressedBonePositions = (uint16_t(*)[3])GLMC_MALLOC((totalBoneCount - validEntityCount) * sizeof(uint16_t[3]));
		glmFileRead(rootBonePositions, sizeof(float), validEntityCount * 3, fp);
		glmFileReadUInt16(compressedBonePositions[0], (totalBoneCount - validEntityCount) * 3, fp);
		for (iEntityType = 0; iEntityType < data->_entityTypeCount; ++iEntityType)
		{
			unsigned int iEntity;
			for (iEntity = 0; iEntity < data->_entityCountPerEntityType[iEntityType]; ++iEntity)
			{
				unsigned int iBone;
				unsigned int iBoneOffset = data->_iBoneOffsetPerEntityType[iEntityType] + iEntity * data->_boneCount[iEntityType];
				memcpy(bonesPositions[iBoneOffset], rootBonePositions[iRootBone], sizeof(float[3]));
				++iRootBone;
				for (iBone = 1; iBone < data->_boneCount[iEntityType]; ++iBone)
				{
					unsigned int i = iBone + iBoneOffset;
					glmUncompressPosition48(localPosition, compressedBonePositions[i - iRootBone], data->_maxBonesHierarchyLength[iEntityType]);
					bonesPositions[i][0] = localPosition[0] + rootBonePositions[iRootBone - 1][0];
					bonesPositions[i][1] = localPosition[1] + rootBonePositions[iRootBone - 1][1];
					bonesPositions[i][2] = localPosition[2] + rootBonePositions[iRootBone - 1][2];
				}
			}
		}
		GLMC_FREE(compressedBonePositions);
		GLMC_FREE(rootBonePositions);
	}
	break;
	default:
		glmFileRead(bonesPositions, sizeof(float), totalBoneCount * 3, fp);
	}
}

//----------------------------------------------------------------------------
void glmFileReadClothVertices(GlmFrameData* frameData, FILE* fp, GlmSimulationCacheFormat format)
{
	switch (format)
	{
	case GSC_O32_P48:
	case GSC_O64_P48:
	case GSC_O128_P48:
	{
		unsigned int iClothEntityMesh = 0;
		unsigned int iClothVertex = 0;
		float * vertex = NULL;

		int iAbsoluteMeshVertex = 0;
		int iAbsoluteClothMesh = 0;
		unsigned int iClothEntity = 0;
		uint16_t(*compressedClothVertices)[3] = (uint16_t(*)[3])GLMC_MALLOC(frameData->_clothTotalVertices * sizeof(uint16_t[3]));

		glmFileReadUInt16(compressedClothVertices[0], frameData->_clothTotalVertices * 3, fp);

		// iteration per cloth entity:
		for (iClothEntity = 0; iClothEntity < frameData->_clothEntityCount; iClothEntity++)
		{
			// cloth max extent and reference must be read priori to calling this function
			float *referencePosition = frameData->_clothEntityQuantizationReference[iClothEntity];
			float maxExtent = frameData->_clothEntityQuantizationMaxExtent[iClothEntity];

			for (iClothEntityMesh = 0; iClothEntityMesh < frameData->_clothEntityMeshCount[iClothEntity]; iClothEntityMesh++)
			{
				for (iClothVertex = 0; iClothVertex < frameData->_clothMeshVertexCount[iAbsoluteClothMesh]; iClothVertex++)
				{
					vertex = frameData->_clothVertices[iAbsoluteMeshVertex];

					glmUncompressPosition48(vertex, compressedClothVertices[iAbsoluteMeshVertex], maxExtent);

					vertex[0] += referencePosition[0];
					vertex[1] += referencePosition[1];
					vertex[2] += referencePosition[2];

					iAbsoluteMeshVertex++;
				}

				iAbsoluteClothMesh++;
			}
		}

		GLMC_FREE(compressedClothVertices);
	}
	break;
	default:
		glmFileRead(frameData->_clothVertices, sizeof(float), frameData->_clothTotalVertices * 3, fp);
	}
}

//----------------------------------------------------------------------------
void glmCreateClothData(const GlmSimulationData* simuData, GlmFrameData* data, unsigned int clothEntityCount, unsigned int clothIndices, unsigned int clothVertices)
{
	if (clothEntityCount > 0)
	{
		if (data->_entityClothIndex == NULL)
		{
			data->_entityClothIndex = (int32_t*)GLMC_MALLOC(simuData->_entityCount * sizeof(int32_t));			
		}

		if (clothEntityCount > data->_clothAllocatedEntities)
		{
			GLMC_FREE(data->_clothEntityFirstAssetMeshIndex);
			GLMC_FREE(data->_clothEntityMeshCount);
			GLMC_FREE(data->_clothEntityQuantizationReference);
			GLMC_FREE(data->_clothEntityQuantizationMaxExtent);

			data->_clothEntityFirstAssetMeshIndex = (uint32_t*)GLMC_MALLOC(clothEntityCount * sizeof(uint32_t));
			data->_clothEntityFirstMeshVertex = (uint32_t*)GLMC_MALLOC(clothEntityCount * sizeof(uint32_t));
			data->_clothEntityMeshCount = (uint32_t*)GLMC_MALLOC(clothEntityCount * sizeof(uint32_t));
			data->_clothEntityQuantizationReference = (float(*)[3])GLMC_MALLOC(clothEntityCount * sizeof(float[3]));
			data->_clothEntityQuantizationMaxExtent = (float*)GLMC_MALLOC(clothEntityCount * sizeof(float));

			data->_clothAllocatedEntities = clothEntityCount;
		}

		if (clothIndices > data->_clothAllocatedIndices)
		{
			GLMC_FREE(data->_clothMeshIndicesInCharAssets);
			GLMC_FREE(data->_clothMeshVertexCount);
			//GLMC_FREE(data->_clothMeshVertexOffsetPerClothIndex);

			data->_clothMeshIndicesInCharAssets = (uint32_t*)GLMC_MALLOC(clothIndices * sizeof(uint32_t));
			data->_clothMeshVertexCount = (uint32_t*)GLMC_MALLOC(clothIndices * sizeof(uint32_t));
			//data->_clothMeshVertexOffsetPerClothIndex = (uint32_t*)GLMC_MALLOC(clothIndices * sizeof(uint32_t));
			
			data->_clothAllocatedIndices = clothIndices;
		}
		if (clothVertices > data->_clothAllocatedVertices)
		{
			GLMC_FREE(data->_clothVertices);
			data->_clothVertices = (float(*)[3])GLMC_MALLOC(clothVertices * sizeof(float[3]));
			data->_clothAllocatedVertices = clothVertices;
		}

		data->_clothEntityCount = clothEntityCount;
		data->_clothTotalMeshIndices = clothIndices;
		data->_clothTotalVertices = clothVertices;
	}
}

//----------------------------------------------------------------------------
uint64_t glmComputeFrameDataSize(const GlmFrameData* frameData, const GlmSimulationData* simulationData)
{
	unsigned int i;
	unsigned int totalBoneCount = 0;
	unsigned int totalSnSCount = 0;
	unsigned int totalBlindDataCount = 0;
	unsigned int totalGeoBehaviorCount = 0;

	// allocate frame data
	uint64_t totalSize = sizeof(GlmFrameData);

	// entityType
	for (i = 0; i < simulationData->_entityTypeCount; ++i)
	{
		totalBoneCount += simulationData->_boneCount[i] * simulationData->_entityCountPerEntityType[i];
		totalSnSCount += simulationData->_snsCountPerEntityType[i] * simulationData->_entityCountPerEntityType[i];
		totalBlindDataCount += simulationData->_blindDataCount[i] * simulationData->_entityCountPerEntityType[i];
		if (simulationData->_hasGeoBehavior[i])
		{
			totalGeoBehaviorCount += simulationData->_entityCountPerEntityType[i];
		}
	}

	totalSize += totalBoneCount * 3 * sizeof(float);
	totalSize += totalBoneCount * 4 * sizeof(float);
	totalSize += totalSnSCount * 4 * sizeof(float);
	totalSize += totalBlindDataCount * sizeof(float);
	if (totalGeoBehaviorCount > 0)
	{
		totalSize += totalGeoBehaviorCount * sizeof(uint16_t);
		totalSize += totalGeoBehaviorCount * 3 * sizeof(float);
		totalSize += totalGeoBehaviorCount * sizeof(uint8_t);
	}
	if (frameData->_clothEntityCount)
	{
		totalSize += simulationData->_entityCount * sizeof(uint8_t); // data->_entityUseCloth
		totalSize += frameData->_clothEntityCount * sizeof(uint32_t); // data->_clothMeshIndexCount
		totalSize += frameData->_clothEntityCount * sizeof(float) * 3; // data->_clothReference
		totalSize += frameData->_clothEntityCount * sizeof(float); // data->_clothMaxExtent
		totalSize += frameData->_clothTotalVertices * sizeof(float) * 3; // data->_clothVertices
	}

	return totalSize;
}

//----------------------------------------------------------------------------
GlmSimulationCacheStatus glmReadFrameData(GlmFrameData* data, const GlmSimulationData* simulationData, const char* file)
{
	uint16_t magicNumber;
	uint8_t version;
	uint8_t format;
	unsigned int totalBoneCount = 0;
	unsigned int totalSnSCount = 0;
	unsigned int totalBlindDataCount = 0;
	unsigned int totalGeoBehaviorCount = 0;
	unsigned int i;

#ifdef _MSC_VER				
	FILE* fp;
	errno_t err;
	err = fopen_s(&fp, file, "rb");
	if (err != 0) return GSC_FILE_OPEN_FAILED;
#else
	FILE* fp = fopen(file, "rb");
	if (fp == NULL) return GSC_FILE_OPEN_FAILED;
#endif

	// header
	glmFileReadUInt16(&magicNumber, 1, fp);
	if (magicNumber != GSCF_MAGIC_NUMBER)
	{
		fclose(fp);
		return GSC_FILE_MAGIC_NUMBER_ERROR;
	}
	glmFileRead(&version, sizeof(uint8_t), 1, fp);
	if (version > GSC_VERSION)
	{
		fclose(fp);
		return GSC_FILE_VERSION_ERROR;
	}

	glmFileRead(&format, sizeof(uint8_t), 1, fp);
	data->_cacheFormat = format;

	if ((data->_cacheFormat <= GSC_O128_P96) || (data->_cacheFormat > GSC_O32_P48)) 	
	{
		fclose(fp);
		return GSC_FILE_FORMAT_ERROR;
	}

	glmFileReadUInt32(&data->_simulationContentHashKey, 1, fp); // read simulation content hash key, check that it matches the simulation :

	if (data->_simulationContentHashKey != simulationData->_contentHashKey)
	{
		fclose(fp);
		return GSC_SIMULATION_FILE_DOES_NOT_MATCH;
	}

	// entityType
	for (i = 0; i < simulationData->_entityTypeCount; ++i)
	{
		totalBoneCount += simulationData->_boneCount[i] * simulationData->_entityCountPerEntityType[i];
		totalSnSCount += simulationData->_snsCountPerEntityType[i] * simulationData->_entityCountPerEntityType[i];
		totalBlindDataCount += simulationData->_blindDataCount[i] * simulationData->_entityCountPerEntityType[i];
		if (simulationData->_hasGeoBehavior[i])
		{
			totalGeoBehaviorCount += simulationData->_entityCountPerEntityType[i];
		}
	}
	glmFileReadPositions(data->_bonePositions, totalBoneCount, simulationData, fp, (GlmSimulationCacheFormat)data->_cacheFormat);
	glmFileReadOrientations(data->_boneOrientations, totalBoneCount, fp, (GlmSimulationCacheFormat)data->_cacheFormat);
	if (simulationData->_version < 0x01)
	{
		int iBone;
		float(*snsValue)[4] = data->_snsValues;
		float(*snsValueSource)[3] = (float(*)[3])data->_snsValues;

		glmFileRead(data->_snsValues, sizeof(float), totalSnSCount * 4, fp);

		snsValue += totalSnSCount - 1;
		snsValueSource += totalSnSCount - 1;
		for (iBone = totalSnSCount-1; iBone >= 0; iBone--)
		{
			(*snsValue)[0] = (*snsValueSource)[0];
			(*snsValue)[1] = (*snsValueSource)[1];
			(*snsValue)[2] = (*snsValueSource)[2];
			(*snsValue)[3] = 1.f;
			snsValue--;
			snsValueSource--;
		}
	}
	else
		glmFileRead(data->_snsValues, sizeof(float), totalSnSCount * 4, fp);

	glmFileRead(data->_blindData, sizeof(float), totalBlindDataCount, fp);
	if (totalGeoBehaviorCount > 0)
	{
		glmFileReadUInt16(data->_geoBehaviorGeometryIds, totalGeoBehaviorCount, fp);
		glmFileRead(data->_geoBehaviorAnimFrameInfo[0], sizeof(float), totalGeoBehaviorCount * 3, fp);
		glmFileRead(data->_geoBehaviorBlendModes, sizeof(uint8_t), totalGeoBehaviorCount, fp);
	}
	else
	{
		data->_geoBehaviorGeometryIds = NULL;
		data->_geoBehaviorAnimFrameInfo = NULL;
		data->_geoBehaviorBlendModes = NULL;
	}

	// default is not using cloth, arrays are NULL
	glmFileReadUInt32(&data->_clothEntityCount, 1, fp);
	if (data->_clothEntityCount != 0)
	{
		glmFileReadUInt32(&data->_clothTotalMeshIndices, 1, fp);
		glmFileReadUInt32(&data->_clothTotalVertices, 1, fp);

		if (data->_clothTotalMeshIndices != 0)
		{
			int iClothEntityIndex = 0;
			int iClothEntityFirstMeshIndex = 0;
			int iClothEntityFirstVertexIndex = 0;
			uint32_t iEntity = 0;
			uint8_t* entityUseCloth = NULL;

			glmCreateClothData(simulationData, data, data->_clothEntityCount, data->_clothTotalMeshIndices, data->_clothTotalVertices);

			// temporary reading buffer
			entityUseCloth = (uint8_t*)GLMC_MALLOC(simulationData->_entityCount * sizeof(uint8_t));

			glmFileRead(entityUseCloth, sizeof(uint8_t), simulationData->_entityCount, fp);
			glmFileReadUInt32(data->_clothEntityMeshCount, data->_clothEntityCount, fp);
			glmFileRead(data->_clothEntityQuantizationReference, sizeof(float), data->_clothEntityCount * 3, fp);
			glmFileRead(data->_clothEntityQuantizationMaxExtent, sizeof(float), data->_clothEntityCount, fp);
			glmFileReadUInt32(data->_clothMeshIndicesInCharAssets, data->_clothTotalMeshIndices, fp);
			glmFileReadUInt32(data->_clothMeshVertexCount, data->_clothTotalMeshIndices, fp);

			if (data->_clothTotalVertices > 0)
			{
				glmFileReadClothVertices(data, fp, (GlmSimulationCacheFormat)data->_cacheFormat);
			}

			// create runtime helpers from serialized data
			for (iEntity = 0; iEntity < simulationData->_entityCount; iEntity++)
			{
				if (entityUseCloth[iEntity])
				{
					uint32_t iClothMeshIndex;
					uint32_t clothMeshCount;
					data->_entityClothIndex[iEntity] = iClothEntityIndex;
					
					data->_clothEntityFirstAssetMeshIndex[iClothEntityIndex] = iClothEntityFirstMeshIndex;
					data->_clothEntityFirstMeshVertex[iClothEntityIndex] = iClothEntityFirstVertexIndex;

					// advance indices (add current cloth meshes vertices, and advance first mesh index)
					for (iClothMeshIndex = 0, clothMeshCount = data->_clothEntityMeshCount[iClothEntityIndex]; iClothMeshIndex < clothMeshCount; iClothMeshIndex++)
					{
						iClothEntityFirstVertexIndex += data->_clothMeshVertexCount[iClothEntityFirstMeshIndex + iClothMeshIndex];
					}

					iClothEntityFirstMeshIndex += data->_clothEntityMeshCount[iClothEntityIndex];

					iClothEntityIndex++;
				}
				else
				{
					data->_entityClothIndex[iEntity] = -1;
				}
			}

			GLMC_FREE(entityUseCloth);
		}
	}

	if (version < 0x02)
	{
		uint8_t ppAttributeCount = simulationData->_ppFloatAttributeCount + simulationData->_ppVectorAttributeCount;
		int floatAttrCount = 0;
		int vectorAttrCount = 0;
		for (i = 0; i < ppAttributeCount; ++i)
		{
			switch (simulationData->_backwardCompatPPAttributeTypes[i])
			{
			case GSC_PP_FLOAT:
				glmFileRead(data->_ppFloatAttributeData[floatAttrCount], sizeof(float), simulationData->_entityCount, fp);
				floatAttrCount++;
				break;
			case GSC_PP_VECTOR:
				glmFileRead(data->_ppVectorAttributeData[vectorAttrCount], sizeof(float), simulationData->_entityCount * 3, fp);
				vectorAttrCount++;
				break;
			default:
				break;
			}
		}
	}
	else
	{
	// ppAttribute
	for (i = 0; i < simulationData->_ppFloatAttributeCount; ++i)
	{
		glmFileRead(data->_ppFloatAttributeData[i], sizeof(float), simulationData->_entityCount, fp);
	}
	for (i = 0; i < simulationData->_ppVectorAttributeCount; ++i)
	{
		glmFileRead(data->_ppVectorAttributeData[i], sizeof(float), simulationData->_entityCount * 3, fp);
	}
	}

	fclose(fp);

	return GSC_SUCCESS;
}

//----------------------------------------------------------------------------
void glmFileWriteOrientations(float(*bonesOrientations)[4], unsigned int totalBoneCount, FILE* fp, GlmSimulationCacheFormat format)
{
	switch (format)
	{
	case GSC_O32_P48:
	case GSC_O32_P96:
	{
		unsigned int i;
		uint32_t* compressedBoneOrientations = (uint32_t*)GLMC_MALLOC(totalBoneCount * sizeof(uint32_t));
		for (i = 0; i < totalBoneCount; ++i)
		{
			glmCompressQuaternion32(&compressedBoneOrientations[i], bonesOrientations[i]);
		}
		glmFileWriteUInt32(compressedBoneOrientations, totalBoneCount, fp);
		GLMC_FREE(compressedBoneOrientations);
	}
	break;
	case GSC_O64_P96:
	case GSC_O64_P48:
	{
		unsigned int i;
		uint64_t* compressedBoneOrientations = (uint64_t*)GLMC_MALLOC(totalBoneCount * sizeof(uint64_t));
		for (i = 0; i < totalBoneCount; ++i)
		{
			glmCompressQuaternion64(&compressedBoneOrientations[i], bonesOrientations[i]);
		}
		glmFileWriteUInt64(compressedBoneOrientations, totalBoneCount, fp);
		GLMC_FREE(compressedBoneOrientations);
	}
	break;
	default:
		glmFileWrite(bonesOrientations, sizeof(float), 4 * totalBoneCount, fp);
	}
}

//----------------------------------------------------------------------------
void glmFileWritePositions(float(*bonesPositions)[3], unsigned int totalBoneCount, const GlmSimulationData* data, FILE* fp, GlmSimulationCacheFormat format)
{
	switch (format)
	{
	case GSC_O32_P48:
	case GSC_O64_P48:
	case GSC_O128_P48:
	{
		unsigned int iEntityType;
		unsigned int iRootBone = 0;
		float localPosition[3];
		float(*rootBonePositions)[3];
		uint16_t(*compressedBonePositions)[3];

		uint32_t validEntityCount = 0;
		for (iEntityType = 0; iEntityType < data->_entityTypeCount; ++iEntityType)
		{
			validEntityCount += data->_entityCountPerEntityType[iEntityType];
		}

		rootBonePositions = (float(*)[3])GLMC_MALLOC(validEntityCount * sizeof(float[3]));
		compressedBonePositions = (uint16_t(*)[3])GLMC_MALLOC((totalBoneCount - validEntityCount) * sizeof(uint16_t[3]));

		for (iEntityType = 0; iEntityType < data->_entityTypeCount; ++iEntityType)
		{
			unsigned int iEntity;
			for (iEntity = 0; iEntity < data->_entityCountPerEntityType[iEntityType]; ++iEntity)
			{
				unsigned int iBone;
				unsigned int iBoneOffset = data->_iBoneOffsetPerEntityType[iEntityType] + iEntity * data->_boneCount[iEntityType];
				memcpy(rootBonePositions[iRootBone], bonesPositions[iBoneOffset], sizeof(float[3]));
				++iRootBone;
				for (iBone = 1; iBone < data->_boneCount[iEntityType]; ++iBone)
				{
					unsigned int i = iBone + iBoneOffset;
					localPosition[0] = bonesPositions[i][0] - rootBonePositions[iRootBone - 1][0];
					localPosition[1] = bonesPositions[i][1] - rootBonePositions[iRootBone - 1][1];
					localPosition[2] = bonesPositions[i][2] - rootBonePositions[iRootBone - 1][2];

					glmCompressPosition48(compressedBonePositions[i - iRootBone], localPosition, data->_maxBonesHierarchyLength[iEntityType]);
				}
			}
		}
		glmFileWrite(rootBonePositions, sizeof(float), validEntityCount * 3, fp);
		glmFileWriteUInt16(compressedBonePositions[0], (totalBoneCount - validEntityCount) * 3, fp);
		GLMC_FREE(compressedBonePositions);
		GLMC_FREE(rootBonePositions);
	}
	break;
	default:
		glmFileWrite(bonesPositions, sizeof(float), 3 * totalBoneCount, fp);
	}
}

//----------------------------------------------------------------------------
void glmFileWriteClothVertices(const GlmFrameData* frameData, FILE* fp, GlmSimulationCacheFormat format) // clothReference and clothMaxExtents must be valid
{
	switch (format)
	{
	case GSC_O32_P48:
	case GSC_O64_P48:
	case GSC_O128_P48:
	{
		unsigned int iClothEntity = 0;
		unsigned int iClothEntityMesh = 0;
		unsigned int iClothVertex = 0;
		float* currentVertex = NULL;

		uint16_t(*compressedVertices)[3] = (uint16_t(*)[3])GLMC_MALLOC(frameData->_clothTotalVertices * sizeof(uint16_t[3]));

		int iAbsoluteClothMesh = 0;
		int iAbsoluteMeshVertex = 0;

		// iteration per cloth entity:
		for (iClothEntity = 0; iClothEntity < frameData->_clothEntityCount; iClothEntity++)
		{
			float *referencePosition = frameData->_clothEntityQuantizationReference[iClothEntity];
			float maxExtent = frameData->_clothEntityQuantizationMaxExtent[iClothEntity];

			for (iClothEntityMesh = 0; iClothEntityMesh < frameData->_clothEntityMeshCount[iClothEntity]; iClothEntityMesh++)
			{
				for (iClothVertex = 0; iClothVertex < frameData->_clothMeshVertexCount[iAbsoluteClothMesh]; iClothVertex++)
				{
					currentVertex = frameData->_clothVertices[iAbsoluteMeshVertex];

					// take out ref :
					currentVertex[0] -= referencePosition[0];
					currentVertex[1] -= referencePosition[1];
					currentVertex[2] -= referencePosition[2];

					glmCompressPosition48(compressedVertices[iAbsoluteMeshVertex], currentVertex, maxExtent);
					iAbsoluteMeshVertex++;
				}

				iAbsoluteClothMesh++;
			}
		}
		glmFileWriteUInt16(compressedVertices[0], frameData->_clothTotalVertices * 3, fp);
		GLMC_FREE(compressedVertices);
	}
	break;
	default:
		glmFileWrite(frameData->_clothVertices, sizeof(float), 3 * frameData->_clothTotalVertices, fp);
	}
}

//----------------------------------------------------------------------------
GlmSimulationCacheStatus glmWriteFrameData(const char* file, const GlmFrameData* data, const GlmSimulationData* simulationData)
{
	uint16_t magicNumber;
	uint8_t version;
	unsigned int totalBoneCount = 0;
	unsigned int totalSnSCount = 0;
	unsigned int totalBlindDataCount = 0;
	unsigned int totalGeoBehaviorCount = 0;
	unsigned int i;

#ifdef _MSC_VER				
	FILE* fp;
	errno_t err;
	err = fopen_s(&fp, file, "wb");
	if (err != 0) return GSC_FILE_OPEN_FAILED;
#else
	FILE* fp = fopen(file, "wb");
	if (fp == NULL) return GSC_FILE_OPEN_FAILED;
#endif

	// entityType
	for (i = 0; i < simulationData->_entityTypeCount; ++i)
	{
		totalBoneCount += simulationData->_boneCount[i] * simulationData->_entityCountPerEntityType[i];
		totalSnSCount += simulationData->_snsCountPerEntityType[i] * simulationData->_entityCountPerEntityType[i];
		totalBlindDataCount += simulationData->_blindDataCount[i] * simulationData->_entityCountPerEntityType[i];
		if (simulationData->_hasGeoBehavior[i])
		{
			totalGeoBehaviorCount += simulationData->_entityCountPerEntityType[i];
		}
	}

	// header
	magicNumber = GSCF_MAGIC_NUMBER;
	glmFileWriteUInt16(&magicNumber, 1, fp);
	version = GSC_VERSION;
	glmFileWrite(&version, sizeof(uint8_t), 1, fp); 
	if (version > GSC_VERSION)
	{
		fclose(fp);
		return GSC_FILE_VERSION_ERROR;
	}

	glmFileWrite(&(data->_cacheFormat), sizeof(uint8_t), 1, fp);

	glmFileWriteUInt32((uint32_t*)&data->_simulationContentHashKey, 1, fp);

	glmFileWritePositions(data->_bonePositions, totalBoneCount, simulationData, fp, (GlmSimulationCacheFormat)data->_cacheFormat);
	glmFileWriteOrientations(data->_boneOrientations, totalBoneCount, fp, (GlmSimulationCacheFormat)data->_cacheFormat);
	glmFileWrite(data->_snsValues, sizeof(float), totalSnSCount * 4, fp);
	glmFileWrite(data->_blindData, sizeof(float), totalBlindDataCount, fp);
	if (totalGeoBehaviorCount > 0)
	{
		glmFileWriteUInt16(data->_geoBehaviorGeometryIds, totalGeoBehaviorCount, fp);
		glmFileWrite(data->_geoBehaviorAnimFrameInfo[0], sizeof(float), totalGeoBehaviorCount * 3, fp);
		glmFileWrite(data->_geoBehaviorBlendModes, sizeof(uint8_t), totalGeoBehaviorCount, fp);
	}

	// cloth counts
	glmFileWriteUInt32((uint32_t*)&data->_clothEntityCount, 1, fp);
	if (data->_clothEntityCount != 0)
	{
		glmFileWriteUInt32((uint32_t*)&data->_clothTotalMeshIndices, 1, fp);
		glmFileWriteUInt32((uint32_t*)&data->_clothTotalVertices, 1, fp);

		if (data->_clothTotalMeshIndices > 0)
		{
			// do not serialize helpers, just 'using cloth' or not
			uint8_t* entityUseCloth = (uint8_t*)GLMC_MALLOC(simulationData->_entityCount * sizeof(uint8_t));
			uint32_t iEntity = 0;
			for (; iEntity < simulationData->_entityCount; iEntity++)
			{
				entityUseCloth[iEntity] = data->_entityClothIndex[iEntity] == -1 ? 0 : 1;
			}
			glmFileWrite(entityUseCloth, sizeof(uint8_t), simulationData->_entityCount, fp);
			GLMC_FREE(entityUseCloth);

			// note : _clothEntityFirstMeshVertex & _clothEntityFirstAssetMeshIndex are recomputed, not serialized
			glmFileWriteUInt32(data->_clothEntityMeshCount, data->_clothEntityCount, fp);
			glmFileWrite(data->_clothEntityQuantizationReference, sizeof(float), data->_clothEntityCount * 3, fp);
			glmFileWrite(data->_clothEntityQuantizationMaxExtent, sizeof(float), data->_clothEntityCount, fp);

			glmFileWriteUInt32(data->_clothMeshIndicesInCharAssets, data->_clothTotalMeshIndices, fp);
			glmFileWriteUInt32(data->_clothMeshVertexCount, data->_clothTotalMeshIndices, fp);

			if (data->_clothTotalVertices > 0)
			{
				glmFileWriteClothVertices(data, fp, (GlmSimulationCacheFormat)data->_cacheFormat);
			}
		}
	}

	// ppAttribute
	for (i = 0; i < simulationData->_ppFloatAttributeCount; ++i)
	{
		glmFileWrite(data->_ppFloatAttributeData[i], sizeof(float), simulationData->_entityCount, fp);
	}
	for (i = 0; i < simulationData->_ppVectorAttributeCount; ++i)
	{
		glmFileWrite(data->_ppVectorAttributeData[i], sizeof(float), simulationData->_entityCount * 3, fp);
	}

	fclose(fp);

	return GSC_SUCCESS;
}

//----------------------------------------------------------------------------
void glmDestroyFrameData(GlmFrameData** frameData, const GlmSimulationData* simulationData)
{
	unsigned int i;
	GlmFrameData* data = *frameData;
	GLMC_ASSERT((data != NULL) && "Simulation data must be created before being destroyed");
	GLMC_FREE(data->_bonePositions);
	GLMC_FREE(data->_boneOrientations);
	GLMC_FREE(data->_snsValues);
	GLMC_FREE(data->_blindData);
	GLMC_FREE(data->_geoBehaviorGeometryIds);
	GLMC_FREE(data->_geoBehaviorAnimFrameInfo);
	GLMC_FREE(data->_geoBehaviorBlendModes);

	GLMC_FREE(data->_entityClothIndex);
	GLMC_FREE(data->_clothEntityFirstAssetMeshIndex);
	GLMC_FREE(data->_clothEntityFirstMeshVertex);
	GLMC_FREE(data->_clothEntityMeshCount);
	GLMC_FREE(data->_clothEntityQuantizationReference);
	GLMC_FREE(data->_clothEntityQuantizationMaxExtent);
	GLMC_FREE(data->_clothMeshIndicesInCharAssets);
	GLMC_FREE(data->_clothMeshVertexCount);

	GLMC_FREE(data->_clothVertices);

	for (i = 0; i < simulationData->_ppFloatAttributeCount; ++i)
	{
		GLMC_FREE(data->_ppFloatAttributeData[i]);
	}
	GLMC_FREE(data->_ppFloatAttributeData);

	for (i = 0; i < simulationData->_ppVectorAttributeCount; ++i)
	{
		GLMC_FREE(data->_ppVectorAttributeData[i]);
	}
	GLMC_FREE(data->_ppVectorAttributeData);

	GLMC_FREE(data);
	*frameData = NULL;
}

//----------------------------------------------------------------------------
void glmCreateHistory(GlmHistory** history, unsigned int transformCount, unsigned int transformGroupCount, unsigned int entityCount, unsigned int totalPostureCount, unsigned int totalPostureBoneCount, unsigned int totalEntityTypesBoneCount, unsigned int totalMeshAssetsOverride, unsigned int duplicatedEntityCount, unsigned int totalEntityTypeCount, unsigned int expandCount, unsigned int totalFrameOffsetCount, unsigned int totalFrameWarpCount, unsigned int scaleRangeCount, unsigned int perFramePosOriCount, unsigned int snapToTotalCount)
{
	unsigned int i;
	GlmHistory* data;

	// allocate frame data
	*history = (GlmHistory*)GLMC_MALLOC(sizeof(GlmHistory));
	data = *history;

	data->_entityArrayCount = (uint32_t *)GLMC_MALLOC( transformCount * sizeof(uint32_t) );
	data->_entityArrayStartIndex = (uint32_t *)GLMC_MALLOC( transformCount * sizeof(uint32_t) );
	data->_entityIds = (int64_t *)GLMC_MALLOC( entityCount * sizeof(int64_t) );

	data->_transformRotate = (float(*)[4])GLMC_MALLOC( transformCount * sizeof(float[4]) );
	data->_transformTranslate = (float(*)[3])GLMC_MALLOC( transformCount * sizeof(float[3]) );
	data->_transformPivot = (float(*)[3])GLMC_MALLOC( transformCount * sizeof(float[3]) );
	data->_scale = (float*)GLMC_MALLOC( transformCount * sizeof(float) );

	data->_transformTypes = (uint8_t*)GLMC_MALLOC( transformCount *sizeof(uint8_t) );
	data->_active = (uint8_t*)GLMC_MALLOC( transformCount *sizeof(uint8_t) );
	data->_boneIndex = (uint32_t*)GLMC_MALLOC( transformCount *sizeof(uint32_t) );
	data->_renderingTypeIdx = (uint32_t*)GLMC_MALLOC(transformCount *sizeof(uint32_t));
	data->_clothIndice = (uint32_t*)GLMC_MALLOC(transformCount *sizeof(uint32_t));
	data->_enableCloth = (uint32_t*)GLMC_MALLOC(transformCount *sizeof(uint32_t));

	// duplicated entities
	data->_duplicatedEntityCount = duplicatedEntityCount;
	data->_duplicatedEntityIds = (int64_t *)GLMC_MALLOC(duplicatedEntityCount * sizeof(int64_t));
	data->_duplicatedEntityArrayStartIndex = (uint32_t*)GLMC_MALLOC(transformCount *sizeof(uint32_t));
	data->_duplicatedEntityArrayCount = (uint32_t*)GLMC_MALLOC(transformCount *sizeof(uint32_t));

	// hierarchical bones
	data->_expandCount = expandCount;
	data->_expands = (float(*)[3])GLMC_MALLOC(expandCount * sizeof(float[3]));
	data->_expandArrayStartIndex = (uint32_t*)GLMC_MALLOC(transformCount *sizeof(uint32_t));
	data->_expandArrayCount = (uint32_t*)GLMC_MALLOC(transformCount *sizeof(uint32_t));

	// per frame pos/ori
	data->_perFramePosOriCount = perFramePosOriCount;
	data->_frameCurvePos = (float(*)[3])GLMC_MALLOC(perFramePosOriCount * sizeof(float[3]));
	data->_frameCachePos = (float(*)[3])GLMC_MALLOC(perFramePosOriCount * sizeof(float[3]));
	data->_framePos = (float(*)[3])GLMC_MALLOC(perFramePosOriCount * sizeof(float[3]));
	data->_frameOri = (float(*)[4])GLMC_MALLOC(perFramePosOriCount * sizeof(float[4]));
	data->_perFramePosOriArrayStartIndex = (uint32_t*)GLMC_MALLOC(transformCount *sizeof(uint32_t));
	data->_perFramePosOriArrayCount = (uint32_t*)GLMC_MALLOC(transformCount *sizeof(uint32_t));

	// scale range entities
	data->_scaleRangeCount = scaleRangeCount;
	data->_scaleRanges = (float*)GLMC_MALLOC(scaleRangeCount * sizeof(float));
	data->_scaleRangeArrayStartIndex = (uint32_t*)GLMC_MALLOC(transformCount *sizeof(uint32_t));
	data->_scaleRangeArrayCount = (uint32_t*)GLMC_MALLOC(transformCount *sizeof(uint32_t));

	// hierarchical bones
	data->_localBoneCount = totalEntityTypesBoneCount;
	data->_localBoneOrientation = (float(*)[4])GLMC_MALLOC(totalEntityTypesBoneCount * sizeof(float[4]));
	data->_localBonePosition = (float(*)[3])GLMC_MALLOC(totalEntityTypesBoneCount * sizeof(float[3]));
	data->_localBoneParent = (uint32_t*)GLMC_MALLOC(totalEntityTypesBoneCount *sizeof(uint32_t));

	data->_localBoneOffsetCount = totalEntityTypeCount;
	data->_localBoneOffset = (uint32_t*)GLMC_MALLOC(totalEntityTypeCount *sizeof(uint32_t));

	// mesh assets override
	data->_meshAssetsOverrideTotalCount = totalMeshAssetsOverride;
	data->_meshAssetsOverride = (uint32_t*)GLMC_MALLOC( totalMeshAssetsOverride *sizeof(uint32_t) );
	data->_meshAssetsOverrideStartIndex = (uint32_t*)GLMC_MALLOC( entityCount *sizeof(uint32_t) );
	data->_meshAssetsOverrideCount = (uint32_t*)GLMC_MALLOC( entityCount *sizeof(uint32_t) );
	memset(data->_meshAssetsOverrideCount, 0, entityCount *sizeof(uint32_t));

	// posture edit
	data->_posturesFrameCount = (uint32_t *)GLMC_MALLOC( transformCount * sizeof(uint32_t) );
	data->_posturesFrameStart = (uint32_t *)GLMC_MALLOC( transformCount * sizeof(uint32_t) );

	data->_posturesFrames = (uint32_t *)GLMC_MALLOC( totalPostureCount * sizeof(uint32_t) );

	// posture edit bones
	data->_posturesPositions = (float(*)[3])GLMC_MALLOC( totalPostureBoneCount * sizeof(float[3]) );
	data->_posturesOrientations = (float(*)[4])GLMC_MALLOC( totalPostureBoneCount * sizeof(float[4]) );

	data->_postureTotalBoneCount = totalPostureBoneCount;
	data->_postureCount = totalPostureCount;
	data->_entityCount = entityCount;
	data->_transformCount = transformCount;

	// groups
	data->_transformGroupCount = transformGroupCount;
	data->_transformGroupActive = (uint8_t*)GLMC_MALLOC(transformGroupCount *sizeof(uint8_t));
	data->_transformGroupName = (char(*)[GSC_PP_MAX_NAME_LENGTH])GLMC_MALLOC(transformGroupCount * sizeof(char[GSC_PP_MAX_NAME_LENGTH]));
	// init string marker
	for (i = 0; i < transformGroupCount; i++)
		data->_transformGroupName[i][0] = 0;
	data->_transformGroupBoundaries = (uint32_t(*)[2])GLMC_MALLOC(transformGroupCount * sizeof(uint32_t[2]));

	// frame offset
	data->_frameOffsetCount = totalFrameOffsetCount;
	data->_frameOffsets = (float *)GLMC_MALLOC(totalFrameOffsetCount * sizeof(float));
	data->_frameOffsetArrayStartIndex = (uint32_t*)GLMC_MALLOC(transformCount *sizeof(uint32_t));
	data->_frameOffsetArrayCount = (uint32_t*)GLMC_MALLOC(transformCount *sizeof(uint32_t));

	// frame scale
	data->_frameWarpCount = totalFrameWarpCount;
	data->_frameWarps = (float *)GLMC_MALLOC(totalFrameWarpCount * sizeof(float));
	data->_frameWarpArrayStartIndex = (uint32_t*)GLMC_MALLOC(transformCount *sizeof(uint32_t));
	data->_frameWarpArrayCount = (uint32_t*)GLMC_MALLOC(transformCount *sizeof(uint32_t));
	
	// frame parameters
	data->_frameOffsetMin = (float *)GLMC_MALLOC(transformCount * sizeof(float));
	data->_frameOffsetMax = (float *)GLMC_MALLOC(transformCount * sizeof(float));
	data->_frameWarpMin = (float *)GLMC_MALLOC(transformCount * sizeof(float));
	data->_frameWarpMax = (float *)GLMC_MALLOC(transformCount * sizeof(float));

	// scale range bound
	data->_scaleRangeMin = (float *)GLMC_MALLOC(transformCount * sizeof(float));
	data->_scaleRangeMax = (float *)GLMC_MALLOC(transformCount * sizeof(float));

	// per frame pos/ori
	data->_frameCount = (uint32_t *)GLMC_MALLOC(transformCount * sizeof(uint32_t));
	data->_startFrame = (uint32_t *)GLMC_MALLOC(transformCount * sizeof(uint32_t));
	data->_trajectoryMode = (uint32_t *)GLMC_MALLOC(transformCount * sizeof(uint32_t));
	data->_trajectorySteps = (uint32_t *)GLMC_MALLOC(transformCount * sizeof(uint32_t));
	data->_smoothIterationCount = (uint32_t *)GLMC_MALLOC(transformCount * sizeof(uint32_t));
	data->_smoothFrontBackRatio = (float *)GLMC_MALLOC(transformCount * sizeof(float));
	data->_smoothComponents = (float(*)[3])GLMC_MALLOC(transformCount * sizeof(float) * 3);

	// default init
	for (i = 0; i < transformCount; i++)
	{
		data->_trajectoryMode[i] = 0;
		data->_trajectorySteps[i] = 1;
		data->_smoothIterationCount[i] = 0;
		data->_smoothFrontBackRatio[i] = 0.5f;
		data->_smoothComponents[i][0] = 1.f;
		data->_smoothComponents[i][1] = 1.f;
		data->_smoothComponents[i][2] = 1.f;
	}

	data->_snapToTarget = (char(*)[GSC_PP_MAX_NAME_LENGTH])GLMC_MALLOC(transformCount * sizeof(char[GSC_PP_MAX_NAME_LENGTH]));
	data->_shaderAttribute = (char(*)[GSC_PP_MAX_NAME_LENGTH])GLMC_MALLOC(transformCount * sizeof(char[GSC_PP_MAX_NAME_LENGTH]));

	// init string marker
	for (i = 0; i < transformCount; i++)
	{
		data->_snapToTarget[i][0] = 0;
		data->_shaderAttribute[i][0] = 0;
	}

	data->_snapToTotalCount = snapToTotalCount;
	data->_snapToStartIndex = (uint32_t*)GLMC_MALLOC(transformCount *sizeof(uint32_t));
	data->_snapToCount = (uint32_t*)GLMC_MALLOC(transformCount *sizeof(uint32_t));
	data->_snapToPositions = (float(*)[3])GLMC_MALLOC(snapToTotalCount * sizeof(float[3]));
	data->_snapToRotations = (float(*)[4])GLMC_MALLOC(snapToTotalCount * sizeof(float[4]));

	data->_terrainMeshSource = NULL;
	data->_terrainMeshDestination = NULL;
}

//----------------------------------------------------------------------------
void glmDestroyHistory(GlmHistory** history)
{
	GlmHistory* data = *history;
	if (!data)
		return;

	GLMC_FREE(data->_transformTypes);
	GLMC_FREE(data->_active);
	GLMC_FREE(data->_boneIndex);
	GLMC_FREE(data->_renderingTypeIdx);
	GLMC_FREE(data->_clothIndice);
	GLMC_FREE(data->_enableCloth);
	
	GLMC_FREE(data->_transformRotate);
	GLMC_FREE(data->_transformTranslate);
	GLMC_FREE(data->_transformPivot);
	GLMC_FREE(data->_scale);
	
	GLMC_FREE(data->_entityIds);
	GLMC_FREE(data->_entityArrayStartIndex);
	GLMC_FREE(data->_entityArrayCount);

	// duplicated entities
	GLMC_FREE(data->_duplicatedEntityIds);
	GLMC_FREE(data->_duplicatedEntityArrayStartIndex);
	GLMC_FREE(data->_duplicatedEntityArrayCount);

	// hierarchy
	GLMC_FREE(data->_expands);
	GLMC_FREE(data->_expandArrayStartIndex);
	GLMC_FREE(data->_expandArrayCount);

	// per frame pos/ori
	GLMC_FREE(data->_frameCurvePos);
	GLMC_FREE(data->_frameCachePos);
	GLMC_FREE(data->_framePos);
	GLMC_FREE(data->_frameOri);
	GLMC_FREE(data->_perFramePosOriArrayStartIndex);
	GLMC_FREE(data->_perFramePosOriArrayCount);

	// scale range entities
	GLMC_FREE(data->_scaleRanges);
	GLMC_FREE(data->_scaleRangeArrayStartIndex);
	GLMC_FREE(data->_scaleRangeArrayCount);

	// hierarchy
	GLMC_FREE(data->_localBoneOrientation);
	GLMC_FREE(data->_localBonePosition);
	GLMC_FREE(data->_localBoneParent);
	GLMC_FREE(data->_localBoneOffset);

	// mesh assets override
	GLMC_FREE(data->_meshAssetsOverride);
	GLMC_FREE(data->_meshAssetsOverrideStartIndex);
	GLMC_FREE(data->_meshAssetsOverrideCount);

	// posture edit
	GLMC_FREE(data->_posturesFrameCount);
	GLMC_FREE(data->_posturesFrameStart);
	GLMC_FREE(data->_posturesFrames);

	// posture edit bones
	GLMC_FREE(data->_posturesPositions);
	GLMC_FREE(data->_posturesOrientations);

	// Groups
	GLMC_FREE(data->_transformGroupActive);
	GLMC_FREE(data->_transformGroupName);
	GLMC_FREE(data->_transformGroupBoundaries);

	// frame offsets
	GLMC_FREE(data->_frameOffsets);
	GLMC_FREE(data->_frameOffsetArrayStartIndex);
	GLMC_FREE(data->_frameOffsetArrayCount);

	// frame scales
	GLMC_FREE(data->_frameWarps);
	GLMC_FREE(data->_frameWarpArrayStartIndex);
	GLMC_FREE(data->_frameWarpArrayCount);

	// frame parameters
	GLMC_FREE(data->_frameOffsetMin);
	GLMC_FREE(data->_frameOffsetMax);
	GLMC_FREE(data->_frameWarpMin);
	GLMC_FREE(data->_frameWarpMax);

	// scale range bounds
	GLMC_FREE(data->_scaleRangeMin);
	GLMC_FREE(data->_scaleRangeMax);

	// per frame
	GLMC_FREE(data->_startFrame);
	GLMC_FREE(data->_frameCount);
	GLMC_FREE(data->_trajectoryMode);
	GLMC_FREE(data->_trajectorySteps);
	GLMC_FREE(data->_smoothIterationCount);
	GLMC_FREE(data->_smoothFrontBackRatio);
	GLMC_FREE(data->_smoothComponents);

	// SnapTo
	GLMC_FREE(data->_snapToTarget);
	GLMC_FREE(data->_snapToStartIndex);
	GLMC_FREE(data->_snapToCount);
	GLMC_FREE(data->_snapToPositions);
	GLMC_FREE(data->_snapToRotations);

	// shader attribute
	GLMC_FREE(data->_shaderAttribute);

	GLMC_FREE(data);
	*history = NULL;
}

#ifndef GLMC_NO_JSON

#include <stdarg.h>

int glmsprintf( char *str, int strSize, const char *format, ... )
{
	int rt;
	va_list ptr_arg;
	va_start(ptr_arg, format);
#ifdef _MSC_VER
	rt = vsprintf_s(str, strSize, format, ptr_arg);
#else
	rt = vsprintf(str, format, ptr_arg);
#endif
	va_end(ptr_arg);
	return rt;
}

//----------------------------------------------------------------------------
GlmSimulationCacheStatus glmWriteHistoryJSON(const char* file, GlmHistory* history)
{
	char tmps[1024];
	GlmHistory* data = history;
	unsigned int i, j;
	unsigned int addComa = 0;

	// header

#ifdef _MSC_VER				
	FILE* fp;
	errno_t err;
	err = fopen_s(&fp, file, "wb");
	if (err != 0) return GSC_FILE_OPEN_FAILED;
#else
	FILE* fp = fopen(file, "wb");
	if (fp == NULL) return GSC_FILE_OPEN_FAILED;
#endif

	if (fp == NULL) return GSC_FILE_OPEN_FAILED;

	// header
	fputs("{", fp);
	glmsprintf(tmps, sizeof(tmps), "\"transformGroupCount\":%d,\"entityCount\":%d,\"postureCount\":%d,\"postureTotalBoneCount\":%d,\"localBoneCount\":%d,\"meshAssetsOverrideTotalCount\":%d,\"options\":%d,\"duplicatedEntityCount\":%d,\"entityTypeCount\":%d,\"expandCount\":%d,\"frameOffsetCount\":%d,\"frameWarpCount\":%d,\"scaleRangeCount\":%d,\"perFramePosOriCount\":%d,\"snapToTotalCount\":%d\n",
		data->_transformGroupCount,
		data->_entityCount,
		data->_postureCount,
		data->_postureTotalBoneCount,
		data->_localBoneCount,
		data->_meshAssetsOverrideTotalCount,
		data->_options,
		data->_duplicatedEntityCount,
		data->_localBoneOffsetCount,
		data->_expandCount,
		data->_frameOffsetCount,
		data->_frameWarpCount,
		data->_scaleRangeCount,
		data->_perFramePosOriCount,
		data->_snapToTotalCount
		);
	fputs(tmps, fp);

	// transformations
	fputs(",\"transformations\":[", fp);
	for (i=0;i<data->_transformCount;i++)
	{
		if (i)
			fputs(",\n", fp);
		glmsprintf(tmps, sizeof(tmps), "{\n\"transformType\":%d,\"active\":%d,\"boneIndex\":%d,\"renderingTypeIdx\":%d,\"scale\":%f,\"rotate\":[%f,%f,%f,%f],\"translate\":[%f,%f,%f],\"pivot\":[%f,%f,%f],",
			data->_transformTypes[i],
			data->_active[i] ? 1 : 0,
			data->_boneIndex[i],
			data->_renderingTypeIdx[i],
			data->_scale[i],
			data->_transformRotate[i][0], data->_transformRotate[i][1], data->_transformRotate[i][2], data->_transformRotate[i][3],
			data->_transformTranslate[i][0], data->_transformTranslate[i][1], data->_transformTranslate[i][2],
			data->_transformPivot[i][0], data->_transformPivot[i][1], data->_transformPivot[i][2]);
		fputs(tmps, fp);

		glmsprintf(tmps, sizeof(tmps), "\"clothIndice\":%d,\"enableCloth\":%d,\"frameOffsetMin\":%f,\"frameOffsetMax\":%f,\"frameWarpMin\":%f,\"frameWarpMax\":%f,\"scaleRangeMin\":%f,\"scaleRangeMax\":%f,",
			data->_clothIndice[i],
			data->_enableCloth[i],
			data->_frameOffsetMin[i],
			data->_frameOffsetMax[i],
			data->_frameWarpMin[i],
			data->_frameWarpMax[i],
			data->_scaleRangeMin[i],
			data->_scaleRangeMax[i]);
		fputs(tmps, fp);

		glmsprintf(tmps, sizeof(tmps), "\"frameCount\":%d,\"startFrame\":%d,\"snapToTarget\":\"%s\",\"snapToStartIndex\":%d,\"snapToCount\":%d,\"trajectoryMode\":%d,\"trajectorySteps\":%d,\"smoothIterations\":%d,\"smoothRatio\":%f,\"smoothX\":%f,\"smoothY\":%f,\"smoothZ\":%f,",
			(int)data->_frameCount[i],
			(int)data->_startFrame[i],
			data->_snapToTarget[i],
			data->_snapToStartIndex[i],
			data->_snapToCount[i],
			data->_trajectoryMode[i],
			data->_trajectorySteps[i],
			data->_smoothIterationCount[i],
			data->_smoothFrontBackRatio[i],
			data->_smoothComponents[i][0],
			data->_smoothComponents[i][1],
			data->_smoothComponents[i][2]);
		fputs(tmps, fp);

		glmsprintf(tmps, sizeof(tmps), "\"shaderAttribute\":\"%s\",\n",	data->_shaderAttribute[i]);
		fputs(tmps, fp);

		// entities
		fputs("\"entities\":[", fp);
		for(j=0;j<data->_entityArrayCount[i];j++)
		{
			if (j) fputs(",",fp);
			glmsprintf(tmps, sizeof(tmps), "%d", data->_entityIds[data->_entityArrayStartIndex[i] + j]);
			fputs(tmps, fp);
		}
		fputs("]\n",fp);
		// set mesh asset
		fputs(",\"meshAssetStart\":[", fp);
		for (j = 0; j<data->_entityArrayCount[i]; j++)
		{
			if (j) fputs(",", fp);
			glmsprintf(tmps, sizeof(tmps), "%d", data->_meshAssetsOverrideStartIndex[data->_entityArrayStartIndex[i] + j]);
			fputs(tmps, fp);
		}
		fputs("]\n", fp);
		fputs(",\"meshAssetCount\":[", fp);
		for (j = 0; j<data->_entityArrayCount[i]; j++)
		{
			if (j) fputs(",", fp);
			glmsprintf(tmps, sizeof(tmps), "%d", data->_meshAssetsOverrideCount[data->_entityArrayStartIndex[i] + j]);
			fputs(tmps, fp);
		}
		fputs("]\n", fp);
		// duplicated entities
		fputs(",\"duplicatedEntities\":[", fp);
		for (j = 0; j<data->_duplicatedEntityArrayCount[i]; j++)
		{
			if (j) fputs(",", fp);
			glmsprintf(tmps, sizeof(tmps), "%d", data->_duplicatedEntityIds[data->_duplicatedEntityArrayStartIndex[i] + j]);
			fputs(tmps, fp);
		}
		fputs("]\n", fp);

		// frame offset
		fputs(",\"frameOffsets\":[", fp);
		for (j = 0; j<data->_frameOffsetArrayCount[i]; j++)
		{
			if (j) fputs(",", fp);
			glmsprintf(tmps, sizeof(tmps), "%f", data->_frameOffsets[data->_frameOffsetArrayStartIndex[i] + j]);
			fputs(tmps, fp);
		}
		fputs("]\n", fp);

		// frame scales
		fputs(",\"frameWarps\":[", fp);
		for (j = 0; j<data->_frameWarpArrayCount[i]; j++)
		{
			if (j) fputs(",", fp);
			glmsprintf(tmps, sizeof(tmps), "%f", data->_frameWarps[data->_frameWarpArrayStartIndex[i] + j]);
			fputs(tmps, fp);
		}
		fputs("]\n", fp);

		// expand entities
		fputs(",\"expands\":[", fp);
		for (j = 0; j<data->_expandArrayCount[i]; j++)
		{
			if (j) fputs(",", fp);
			glmsprintf(tmps, sizeof(tmps), "%f,%f,%f", data->_expands[data->_expandArrayStartIndex[i] + j][0],
				data->_expands[data->_expandArrayStartIndex[i] + j][1],
				data->_expands[data->_expandArrayStartIndex[i] + j][2]
				);
			fputs(tmps, fp);
		}
		fputs("]\n", fp);

		// expand entities
		fputs(",\"perFramePosOri\":[", fp);
		for (j = 0; j<data->_perFramePosOriArrayCount[i]; j++)
		{
			if (j) fputs(",", fp);
			glmsprintf(tmps, sizeof(tmps), "%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n", 
				data->_frameCachePos[data->_perFramePosOriArrayStartIndex[i] + j][0],
				data->_frameCachePos[data->_perFramePosOriArrayStartIndex[i] + j][1],
				data->_frameCachePos[data->_perFramePosOriArrayStartIndex[i] + j][2],

				data->_frameCurvePos[data->_perFramePosOriArrayStartIndex[i] + j][0],
				data->_frameCurvePos[data->_perFramePosOriArrayStartIndex[i] + j][1],
				data->_frameCurvePos[data->_perFramePosOriArrayStartIndex[i] + j][2],
				
				data->_framePos[data->_perFramePosOriArrayStartIndex[i] + j][0],
				data->_framePos[data->_perFramePosOriArrayStartIndex[i] + j][1],
				data->_framePos[data->_perFramePosOriArrayStartIndex[i] + j][2],

				data->_frameOri[data->_perFramePosOriArrayStartIndex[i] + j][0],
				data->_frameOri[data->_perFramePosOriArrayStartIndex[i] + j][1],
				data->_frameOri[data->_perFramePosOriArrayStartIndex[i] + j][2],
				data->_frameOri[data->_perFramePosOriArrayStartIndex[i] + j][3]
				);
			fputs(tmps, fp);
		}
		fputs("]\n", fp);

		// scale range entities
		fputs(",\"scaleRanges\":[", fp);
		for (j = 0; j<data->_scaleRangeArrayCount[i]; j++)
		{
			if (j) fputs(",", fp);
			glmsprintf(tmps, sizeof(tmps), "%f", data->_scaleRanges[data->_scaleRangeArrayStartIndex[i] + j]);
			fputs(tmps, fp);
		}
		fputs("]\n", fp);

		// posture frames
		if (data->_transformTypes[i] == SimulationCachePosture)
		{
			fputs(",\"postureFrames\":[", fp);
			for(j=0;j<data->_posturesFrameCount[i];j++)
			{
				if (j) fputs(",",fp);
				glmsprintf(tmps, sizeof(tmps), "%d", data->_posturesFrames[data->_posturesFrameStart[i] + j]);
				fputs(tmps, fp);
			}
			fputs("]\n",fp);
		}

		fputs(",\"snapTo\":[", fp);
		for (j = 0; j<data->_snapToCount[i]; j++)
		{
			if (j) fputs(",", fp);
			glmsprintf(tmps, sizeof(tmps), "{\"pos\":[%f,%f,%f],\"rot\":[%f,%f,%f,%f]}\n",
						data->_snapToPositions[data->_snapToStartIndex[i] + j][0], data->_snapToPositions[data->_snapToStartIndex[i] + j][1], data->_snapToPositions[data->_snapToStartIndex[i] + j][2],
						data->_snapToRotations[data->_snapToStartIndex[i] + j][0], data->_snapToRotations[data->_snapToStartIndex[i] + j][1], data->_snapToRotations[data->_snapToStartIndex[i] + j][2], data->_snapToRotations[data->_snapToStartIndex[i] + j][3]);
			fputs(tmps, fp);
		}
		fputs("]\n", fp);

		fputs("}", fp); // end transform object
	}
	fputs("\n]\n", fp);
	
	// posture edit bones
	if (data->_transformGroupCount)
	{
		fputs(",\"transformGroup\":[", fp);
		for (j = 0; j<data->_transformGroupCount; j++)
		{
			if (j)
				fputs(",\n", fp);
			glmsprintf(tmps, sizeof(tmps), "{\"groupName\":\"%s\",\"groupActive\":%d,\"groupFrom\":%d,\"groupTo\":%d}",
					   data->_transformGroupName[j],
					   data->_transformGroupActive[j],
					   data->_transformGroupBoundaries[j][0],
					   data->_transformGroupBoundaries[j][1]
					   );
			fputs(tmps, fp);
		}
		fputs("]\n", fp);
	}

	// posture edit bones
	if (data->_postureTotalBoneCount)
	{
		fputs(",\"postureBones\":[", fp);
		for(j=0;j<data->_postureTotalBoneCount;j++)
		{
			if (j) fputs(",",fp);
			glmsprintf(tmps, sizeof(tmps), "{\"pos\":[%f,%f,%f],\"ori\":[%f,%f,%f,%f]}\n", 
				data->_posturesPositions[j][0],data->_posturesPositions[j][1],data->_posturesPositions[j][2],
				data->_posturesOrientations[j][0],data->_posturesOrientations[j][1],data->_posturesOrientations[j][2],data->_posturesOrientations[j][3]);
			fputs(tmps, fp);
		}
		fputs("]\n",fp);
	}

	// hierarchy
	fputs(",\"boneHierarchies\":[", fp);
	for(j=0;j<data->_localBoneCount;j++)
	{
		if (j) fputs(",",fp);
		glmsprintf(tmps, sizeof(tmps), "{\"pos\":[%f,%f,%f],\"ori\":[%f,%f,%f,%f],\"parent\":%d}\n", 
			data->_localBonePosition[j][0],data->_localBonePosition[j][1],data->_localBonePosition[j][2],
			data->_localBoneOrientation[j][0],data->_localBoneOrientation[j][1],data->_localBoneOrientation[j][2],data->_localBoneOrientation[j][3],
			data->_localBoneParent[j]);
		fputs(tmps, fp);
	}
	fputs("]\n", fp);

	fputs(",\"localBoneOffset\":[", fp);
	for (j = 0; j<data->_localBoneOffsetCount; j++)
	{
		if (j) fputs(",", fp);
		glmsprintf(tmps, sizeof(tmps), "%d", data->_localBoneOffset[j]);
		fputs(tmps, fp);
	}
	fputs("]\n", fp);

	// asset override
	fputs(",\"assetsOverride\":[", fp);
	for (i = 0; i<data->_entityCount; i++)
	{
		if (data->_meshAssetsOverrideCount[i] && addComa)
		{
			fputs(",", fp);
			addComa = 0;
		}

		for (j = 0; j<data->_meshAssetsOverrideCount[i]; j++)
		{
			if (j) fputs(",", fp);
			glmsprintf(tmps, sizeof(tmps), "%d", data->_meshAssetsOverride[data->_meshAssetsOverrideStartIndex[i] + j]);
			fputs(tmps, fp);
		}

		if (data->_meshAssetsOverrideCount[i]) addComa = 1;
	}
	fputs("]\n", fp);

	// end file object
	fputs("}\n", fp);
	
	fclose(fp);

	return GSC_SUCCESS;
}

//----------------------------------------------------------------------------
int glmReadJSONFloatArray(struct json_value_s *value, float *floatArray, int expectedSize)
{
	int elementIndex = 0;
	struct json_array_s* jsonArray;
	struct json_array_element_s *objectElement;

	if (value->type != json_type_array)
		return 0;

	jsonArray = (struct json_array_s*)value->payload;
	if (expectedSize && (int)jsonArray->length != expectedSize)
		return 0;
	
	objectElement = jsonArray->start;
	while(objectElement)
	{
		struct json_value_s *elementValue = objectElement->value;
		if (elementValue->type == json_type_number)
			floatArray[elementIndex] = (float)atof(((struct json_number_s*)(elementValue->payload))->number);

		elementIndex++;
		objectElement = objectElement->next;
	};
	return elementIndex;
}

//----------------------------------------------------------------------------
int glmReadJSONPosOriArray(struct json_value_s *value, float *floatCurveArrayPos, float *floatCacheArrayPos, float *floatArrayPos, float *floatArrayOri, int expectedSize)
{
	static const int arrayIndex[13] = { 0,0,0, 1,1,1, 2,2,2, 3,3,3,3 };
	static const int arraySizes[13] = { 3,3,3, 3,3,3, 3,3,3, 4,4,4,4 };
	static const int arrayElementIndex[13] = { 0,1,2, 0,1,2, 0,1,2, 0,1,2,3 };
	static const int elementsModulo = sizeof(arrayIndex) / sizeof(int);

	int elementIndex = 0;
	struct json_array_s* jsonArray;
	struct json_array_element_s *objectElement;
	float * floatArrays[4];
	floatArrays[0] = floatCurveArrayPos;
	floatArrays[1] = floatCacheArrayPos;
	floatArrays[2] = floatArrayPos;
	floatArrays[3] = floatArrayOri;

	if (value->type != json_type_array)
		return 0;

	jsonArray = (struct json_array_s*)value->payload;
	if (expectedSize && (int)jsonArray->length != expectedSize)
		return 0;

	
	objectElement = jsonArray->start;
	while (objectElement)
	{
		struct json_value_s *elementValue = objectElement->value;
		if (elementValue->type == json_type_number)
		{
			int currentArrayIndex = arrayIndex[elementIndex%elementsModulo];
			int currentElementIndex = arrayElementIndex[elementIndex % elementsModulo];
			int itemIndex = (elementIndex / elementsModulo) * arraySizes[elementIndex % elementsModulo];
			floatArrays[currentArrayIndex][currentElementIndex + itemIndex] = (float)atof(((struct json_number_s*)(elementValue->payload))->number);
		}

		elementIndex++;
		objectElement = objectElement->next;
	};
	return elementIndex/ elementsModulo;
}

//----------------------------------------------------------------------------
int glmReadJSONInt64Array(struct json_value_s *value, int64_t *intArray)
{
	int elementIndex = 0;
	struct json_array_s* jsonArray;
	struct json_array_element_s *objectElement;

	if (value->type != json_type_array)
		return 0;

	jsonArray = (struct json_array_s*)value->payload;
	
	objectElement = jsonArray->start;
	while(objectElement)
	{
		struct json_value_s *elementValue = objectElement->value;
		if (elementValue->type == json_type_number)
			intArray[elementIndex] = atoi(((struct json_number_s*)(elementValue->payload))->number);

		elementIndex++;
		objectElement = objectElement->next;
	};

	return (int)jsonArray->length;
}

//----------------------------------------------------------------------------
int glmReadJSONIntArray(struct json_value_s *value, uint32_t *intArray)
{
	int elementIndex = 0;
	struct json_array_s* jsonArray;
	struct json_array_element_s *objectElement;

	if (value->type != json_type_array)
		return 0;

	jsonArray = (struct json_array_s*)value->payload;
	
	objectElement = jsonArray->start;
	while(objectElement)
	{
		struct json_value_s *elementValue = objectElement->value;
		if (elementValue->type == json_type_number)
			intArray[elementIndex] = atoi(((struct json_number_s*)(elementValue->payload))->number);

		elementIndex++;
		objectElement = objectElement->next;
	};

	return (int)jsonArray->length;
}

//----------------------------------------------------------------------------
GlmSimulationCacheStatus glmCreateAndReadHistoryJSON(GlmHistory** history, const char* file)
{
	GlmSimulationCacheStatus status = GSC_SUCCESS;
	int fileSize;
	GlmHistory* data = NULL;
	void *fileSource;
	uint32_t transformGroupCount = 0;
	uint32_t entityCount = 0;
	uint32_t transformCount = 0;
	uint32_t totalPostureCount = 0;
	uint32_t totalPostureBoneCount = 0;
	uint32_t localBoneCount = 0;
	uint32_t meshAssetsOverrideTotalCount = 0;
	uint32_t options = 0;
	uint32_t duplicatedEntityCount = 0;
	uint32_t entityTypeCount = 0;
	uint32_t frameOffsetCount = 0;
	uint32_t frameWarpCount = 0;
	uint32_t expandCount = 0;
	uint32_t scaleRangeCount = 0;
	uint32_t perFramePosOriCount = 0;
	uint32_t snapToTotalCount = 0;

	struct json_value_s *jsonvalue;
	int transformIndex = 0;
	int transformGroupIndex = 0;
	int entitiesStartIndex = 0;
	int meshAssetStartIndex = 0;
	int meshAssetCountIndex = 0;
	int boneHierarchyStartIndex = 0;
	int postureBoneStartIndex = 0;
	int postureFramesStartIndex = 0;
	int duplicatedEntitiesStartIndex = 0;
	int frameOffsetStartIndex = 0;
	int frameWarpStartIndex = 0;
	int expandStartIndex = 0;
	int scaleRangeStartIndex = 0;
	int perFramePosOriStartIndex = 0;
	int snapToStartIndex = 0;

#ifdef _MSC_VER				
	FILE* fp;
	errno_t err;
	err = fopen_s(&fp, file, "rb");
	if (err != 0) return GSC_FILE_OPEN_FAILED;
#else
	FILE* fp = fopen(file, "rb");
	if (fp == NULL) return GSC_FILE_OPEN_FAILED;
#endif

	if (fp == NULL) return GSC_FILE_OPEN_FAILED;

	fseek(fp, 0, SEEK_END);
	fileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	fileSource = GLMC_MALLOC(fileSize);
	fread(fileSource, 1, fileSize, fp);
	fclose(fp);

	jsonvalue = (struct json_value_s*)json_parse(fileSource, fileSize);
	if (jsonvalue && jsonvalue->type == json_type_object)
	{
		struct json_object_s *json_oject = (struct json_object_s*)(jsonvalue->payload);
		struct json_object_element_s* element = json_oject->start;
		while (element)
		{
			const char* elementName = (const char*)(element->name->string);
			struct json_value_s* elementValue = (struct json_value_s*)element->value;
			if (!strcmp(elementName, "transformGroupCount"))
			{
				if (elementValue->type == json_type_number)
				{
					struct json_number_s *jsonnumber = (struct json_number_s*)elementValue->payload;
					transformGroupCount = atoi(jsonnumber->number);
				}
			}
			else if (!strcmp(elementName,"entityCount"))
			{
				if (elementValue->type == json_type_number)
				{
					struct json_number_s *jsonnumber = (struct json_number_s*)elementValue->payload;
					entityCount = atoi(jsonnumber->number);
				}
			}
			else if (!strcmp(elementName, "duplicatedEntityCount"))
			{
				if (elementValue->type == json_type_number)
				{
					struct json_number_s *jsonnumber = (struct json_number_s*)elementValue->payload;
					duplicatedEntityCount = atoi(jsonnumber->number);
				}
			}
			else if (!strcmp(elementName,"postureCount"))
			{
				if (elementValue->type == json_type_number)
				{
					struct json_number_s *jsonnumber = (struct json_number_s*)elementValue->payload;
					totalPostureCount = atoi(jsonnumber->number);
				}
			}
			else if (!strcmp(elementName, "entityTypeCount"))
			{
				if (elementValue->type == json_type_number)
				{
					struct json_number_s *jsonnumber = (struct json_number_s*)elementValue->payload;
					entityTypeCount = atoi(jsonnumber->number);
				}
			}
			else if (!strcmp(elementName, "frameOffsetCount"))
			{
				if (elementValue->type == json_type_number)
				{
					struct json_number_s *jsonnumber = (struct json_number_s*)elementValue->payload;
					frameOffsetCount = atoi(jsonnumber->number);
				}
			}
			else if (!strcmp(elementName, "frameWarpCount"))
			{
				if (elementValue->type == json_type_number)
				{
					struct json_number_s *jsonnumber = (struct json_number_s*)elementValue->payload;
					frameWarpCount = atoi(jsonnumber->number);
				}
			}
			else if (!strcmp(elementName,"postureTotalBoneCount"))
			{
				if (elementValue->type == json_type_number)
				{
					struct json_number_s *jsonnumber = (struct json_number_s*)elementValue->payload;
					totalPostureBoneCount = atoi(jsonnumber->number);
				}
			}
			else if (!strcmp(elementName,"localBoneCount"))
			{
				if (elementValue->type == json_type_number)
				{
					struct json_number_s *jsonnumber = (struct json_number_s*)elementValue->payload;
					localBoneCount = atoi(jsonnumber->number);
				}
			}
			else if (!strcmp(elementName, "expandCount"))
			{
				if (elementValue->type == json_type_number)
				{
					struct json_number_s *jsonnumber = (struct json_number_s*)elementValue->payload;
					expandCount = atoi(jsonnumber->number);
				}
			}
			else if (!strcmp(elementName, "scaleRangeCount"))
			{
				if (elementValue->type == json_type_number)
				{
					struct json_number_s *jsonnumber = (struct json_number_s*)elementValue->payload;
					scaleRangeCount = atoi(jsonnumber->number);
				}
			}
			else if (!strcmp(elementName, "perFramePosOriCount"))
			{
				if (elementValue->type == json_type_number)
				{
					struct json_number_s *jsonnumber = (struct json_number_s*)elementValue->payload;
					perFramePosOriCount = atoi(jsonnumber->number);
				}
			}
			else if (!strcmp(elementName,"meshAssetsOverrideTotalCount"))
			{
				if (elementValue->type == json_type_number)
				{
					struct json_number_s *jsonnumber = (struct json_number_s*)elementValue->payload;
					meshAssetsOverrideTotalCount = atoi(jsonnumber->number);
				}
			}
			else if (!strcmp(elementName, "snapToTotalCount"))
			{
				if (elementValue->type == json_type_number)
				{
					struct json_number_s *jsonnumber = (struct json_number_s*)elementValue->payload;
					snapToTotalCount = atoi(jsonnumber->number);
				}
			}
			else if (!strcmp(elementName,"options"))
			{
				if (elementValue->type == json_type_number)
				{
					struct json_number_s *jsonnumber = (struct json_number_s*)elementValue->payload;
					options = atoi(jsonnumber->number);
				}
			}
			else if (!strcmp(elementName,"transformations"))
			{
				if (elementValue->type == json_type_array)
				{
					struct json_array_element_s* transformElement;
					struct json_array_s* jsonarray = (struct json_array_s*)elementValue->payload;
					transformCount = (uint32_t)jsonarray->length;

					glmCreateHistory(history, transformCount, transformGroupCount, entityCount, totalPostureCount, totalPostureBoneCount, localBoneCount, meshAssetsOverrideTotalCount, duplicatedEntityCount, entityTypeCount, expandCount, frameOffsetCount, frameWarpCount, scaleRangeCount, perFramePosOriCount, snapToTotalCount);
					data = *history;
					data->_options = options;

					transformElement = jsonarray->start;
					while (transformElement)
					{
						// inittransform
						data->_entityArrayCount[transformIndex] = 0;
						data->_entityArrayStartIndex[transformIndex] = entitiesStartIndex;

						data->_duplicatedEntityArrayCount[transformIndex] = 0;
						data->_duplicatedEntityArrayStartIndex[transformIndex] = duplicatedEntitiesStartIndex;

						data->_posturesFrameCount[transformIndex] = 0;
						data->_posturesFrameStart[transformIndex] = postureFramesStartIndex;

						data->_frameOffsetArrayCount[transformIndex] = 0;
						data->_frameOffsetArrayStartIndex[transformIndex] = frameOffsetStartIndex;

						data->_frameWarpArrayCount[transformIndex] = 0;
						data->_frameWarpArrayStartIndex[transformIndex] = frameWarpStartIndex;

						data->_expandArrayCount[transformIndex] = 0;
						data->_expandArrayStartIndex[transformIndex] = expandStartIndex;
						// get transform values
						data->_scaleRangeArrayCount[transformIndex] = 0;
						data->_scaleRangeArrayStartIndex[transformIndex] = scaleRangeStartIndex;

						data->_perFramePosOriArrayCount[transformIndex] = 0;
						data->_perFramePosOriArrayStartIndex[transformIndex] = perFramePosOriStartIndex;

						// get transform values
						if (transformElement->value->type == json_type_object)
						{
							struct json_object_s * transformObject = (struct json_object_s *)transformElement->value->payload;
							struct json_object_element_s* transformObjectElement = transformObject->start;
							while (transformObjectElement)
							{
								const char* transformObjectElementName = (const char*)(transformObjectElement->name->string);
								struct json_value_s * transformElementValue = transformObjectElement->value;
								size_t transformElementValueType = transformElementValue->type;
								if (!strcmp(transformObjectElementName,"transformType"))
								{
									if (transformElementValueType == json_type_number)
										data->_transformTypes[transformIndex] = (uint8_t)atoi(((struct json_number_s*)(transformElementValue->payload))->number);
								}
								else if (!strcmp(transformObjectElementName,"active"))
								{
									if (transformElementValueType == json_type_number)
										data->_active[transformIndex] = (uint8_t)atoi(((struct json_number_s*)(transformElementValue->payload))->number);
								}
								else if (!strcmp(transformObjectElementName,"boneIndex"))
								{
									if (transformElementValueType == json_type_number)
										data->_boneIndex[transformIndex] = (uint32_t)atoi(((struct json_number_s*)(transformElementValue->payload))->number);
								}
								else if (!strcmp(transformObjectElementName, "renderingTypeIdx"))
								{
									if (transformElementValueType == json_type_number)
										data->_renderingTypeIdx[transformIndex] = (uint32_t)atoi(((struct json_number_s*)(transformElementValue->payload))->number);
								}
								else if (!strcmp(transformObjectElementName,"scale"))
								{
									if (transformElementValueType == json_type_number)
										data->_scale[transformIndex] = (float)atof(((struct json_number_s*)(transformElementValue->payload))->number);
								}
								else if (!strcmp(transformObjectElementName, "frameWarpMin"))
								{
									if (transformElementValueType == json_type_number)
										data->_frameWarpMin[transformIndex] = (float)atof(((struct json_number_s*)(transformElementValue->payload))->number);
								}
								else if (!strcmp(transformObjectElementName, "frameWarpMax"))
								{
									if (transformElementValueType == json_type_number)
										data->_frameWarpMax[transformIndex] = (float)atof(((struct json_number_s*)(transformElementValue->payload))->number);
								}
								else if (!strcmp(transformObjectElementName, "frameOffsetMin"))
								{
									if (transformElementValueType == json_type_number)
										data->_frameOffsetMin[transformIndex] = (float)atof(((struct json_number_s*)(transformElementValue->payload))->number);
								}
								else if (!strcmp(transformObjectElementName, "frameOffsetMax"))
								{
									if (transformElementValueType == json_type_number)
										data->_frameOffsetMax[transformIndex] = (float)atof(((struct json_number_s*)(transformElementValue->payload))->number);
								}
								else if (!strcmp(transformObjectElementName, "scaleRangeMin"))
								{
									if (transformElementValueType == json_type_number)
										data->_scaleRangeMin[transformIndex] = (float)atof(((struct json_number_s*)(transformElementValue->payload))->number);
								}
								else if (!strcmp(transformObjectElementName, "scaleRangeMax"))
								{
									if (transformElementValueType == json_type_number)
										data->_scaleRangeMax[transformIndex] = (float)atof(((struct json_number_s*)(transformElementValue->payload))->number);
								}
								else if (!strcmp(transformObjectElementName, "startFrame"))
								{
									if (transformElementValueType == json_type_number)
										data->_startFrame[transformIndex] = (uint32_t)atoi(((struct json_number_s*)(transformElementValue->payload))->number);
								}
								else if (!strcmp(transformObjectElementName, "frameCount"))
								{
									if (transformElementValueType == json_type_number)
										data->_frameCount[transformIndex] = (uint32_t)atoi(((struct json_number_s*)(transformElementValue->payload))->number);
								}
								else if (!strcmp(transformObjectElementName, "trajectoryMode"))
								{
									if (transformElementValueType == json_type_number)
										data->_trajectoryMode[transformIndex] = (uint32_t)atoi(((struct json_number_s*)(transformElementValue->payload))->number);
								}
								else if (!strcmp(transformObjectElementName, "trajectorySteps"))
								{
									if (transformElementValueType == json_type_number)
										data->_trajectorySteps[transformIndex] = (uint32_t)atoi(((struct json_number_s*)(transformElementValue->payload))->number);
								}
								else if (!strcmp(transformObjectElementName, "smoothIterations"))
								{
									if (transformElementValueType == json_type_number)
										data->_smoothIterationCount[transformIndex] = (uint32_t)atoi(((struct json_number_s*)(transformElementValue->payload))->number);
								}
								else if (!strcmp(transformObjectElementName, "smoothRatio"))
								{
									if (transformElementValueType == json_type_number)
										data->_smoothFrontBackRatio[transformIndex] = (float)atof(((struct json_number_s*)(transformElementValue->payload))->number);
								}
								else if (!strcmp(transformObjectElementName, "smoothX"))
								{
									if (transformElementValueType == json_type_number)
										data->_smoothComponents[transformIndex][0] = (float)atof(((struct json_number_s*)(transformElementValue->payload))->number);
								}
								else if (!strcmp(transformObjectElementName, "smoothY"))
								{
									if (transformElementValueType == json_type_number)
										data->_smoothComponents[transformIndex][1] = (float)atof(((struct json_number_s*)(transformElementValue->payload))->number);
								}
								else if (!strcmp(transformObjectElementName, "smoothZ"))
								{
									if (transformElementValueType == json_type_number)
										data->_smoothComponents[transformIndex][2] = (float)atof(((struct json_number_s*)(transformElementValue->payload))->number);
								}
								else if (!strcmp(transformObjectElementName,"rotate"))
								{
									glmReadJSONFloatArray(transformElementValue, data->_transformRotate[transformIndex], 4);
								}
								else if (!strcmp(transformObjectElementName,"translate"))
								{
									glmReadJSONFloatArray(transformElementValue, data->_transformTranslate[transformIndex], 3);
								}
								else if (!strcmp(transformObjectElementName,"pivot"))
								{
									glmReadJSONFloatArray(transformElementValue, data->_transformPivot[transformIndex], 3);
								}
								else if (!strcmp(transformObjectElementName, "clothIndice"))
								{
									if (transformElementValueType == json_type_number)
										data->_clothIndice[transformIndex] = (uint32_t)atoi(((struct json_number_s*)(transformElementValue->payload))->number);
								}
								else if (!strcmp(transformObjectElementName, "enableCloth"))
								{
									if (transformElementValueType == json_type_number)
										data->_enableCloth[transformIndex] = (uint32_t)atoi(((struct json_number_s*)(transformElementValue->payload))->number);
								}
								else if (!strcmp(transformObjectElementName,"entities"))
								{
									uint32_t entityCount = (uint32_t)glmReadJSONInt64Array(transformElementValue, &data->_entityIds[entitiesStartIndex]);
									data->_entityArrayCount[transformIndex] = entityCount;
									entitiesStartIndex += entityCount;
								}
								else if (!strcmp(transformObjectElementName, "meshAssetStart"))
								{
									uint32_t entityCount = (uint32_t)glmReadJSONIntArray(transformElementValue, &data->_meshAssetsOverrideStartIndex[meshAssetStartIndex]);
									meshAssetStartIndex += entityCount;
								}
								else if (!strcmp(transformObjectElementName, "meshAssetCount"))
								{
									uint32_t entityCount = (uint32_t)glmReadJSONIntArray(transformElementValue, &data->_meshAssetsOverrideCount[meshAssetCountIndex]);
									meshAssetCountIndex += entityCount;
								}
								else if (!strcmp(transformObjectElementName, "duplicatedEntities"))
								{
									uint32_t duplicatedEntityCount = (uint32_t)glmReadJSONInt64Array(transformElementValue, &data->_duplicatedEntityIds[duplicatedEntitiesStartIndex]);
									data->_duplicatedEntityArrayCount[transformIndex] = duplicatedEntityCount;
									duplicatedEntitiesStartIndex += duplicatedEntityCount;
								}
								else if (!strcmp(transformObjectElementName, "expands"))
								{
									uint32_t expandCount = (uint32_t)glmReadJSONFloatArray(transformElementValue, (float*)&data->_expands[expandStartIndex], 0);
									data->_expandArrayCount[transformIndex] = expandCount/3;
									expandStartIndex += expandCount / 3;
								}
								else if (!strcmp(transformObjectElementName, "scaleRanges"))
								{
									uint32_t scaleRangeCount = (uint32_t)glmReadJSONFloatArray(transformElementValue, (float*)&data->_scaleRanges[scaleRangeStartIndex], 0);
									data->_scaleRangeArrayCount[transformIndex] = scaleRangeCount;
									scaleRangeStartIndex += scaleRangeCount;
								}
								else if (!strcmp(transformObjectElementName, "perFramePosOri"))
								{
									uint32_t perFramePosOriCount = (uint32_t)glmReadJSONPosOriArray(transformElementValue, 
										(float*)&data->_frameCachePos[perFramePosOriStartIndex],
										(float*)&data->_frameCurvePos[perFramePosOriStartIndex],
										(float*)&data->_framePos[perFramePosOriStartIndex],
										(float*)&data->_frameOri[perFramePosOriStartIndex], 0);

									data->_perFramePosOriArrayCount[transformIndex] = perFramePosOriCount;
									perFramePosOriStartIndex += perFramePosOriCount;
								}
								else if (!strcmp(transformObjectElementName,"postureFrames"))
								{
									uint32_t postureCount = (uint32_t)glmReadJSONIntArray(transformElementValue, &data->_posturesFrames[postureFramesStartIndex]);
									data->_posturesFrameCount[transformIndex] = postureCount;
									postureFramesStartIndex += postureCount;
								}
								else if (!strcmp(transformObjectElementName, "frameOffsets"))
								{
									size_t localFrameOffsetCount = ((struct json_array_s*)transformElementValue->payload)->length;
									glmReadJSONFloatArray(transformElementValue, &data->_frameOffsets[frameOffsetStartIndex], (int)localFrameOffsetCount);
									data->_frameOffsetArrayCount[transformIndex] = (uint32_t)localFrameOffsetCount;
									frameOffsetStartIndex += (uint32_t)localFrameOffsetCount;
								}
								else if (!strcmp(transformObjectElementName, "frameWarps"))
								{
									size_t localFrameWarpCount = ((struct json_array_s*)transformElementValue->payload)->length;
									glmReadJSONFloatArray(transformElementValue, &data->_frameWarps[frameWarpStartIndex], (int)localFrameWarpCount);
									data->_frameWarpArrayCount[transformIndex] = (uint32_t)localFrameWarpCount;
									frameWarpStartIndex += (uint32_t)localFrameWarpCount;
								}
								else if (!strcmp(transformObjectElementName, "snapToTarget"))
								{
									if (transformElementValueType == json_type_string)
									{
										struct json_string_s *jsonstring = (struct json_string_s*)transformElementValue->payload;
										const char* snapToTarget = (const char*)(jsonstring->string);
										memcpy(data->_snapToTarget[transformIndex], snapToTarget, jsonstring->string_size + 1);
										data->_snapToTarget[transformIndex][jsonstring->string_size] = '\0';
									}
								}
								else if (!strcmp(transformObjectElementName, "shaderAttribute"))
								{
									if (transformElementValueType == json_type_string)
									{
										struct json_string_s *jsonstring = (struct json_string_s*)transformElementValue->payload;
										const char* shaderAttribute = (const char*)(jsonstring->string);
										memcpy(data->_shaderAttribute[transformIndex], shaderAttribute, jsonstring->string_size + 1);
										data->_shaderAttribute[transformIndex][jsonstring->string_size] = '\0';
									}
								}
								else if (!strcmp(transformObjectElementName, "snapToStartIndex"))
								{
									if (transformElementValueType == json_type_number)
										data->_snapToStartIndex[transformIndex] = (uint32_t)atoi(((struct json_number_s*)(transformElementValue->payload))->number);
								}
								else if (!strcmp(transformObjectElementName, "snapToCount"))
								{
									if (transformElementValueType == json_type_number)
										data->_snapToCount[transformIndex] = (uint32_t)atoi(((struct json_number_s*)(transformElementValue->payload))->number);
								}
								else if (!strcmp(transformObjectElementName, "snapTo"))
								{
									if (transformElementValueType == json_type_array)
									{
										struct json_array_element_s * snapToElement;
										struct json_array_s* jsonarray = (struct json_array_s*)transformElementValue->payload;

										snapToElement = jsonarray->start;
										while (snapToElement)
										{
											if (snapToElement->value->type == json_type_object)
											{
												struct json_object_s * snapToElementObject = (struct json_object_s *)snapToElement->value->payload;
												struct json_object_element_s* snapToElementElement = snapToElementObject->start;
												while (snapToElementElement)
												{
													const char* snapToElementElementName = (const char*)(snapToElementElement->name->string);

													if (!strcmp(snapToElementElementName, "pos"))
														glmReadJSONFloatArray(snapToElementElement->value, data->_snapToPositions[snapToStartIndex], 3);
													else if (!strcmp(snapToElementElementName, "rot"))
														glmReadJSONFloatArray(snapToElementElement->value, data->_snapToRotations[snapToStartIndex], 4);

													snapToElementElement = snapToElementElement->next;
												}; // snapToElementElement
											}
											snapToStartIndex++;
											snapToElement = snapToElement->next;
										}; // snapToElement
									}
								}

								transformObjectElement = transformObjectElement->next;
							}
						}
						transformElement = transformElement->next;
						transformIndex++;
					} // while(transformElement)
				}
			}
			else if (!strcmp(elementName, "transformGroup"))
			{
				if (elementValue->type == json_type_array)
				{
					struct json_array_element_s * transformGroupElement;
					struct json_array_s* jsonarray = (struct json_array_s*)elementValue->payload;

					transformGroupElement = jsonarray->start;
					while (transformGroupElement)
					{
						if (transformGroupElement->value->type == json_type_object)
						{
							struct json_object_s * transformGroupElementObject = (struct json_object_s *)transformGroupElement->value->payload;
							struct json_object_element_s* transformGroupElementElement = transformGroupElementObject->start;
							while (transformGroupElementElement)
							{
								const char* transformGroupElementElementName = (const char*)(transformGroupElementElement->name->string);
								struct json_value_s * transformGroupElementElementValue = transformGroupElementElement->value;
								size_t transformGroupElementElementValueType = transformGroupElementElementValue->type;

								if (!strcmp(transformGroupElementElementName, "groupName"))
								{
									if (transformGroupElementElementValueType == json_type_string)
									{
										struct json_string_s *jsonstring = (struct json_string_s*)transformGroupElementElementValue->payload;
										const char* groupName = (const char*)(jsonstring->string);
										memcpy(data->_transformGroupName[transformGroupIndex], groupName, jsonstring->string_size + 1);
										data->_transformGroupName[transformGroupIndex][jsonstring->string_size] = '\0';
									}
								}
								else if (!strcmp(transformGroupElementElementName, "groupActive"))
								{
									if (transformGroupElementElementValueType == json_type_number)
										data->_transformGroupActive[transformGroupIndex] = (uint8_t)atoi(((struct json_number_s*)(transformGroupElementElementValue->payload))->number);
								}
								else if (!strcmp(transformGroupElementElementName, "groupFrom"))
								{
									if (transformGroupElementElementValueType == json_type_number)
										data->_transformGroupBoundaries[transformGroupIndex][0] = atoi(((struct json_number_s*)(transformGroupElementElementValue->payload))->number);
								}
								else if (!strcmp(transformGroupElementElementName, "groupTo"))
								{ 
									if (transformGroupElementElementValueType == json_type_number)
										data->_transformGroupBoundaries[transformGroupIndex][1] = atoi(((struct json_number_s*)(transformGroupElementElementValue->payload))->number);
								}

								transformGroupElementElement = transformGroupElementElement->next;
							}; // transformGroupElementElement
						}
						transformGroupIndex++;
						transformGroupElement = transformGroupElement->next;
					}; // transformGroupElement
				}
			}
			else if (!strcmp(elementName,"postureBones"))
			{
				if (elementValue->type == json_type_array)
				{
					struct json_array_element_s * postureBoneElement;
					struct json_array_s* jsonarray = (struct json_array_s*)elementValue->payload;

					postureBoneElement = jsonarray->start;
					while(postureBoneElement)
					{
						if (postureBoneElement->value->type == json_type_object)
						{
							struct json_object_s * postureBoneElementObject = (struct json_object_s *)postureBoneElement->value->payload;
							struct json_object_element_s* postureBoneElementElement = postureBoneElementObject->start;
							while (postureBoneElementElement)
							{
								const char* postureBoneElementElementName = (const char*)(postureBoneElementElement->name->string);

								if (!strcmp(postureBoneElementElementName,"pos"))
									glmReadJSONFloatArray(postureBoneElementElement->value, data->_posturesPositions[postureBoneStartIndex], 3);
								else if (!strcmp(postureBoneElementElementName,"ori"))
									glmReadJSONFloatArray(postureBoneElementElement->value, data->_posturesOrientations[postureBoneStartIndex], 4);

								postureBoneElementElement = postureBoneElementElement->next;
							}; // postureBoneElementElement
						}
						postureBoneStartIndex++;
						postureBoneElement = postureBoneElement->next;
					}; // postureBoneElement
				}
			}
			else if (!strcmp(elementName,"boneHierarchies"))
			{
				if (elementValue->type == json_type_array)
				{
					struct json_array_element_s * boneHierarchyElement;
					struct json_array_s* jsonarray = (struct json_array_s*)elementValue->payload;
					localBoneCount = (uint32_t)jsonarray->length;

					boneHierarchyElement = jsonarray->start;
					while(boneHierarchyElement)
					{
						if (boneHierarchyElement->value->type == json_type_object)
						{
							struct json_object_s * boneHierarchyElementObject = (struct json_object_s *)boneHierarchyElement->value->payload;
							struct json_object_element_s* boneHierarchyElementElement = boneHierarchyElementObject->start;
							while (boneHierarchyElementElement)
							{
								const char* boneHierarchyElementElementName = (const char*)(boneHierarchyElementElement->name->string);

								if (!strcmp(boneHierarchyElementElementName,"pos"))
									glmReadJSONFloatArray(boneHierarchyElementElement->value, data->_localBonePosition[boneHierarchyStartIndex], 3);
								else if (!strcmp(boneHierarchyElementElementName, "ori"))
									glmReadJSONFloatArray(boneHierarchyElementElement->value, data->_localBoneOrientation[boneHierarchyStartIndex], 4);
								else if (!strcmp(boneHierarchyElementElementName, "parent"))
									data->_localBoneParent[boneHierarchyStartIndex] = atoi(((struct json_number_s*)(boneHierarchyElementElement->value->payload))->number);

								boneHierarchyElementElement = boneHierarchyElementElement->next;
							}; // boneHierarchyElementElement
						}
						boneHierarchyStartIndex++;
						boneHierarchyElement = boneHierarchyElement->next;
					}; // boneHierarchyElement
				}
			}
			else if (!strcmp(elementName, "localBoneOffset"))
			{
				if (elementValue->type == json_type_array)
				{
					int boneOffsetIndex = 0;
					struct json_array_element_s * boneOffsetElement;
					struct json_array_s* jsonarray = (struct json_array_s*)elementValue->payload;
					
					boneOffsetElement = jsonarray->start;
					while (boneOffsetElement)
					{
						data->_localBoneOffset[boneOffsetIndex] = atoi(((struct json_number_s*)(boneOffsetElement->value->payload))->number);
						boneOffsetIndex++;

						boneOffsetElement = boneOffsetElement->next;
					}; // boneHierarchyElement
				}
			}
			else if (!strcmp(elementName, "assetsOverride"))
			{
				if (elementValue->type == json_type_array)
				{
					int assetsIndex = 0;
					struct json_array_element_s * assetsOverrideElement;
					struct json_array_s* jsonarray = (struct json_array_s*)elementValue->payload;
					
					assetsOverrideElement = jsonarray->start;
					while (assetsOverrideElement)
					{
						data->_meshAssetsOverride[assetsIndex] = atoi(((struct json_number_s*)(assetsOverrideElement->value->payload))->number);
						assetsIndex++;

						assetsOverrideElement = assetsOverrideElement->next;
					}; // assetsOverrideElement
				}
			}
			
			element = element->next;
		};
	}
	else
	{
		status = GSC_FILE_FORMAT_ERROR;
	}

	GLMC_FREE(fileSource);
	return status;
}
#endif
void glmSetIdentityMatrix(float *matrix)
{
	matrix[0] = 1.f; matrix[1] = 0.f; matrix[2] = 0.f; matrix[3] = 0.f;
	matrix[4] = 0.f; matrix[5] = 1.f; matrix[6] = 0.f; matrix[7] = 0.f;
	matrix[8] = 0.f; matrix[9] = 0.f; matrix[10]= 1.f; matrix[11]= 0.f;
	matrix[12]= 0.f; matrix[13]= 0.f; matrix[14]= 0.f; matrix[15]= 1.f;
}

void glmSetIdentityQuaternion(float *quaternion)
{
	quaternion[0] = 0.f; quaternion[1] = 0.f; quaternion[2] = 0.f; quaternion[3] = 1.f;
}

int glmFindEntityInSimulation(GlmEntityTransform* entityTransforms, unsigned int entityTransformCount, int64_t entityId)
{
	unsigned int i;
	for (i = 0;i<entityTransformCount;i++)
	{
		if (entityTransforms[i]._entityId == entityId)
			return i;
	}
	return -1;
}

void glmConvertMatrix(float* result, const float *translation, const float *rotation)
{
	// rotation
	float twoX  = 2.f*rotation[0];
	float twoY  = 2.f*rotation[1];
	float twoZ  = 2.f*rotation[2];
	float twoWX = twoX*rotation[3];
	float twoWY = twoY*rotation[3];
	float twoWZ = twoZ*rotation[3];
	float twoXX = twoX*rotation[0];
	float twoXY = twoY*rotation[0];
	float twoXZ = twoZ*rotation[0];
	float twoYY = twoY*rotation[1];
	float twoYZ = twoZ*rotation[1];
	float twoZZ = twoZ*rotation[2];

	result[0] = 1 - (twoYY + twoZZ);
	result[4] = twoXY - twoWZ;
	result[8] = twoXZ + twoWY;
	result[1] = twoXY + twoWZ;
	result[5] = 1 - (twoXX + twoZZ);
	result[9] = twoYZ - twoWX;
	result[2] = twoXZ - twoWY;
	result[6] = twoYZ + twoWX;
	result[10] = 1 - (twoXX + twoYY);

	result[3] = 0;
	result[7] = 0;
	result[11] = 0;
	
	// translation
	result[12] = translation[0];
	result[13] = translation[1];
	result[14] = translation[2];

	result[15] = 1;
}

// r = a*b;
void glmMultMatrix(const float *a, const float *b, float *r)
{
	r[0] = a[0]*b[0] + a[1]*b[4] + a[2]*b[8]  + a[3]*b[12];
	r[1] = a[0]*b[1] + a[1]*b[5] + a[2]*b[9]  + a[3]*b[13];
	r[2] = a[0]*b[2] + a[1]*b[6] + a[2]*b[10] + a[3]*b[14];
	r[3] = a[0]*b[3] + a[1]*b[7] + a[2]*b[11] + a[3]*b[15];

	r[4] = a[4]*b[0] + a[5]*b[4] + a[6]*b[8]  + a[7]*b[12];
	r[5] = a[4]*b[1] + a[5]*b[5] + a[6]*b[9]  + a[7]*b[13];
	r[6] = a[4]*b[2] + a[5]*b[6] + a[6]*b[10] + a[7]*b[14];
	r[7] = a[4]*b[3] + a[5]*b[7] + a[6]*b[11] + a[7]*b[15];

	r[8] = a[8]*b[0] + a[9]*b[4] + a[10]*b[8] + a[11]*b[12];
	r[9] = a[8]*b[1] + a[9]*b[5] + a[10]*b[9] + a[11]*b[13];
	r[10]= a[8]*b[2] + a[9]*b[6] + a[10]*b[10]+ a[11]*b[14];
	r[11]= a[8]*b[3] + a[9]*b[7] + a[10]*b[11]+ a[11]*b[15];

	r[12]= a[12]*b[0]+ a[13]*b[4]+ a[14]*b[8] + a[15]*b[12];
	r[13]= a[12]*b[1]+ a[13]*b[5]+ a[14]*b[9] + a[15]*b[13];
	r[14]= a[12]*b[2]+ a[13]*b[6]+ a[14]*b[10]+ a[15]*b[14];
	r[15]= a[12]*b[3]+ a[13]*b[7]+ a[14]*b[11]+ a[15]*b[15];
}

//-------------------------------------------------------------------------
void glmTransformPoint(const float *point, const float *matrix, float * destination)
{
	destination[0] = point[0] * matrix[0*4+0] + point[1] * matrix[1*4+0] + point[2] * matrix[2*4+0] + matrix[3*4+0] ;
	destination[1] = point[0] * matrix[0*4+1] + point[1] * matrix[1*4+1] + point[2] * matrix[2*4+1] + matrix[3*4+1] ;
	destination[2] = point[0] * matrix[0*4+2] + point[1] * matrix[1*4+2] + point[2] * matrix[2*4+2] + matrix[3*4+2] ;
	//destination[3] = point[0] * matrix[0*4+3] + point[1] * matrix[1*4+3] + point[2] * matrix[2*4+3] + matrix[3*4+3] ;
}

//-------------------------------------------------------------------------
void glmMultQuaternion (const float *a ,const float *b, float *r)
{
	float ww = (a[2] + a[0]) * (b[0] + b[1]);
	float yy = (a[3] - a[1]) * (b[3] + b[2]);
	float zz = (a[3] + a[1]) * (b[3] - b[2]);
	float xx = ww + yy + zz;
	float qq = 0.5f * (xx + (a[2] - a[0]) * (b[0] - b[1]));

	r[3] = qq - ww + (a[2] - a[1]) * (b[1] - b[2]);
	r[0] = qq - xx + (a[0] + a[3]) * (b[0] + b[3]);
	r[1] = qq - yy + (a[3] - a[0]) * (b[1] + b[2]);
	r[2] = qq - zz + (a[2] + a[1]) * (b[3] - b[0]);
}

//-------------------------------------------------------------------------
void glmInverseQuaternion (const float *s, float *r)
{
	r[0] = s[0];
	r[1] = s[1];
	r[2] = s[2];
	r[3] =-s[3];
}

//-------------------------------------------------------------------------
void glmNormalizeQuaternion(float *r)
{
	static const float very_small_float = 1.0e-037f; // from http://altdevblogaday.com/2011/08/21/practical-flt-point-tricks/, adding a very small float avoid testing if == 0
	float factor = (1.f / sqrtf(r[0]*r[0] + r[1]*r[1] + r[2]*r[2] + r[3]*r[3] + very_small_float));
	r[0] *= factor;
	r[1] *= factor;
	r[2] *= factor;
	r[3] *= factor;
}


void glmBuildQuaternion(float angle, const float * axis, float *r)
{
	float sinAngleDiv2 = sinf(angle * 0.5f);
	float cosAngleDiv2 = cosf(angle * 0.5f);
	r[0] = sinAngleDiv2 * axis[0];
	r[1] = sinAngleDiv2 * axis[1];
	r[2] = sinAngleDiv2 * axis[2];
	r[3] = cosAngleDiv2;
}

//-------------------------------------------------------------------------
void glmNormalizeVec3(float *r)
{
	static const float very_small_float = 1.0e-037f; // from http://altdevblogaday.com/2011/08/21/practical-flt-point-tricks/, adding a very small float avoid testing if == 0
	float factor = (1.f / sqrtf(r[0]*r[0] + r[1]*r[1] + r[2]*r[2] + very_small_float));
	r[0] *= factor;
	r[1] *= factor;
	r[2] *= factor;
}

//-------------------------------------------------------------------------
//	Quaternion result;     
//	Vector3 H = from;
//	H += to;     
//	H.normalizeSelf();  

//	result->w = (from.x * H.x) + (from.y * H.y) + (from.z * H.z);     
//	result->x = from.y*H.z - from.z*H.y;     
//	result->y = from.z*H.x - from.x*H.z;     
//	result->z = from.x*H.y - from.y*H.x;     
//	return result;
//}
void glmRotationBetweenUnitVectors(const float* from, const float* to, float *r)
{
	// From http://www.gamedev.net/topic/429507-finding-the-quaternion-betwee-two-vectors/?p=3856228#entry3856228
	// and http://www.ogre3d.org/docs/api/html/OgreVector3_8h_source.html for limit cases
	float dotProduct = from[0] * to[0] + from[1] * to[1] + from[2] * to[2];

	if (dotProduct >= 1.0f - 1e-6f)
	{
		// aligned vectors
		r[0] = r[1] = r[2] = 0.f; r[3] = 1;
		return ;
	}

	if (dotProduct < 1e-6f - 1.0f)
	{
		// opposite vectors

		// find an axis
		float axis[3];
		if (fabsf(fabsf(from[0]) - 1.f) < GLMC_EPSILON)
		{
			// from is x aligned
			axis[0] = 0.f;
			axis[1] = 0.f;
			axis[2] = 1.f;
		}
		else
		{
			// from is not x aligned
			// axis = from ^ (1, 0, 0)
			axis[0] = 0;
			axis[1] = from[2];
			axis[2] = -from[1];
			glmNormalizeVec3(axis);
		}
		glmBuildQuaternion(GLMC_PI, axis, r);
		return;
	}


	r[0] = from[1] * to[2] - from[2] * to[1]; // cross product
	r[1] = from[2] * to[0] - from[0] * to[2]; // cross product
	r[2] = from[0] * to[1] - from[1] * to[0]; // cross product
	r[3] = 1.f + dotProduct;

	glmNormalizeQuaternion(r);
}


//-------------------------------------------------------------------------
void glmAddVec3(const float *a, const float *b, float *r)
{
	r[0] = a[0] + b[0];
	r[1] = a[1] + b[1];
	r[2] = a[2] + b[2];
}

//-------------------------------------------------------------------------
void glmMultVec3Scalar(const float *a, const float s, float *r)
{
	r[0] = a[0] * s;
	r[1] = a[1] * s;
	r[2] = a[2] * s;
}

//-------------------------------------------------------------------------
void glmMemberToMemberProduct(const float *a, const float *b, float *r)
{
	r[0] = a[0] * b[0];
	r[1] = a[1] * b[1];
	r[2] = a[2] * b[2];
}

//-------------------------------------------------------------------------
void glmMultVec3Quaternion(const float *rot, const float *pos, float *r)
{
	float	x2 = (rot[0] + rot[0]);
	float	y2 = (rot[1] + rot[1]);
	float	z2 = (rot[2] + rot[2]);
	float	xx = (rot[0] * x2);
	float	xy = (rot[0] * y2);
	float	xz = (rot[0] * z2);
	float	yy = (rot[1] * y2);
	float	yz = (rot[1] * z2);
	float	zz = (rot[2] * z2);
	float	wx = (rot[3] * x2);
	float	wy = (rot[3] * y2);
	float	wz = (rot[3] * z2);

	r[0] = (1.f - (yy + zz)) * pos[0] + (xy - wz) * pos[1] + (xz + wy) * pos[2];
	r[1] = (xy + wz) * pos[0] + (1.f - (xx + zz))	* pos[1] + (yz - wx)* pos[2];
	r[2] = (xz - wy) * pos[0] + (yz + wx) * pos[1] + (1.f - (xx + yy)) * pos[2];
}


float interpolateFloat(float value1, float value2, float ratio)
{
	return value1 + (value2 - value1) * ratio;
}

//-------------------------------------------------------------------------
void interpolateNFloats(float* value1, float *value2, float ratio, float *res, int count)
{
	int i;
	for (i = 0; i < count; i++)
	{
		res[i] = value1[i] + (value2[i] - value1[i]) * ratio;
	}
}

//-------------------------------------------------------------------------
void glmComputeRestRelativesOrientationFromPosture(const float(*worldOrientations)[4], const float(*boneLocalOri)[4], uint32_t *parentIndex, const float(worldOrientation)[4], float(*restRelativeOrientations)[4], unsigned int boneCount)
{
	float worldOrientationInverse[4];
	unsigned int i;

	//root bone:
	//worldPositions[0] = worldPosition + worldOrientation * bones[0]->getLocalPos();
	glmInverseQuaternion(worldOrientation, worldOrientationInverse);
	glmMultQuaternion(worldOrientationInverse, &(*worldOrientations)[0], &(*restRelativeOrientations)[0]);
	glmNormalizeQuaternion(restRelativeOrientations[0]);
	//other bones:
	//WO(bone) = WO(bone->father) * bone->getLocalOri() * PO(bone)
	//WP(bone) = WP(bone->father) + WO(bone->father) * bone->getLocalPos()
	for (i = 1; i < boneCount; ++i)
	{
		float parentOri[4];
		float parentOriInverse[4];
		int boneParentIndex = parentIndex[i]>>16;
		glmMultQuaternion( &*worldOrientations[boneParentIndex], boneLocalOri[i], parentOri);
		glmInverseQuaternion(parentOri, parentOriInverse);
		glmMultQuaternion( parentOriInverse, worldOrientations[i], restRelativeOrientations[i]);
		glmNormalizeQuaternion(restRelativeOrientations[i]);
	}
}

void glmComputePostureFromRestRelativeOrientations(float(*worldPositions)[3], float(*worldOrientations)[4], const float(*boneLocalOri)[4], const float(*boneLocalPosUnscaled)[3], uint32_t *parentIndex, float worldPosition[3], float worldOrientation[4], const float(*restRelativeOrientations)[4], const float skeletonScale, unsigned int boneCount)
{
	unsigned int i;
	//root bone:
	memcpy(worldOrientations[0], worldOrientation, sizeof(float) *4);	// * restRelativeOrientations[0]; //NOTE: bones[0]->getLocalOri() is pre-multiplied in restRelativeOrientations[0]
	memcpy(worldPositions[0], worldPosition, sizeof(float) *3);	// + worldOrientation * bones[0]->getLocalPosUnscaled() * skeletonScale;

	//other bones:
	//WO(bone) = WO(bone->father) * bone->getLocalOri() * PO(bone)
	//WP(bone) = WP(bone->father) + WO(bone->father) * bone->getLocalPos()
	for (i = 1; i < boneCount; ++i)
	{
		float ori[4];
		float pos[3];
		float posScaled[3];

		int boneParentIndex = parentIndex[i]>>16;
		
		glmMultQuaternion( worldOrientations[boneParentIndex], boneLocalOri[i], ori);
		glmMultQuaternion( ori, restRelativeOrientations[i], worldOrientations[i]);

		glmMultVec3Scalar(boneLocalPosUnscaled[i], skeletonScale, posScaled);
		glmMultVec3Quaternion(worldOrientations[boneParentIndex], posScaled, pos);
		glmAddVec3(worldPositions[boneParentIndex], pos, worldPositions[i]);
	}
}

void glmComputePostureFromRestRelativeOrientationsScales(float(*worldPositions)[3], float(*worldOrientations)[4], const float(*boneLocalOri)[4], const float(*boneLocalPosUnscaled)[3], uint32_t *parentIndex, float worldPosition[3], float worldOrientation[4], const float(*restRelativeOrientations)[4], const float skeletonScale, unsigned int boneCount, const float(*boneScales)[4])
{
	unsigned int i;
	//root bone:
	memcpy(worldOrientations[0], worldOrientation, sizeof(float) * 4);	// * restRelativeOrientations[0]; //NOTE: bones[0]->getLocalOri() is pre-multiplied in restRelativeOrientations[0]
	memcpy(worldPositions[0], worldPosition, sizeof(float) * 3);	// + worldOrientation * bones[0]->getLocalPosUnscaled() * skeletonScale;

	//other bones:
	//WO(bone) = WO(bone->father) * bone->getLocalOri() * PO(bone)
	//WP(bone) = WP(bone->father) + WO(bone->father) * bone->getLocalPos()
	for (i = 1; i < boneCount; ++i)
	{
		float ori[4];
		float pos[3];

		float localPos[3];
		float posScaled[3];
		
		int boneParentIndex = parentIndex[i]>>16;
		glmMultQuaternion( &*worldOrientations[boneParentIndex], boneLocalOri[i], ori);
		glmMultQuaternion( ori, restRelativeOrientations[i], worldOrientations[i]);

		glmMultVec3Scalar(boneLocalPosUnscaled[i], boneScales[boneParentIndex][3], localPos);

		glmMultVec3Scalar(localPos, skeletonScale, posScaled);
		glmMultVec3Quaternion(worldOrientations[boneParentIndex], posScaled, pos);
		glmAddVec3(worldPositions[boneParentIndex], pos, worldPositions[i]);
	}
}

void glmAllocateEntityTransformBoneEdit(GlmEntityTransform* tr, unsigned int boneCount, GlmEntityTransform* trSource)
{
	unsigned int iLocal;
	
	tr->_boneRestRelativeOrientation = (float(*)[4])GLMC_MALLOC(boneCount * sizeof(float[4]));
	tr->_boneRestRelativePosition = (float(*)[3])GLMC_MALLOC(boneCount * sizeof(float[3]));
	if (trSource == NULL)
	{
		for (iLocal = 0; iLocal < boneCount; iLocal++)
		{
			glmSetIdentityQuaternion(tr->_boneRestRelativeOrientation[iLocal]);

			tr->_boneRestRelativePosition[iLocal][0] = 0.f;
			tr->_boneRestRelativePosition[iLocal][1] = 0.f;
			tr->_boneRestRelativePosition[iLocal][2] = 0.f;
		}
	}
	else
	{
		memcpy(tr->_boneRestRelativeOrientation, trSource->_boneRestRelativeOrientation, boneCount * sizeof(float[4]));
		memcpy(tr->_boneRestRelativePosition, trSource->_boneRestRelativePosition, boneCount * sizeof(float[3]));
	}
}

//---------------------------------------------------------------------------
void glmCopyTransform(GlmSimulationData* simulationData, GlmEntityTransform* transformDestination, GlmEntityTransform* transformSource)
{
	unsigned int boneCount = simulationData->_boneCount[simulationData->_entityTypes[transformSource->_sourceIndexInCrowdField]];
	// duplicate matrices
	memcpy(&transformDestination->_matrixBase, &transformSource->_matrixBase, sizeof(float) * 16);
	transformDestination->_scale = transformSource->_scale;
	memcpy(&transformDestination->_orientationBase[0], &transformSource->_orientationBase[0], sizeof(float) * 4);
	transformDestination->_sourceIndexInCrowdField = transformSource->_sourceIndexInCrowdField;
	transformDestination->_postureBoneCount = transformSource->_postureBoneCount;
	transformDestination->_killed = transformSource->_killed;

	// duplicate bone edit
	if (transformSource->_boneRestRelativeOrientation)
	{
		glmAllocateEntityTransformBoneEdit(transformDestination, boneCount, transformSource);
	}
}

//---------------------------------------------------------------------------
void glmSetSnapToPositionOrientation(GlmEntityTransform* transformDestination, const float *snapPosition, const float *snapRotation)
{
	float matrix[16];
	float matrixSource[16];

	float orientationSource[4];

	memcpy(matrixSource, transformDestination->_matrixBase, sizeof(float) * 16);
	memcpy(orientationSource, transformDestination->_orientationBase, sizeof(float) * 4);

	glmConvertMatrix(matrix, snapPosition, snapRotation);
	memcpy(transformDestination->_matrixBase, matrix, sizeof(float) * 16);
	memcpy(transformDestination->_orientationBase, snapRotation, sizeof(float) * 4);
	glmNormalizeQuaternion(transformDestination->_orientationBase);
}

//---------------------------------------------------------------------------
void glmCreateEntityTransforms(GlmSimulationData* simulationData, GlmHistory* history, GlmEntityTransform** entityTransforms, int *entityTransformCount)
{
	// count duplications, allocate array
	unsigned int i;
	unsigned int j;
	unsigned int duplicateCount = 0;
	unsigned int duplicateIndex;
	unsigned int sourceEntityCount = 0;
	unsigned int entityAv = 0;
	unsigned int totalPostureCount = 0;
	unsigned int totalPostureBoneCount = 0;
	unsigned int maxBonesPerEntity = 0;
	unsigned int firstDuplicatedEntityIndex = 0;
	unsigned int patchedDuplicateCount = 0;

	uint32_t *postureFrames;
	float(*posturesPositions)[3];
	float(*posturesOrientations)[4];
	float(*posturesPositionsHistorySource)[3]; 
	float(*posturesOrientationsHistorySource)[4];

	GlmEntityTransform* data;

	unsigned int frameOffsetsAv = 0;
	unsigned int frameWarpAv = 0;

	static const float identityRotation[4] = { 0.f,0.f,0.f,1.f };

	for (i=0;i<history->_transformCount;i++)
	{
		if (history->_active[i] == 0 || history->_transformTypes[i] == SimulationCacheNoop)
			continue;
		if (history->_transformTypes[i] == SimulationCacheDuplicate || history->_transformTypes[i] == SimulationCacheSnapTo)
			duplicateCount += history->_duplicatedEntityArrayCount[i];
	}

	// we don't store invalid entities anymore :
	sourceEntityCount += simulationData->_entityCount;

	// get maximum bone count
	maxBonesPerEntity = simulationData->_boneCount[0];
	for (i = 1;i<simulationData->_entityTypeCount;i++)
	{
		maxBonesPerEntity = (maxBonesPerEntity>simulationData->_boneCount[i])?maxBonesPerEntity:simulationData->_boneCount[i];
	}

	*entityTransformCount = sourceEntityCount + duplicateCount;
	*entityTransforms = (GlmEntityTransform*)GLMC_MALLOC(*entityTransformCount * sizeof(GlmEntityTransform));

	// set array
	data = *entityTransforms;

	// working arrays 
	data->_sortedBonesWorldOri = (float(*)[4])GLMC_MALLOC(maxBonesPerEntity * sizeof(float[4]));
	data->_sortedBonesWorldPos = (float(*)[3])GLMC_MALLOC(maxBonesPerEntity * sizeof(float[3]));
	data->_sortedBonesScale = (float(*)[4])GLMC_MALLOC(maxBonesPerEntity * sizeof(float[4]));
	data->_restRelativeOri = (float(*)[4])GLMC_MALLOC(maxBonesPerEntity * sizeof(float[4]));
	
	// legacy entities
	for (i = 0;i<simulationData->_entityCount;i++)
	{
		GlmEntityTransform* tr = &data[entityAv];

		tr->_entityId = simulationData->_entityIds[i];
		tr->_sourceIndexInCrowdField = i;
		glmSetIdentityMatrix(tr->_matrix);
		glmSetIdentityMatrix(tr->_matrixBase);
		glmSetIdentityQuaternion(tr->_orientationBase);
		glmSetIdentityQuaternion(tr->_orientation);
		tr->_lastEditPostureHistoryIndex = 0;
		tr->_useCloth = 0;
		tr->_killed = 0;
		tr->_scale = 1.f;
		tr->_postureCount = 0;
		tr->_boneRestRelativeOrientation = 0;
		tr->_boneRestRelativePosition = 0;
		tr->_postureBoneCount = simulationData->_boneCount[simulationData->_entityTypes[i]];
		tr->_groundAdaptOffset[0] = 0.f;
		tr->_groundAdaptOffset[1] = 0.f;
		tr->_groundAdaptOffset[2] = 0.f;
		tr->_frameOffset._frameOffset = 0.f;
		tr->_frameOffset._frameWarp = 1.f;
		tr->_frameOffset._fraction = 0.f;
		tr->_outOfCache = 0;
		tr->_perFramePosOriIndex = INT_MIN;
		tr->_perFramePosOriArrayCount = 0;
		glmSetIdentityQuaternion(tr->_groundAdaptOrientation);
		entityAv++;
	}
	// init array for duplicates
	firstDuplicatedEntityIndex = entityAv;
	for (i = 0;i<duplicateCount;i++)
	{
		GlmEntityTransform* tr = &data[entityAv];
		tr->_entityId = -1;
		
		glmSetIdentityMatrix(tr->_matrix);
		glmSetIdentityMatrix(tr->_matrixBase);
		tr->_sourceIndexInCrowdField = -1;
		glmSetIdentityQuaternion(tr->_orientation);
		glmSetIdentityQuaternion(tr->_orientationBase);
		tr->_lastEditPostureHistoryIndex = 0;
		tr->_useCloth = 0;
		tr->_killed = 0;
		tr->_scale = 1.f;
		tr->_postureCount = 0;
		tr->_boneRestRelativeOrientation = 0;
		tr->_boneRestRelativePosition = 0;
		tr->_postureBoneCount = 0xFFFFFFFF;
		tr->_groundAdaptOffset[0] = 0.f;
		tr->_groundAdaptOffset[1] = 0.f;
		tr->_groundAdaptOffset[2] = 0.f;
		tr->_frameOffset._frameOffset = 0.f;
		tr->_frameOffset._frameWarp = 1.f;
		tr->_frameOffset._fraction = 0.f;
		tr->_outOfCache = 0;
		tr->_perFramePosOriIndex = INT_MIN;
		tr->_perFramePosOriArrayCount = 0;
		glmSetIdentityQuaternion(tr->_groundAdaptOrientation);
		entityAv++;
	}
	duplicateIndex = sourceEntityCount;

	// recompute entityId for duplicates
	for (i = 0; i<history->_transformCount; i++)
	{
		if (history->_active[i] == 0 || history->_transformTypes[i] == SimulationCacheNoop)
		{
			if (history->_transformTypes[i] == SimulationCacheDuplicate || history->_transformTypes[i] == SimulationCacheSnapTo)
				patchedDuplicateCount += history->_duplicatedEntityArrayCount[i];
			continue;
		}
		if (history->_transformTypes[i] == SimulationCacheDuplicate || history->_transformTypes[i] == SimulationCacheSnapTo)
		{
			for (j = 0; j < history->_duplicatedEntityArrayCount[i]; j++)
			{
				GlmEntityTransform* tr = &data[duplicateIndex++];
				tr->_entityId = history->_duplicatedEntityIds[patchedDuplicateCount+j];
			}
			patchedDuplicateCount += history->_duplicatedEntityArrayCount[i];
		}
	}
	duplicateIndex = sourceEntityCount;

	// apply transformations
	for (i=0;i<history->_transformCount;i++)
	{
		if ( history->_active[i] == 0 || history->_transformTypes[i] == SimulationCacheNoop )
			continue;
		for (j=0;j<history->_entityArrayCount[i];j++)
		{
			int entityPositionInCrowdField;
			entityPositionInCrowdField = glmFindEntityInSimulation(data, *entityTransformCount, history->_entityIds[history->_entityArrayStartIndex[i] + j] );
			if ( entityPositionInCrowdField != -1 )
			{
				switch (history->_transformTypes[i])
				{
				case SimulationCacheRotate:
					{
						float matrix[16];
						float matrixSource[16];

						float orientationSource[4];
						memcpy(matrixSource, data[entityPositionInCrowdField]._matrixBase, sizeof(float)*16);
						memcpy(orientationSource, data[entityPositionInCrowdField]._orientationBase, sizeof(float)*4);

						glmConvertMatrix( matrix, history->_transformTranslate[i], history->_transformRotate[i]);

						glmMultMatrix(matrixSource, matrix, data[entityPositionInCrowdField]._matrixBase);
						glmMultQuaternion(orientationSource, history->_transformRotate[i], data[entityPositionInCrowdField]._orientationBase);
						glmNormalizeQuaternion(data[entityPositionInCrowdField]._orientationBase);
					}
					break;
				case SimulationCacheScale:
					data[entityPositionInCrowdField]._scale *= history->_scale[i];
					break;
				case SimulationCacheTranslate:
					{
						float matrix[16];
						float matrixSource[16];

						memcpy(matrixSource, data[entityPositionInCrowdField]._matrixBase, sizeof(float)*16);
						glmConvertMatrix( matrix, history->_transformTranslate[i], identityRotation);
						glmMultMatrix( matrixSource, matrix, data[entityPositionInCrowdField]._matrixBase);
					}
					break;
				case SimulationCacheExpand:
					{
						float matrix[16];
						float matrixSource[16];
						float expandVector[3];
						int iComponent;

						for (iComponent = 0; iComponent < 3; iComponent++)
						{
							expandVector[iComponent] = history->_expands[history->_expandArrayStartIndex[i] + j][iComponent] * (history->_scale[i] - 1.f);
						}
						expandVector[1] = 0.f;

						memcpy(matrixSource, data[entityPositionInCrowdField]._matrixBase, sizeof(float) * 16);
						glmConvertMatrix(matrix, expandVector, identityRotation);
						glmMultMatrix(matrixSource, matrix, data[entityPositionInCrowdField]._matrixBase);
					}
					break;
				case SimulationCacheSnapTo:
					{
						int snapToIndex;
						if (j >= history->_snapToCount[i])
							continue;

						snapToIndex = history->_snapToStartIndex[i] + j;
						glmSetSnapToPositionOrientation(&data[entityPositionInCrowdField], history->_snapToPositions[snapToIndex], history->_snapToRotations[snapToIndex]);

						if (j == history->_entityArrayCount[i] - 1) // move duplicated entities
						{
							unsigned int iSnapToDuplicate = 0;
							int entityToDuplicateCount = history->_duplicatedEntityArrayCount[i];
							// make duplicates
							while (entityToDuplicateCount > 0)
							{
								int iCharacter;
								for (iCharacter = 0; iCharacter < simulationData->_entityTypeCount; iCharacter++)
								{
									unsigned int iEntity;
									for (iEntity = 0; iEntity < history->_entityArrayCount[i]; iEntity++)
									{
										int entityPositionInCrowdField;
										entityPositionInCrowdField = glmFindEntityInSimulation(data, *entityTransformCount, history->_entityIds[history->_entityArrayStartIndex[i] + iEntity]);

										if (simulationData->_entityTypes[entityPositionInCrowdField] != iCharacter)
											continue;

										// copy source transform
										glmCopyTransform(simulationData, &data[duplicateIndex], &data[entityPositionInCrowdField]);

										// set snap to
										snapToIndex = history->_snapToStartIndex[i] + history->_entityArrayCount[i] + iSnapToDuplicate;
										glmSetSnapToPositionOrientation(&data[duplicateIndex], history->_snapToPositions[snapToIndex], history->_snapToRotations[snapToIndex]);

										// duplicate posture edit
										duplicateIndex++;
										entityToDuplicateCount--;
										iSnapToDuplicate++;
									} // entity
								} // simulationData->_entityTypeCount
							} // entityToDuplicateCount > 0)
						}
					}
					break;
				case SimulationCacheScaleRange:
					{
						data[entityPositionInCrowdField]._scale *= history->_scaleRanges[history->_scaleRangeArrayStartIndex[i] + j];
					}
					break;
				case SimulationCacheDuplicate:
					{
						GlmEntityTransform* tr = &data[duplicateIndex];
						unsigned int boneCount = simulationData->_boneCount[simulationData->_entityTypes[data[entityPositionInCrowdField]._sourceIndexInCrowdField]];
						// duplicate matrices
						memcpy(&tr->_matrixBase, &data[entityPositionInCrowdField]._matrixBase, sizeof(float) * 16);
						tr->_scale = data[entityPositionInCrowdField]._scale;
						memcpy(&tr->_orientationBase[0], &data[entityPositionInCrowdField]._orientationBase[0], sizeof(float) * 4);
						tr->_sourceIndexInCrowdField = data[entityPositionInCrowdField]._sourceIndexInCrowdField;
						tr->_postureBoneCount = data[entityPositionInCrowdField]._postureBoneCount;
						tr->_killed = data[entityPositionInCrowdField]._killed;

						// duplicate bone edit
						if (data[entityPositionInCrowdField]._boneRestRelativeOrientation)
						{
							glmAllocateEntityTransformBoneEdit(tr, boneCount, &data[entityPositionInCrowdField]);
						}
						// duplicate posture edit
						duplicateIndex++;
					}
					break;
				case SimulationCacheUnkill:
				case SimulationCacheKill:
					data[entityPositionInCrowdField]._killed = (history->_transformTypes[i] == SimulationCacheKill);
					break;
				case SimulationCachePosture:
					{
						GlmEntityTransform* tr = &data[entityPositionInCrowdField];
						tr->_postureCount += history->_postureCount; // maximum, will be lower than that
						tr->_lastEditPostureHistoryIndex = i;
					}
					break;
				case SimulationCachePostureBoneEdit:
					{
						float localBoneOrientation[4];
						//float localBonePosition[3];
						int boneIndex = history->_boneIndex[i];

						GlmEntityTransform* tr = &data[entityPositionInCrowdField];
						
						if (tr->_sourceIndexInCrowdField == -1)
							continue;
						if (tr->_boneRestRelativeOrientation == 0)
						{
							unsigned int boneCount = simulationData->_boneCount[simulationData->_entityTypes[tr->_sourceIndexInCrowdField]];
							glmAllocateEntityTransformBoneEdit(tr, boneCount, NULL);
						}
						
						// multiply
						memcpy(localBoneOrientation, tr->_boneRestRelativeOrientation[boneIndex], sizeof(float) * 4);
						//memcpy(localBonePosition, tr->_boneRestRelativePosition[boneIndex], sizeof(float) * 3); // unused for now
						glmMultQuaternion(localBoneOrientation, history->_transformRotate[i], tr->_boneRestRelativeOrientation[boneIndex]);
						glmNormalizeQuaternion(tr->_boneRestRelativeOrientation[boneIndex]);
					}
					break;
				case SimulationCacheFrameOffset:
					data[entityPositionInCrowdField]._frameOffset._frameOffset += history->_frameOffsets[frameOffsetsAv++];
					break;
				case SimulationCacheFrameWarp:
					data[entityPositionInCrowdField]._frameOffset._frameWarp *= history->_frameWarps[frameWarpAv++];
					break;
				case SimulationCacheTrajectoryEdit:
					{
						// just add currentFrame
						data[entityPositionInCrowdField]._perFramePosOriIndex = history->_perFramePosOriArrayStartIndex[i] + j * history->_frameCount[i] - history->_startFrame[i];
						data[entityPositionInCrowdField]._perFramePosOriArrayCount = history->_perFramePosOriArrayCount[i];
					}
					break;
				}
			}
		}
	}

	for (i = 0;i<entityAv;i++)
	{
		GlmEntityTransform* tr = &data[i];
		totalPostureCount += tr->_postureCount;
		totalPostureBoneCount += tr->_postureBoneCount * tr->_postureCount;
	}

	// postures edit. 1st transform has the pointer to the allocated posture bones & posture frames
	
	data->_postureFrames = (uint32_t*)GLMC_MALLOC(totalPostureCount * sizeof(uint32_t));
	data->_posturesPositions = (float(*)[3])GLMC_MALLOC(totalPostureBoneCount * sizeof(float[3]));
	data->_posturesOrientations = (float(*)[4])GLMC_MALLOC(totalPostureBoneCount * sizeof(float[4]));
	
	postureFrames = data->_postureFrames;
	posturesPositions = data->_posturesPositions;
	posturesOrientations = data->_posturesOrientations;
	
	// set allocated pointers
	for (i = 0;i<entityAv;i++)
	{
		GlmEntityTransform* tr = &data[i];
		tr->_postureFrames = postureFrames;
		tr->_posturesPositions = posturesPositions;
		tr->_posturesOrientations = posturesOrientations;
		
		postureFrames += tr->_postureCount;
		posturesPositions += tr->_postureCount * tr->_postureBoneCount;
		posturesOrientations += tr->_postureCount * tr->_postureBoneCount;

		tr->_postureCount = 0;
	}

	// 
	posturesPositionsHistorySource = history->_posturesPositions;
	posturesOrientationsHistorySource = history->_posturesOrientations;
	for (i=0;i<history->_transformCount;i++)
	{
		if ( history->_active[i] == 0 || history->_transformTypes[i] == SimulationCacheNoop )
			continue;
		for (j=0;j<history->_entityArrayCount[i];j++)
		{
			int entityPositionInCrowdField;
			entityPositionInCrowdField = glmFindEntityInSimulation(data, *entityTransformCount, history->_entityIds[history->_entityArrayStartIndex[i] + j] );
			if ( entityPositionInCrowdField != -1 )
			{
				if (history->_transformTypes[i]==SimulationCachePosture)
				{
					unsigned int iPosture;
					unsigned int boneOffset;

					GlmEntityTransform* tr = &data[entityPositionInCrowdField];

					// copy frames & posture
					for (iPosture = 0;iPosture<history->_posturesFrameCount[i];iPosture++)
					{
						tr->_postureFrames[iPosture + tr->_postureCount] = history->_posturesFrames[history->_posturesFrameStart[i] + iPosture];

						boneOffset = (iPosture + tr->_postureCount) * tr->_postureBoneCount;
						memcpy( tr->_posturesPositions + boneOffset, posturesPositionsHistorySource, sizeof(float) * 3 * tr->_postureBoneCount);
						memcpy( tr->_posturesOrientations + boneOffset, posturesOrientationsHistorySource, sizeof(float) * 4 * tr->_postureBoneCount);

						posturesPositionsHistorySource += tr->_postureBoneCount;
						posturesOrientationsHistorySource += tr->_postureBoneCount;
					}
					
					tr->_postureCount += history->_posturesFrameCount[i];
				}
			}
		}
	}
}

//---------------------------------------------------------------------------
void glmDestroyEntityTransforms(GlmEntityTransform** entityTransforms, unsigned int entityTransformCount)
{
	unsigned int i;
	GlmEntityTransform* data = *entityTransforms;
	for (i = 0;i<entityTransformCount; i++)
	{
		if ( data[i]._boneRestRelativeOrientation )
			GLMC_FREE(data[i]._boneRestRelativeOrientation);
		if ( data[i]._boneRestRelativePosition )
			GLMC_FREE(data[i]._boneRestRelativePosition);
	}

	GLMC_FREE(data->_sortedBonesWorldOri);
	GLMC_FREE(data->_sortedBonesWorldPos);
	GLMC_FREE(data->_sortedBonesScale);
	GLMC_FREE(data->_restRelativeOri);

	GLMC_FREE(data->_postureFrames);
	GLMC_FREE(data->_posturesPositions);
	GLMC_FREE(data->_posturesOrientations);
	GLMC_FREE(data);
	*entityTransforms = NULL;
}

//---------------------------------------------------------------------------
void glmCreateModifiedSimulationData(GlmSimulationData* simulationDataSource, GlmEntityTransform* entityTransforms, unsigned int entityTransformCount, GlmSimulationData** simulationDataDestination )
{
	unsigned int i;
	GlmSimulationData* data;
	float *maxScales;

	unsigned int iBoneOffsetPerEntityType;
	unsigned int iSnSOffsetPerEntityType;
	unsigned int iBlindDataOffsetPerEntityType;
	unsigned int iGeoBehaviorOffsetPerEntityType;

	glmCreateSimulationData(simulationDataDestination, entityTransformCount, simulationDataSource->_entityTypeCount, simulationDataSource->_ppFloatAttributeCount, simulationDataSource->_ppVectorAttributeCount);
	data = *simulationDataDestination;

	// clear entityTypeCount 
	maxScales = (float*)GLMC_MALLOC(sizeof(float) * simulationDataSource->_entityTypeCount);
	for (i = 0;i<simulationDataSource->_entityTypeCount;i++)
	{
		data->_entityCountPerEntityType[i] = 0;
		maxScales[i] = -1.f;
	}

	// copy/append entity infos
	for (i = 0;i<entityTransformCount;i++)
	{
		uint16_t entityType;
		int sourceIndex;

		data->_entityIds[i] = entityTransforms[i]._entityId;
	
		sourceIndex = entityTransforms[i]._sourceIndexInCrowdField;
		data->_entityTypes[i] = simulationDataSource->_entityTypes[sourceIndex];
		data->_scales[i] = simulationDataSource->_scales[sourceIndex];
		data->_entityRadius[i] = simulationDataSource->_entityRadius[sourceIndex];
		data->_entityHeight[i] = simulationDataSource->_entityHeight[sourceIndex];


		entityType = data->_entityTypes[i];
		data->_indexInEntityType[i] = data->_entityCountPerEntityType[entityType];
		data->_entityCountPerEntityType[entityType]++;
	}
	
	// entityType
	memcpy( data->_boneCount, simulationDataSource->_boneCount, sizeof(uint16_t) * simulationDataSource->_entityTypeCount );
	memcpy( data->_maxBonesHierarchyLength, simulationDataSource->_maxBonesHierarchyLength, sizeof(float) * simulationDataSource->_entityTypeCount );
	memcpy( data->_blindDataCount, simulationDataSource->_blindDataCount, sizeof(uint16_t) * simulationDataSource->_entityTypeCount );
	memcpy( data->_hasGeoBehavior, simulationDataSource->_hasGeoBehavior, sizeof(uint8_t) * simulationDataSource->_entityTypeCount );
	memcpy( data->_snsCountPerEntityType, simulationDataSource->_snsCountPerEntityType, sizeof(uint16_t) * simulationDataSource->_entityTypeCount );

	// update offsets
	iBoneOffsetPerEntityType = 0;
	iSnSOffsetPerEntityType = 0;
	iBlindDataOffsetPerEntityType = 0;
	iGeoBehaviorOffsetPerEntityType = 0;	

	for (i = 0; i < data->_entityTypeCount; ++i)
	{
		data->_iBoneOffsetPerEntityType[i] = iBoneOffsetPerEntityType;
		iBoneOffsetPerEntityType += data->_entityCountPerEntityType[i] * data->_boneCount[i];

		data->_iBlindDataOffsetPerEntityType[i] = iBlindDataOffsetPerEntityType;
		iBlindDataOffsetPerEntityType += data->_entityCountPerEntityType[i] * data->_blindDataCount[i];

		data->_iGeoBehaviorOffsetPerEntityType[i] = iGeoBehaviorOffsetPerEntityType;
		if (data->_hasGeoBehavior[i])
			iGeoBehaviorOffsetPerEntityType += data->_entityCountPerEntityType[i];
			
		data->_snsOffsetPerEntityType[i] = iSnSOffsetPerEntityType;
		iSnSOffsetPerEntityType += data->_entityCountPerEntityType[i] * data->_snsCountPerEntityType[i];
	}

	// ppAttribute
	memcpy( data->_ppFloatAttributeNames, simulationDataSource->_ppFloatAttributeNames, sizeof(char) * simulationDataSource->_ppFloatAttributeCount * GSC_PP_MAX_NAME_LENGTH );
	memcpy( data->_ppVectorAttributeNames, simulationDataSource->_ppVectorAttributeNames, sizeof(char) * simulationDataSource->_ppVectorAttributeCount * GSC_PP_MAX_NAME_LENGTH);

	// update entities scales (legacy & duplicates)
	for (i = 0;i<entityTransformCount;i++)
	{
		float entitymaxBonesHierarchyLength;
		float maxScale;
		float newScale;

		uint16_t entityType;
		float scale = entityTransforms[i]._scale;
		//if (data->_entityIds[i]<0)
		//	continue;

		entityType = data->_entityTypes[i];

		data->_scales[i] *= scale;
		data->_entityRadius[i] *= scale;
		data->_entityHeight[i] *= scale;

		// set biggest
		entitymaxBonesHierarchyLength = simulationDataSource->_maxBonesHierarchyLength[entityType] * scale;
		maxScale = maxScales[entityType];
		newScale = (maxScale>entitymaxBonesHierarchyLength)?maxScale:entitymaxBonesHierarchyLength;
		maxScales[entityType] = newScale;
	}

	// update max scales
	for (i = 0;i<data->_entityTypeCount;i++)
	{
		if (maxScales[i]>0.f) // <0 means scale not touched
			data->_maxBonesHierarchyLength[i] = maxScales[i]; // per entity type
	}

	data->_contentHashKey = simulationDataSource->_contentHashKey;
	//
	GLMC_FREE(maxScales);
}

// -----------------------------------------------------------------------------
void glmVoidEntityFrameData(GlmFrameData*frameDataOut, GlmSimulationData *simuDataIn, GlmSimulationData *simuDataOut, int sourceIndexInCrowdField, int destIndexInCrowdField, int geoBeDestination)
{
	unsigned int i;

	// transform entity
	uint16_t entityTypeIndex = simuDataIn->_entityTypes[sourceIndexInCrowdField];
	uint16_t boneCount = simuDataIn->_boneCount[entityTypeIndex];
	uint16_t snsCount = simuDataIn->_snsCountPerEntityType[entityTypeIndex];
	uint16_t blindDataCount = simuDataIn->_blindDataCount[entityTypeIndex];

	// copy pos/ori
	uint32_t offsetDest = simuDataOut->_iBoneOffsetPerEntityType[entityTypeIndex] + boneCount * simuDataOut->_indexInEntityType[destIndexInCrowdField];
	float(*bonePositionsPtrDest)[3] = frameDataOut->_bonePositions + offsetDest;
	float(*boneOrientationPtrdest)[4] = frameDataOut->_boneOrientations + offsetDest;

	for (i = 0; i<boneCount; i++)
	{
		memset(bonePositionsPtrDest[i], 0, sizeof(float) * 3);
		glmSetIdentityQuaternion(boneOrientationPtrdest[i]);
	}
	
	// copy snsValues
	if (snsCount)
	{
		uint32_t offsetSnsDest = simuDataOut->_snsOffsetPerEntityType[entityTypeIndex] + snsCount * simuDataOut->_indexInEntityType[destIndexInCrowdField];
		float(*boneSnsPtrDest)[4] = frameDataOut->_snsValues + offsetSnsDest;

		memset(boneSnsPtrDest, 0, sizeof(float) * 4 * snsCount);
	}

	// blind data
	if (blindDataCount)
	{
		uint32_t offsetBDDest = simuDataOut->_iBlindDataOffsetPerEntityType[entityTypeIndex] + blindDataCount * simuDataOut->_indexInEntityType[destIndexInCrowdField];
		float *boneBDPtrDest = frameDataOut->_blindData + offsetBDDest;

		memset(boneBDPtrDest, 0, sizeof(float) * blindDataCount);
	}

	// Geo Be
	if (simuDataIn->_hasGeoBehavior[entityTypeIndex])
	{
		frameDataOut->_geoBehaviorGeometryIds[geoBeDestination] = 0;
		memset(frameDataOut->_geoBehaviorAnimFrameInfo[geoBeDestination], 0, sizeof(float) * 3);
		frameDataOut->_geoBehaviorBlendModes[geoBeDestination] = 0;
	}
}

// -----------------------------------------------------------------------------
void glmCopyEntityFrameData(const GlmFrameData*frameDataIn, GlmFrameData*frameDataOut, GlmSimulationData *simuDataIn, GlmSimulationData *simuDataOut, int sourceIndexInCrowdField, int destIndexInCrowdField, int geoBeSource, int geoBeDestination, GlmEntityTransform *transform )
{
	unsigned int i, j;
	float *matrix = transform->_matrix;
	float scale = transform->_scale;

	// transform entity
	uint16_t entityTypeIndex = simuDataIn->_entityTypes[sourceIndexInCrowdField];
	uint16_t boneCount = simuDataIn->_boneCount[entityTypeIndex];
	uint16_t snsCount = simuDataIn->_snsCountPerEntityType[entityTypeIndex];
	uint16_t blindDataCount = simuDataIn->_blindDataCount[entityTypeIndex];

	// copy pos/ori
	uint32_t offsetSource = simuDataIn->_iBoneOffsetPerEntityType[entityTypeIndex] + boneCount * simuDataIn->_indexInEntityType[sourceIndexInCrowdField];
	uint32_t offsetDest = simuDataOut->_iBoneOffsetPerEntityType[entityTypeIndex] + boneCount * simuDataOut->_indexInEntityType[destIndexInCrowdField];
	float(*bonePositionsPtrSource)[3] = frameDataIn->_bonePositions + offsetSource;
	float(*boneOrientationPtrSource)[4] = frameDataIn->_boneOrientations + offsetSource;
	float(*bonePositionsPtrDest)[3] = frameDataOut->_bonePositions + offsetDest;
	float(*boneOrientationPtrdest)[4] = frameDataOut->_boneOrientations + offsetDest;
	
	for (i = 0;i<boneCount;i++)
	{
		glmTransformPoint(bonePositionsPtrSource[i], matrix, bonePositionsPtrDest[i]);
		glmMultQuaternion(&transform->_orientation[0], boneOrientationPtrSource[i], boneOrientationPtrdest[i]);
		glmNormalizeQuaternion(boneOrientationPtrdest[i]);
	}

	// scale posture
	for (i = 1;i<boneCount;i++)
	{
		for (j = 0;j<3;j++)
		{
			float ref = bonePositionsPtrDest[0][j];
			bonePositionsPtrDest[i][j] = (bonePositionsPtrDest[i][j]-ref) * scale + ref;
			transform->_scalePivot[j] = ref;
		}
	}

	// copy snsValues
	if (snsCount)
	{
		uint32_t offsetSnsSource = simuDataIn->_snsOffsetPerEntityType[entityTypeIndex] + snsCount * simuDataIn->_indexInEntityType[sourceIndexInCrowdField];
		uint32_t offsetSnsDest = simuDataOut->_snsOffsetPerEntityType[entityTypeIndex] + snsCount * simuDataOut->_indexInEntityType[destIndexInCrowdField];
		float(*boneSnsPtrSource)[4] = frameDataIn->_snsValues + offsetSnsSource;
		float(*boneSnsPtrDest)[4] = frameDataOut->_snsValues + offsetSnsDest;

		memcpy(boneSnsPtrDest, boneSnsPtrSource, sizeof(float) * 4 * snsCount);
	}

	// blind data
	if (blindDataCount )
	{
		uint32_t offsetBDSource = simuDataIn->_iBlindDataOffsetPerEntityType[entityTypeIndex] + blindDataCount * simuDataIn->_indexInEntityType[sourceIndexInCrowdField];
		uint32_t offsetBDDest = simuDataOut->_iBlindDataOffsetPerEntityType[entityTypeIndex] + blindDataCount * simuDataOut->_indexInEntityType[destIndexInCrowdField];
		float *boneBDPtrSource = frameDataIn->_blindData + offsetBDSource;
		float *boneBDPtrDest = frameDataOut->_blindData + offsetBDDest;

		memcpy(boneBDPtrDest, boneBDPtrSource, sizeof(float) * blindDataCount);
	}

	// Geo Be
	if (simuDataIn->_hasGeoBehavior[entityTypeIndex])
	{
		frameDataOut->_geoBehaviorGeometryIds[geoBeDestination] = frameDataIn->_geoBehaviorGeometryIds[geoBeSource];
		memcpy(frameDataOut->_geoBehaviorAnimFrameInfo[geoBeDestination], frameDataIn->_geoBehaviorAnimFrameInfo[geoBeSource], sizeof(float) *3);
		frameDataOut->_geoBehaviorBlendModes[geoBeDestination] = frameDataIn->_geoBehaviorBlendModes[geoBeSource];
	}

}

void glmInterpolateEntityFrameData(const GlmFrameData*frameDataIn1, const GlmFrameData*frameDataIn2, float fraction, GlmFrameData*frameDataOut, GlmSimulationData *simuDataIn, GlmSimulationData *simuDataOut, int sourceIndexInCrowdField, int destIndexInCrowdField, int geoBeSource, int geoBeDestination, GlmEntityTransform *transform)
{
	unsigned int i, j;
	float *matrix = transform->_matrix;
	float scale = transform->_scale;
//---------------------------------------------------------------------------
	// transform entity
	uint16_t entityTypeIndex = simuDataIn->_entityTypes[sourceIndexInCrowdField];
	uint16_t boneCount = simuDataIn->_boneCount[entityTypeIndex];
	uint16_t snsCount = simuDataIn->_snsCountPerEntityType[entityTypeIndex];
	uint16_t blindDataCount = simuDataIn->_blindDataCount[entityTypeIndex];

	// copy pos/ori
	uint32_t offsetSource = simuDataIn->_iBoneOffsetPerEntityType[entityTypeIndex] + boneCount * simuDataIn->_indexInEntityType[sourceIndexInCrowdField];
	uint32_t offsetDest = simuDataOut->_iBoneOffsetPerEntityType[entityTypeIndex] + boneCount * simuDataOut->_indexInEntityType[destIndexInCrowdField];
	float(*bonePositionsPtrSource1)[3] = frameDataIn1->_bonePositions + offsetSource;
	float(*boneOrientationPtrSource1)[4] = frameDataIn1->_boneOrientations + offsetSource;
	float(*bonePositionsPtrSource2)[3] = frameDataIn2->_bonePositions + offsetSource;
	float(*boneOrientationPtrSource2)[4] = frameDataIn2->_boneOrientations + offsetSource;
	float(*bonePositionsPtrDest)[3] = frameDataOut->_bonePositions + offsetDest;
	float(*boneOrientationPtrdest)[4] = frameDataOut->_boneOrientations + offsetDest;

	for (i = 0; i<boneCount; i++)
{
		float interpolatedOri[4];
		float interpolatedPos[3];
		interpolateNFloats(boneOrientationPtrSource1[i], boneOrientationPtrSource2[i], fraction, interpolatedOri, 4);
		interpolateNFloats(bonePositionsPtrSource1[i], bonePositionsPtrSource2[i], fraction, interpolatedPos, 3);
		glmNormalizeQuaternion(interpolatedOri);

		glmTransformPoint(interpolatedPos, matrix, bonePositionsPtrDest[i]);
		glmMultQuaternion(&transform->_orientation[0], interpolatedOri, boneOrientationPtrdest[i]);
		glmNormalizeQuaternion(boneOrientationPtrdest[i]);
	}

	// scale posture
	for (i = 1; i<boneCount; i++)
	{
		for (j = 0; j<3; j++)
		{
			float ref = bonePositionsPtrDest[0][j];
			bonePositionsPtrDest[i][j] = (bonePositionsPtrDest[i][j] - ref) * scale + ref;
			transform->_scalePivot[j] = ref;
		}
	}

	// copy snsValues
	if (snsCount)
	{
		uint32_t offsetSnsSource = simuDataIn->_snsOffsetPerEntityType[entityTypeIndex] + snsCount * simuDataIn->_indexInEntityType[sourceIndexInCrowdField];
		uint32_t offsetSnsDest = simuDataOut->_snsOffsetPerEntityType[entityTypeIndex] + snsCount * simuDataOut->_indexInEntityType[destIndexInCrowdField];
		float(*boneSnsPtrSource1)[4] = frameDataIn1->_snsValues + offsetSnsSource;
		float(*boneSnsPtrSource2)[4] = frameDataIn2->_snsValues + offsetSnsSource;
		float(*boneSnsPtrDest)[4] = frameDataOut->_snsValues + offsetSnsDest;

		for (i = 0; i < snsCount; i++)
		{
			float res[4];
			interpolateNFloats(boneSnsPtrSource1[i], boneSnsPtrSource2[i], fraction, res, 4);
			memcpy(boneSnsPtrDest[i], res, sizeof(float) * 4);
		}
	}

	// blind data
	if (blindDataCount)
	{
		uint32_t offsetBDSource = simuDataIn->_iBlindDataOffsetPerEntityType[entityTypeIndex] + blindDataCount * simuDataIn->_indexInEntityType[sourceIndexInCrowdField];
		uint32_t offsetBDDest = simuDataOut->_iBlindDataOffsetPerEntityType[entityTypeIndex] + blindDataCount * simuDataOut->_indexInEntityType[destIndexInCrowdField];
		float *boneBDPtrSource1 = frameDataIn1->_blindData + offsetBDSource;
		float *boneBDPtrSource2 = frameDataIn2->_blindData + offsetBDSource;
		float *boneBDPtrDest = frameDataOut->_blindData + offsetBDDest;

		for (i = 0; i < blindDataCount; i++)
			boneBDPtrDest[i] = interpolateFloat(boneBDPtrSource1[i], boneBDPtrSource2[i], fraction);
	}

	// Geo Be
	if (simuDataIn->_hasGeoBehavior[entityTypeIndex])
	{
		frameDataOut->_geoBehaviorGeometryIds[geoBeDestination] = frameDataIn1->_geoBehaviorGeometryIds[geoBeSource];
		memcpy(frameDataOut->_geoBehaviorAnimFrameInfo[geoBeDestination], frameDataIn1->_geoBehaviorAnimFrameInfo[geoBeSource], sizeof(float) * 3);
		frameDataOut->_geoBehaviorBlendModes[geoBeDestination] = frameDataIn1->_geoBehaviorBlendModes[geoBeSource];
	}
}

//---------------------------------------------------------------------------
int glmSortEntityFrameOffset(const void *a, const void *b)
{
	const GlmFrameOffset *pa = (const GlmFrameOffset*)a;
	const GlmFrameOffset *pb = (const GlmFrameOffset*)b;
	if (pa->_frameIndex < pb->_frameIndex)
		return -1;
	if (pa->_frameIndex > pb->_frameIndex)
		return 1;
	return 0;
}

//---------------------------------------------------------------------------
unsigned int glmComputeEntityFrameOffsets(GlmFrameOffset *frameOffsets, int entityCount, int currentFrame, GlmFrameToLoad *frames)
{
	int i, j;
	int lastFrame;
	int differenteFrameCount = 1;
	int entityIndex = 0;
	int noFrameOffset = 1;
	for (i = 0; i < (int)entityCount; i++)
	{
		float entityFrame;
		GlmFrameOffset *frameOffset = &frameOffsets[i];
		if (fabsf(frameOffset->_frameOffset) <= FLT_EPSILON && fabsf(frameOffset->_frameWarp - 1) <= FLT_EPSILON)
		{
			GlmFrameOffset *frameOffset = &frameOffsets[i];
			frameOffset->_frameIndex = currentFrame;
			frameOffset->_framesLoadedIndex = 0;
			frameOffset->_fraction = 0.f;
			continue;
		}
		noFrameOffset = 0;
		entityFrame = (float)currentFrame * frameOffset->_frameWarp + frameOffset->_frameOffset;
		if (entityFrame < 0.f)
		{
			frameOffset->_frameIndex = (int)entityFrame - 1;
			frameOffset->_fraction = 1.f - (fabsf(entityFrame) - floorf(fabsf(entityFrame)));
		}
		else
		{
			frameOffset->_frameIndex = (int)entityFrame;
			frameOffset->_fraction = entityFrame - floorf(entityFrame);
		}
	}
	if (noFrameOffset)
	{
		frames[0]._frameIndex = currentFrame;
		return 1;
	}
	// sort by _relativeFrame
	qsort(frameOffsets, entityCount, sizeof(GlmFrameOffset), glmSortEntityFrameOffset);

	// get all different frames
	lastFrame = frames[0]._frameIndex = frameOffsets[0]._frameIndex;
	for (i = 1; i < (int)entityCount; i++)
	{
		int framIndexe = frameOffsets[i]._frameIndex;
		if (framIndexe != lastFrame)
		{
			frames[differenteFrameCount++]._frameIndex = framIndexe;
			lastFrame = framIndexe;
		}
	}

	// insert last capping frame
	frames[differenteFrameCount]._frameIndex = frames[differenteFrameCount - 1]._frameIndex + 1;
	differenteFrameCount++;

	// insert in betweens
	for (i = 0; i < differenteFrameCount - 1; i++)
	{
		if ((frames[i]._frameIndex + 1) == frames[i + 1]._frameIndex)
			continue;

		// if not, insert it
		for (j = differenteFrameCount; j > i; j--)
		{
			frames[j]._frameIndex = frames[j - 1]._frameIndex;
		}
		frames[i + 1]._frameIndex++;
		differenteFrameCount++;
		i++; // skip what we inserted
	}

	// set loc
	
	for (i = 0; i < differenteFrameCount; i++)
	{
		while (frameOffsets[entityIndex]._frameIndex == frames[i]._frameIndex)
		{
			frameOffsets[entityIndex]._framesLoadedIndex = i;
			entityIndex++;
		};
	}

	// set every frame as not loaded
	for (i = 0; i < differenteFrameCount; i++)
		frames[i]._frame = NULL;

	return (unsigned int)differenteFrameCount;
}
//---------------------------------------------------------------------------
int glmComputeValidFrameIndex(int frameIndex, const char *filePathModel, const char *cacheDirectory)
{
#if _MSC_VER
	struct _finddata_t fileinfo;
	long fret;
	intptr_t fndhand;
	char pathFind[2048];
#else
	struct dirent *lecture;
	DIR *rep;
#endif
	int bestFrameIndex = INT32_MIN;
	const char *filePathModelNoDir = filePathModel;
	filePathModelNoDir += strlen(filePathModel) - 1;
	while (filePathModelNoDir > (filePathModel+1) && *(filePathModelNoDir-1) != '/' && *(filePathModelNoDir-1) != '\\')
	{
		filePathModelNoDir--;
	}
#if _MSC_VER
	strcpy_s(pathFind, sizeof(pathFind), cacheDirectory);
	strcat_s(pathFind, sizeof(pathFind), "/*");
	
	fndhand = _findfirst(pathFind, &fileinfo);
	if (fndhand != -1)
	{
		do
		{
			int frameIndexFound;
			if (sscanf_s(fileinfo.name, filePathModelNoDir, &frameIndexFound))
			{
				if (frameIndexFound > bestFrameIndex && frameIndexFound <= frameIndex)
					bestFrameIndex = frameIndexFound;
			}
			fret = _findnext(fndhand, &fileinfo);
		} while (fret != -1);
	}
	_findclose(fndhand);

#else
	rep = opendir(cacheDirectory);

	while ((rep != NULL) && (lecture = readdir(rep)))
	{
		int frameIndexFound;
		if (sscanf(lecture->d_name, filePathModelNoDir, &frameIndexFound))
		{
			if (frameIndexFound > bestFrameIndex && frameIndexFound <= frameIndex)
				bestFrameIndex = frameIndexFound;
		}
	}

	closedir(rep);

#endif 

	return bestFrameIndex;
}

//---------------------------------------------------------------------------
GlmSimulationCacheStatus glmCreateModifiedFrameData(GlmSimulationData* simulationDataIn, GlmFrameData* frameDataIn, GlmEntityTransform* entityTransforms, unsigned int entityTransformCount, GlmHistory* history, GlmSimulationData* simulationDataOut, GlmFrameData** frameDataOut, int currentFrame, const char * filePathModel, const char * cacheDirectory)
{
	GlmFrameData *frameOut;
	unsigned int i;
	unsigned int iTransform;
	unsigned int iFrame;
	unsigned int entityTypeIndex;

	// entityType
	unsigned int totalBoneCount = 0;
	unsigned int totalSnSCount = 0;
	unsigned int totalBlindDataCount = 0;
	unsigned int totalGeoBehaviorCount = 0;
	
	unsigned int totalClothEntityCount = 0;
	unsigned int totalClothTotalIndices = 0;
	unsigned int totalClothTotalVertices = 0;
	
	// cloth pointers
	unsigned int clothedEntityIndex = 0;

	uint32_t* clothIndicesSource = 0;
	uint32_t* clothMeshVertexCountPerClothIndexSource = 0;
	float(*clothVerticesSource)[3] = 0;
	
	// frames to load
	unsigned int totalFrameOffsets;

	// patch transforms for cloths
	// need frame offset/frame scale?
	GlmFrameOffset *entityFrameOffsets = (GlmFrameOffset*)GLMC_MALLOC(sizeof(GlmFrameOffset) * entityTransformCount);
	GlmFrameToLoad *framesToLoad = (GlmFrameToLoad*)GLMC_MALLOC(sizeof(GlmFrameToLoad) * entityTransformCount * 2);

	for (i = 0; i < entityTransformCount; i++)
	{
		GlmFrameOffset *offset = &entityFrameOffsets[i];
		memcpy(offset, &entityTransforms[i]._frameOffset, sizeof(GlmFrameOffset));
		offset->_entityIndex = i;
	}
	
	totalFrameOffsets = glmComputeEntityFrameOffsets(entityFrameOffsets, entityTransformCount, currentFrame, framesToLoad);

	// set it back to entities modification
	for (i = 0; i < entityTransformCount; i++)
	{
		int destinationTransformIndex = entityFrameOffsets[i]._entityIndex;
		memcpy(&entityTransforms[destinationTransformIndex]._frameOffset, &entityFrameOffsets[i], sizeof(GlmFrameOffset));
	}
	GLMC_FREE(entityFrameOffsets);

	// set current frame as loaded
	for (i = 0; i < totalFrameOffsets; i++)
	{
		if (framesToLoad[i]._frameIndex == currentFrame)
			framesToLoad[i]._frame = frameDataIn;
	}
	
	// load missing frames 
	for (i = 0; i < totalFrameOffsets; i++)
	{
		if (!framesToLoad[i]._frame)
		{
			GlmSimulationCacheStatus status;
			char frameFilePath[2048];
			glmsprintf(frameFilePath, 2048, filePathModel, framesToLoad[i]._frameIndex);
			glmCreateFrameData(&framesToLoad[i]._frame, simulationDataIn);
			status = glmReadFrameData(framesToLoad[i]._frame, simulationDataIn, frameFilePath);
			if (status != GSC_SUCCESS)
			{
				// try to find previous
				int previousValidFrameIndex = glmComputeValidFrameIndex(framesToLoad[i]._frameIndex, filePathModel, cacheDirectory);
				if (previousValidFrameIndex != INT32_MIN)
				{
					glmsprintf(frameFilePath, 2048, filePathModel, previousValidFrameIndex);
					status = glmReadFrameData(framesToLoad[i]._frame, simulationDataIn, frameFilePath);
					if (status != GSC_SUCCESS)
						previousValidFrameIndex = INT32_MIN;
				}

				if (previousValidFrameIndex == INT32_MIN)
					glmDestroyFrameData(&framesToLoad[i]._frame, simulationDataIn);
			}
		}
	}

	// frame 0 not found, catch the 1st one
	if (!frameDataIn)
	{
		for (i = 0; i < totalFrameOffsets; i++)
		{
			if (framesToLoad[i]._frame)
				frameDataIn = framesToLoad[i]._frame;
		}
	}

	if (!frameDataIn)
		return GSC_SIMULATION_NO_FRAMES_FOUND;

	// cloth info
	if (frameDataIn)
	{
		clothIndicesSource = frameDataIn->_clothMeshIndicesInCharAssets;
		clothMeshVertexCountPerClothIndexSource = frameDataIn->_clothMeshVertexCount;
		clothVerticesSource = frameDataIn->_clothVertices;
	}

	// patch transforms for cloths
	if (frameDataIn->_clothEntityCount)
	{
		// compute the cloth index for source entities
		for (i = 0 ; i < entityTransformCount; ++i)
		{
			int clothEntityIndex;

			entityTransforms[i]._clothTotalMeshIndices = 0;
			entityTransforms[i]._clothTotalVertices = 0;

			clothEntityIndex = frameDataIn->_entityClothIndex[entityTransforms[i]._sourceIndexInCrowdField];

			if ( entityTransforms[i]._sourceIndexInCrowdField == (int)i && 
				clothEntityIndex != -1 )
			{
				unsigned int iVertexGroup;
				unsigned int meshIndexCount;

				entityTransforms[i]._clothedEntityIndex = clothedEntityIndex;
				entityTransforms[i]._clothVerticesSource = clothVerticesSource;
				entityTransforms[i]._clothIndicesSource = clothIndicesSource;
				entityTransforms[i]._clothMeshVertexCountSource = clothMeshVertexCountPerClothIndexSource;

				meshIndexCount = frameDataIn->_clothEntityMeshCount[entityTransforms[i]._clothedEntityIndex];
				
				entityTransforms[i]._clothTotalMeshIndices = meshIndexCount;

				for (iVertexGroup = 0;iVertexGroup<meshIndexCount;iVertexGroup++)
				{
					size_t groupVertexCount = clothMeshVertexCountPerClothIndexSource[iVertexGroup];
					clothVerticesSource += groupVertexCount;
					entityTransforms[i]._clothTotalVertices += (int)groupVertexCount;
				}
				clothIndicesSource += meshIndexCount;
				clothMeshVertexCountPerClothIndexSource += meshIndexCount;
				clothedEntityIndex++;
			}
		}

		for (i = 0 ; i < entityTransformCount; ++i)
		{
			// patch EntityTransform for cloth
			entityTransforms[i]._useCloth = frameDataIn->_entityClothIndex[entityTransforms[i]._sourceIndexInCrowdField] != -1;
			entityTransforms[i]._clothedEntityIndex = entityTransforms[entityTransforms[i]._sourceIndexInCrowdField]._clothedEntityIndex;
			entityTransforms[i]._clothVerticesSource = entityTransforms[entityTransforms[i]._sourceIndexInCrowdField]._clothVerticesSource;
			entityTransforms[i]._clothIndicesSource = entityTransforms[entityTransforms[i]._sourceIndexInCrowdField]._clothIndicesSource;
			entityTransforms[i]._clothMeshVertexCountSource = entityTransforms[entityTransforms[i]._sourceIndexInCrowdField]._clothMeshVertexCountSource;
			
			// now counts cloth/vertices
			if (entityTransforms[i]._useCloth)
			{
				totalClothEntityCount ++;
				totalClothTotalIndices += entityTransforms[entityTransforms[i]._sourceIndexInCrowdField]._clothTotalMeshIndices;
				totalClothTotalVertices += entityTransforms[entityTransforms[i]._sourceIndexInCrowdField]._clothTotalVertices;
			}
		}
	}

	for (i = 0; i < simulationDataIn->_entityTypeCount; ++i)
	{
		totalBoneCount += simulationDataIn->_boneCount[i] * simulationDataIn->_entityCountPerEntityType[i];
		totalSnSCount += simulationDataIn->_snsCountPerEntityType[i] * simulationDataIn->_entityCountPerEntityType[i];
		totalBlindDataCount += simulationDataIn->_blindDataCount[i] * simulationDataIn->_entityCountPerEntityType[i];
		if (simulationDataIn->_hasGeoBehavior[i])
		{
			totalGeoBehaviorCount += simulationDataIn->_entityCountPerEntityType[i];
		}
	}

	glmCreateFrameData(frameDataOut, simulationDataOut);
	frameOut = *frameDataOut;

	frameOut->_cacheFormat = frameDataIn->_cacheFormat;
	frameOut->_hasSquashAndStretch = frameDataIn->_hasSquashAndStretch;
	frameOut->_simulationContentHashKey = simulationDataOut->_contentHashKey;

	// ground adaptation
	for (iTransform = 0; iTransform < entityTransformCount; iTransform++)
	{
		GlmEntityTransform* tr = &entityTransforms[iTransform];
		memcpy(tr->_matrix, tr->_matrixBase, sizeof(float) * 16);
		memcpy(tr->_orientation, tr->_orientationBase, sizeof(float) *4);
	}

	// per frame pos/ori
	for (iTransform = 0; iTransform < entityTransformCount; iTransform++)
	{
		GlmEntityTransform* tr = &entityTransforms[iTransform];

		if (tr->_perFramePosOriIndex != INT_MIN && tr->_perFramePosOriArrayCount > 0)
		{
			// get frame offset interpolated
			int frameIndexA = (tr->_frameOffset._frameIndex < tr->_perFramePosOriArrayCount) ? tr->_frameOffset._frameIndex : (tr->_perFramePosOriArrayCount - 1);
			int frameIndexB = ((tr->_frameOffset._frameIndex + 1) < tr->_perFramePosOriArrayCount) ? (tr->_frameOffset._frameIndex + 1) : (tr->_perFramePosOriArrayCount - 1);
			int frameOffsetA = tr->_perFramePosOriIndex + frameIndexA;
			int frameOffsetB = tr->_perFramePosOriIndex + frameIndexB;

			float* frameOriA = history->_frameOri[frameOffsetA];
			float* frameOriB = history->_frameOri[frameOffsetB];

			float* framePosA = history->_framePos[frameOffsetA];
			float* framePosB = history->_framePos[frameOffsetB];

			float interpolatedOri[4];
			float interpolatedPos[3];

			// clamp frame

			interpolateNFloats(frameOriA, frameOriB, tr->_frameOffset._fraction, interpolatedOri, 4);
			interpolateNFloats(framePosA, framePosB, tr->_frameOffset._fraction, interpolatedPos, 3);
			glmNormalizeQuaternion(interpolatedOri);

			// set matrix and orientation
			glmConvertMatrix(tr->_matrix, interpolatedPos, interpolatedOri);
			memcpy(tr->_orientation, interpolatedOri, sizeof(float) * 4);

			// set it as base for ground adaptation
			memcpy(tr->_orientationBase, tr->_orientation, sizeof(float) * 4);
			memcpy(tr->_matrixBase, tr->_matrix, sizeof(float) * 16);
		}
	}

	// ground adapt
	if (glmRaycastClosest && (history->_options&(uint32_t)(OptionsGroundAdaptUseTerrain)))
	{
		if (glmTerrainSetFrame)
			glmTerrainSetFrame(history->_terrainMeshSource, history->_terrainMeshDestination, currentFrame);

		for (iTransform = 0; iTransform < entityTransformCount; iTransform++)
		{
			float deltaGroundHeight = 0.f;
			float deltaGroundOri[4];
			float(collisionPointSource)[3];
			float(collisionNormalSource)[3];
			float(collisionPointDestination)[3];
			float(collisionNormalDestination)[3];
			float(rayOriginSource)[3];
			float(rayEndSource)[3];
			float(*bonePositionsPtrSource)[3];
			float(transformedRootPos)[3];
			float deltaGroundMat[16];
			float deltaGroundMatIntermediate[16];
			float groundRot[16];
			float nulTranslation[] = { 0.f,0.f,0.f };
			float preRot[16];
			float postRot[16];
			int iRayCastPass1 = 0;
			int iRayCastPass2 = 0;
			int rayCastStart;
			int rayCastEnd;
			int firstRaycast = 1;
			static const float deltas[] = { -9999999.f, 9999999.f };

			GlmEntityTransform* tr = &entityTransforms[iTransform];
			// transform entity
			uint16_t entityTypeIndex = simulationDataIn->_entityTypes[tr->_sourceIndexInCrowdField];
			uint16_t boneCount = simulationDataIn->_boneCount[entityTypeIndex];

			// copy pos/ori
			uint32_t offsetSource = simulationDataIn->_iBoneOffsetPerEntityType[entityTypeIndex] + boneCount * simulationDataIn->_indexInEntityType[tr->_sourceIndexInCrowdField];
			bonePositionsPtrSource = frameDataIn->_bonePositions + offsetSource;
	
			if (tr->_sourceIndexInCrowdField == -1)
				continue;

			for (iRayCastPass1 = 0; iRayCastPass1 < 2; iRayCastPass1++)
			{

				rayOriginSource[0] = (*bonePositionsPtrSource)[0];
				rayOriginSource[1] = (*bonePositionsPtrSource)[1] + 1.f;
				rayOriginSource[2] = (*bonePositionsPtrSource)[2];

				rayEndSource[0] = rayOriginSource[0];
				rayEndSource[1] = rayOriginSource[1] + deltas[iRayCastPass1];
				rayEndSource[2] = rayOriginSource[2];

				glmSetIdentityQuaternion(deltaGroundOri);

				rayCastStart = 0;
				if (history->_terrainMeshSource)
					rayCastStart = glmRaycastClosest(history->_terrainMeshSource, rayOriginSource, rayEndSource, collisionPointSource, collisionNormalSource, NULL, NULL);
				else
				{
					collisionPointSource[0] = rayOriginSource[0];
					collisionPointSource[1] = 0.f;
					collisionPointSource[2] = rayOriginSource[2];
					collisionNormalSource[0] = 0.f;
					collisionNormalSource[1] = 1.f;
					collisionNormalSource[2] = 0.f;
					rayCastStart = 1;
				}
				if (rayCastStart)
				{
					float(rayOriginDestination)[3];
					// transform ray destination
					glmTransformPoint(rayOriginSource, tr->_matrixBase, rayOriginDestination);
					// --
					for (iRayCastPass2 = 0; iRayCastPass2 < 2; iRayCastPass2++)
					{
						float(rayEndDestination)[3];
						
						rayEndDestination[0] = rayOriginDestination[0];
						rayEndDestination[1] = rayOriginDestination[1] + deltas[iRayCastPass2];
						rayEndDestination[2] = rayOriginDestination[2];

						rayCastEnd = 0;
						if (history->_terrainMeshDestination)
							rayCastEnd = glmRaycastClosest(history->_terrainMeshDestination, rayOriginDestination, rayEndDestination, collisionPointDestination, collisionNormalDestination, simulationDataIn->_proxyMatrix, simulationDataIn->_proxyMatrixInverse);
						else
						{
							collisionPointDestination[0] = rayOriginDestination[0];
							collisionPointDestination[1] = 0.f;
							collisionPointDestination[2] = rayOriginDestination[2];
							collisionNormalDestination[0] = 0.f;
							collisionNormalDestination[1] = 1.f;
							collisionNormalDestination[2] = 0.f;
							rayCastEnd = 1;
						}
						if (rayCastEnd)
						{
							float potentialDeltaGroundHeight = collisionPointDestination[1] - collisionPointSource[1] - tr->_matrixBase[13];
							if (potentialDeltaGroundHeight < deltaGroundHeight || firstRaycast)
							{
								firstRaycast = 0;
								deltaGroundHeight = potentialDeltaGroundHeight;

								if (history->_options&(uint32_t)(OptionsGroundAdaptOrient))
								{
									glmRotationBetweenUnitVectors(collisionNormalSource, collisionNormalDestination, deltaGroundOri);
									glmNormalizeQuaternion(deltaGroundOri);
								}
							}
						}
					} // iRayCastPass2
				} // if (hasRayCast)
			} // iRayCastPass1
			glmTransformPoint((*bonePositionsPtrSource), tr->_matrixBase, transformedRootPos);
			
			glmSetIdentityMatrix(preRot);
			glmSetIdentityMatrix(postRot);

			preRot[12] = -transformedRootPos[0];
			preRot[13] = -transformedRootPos[1];
			preRot[14] = -transformedRootPos[2];

			postRot[12] = transformedRootPos[0];
			postRot[13] = transformedRootPos[1] + deltaGroundHeight;
			postRot[14] = transformedRootPos[2];

			glmConvertMatrix(groundRot, nulTranslation, deltaGroundOri);
			glmMultMatrix(preRot, groundRot, deltaGroundMatIntermediate);
			glmMultMatrix(deltaGroundMatIntermediate, postRot, deltaGroundMat);
			glmMultMatrix(tr->_matrixBase, deltaGroundMat, tr->_matrix);
			glmMultQuaternion(deltaGroundOri, tr->_orientationBase, tr->_orientation);
			
		}
	}


	// copy source
	for (entityTypeIndex = 0; entityTypeIndex<simulationDataIn->_entityTypeCount;entityTypeIndex ++)
	{
		for (iTransform = 0; iTransform < entityTransformCount; iTransform++)
		{
			uint16_t entityTypeId;
			unsigned int iValue;
			int sourceIndexInCrowdField;
			int localIndexSource;
			int localIndexDestination;
			int geoBeSourceIndex;
			int geoBeDestinationIndex;

			if (entityTransforms[iTransform]._entityId<0)
				continue;

			sourceIndexInCrowdField = entityTransforms[iTransform]._sourceIndexInCrowdField;
			entityTypeId = simulationDataIn->_entityTypes[sourceIndexInCrowdField];
			if ( entityTypeId != entityTypeIndex )
				continue;

			localIndexSource = simulationDataIn->_indexInEntityType[sourceIndexInCrowdField];
			localIndexDestination = simulationDataOut->_indexInEntityType[iTransform];

			geoBeSourceIndex = simulationDataIn->_iGeoBehaviorOffsetPerEntityType[entityTypeIndex] + localIndexSource;
			geoBeDestinationIndex = simulationDataOut->_iGeoBehaviorOffsetPerEntityType[entityTypeIndex] + localIndexDestination;


			// copy pp attributes
			for (iValue = 0; iValue < simulationDataIn->_ppFloatAttributeCount; iValue++)
			{
				frameOut->_ppFloatAttributeData[iValue][iTransform] = frameDataIn->_ppFloatAttributeData[iValue][sourceIndexInCrowdField];
			}

			for (iValue = 0; iValue< simulationDataIn->_ppVectorAttributeCount; iValue++)
			{
				unsigned int iComponent;
				for (iComponent = 0; iComponent< 3; iComponent++)
					frameOut->_ppVectorAttributeData[iValue][iTransform][iComponent] = frameDataIn->_ppVectorAttributeData[iValue][sourceIndexInCrowdField][iComponent];
			}
			
			// bones/sns/...
			if (fabsf(entityTransforms[iTransform]._frameOffset._fraction) < FLT_EPSILON)
			{
				// simple copy 
				int framesLoadedIndex = entityTransforms[iTransform]._frameOffset._framesLoadedIndex;
				const GlmFrameData* frameDataInput = framesToLoad[framesLoadedIndex]._frame;

				if (!frameDataInput)
				{
					entityTransforms[iTransform]._outOfCache = 1;
					glmVoidEntityFrameData(frameOut, simulationDataIn, simulationDataOut,
						entityTransforms[iTransform]._sourceIndexInCrowdField, iTransform,
						geoBeDestinationIndex);
				}
				else
				{
					glmCopyEntityFrameData(frameDataInput, frameOut, simulationDataIn, simulationDataOut,
						entityTransforms[iTransform]._sourceIndexInCrowdField, iTransform,
						geoBeSourceIndex, geoBeDestinationIndex,
						&entityTransforms[iTransform]);
				}
			}
			else
			{
				// interpolated copy
				int framesLoadedIndex = entityTransforms[iTransform]._frameOffset._framesLoadedIndex;
				float fraction = entityTransforms[iTransform]._frameOffset._fraction;
				const GlmFrameData* frameDataInput1 = framesToLoad[framesLoadedIndex]._frame;
				const GlmFrameData* frameDataInput2 = framesToLoad[framesLoadedIndex+1]._frame;

				if (frameDataInput1 || frameDataInput2)
				{
					if (!frameDataInput1 || !frameDataInput2)
					{
						// any chance of missing 1 data?
						const GlmFrameData* frameDataInput = frameDataInput1 ? frameDataInput1 : frameDataInput2;
						glmCopyEntityFrameData(frameDataInput, frameOut, simulationDataIn, simulationDataOut,
							entityTransforms[iTransform]._sourceIndexInCrowdField, iTransform,
							geoBeSourceIndex, geoBeDestinationIndex,
							&entityTransforms[iTransform]);
					}
					else
					{
						glmInterpolateEntityFrameData(frameDataInput1, frameDataInput2, fraction, frameOut, simulationDataIn, simulationDataOut,
							entityTransforms[iTransform]._sourceIndexInCrowdField, iTransform,
							geoBeSourceIndex, geoBeDestinationIndex,
							&entityTransforms[iTransform]);
					}
				}
				else
				{
					entityTransforms[iTransform]._outOfCache = 1;
					glmVoidEntityFrameData(frameOut, simulationDataIn, simulationDataOut,
						entityTransforms[iTransform]._sourceIndexInCrowdField, iTransform,
						geoBeDestinationIndex);
				}
			}
		}
	}

	// bone edit : when the posture has been modified for 1 frame or whole simulation 
	for (iTransform = 0; iTransform < entityTransformCount; iTransform++)
	{
		uint16_t entityTypeIndex;
		unsigned int boneCount;
		unsigned int localBoneOffset;
		const float(*boneLocalOri)[4];
		const float(*boneLocalPos)[3];
		uint32_t offsetDest;
		float(*bonePositionsPtrDest)[3];
		float(*boneOrientationPtrDest)[4];
		uint32_t *parentIndex;
		float skeletonScale;
		uint16_t snsCount;
		unsigned int i;

		float(*frameRestRelativeOrientationPtrSrc)[4] = NULL;

		GlmEntityTransform* tr = &entityTransforms[iTransform];

		// search for a full frame reste relative posture
		for (iFrame = 0; iFrame < tr->_postureCount; iFrame++)
		{
			if ( tr->_postureFrames[iFrame] == (unsigned int)currentFrame )
			{
				uint32_t offsetSrc = iFrame * tr->_postureBoneCount;
				frameRestRelativeOrientationPtrSrc = tr->_posturesOrientations + offsetSrc;
			}
		}

		// no RR to apply ?
		if ( tr->_boneRestRelativeOrientation == 0 && frameRestRelativeOrientationPtrSrc == 0)
			continue;

		// init
		entityTypeIndex = simulationDataIn->_entityTypes[tr->_sourceIndexInCrowdField];
		boneCount = simulationDataIn->_boneCount[entityTypeIndex];
		localBoneOffset = history->_localBoneOffset[entityTypeIndex];
		boneLocalOri = (const float(*)[4])history->_localBoneOrientation + localBoneOffset;
		boneLocalPos = (const float(*)[3])history->_localBonePosition + localBoneOffset;
		offsetDest = simulationDataOut->_iBoneOffsetPerEntityType[entityTypeIndex] + tr->_postureBoneCount * simulationDataOut->_indexInEntityType[iTransform];
		bonePositionsPtrDest = frameOut->_bonePositions + offsetDest;
		boneOrientationPtrDest = frameOut->_boneOrientations + offsetDest;
		parentIndex = history->_localBoneParent + localBoneOffset;
		skeletonScale = simulationDataIn->_scales[tr->_sourceIndexInCrowdField] * tr->_scale;
		snsCount = simulationDataIn->_snsCountPerEntityType[entityTypeIndex];

		if (!frameRestRelativeOrientationPtrSrc)
		{
			// map posture to hierarchical order so we can have hierarchical operations
			for (i = 0;i<boneCount;i++)
			{
				int sortedBoneIndex = (parentIndex[i]&0xFFFF);
				memcpy(entityTransforms->_sortedBonesWorldOri[sortedBoneIndex], boneOrientationPtrDest[i], sizeof(float) * 4);
				memcpy(entityTransforms->_sortedBonesWorldPos[sortedBoneIndex], bonePositionsPtrDest[i], sizeof(float) * 3);
			}
			
			// get Rest relative
			glmComputeRestRelativesOrientationFromPosture((const float(*)[4])entityTransforms->_sortedBonesWorldOri, boneLocalOri, parentIndex, boneOrientationPtrDest[0], entityTransforms->_restRelativeOri, boneCount);
		
			// multiply RR (add delta)
			for (i = 0;i<boneCount;i++)
			{
				float workOri[4];
			
				glmMultQuaternion(entityTransforms->_restRelativeOri[i], tr->_boneRestRelativeOrientation[i], workOri);
				glmNormalizeQuaternion(workOri);
				memcpy(entityTransforms->_restRelativeOri[i], workOri, sizeof(float) * 4);
			}
		}
		else
		{
			unsigned int iTransformHistory;
			// copy maya RR posture to local RR posture. frame RR are in Golaem order.
			for (i = 0;i<boneCount;i++)
			{
				memcpy(entityTransforms->_restRelativeOri[i], frameRestRelativeOrientationPtrSrc[i], sizeof(float) * 4);
				//memcpy(entityTransforms->_sortedBonesWorldPos[sortedBoneIndex], frameRestRelativePositionPtrSrc[i], sizeof(float) * 3);
			}
			// apply later bone edit
			for (iTransformHistory = tr->_lastEditPostureHistoryIndex; iTransformHistory < history->_transformCount; iTransformHistory++)
			{
				if (history->_active[iTransformHistory] && 
					history->_transformTypes[iTransformHistory] == SimulationCachePostureBoneEdit && 
					history->_entityIds[history->_entityArrayStartIndex[iTransformHistory]] == tr->_entityId)
				{
					unsigned int boneIndex = history->_boneIndex[iTransformHistory];
					float workOri[4];

					glmMultQuaternion(history->_transformRotate[iTransformHistory], entityTransforms->_restRelativeOri[boneIndex], workOri);
					glmNormalizeQuaternion(workOri);
					memcpy(entityTransforms->_restRelativeOri[boneIndex], workOri, sizeof(float) * 4);
				}
			}
		}

		// compute back world pos/ori from RR
		if (!snsCount)
		{
			glmComputePostureFromRestRelativeOrientations(entityTransforms->_sortedBonesWorldPos, entityTransforms->_sortedBonesWorldOri, boneLocalOri, boneLocalPos, parentIndex, 
				bonePositionsPtrDest[0], boneOrientationPtrDest[0], (const float(*)[4])entityTransforms->_restRelativeOri, skeletonScale, boneCount);
		}
		else
		{
			uint32_t offsetSns = simulationDataOut->_snsOffsetPerEntityType[entityTypeIndex] + snsCount * simulationDataOut->_indexInEntityType[iTransform];
			float(*boneSnsPtr)[4] = frameOut->_snsValues + offsetSns;

			for (i = 0;i<boneCount;i++)
			{
				int sortedBoneIndex = (parentIndex[i]&0xFFFF);
				memcpy(entityTransforms->_sortedBonesScale[sortedBoneIndex], boneSnsPtr[i], sizeof(float) * 4);
			}
			glmComputePostureFromRestRelativeOrientationsScales(entityTransforms->_sortedBonesWorldPos, entityTransforms->_sortedBonesWorldOri, boneLocalOri, 
				boneLocalPos, parentIndex, bonePositionsPtrDest[0], boneOrientationPtrDest[0], (const float(*)[4])entityTransforms->_restRelativeOri, skeletonScale, boneCount, (const float(*)[4])entityTransforms->_sortedBonesScale);
		}

		// remap values -> put back values in cache order
		for (i = 0;i<boneCount;i++)
		{
			int sortedBoneIndex = (parentIndex[i]&0xFFFF);
			memcpy(boneOrientationPtrDest[i], entityTransforms->_sortedBonesWorldOri[sortedBoneIndex], sizeof(float) * 4);
			memcpy(bonePositionsPtrDest[i], entityTransforms->_sortedBonesWorldPos[sortedBoneIndex], sizeof(float) * 3);
		}

	}

	// cloth
	if (totalClothEntityCount)
	{
		int clothAv = 0;
		float(*clothReferenceDest)[3];
		float* clothMaxExtentDest;

		uint32_t* clothIndicesDest;
		uint32_t* clothMeshVertexCountDest;
		float(*clothVerticesDest)[3];

		// allocate cloth
		glmCreateClothData( simulationDataOut, frameOut, totalClothEntityCount, totalClothTotalIndices, totalClothTotalVertices );

		
		clothReferenceDest = frameOut->_clothEntityQuantizationReference;
		clothMaxExtentDest = frameOut->_clothEntityQuantizationMaxExtent;

		clothIndicesDest = frameOut->_clothMeshIndicesInCharAssets;
		clothMeshVertexCountDest = frameOut->_clothMeshVertexCount;
		clothVerticesDest = frameOut->_clothVertices;

		// compute cloth reference/extent
		for (i = 0 ; i < entityTransformCount; ++i)
		{
			// patch EntityTransform for cloth
			frameOut->_entityClothIndex[i] = entityTransforms[i]._useCloth ? entityTransforms[i]._clothedEntityIndex : -1;
			if (frameOut->_entityClothIndex[i] != -1)
			{
				float clothMin[] = {FLT_MAX,FLT_MAX,FLT_MAX};
				float clothMax[] = {-FLT_MAX,-FLT_MAX,-FLT_MAX};
				float maxExtent;
				unsigned int iVertexGroup;
				unsigned int iVertex;
				unsigned int iComp;
				unsigned int meshIndexCount;

				float(*clothVerticesSource)[3] = entityTransforms[i]._clothVerticesSource;
				uint32_t* clothIndicesSource = entityTransforms[i]._clothIndicesSource;
				uint32_t* clothMeshVertexCountSource = entityTransforms[i]._clothMeshVertexCountSource;
								
				// helpers for reading
				frameOut->_clothEntityFirstAssetMeshIndex[clothAv] = (uint32_t)(clothIndicesDest - frameOut->_clothMeshIndicesInCharAssets);  // write indices offset when beginning a new cloth entity for helper
				frameOut->_clothEntityFirstMeshVertex[clothAv] = (uint32_t)(clothVerticesDest - frameOut->_clothVertices); // write vertices offset when beginning a new cloth entity for helper

				meshIndexCount = frameDataIn->_clothEntityMeshCount[entityTransforms[i]._clothedEntityIndex];
				frameOut->_clothEntityMeshCount[clothAv] = meshIndexCount;
				memcpy( clothMeshVertexCountDest, clothMeshVertexCountSource, sizeof(uint32_t) * meshIndexCount );
				memcpy( clothIndicesDest, clothIndicesSource, sizeof(uint32_t) * meshIndexCount );

				clothMeshVertexCountDest += meshIndexCount;
				clothIndicesDest += meshIndexCount;

				for (iVertexGroup = 0;iVertexGroup<meshIndexCount;iVertexGroup++)
				{
					size_t groupVertexCount = frameOut->_clothMeshVertexCount[iVertexGroup];
					
					for (iVertex = 0 ; iVertex< groupVertexCount; iVertex ++)
					{

						glmTransformPoint(&(*clothVerticesSource)[0], entityTransforms[i]._matrix, &(*clothVerticesDest)[0]);
						
						for (iComp=0;iComp<3;iComp++)
						{
							float coord = (*clothVerticesDest)[iComp];
							float pivotCoord = entityTransforms[i]._scalePivot[iComp];
							coord = (coord - pivotCoord) * entityTransforms[i]._scale + pivotCoord;
							if ( coord<clothMin[iComp] ) clothMin[iComp] = coord;
							if ( coord>clothMax[iComp] ) clothMax[iComp] = coord;
							(*clothVerticesDest)[iComp] = coord;
							
						}

						clothVerticesSource++;
						clothVerticesDest++;
					}
				}

				(*clothReferenceDest)[0] = (clothMax[0] + clothMin[0]) * 0.5f;
				(*clothReferenceDest)[1] = (clothMax[1] + clothMin[1]) * 0.5f;
				(*clothReferenceDest)[2] = (clothMax[2] + clothMin[2]) * 0.5f;

				maxExtent =  (clothMax[0] - clothMin[0]) * 0.5f;
				maxExtent = (((clothMax[1] - clothMin[1]) * 0.5f)>maxExtent)?((clothMax[1] - clothMin[1]) * 0.5f):maxExtent;
				maxExtent = (((clothMax[2] - clothMin[2]) * 0.5f)>maxExtent)?((clothMax[2] - clothMin[2]) * 0.5f):maxExtent;

				*clothMaxExtentDest = maxExtent;

				clothReferenceDest++;
				clothMaxExtentDest++;
				clothAv++;
			}
		}
	}

	for (i = 0; i < totalFrameOffsets; i++)
	{
		if (framesToLoad[i]._frame && framesToLoad[i]._frameIndex != currentFrame)
		{
			glmDestroyFrameData(&framesToLoad[i]._frame, simulationDataIn);
		}
	}
	GLMC_FREE(framesToLoad);

	return GSC_SUCCESS;
}

void glmInterpolateFrameData(const GlmSimulationData* simulationData, const GlmFrameData* frameData1, const GlmFrameData* frameData2, float ratio, GlmFrameData* result)
{
	uint32_t boneValuesCount;
	uint32_t snsValuesCount;
	uint32_t blindDataCount;	
	float tempQuat[4];
	uint32_t iValue;
	uint16_t iEntityType;

	boneValuesCount = 0;

	if (!frameData2)
		frameData2 = frameData1;
	if (!frameData1)
		frameData1 = frameData2;
	if (!frameData1)
		return;

	if (frameData1->_simulationContentHashKey != simulationData->_contentHashKey || frameData1->_simulationContentHashKey != frameData2->_simulationContentHashKey)
		return; // forbidden, different simu

				// compute total bone arrays size :
	for (iEntityType = 0; iEntityType < simulationData->_entityTypeCount; iEntityType++)
	{
		boneValuesCount += simulationData->_entityCountPerEntityType[iEntityType] * simulationData->_boneCount[iEntityType];
	}
	
	// interpolate posture positions / orientations
	for (iValue = 0; iValue < boneValuesCount; iValue++)
	{
		float *q1;
		float *q2;
		float *qResult;
		float cosom;

		result->_bonePositions[iValue][0] = interpolateFloat(frameData1->_bonePositions[iValue][0],frameData2->_bonePositions[iValue][0],ratio);
		result->_bonePositions[iValue][1] = interpolateFloat(frameData1->_bonePositions[iValue][1],frameData2->_bonePositions[iValue][1],ratio);
		result->_bonePositions[iValue][2] = interpolateFloat(frameData1->_bonePositions[iValue][2],frameData2->_bonePositions[iValue][2],ratio);

		// see interpolate_noFactorRangeCheck, can't make quaternion here
		q1 = frameData1->_boneOrientations[iValue];
		q2 = frameData2->_boneOrientations[iValue];
		qResult = result->_boneOrientations[iValue];

		cosom = (q1[0] * q2[0] + q1[1] * q2[1] + q1[2] * q2[2] + q1[3] * q2[3]);

		if (cosom < 0.f)
		{
			tempQuat[0] = -q2[0];
			tempQuat[1] = -q2[1];
			tempQuat[2] = -q2[2];
			tempQuat[3] = -q2[3];
		}
		else
		{
			tempQuat[0] = q2[0];
			tempQuat[1] = q2[1];
			tempQuat[2] = q2[2];
			tempQuat[3] = q2[3];
		}

		// linear interp
		qResult[0] = interpolateFloat(q1[0], tempQuat[0], ratio);
		qResult[1] = interpolateFloat(q1[1], tempQuat[1], ratio);
		qResult[2] = interpolateFloat(q1[2], tempQuat[2], ratio);
		qResult[3] = interpolateFloat(q1[3], tempQuat[3], ratio);

		// normalize is needed after per component linear interpolation. SSE is not available here as we may not be aligned
		{
			static const float very_small_float = 1.0e-037f; // from http://altdevblogaday.com/2011/08/21/practical-flt-point-tricks/, adding a very small float avoid testing if == 0
			float factor = (1.f / sqrtf(qResult[0] * qResult[0] + qResult[1] * qResult[1] + qResult[2] * qResult[2] + qResult[3] * qResult[3] + very_small_float));
			qResult[0] *= factor;
			qResult[1] *= factor;
			qResult[2] *= factor;
			qResult[3] *= factor;
		}
	}

	// interpolate snsValues
	snsValuesCount = 0;
	for (iEntityType = 0; iEntityType < simulationData->_entityTypeCount; iEntityType++)
	{
		snsValuesCount += simulationData->_entityCountPerEntityType[iEntityType] * simulationData->_snsCountPerEntityType[iEntityType];
	}
	// interpolate posture positions / orientations
	for (iValue = 0; iValue < snsValuesCount; iValue++)
	{
		result->_snsValues[iValue][0] = interpolateFloat(frameData1->_snsValues[iValue][0], frameData2->_snsValues[iValue][0], ratio);
		result->_snsValues[iValue][1] = interpolateFloat(frameData1->_snsValues[iValue][1], frameData2->_snsValues[iValue][1], ratio);
		result->_snsValues[iValue][2] = interpolateFloat(frameData1->_snsValues[iValue][2], frameData2->_snsValues[iValue][2], ratio);
		result->_snsValues[iValue][3] = interpolateFloat(frameData1->_snsValues[iValue][3], frameData2->_snsValues[iValue][3], ratio);
	}

	// interpolate geoBehavior
	if (frameData1->_geoBehaviorAnimFrameInfo != NULL)
	{
		// IDs and modes should not be interpolated, animframeinfo only on "current" part, so just copy everything from frame1 and change what needs to be changed
		memcpy(result->_geoBehaviorAnimFrameInfo, frameData1->_geoBehaviorAnimFrameInfo, sizeof(float) * 3 * simulationData->_entityCount);
		memcpy(result->_geoBehaviorBlendModes, frameData1->_geoBehaviorBlendModes, sizeof(uint8_t) * simulationData->_entityCount);
		memcpy(result->_geoBehaviorGeometryIds, frameData1->_geoBehaviorGeometryIds, sizeof(uint16_t) * simulationData->_entityCount);

		for (iValue = 0; iValue < simulationData->_entityCount; iValue++) // beware !! this should be processed in entityType/entity order but here order does not matter as we interpolate all
		{
			// animFrameInfo 0 = current, 
			float frame1Current = frameData1->_geoBehaviorAnimFrameInfo[iValue][0];
			float frame2Current = frameData2->_geoBehaviorAnimFrameInfo[iValue][0];
			float* resultCurrent = &(result->_geoBehaviorAnimFrameInfo[iValue][0]);

			if (frame1Current > frame2Current)
			{
				float startAttribute = frameData1->_geoBehaviorAnimFrameInfo[iValue][1];
				float stopAttribute = frameData1->_geoBehaviorAnimFrameInfo[iValue][2];

				// in case of loop behavior, with a cycle [5-25], we can have val1 = 23.5 & val2 = 5.5
				// we have to interpolate 23.5 with 23.5 + 5.5 and to offset the result in the range of the cycle
				float frameStep = (stopAttribute - frame1Current) + (frame2Current - startAttribute);
				float interFrame = frame1Current + frameStep * ratio;
				if (interFrame <= stopAttribute)
					*resultCurrent = interFrame;
				else
					*resultCurrent = interFrame - stopAttribute + startAttribute;
			}
			else
			{
				*resultCurrent = interpolateFloat(frame1Current, frame2Current, ratio);
			}
		}
	}

	// interpolate blindData
	blindDataCount = 0;
	for (iEntityType = 0; iEntityType < simulationData->_entityTypeCount; iEntityType++)
	{
		blindDataCount += simulationData->_entityCountPerEntityType[iEntityType] * simulationData->_blindDataCount[iEntityType];
	}
	for (iValue = 0; iValue < blindDataCount; iValue++)
	{
		result->_blindData[iValue] = interpolateFloat(frameData1->_blindData[iValue], frameData2->_blindData[iValue], ratio);
	}

	// interpolate pp attributes -> they are just copied we can't be sure they can be interpolated
	for (iValue = 0; iValue < simulationData->_ppFloatAttributeCount; iValue++)
	{
		memcpy(result->_ppFloatAttributeData[iValue], frameData1->_ppFloatAttributeData[iValue], simulationData->_entityCount * sizeof(float));
	}

	for (iValue = 0; iValue< simulationData->_ppVectorAttributeCount; iValue++)
	{
		memcpy(result->_ppVectorAttributeData[iValue], frameData1->_ppVectorAttributeData[iValue], simulationData->_entityCount * sizeof(float) * 3);
	}

	// interpolate cloth : copy structures from frame1 and interpolate all vertices only (we have total vertices count in frameData)
	if (frameData1->_clothEntityCount > 0 && frameData1->_clothTotalVertices > 0 && frameData1->_clothTotalVertices == frameData2->_clothTotalVertices)
	{
		glmCreateClothData(simulationData, result, frameData1->_clothEntityCount, frameData1->_clothTotalMeshIndices, frameData1->_clothTotalVertices);

		memcpy(result->_entityClothIndex, frameData1->_entityClothIndex, simulationData->_entityCount * sizeof(int32_t));

		memcpy(result->_clothEntityFirstAssetMeshIndex, frameData1->_clothEntityFirstAssetMeshIndex, result->_clothEntityCount * sizeof(uint32_t));
		memcpy(result->_clothEntityFirstMeshVertex, frameData1->_clothEntityFirstMeshVertex, result->_clothEntityCount * sizeof(uint32_t));
		memcpy(result->_clothEntityMeshCount, frameData1->_clothEntityMeshCount, result->_clothEntityCount * sizeof(uint32_t));
		memcpy(result->_clothEntityQuantizationReference, frameData1->_clothEntityQuantizationReference, result->_clothEntityCount * sizeof(float[3]));
		memcpy(result->_clothEntityQuantizationMaxExtent, frameData1->_clothEntityQuantizationMaxExtent, result->_clothEntityCount * sizeof(float));
		
		memcpy(result->_clothMeshIndicesInCharAssets, frameData1->_clothMeshIndicesInCharAssets, result->_clothAllocatedIndices * sizeof(uint32_t));
		memcpy(result->_clothMeshVertexCount, frameData1->_clothMeshVertexCount, result->_clothAllocatedIndices * sizeof(uint32_t));
		//memcpy(result->_clothMeshVertexOffsetPerClothIndex, frameData1->_clothMeshVertexOffsetPerClothIndex, clothIndices * sizeof(uint32_t));

		// interpolate vertex per vertex
		for (iValue = 0; iValue < frameData1->_clothTotalVertices; iValue++)
		{
			result->_clothVertices[iValue][0] = interpolateFloat(frameData1->_clothVertices[iValue][0], frameData2->_clothVertices[iValue][0], ratio);
			result->_clothVertices[iValue][1] = interpolateFloat(frameData1->_clothVertices[iValue][1], frameData2->_clothVertices[iValue][1], ratio);
			result->_clothVertices[iValue][2] = interpolateFloat(frameData1->_clothVertices[iValue][2], frameData2->_clothVertices[iValue][2], ratio);
		}
	}
}

uint32_t getClothEntityMeshCount(const GlmFrameData* frameData, int clothEntityIndex)
{
	return frameData->_clothEntityMeshCount[clothEntityIndex];
}

uint32_t getClothEntityIMeshVertexCount(const GlmFrameData* frameData, int clothEntityIndex, int iMesh)
{
	int clothMeshIndexInCache = frameData->_clothEntityFirstAssetMeshIndex[clothEntityIndex] + iMesh;
	return frameData->_clothMeshVertexCount[clothMeshIndexInCache];
}

void getClothEntityIMeshVerticesPtr(const GlmFrameData* frameData, int clothEntityIndex, int iMesh, float(**outFirstVertexPtr)[3])
{
	// skip previous meshes count :
	int firstMeshInClothIndices = frameData->_clothEntityFirstAssetMeshIndex[clothEntityIndex];

	int previousMeshesVertexOffset = 0;
	int i = 0;
	for (; i < iMesh; i++)
	{
		previousMeshesVertexOffset += frameData->_clothMeshVertexCount[firstMeshInClothIndices + i];
	}

	// acces to vertices at clothEntity first vertex + previousMeshesVertexOffset:
	*outFirstVertexPtr = &frameData->_clothVertices[frameData->_clothEntityFirstMeshVertex[clothEntityIndex] + previousMeshesVertexOffset];
}

#undef GLMC_IMPLEMENTATION
#endif // GLMC_IMPLEMENTATION
