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

	You can #define GLMC_ASSERT(x) before the #include to avoid using GLMC_ASSERT.h.
	And #define GLMC_MALLOC, GLMC_REALLOC, and GLMC_FREE to avoid using GLMC_MALLOC, GLMC_REALLOC, GLMC_FREE

	QUICK NOTES:
	Primarily of interest to pipeline developers to integrate Golaem Crowd simulation cache

	Brief documentation under "DOCUMENTATION" below.

	Revision 0.00 (2015-03-12) release notes:

	- Functions and structures for creation, read/write and destruction of Golaem Simulation Caches
*/

#ifndef GLM_CROWD_INCLUDE_H
#define GLM_CROWD_INCLUDE_H

#include "stdint.h"

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

	extern const char partioFrameExtension[]; // linux warning else
	extern const char golaemFrameExtension[];
	extern const char golaemSimulationExtension[];

	// Simulation cache format----------------------------------------------------
	typedef enum
	{
		GSC_PARTIO, // Disney Partio particles file format
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
		GSC_PP_INT = 0,
		GSC_PP_FLOAT = 1,
		GSC_PP_VECTOR = 2
	} GlmPPAttributeType;

#define GSC_PP_MAX_NAME_LENGTH 256

	// per-simulation data
	typedef struct GlmSimulationData_v0
	{
		uint32_t	_contentHashKey; // to check if simulation matches frame file

		// entity. 
		// /!\ Order is ParticleSystem one, i.e. entities[i] can be of entityType 3, then 1, then 0, then 1, then 2, etc.
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
		uint16_t* _blendShapeCount; // number of blendshapes for this EntityType, array size = _entityTypeCount
		uint32_t* _iBlendShapeOffsetPerEntityType; // indexes of the first blendshape of the first entity of an EntityType, for _blendShapes, array size = _entityTypeCount
		uint8_t* _hasGeoBehavior; // does this entity type have a geometry behavior?, array size = _entityTypeCount
		uint32_t* _iGeoBehaviorOffsetPerEntityType; // indexes of the first geometry behavior of the first entity of an EntityType, for _geoBehaviorGeometryIds, _geoBehaviorBlendModes and _geoBehaviorBlendModes, array size = _entityTypeCount
		uint16_t* _snsCountPerEntityType;	// count of sns information per entityType (defined by gskm/gch)
		uint32_t* _snsOffsetPerEntityType;	// offset per entityType = sum of previous (_snsCountPerEntityType * entityCountPerEntityType)

		// ppAttribute
		uint8_t _ppAttributeCount; // number of per particle attributes
		char(*_ppAttributeNames)[GSC_PP_MAX_NAME_LENGTH]; // name of the per particle attribute, array size = _ppAttributeCount
		uint8_t* _ppAttributeTypes; // types of the per particle attribute, GlmPPAttributeType, array size = _ppAttributeCount
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
		float(*_snsValues)[3]; // Squatch and stretch values per EntityType, array size = _snsCountPerEntityType * entityCountPerEntityType* entityTypeCount
		float* _blendShapes; // blend shapes per Entity per EntityType, array size = _blendShapeCount * _entityCountPerEntityType * _entityTypeCount : [ET0-E0-0blendshapeCount][ET0-E1-0blendshapeCount][ET1-E0-0blendshapeCount]etc..
		uint16_t* _geoBehaviorGeometryIds; // geometry behavior geometry id, array size = _entityCountPerEntityType * _entityTypeCount
		float(*_geoBehaviorAnimFrameInfo)[3]; // geometry behavior animation frame info per Entity of an Entitype, array size =  _entityCountPerEntityType * _entityTypeCount, NULL if no geometry behavior
		uint8_t* _geoBehaviorBlendModes; // geometry behavior blend mode per Entity of an Entitype, array size =  _entityCountPerEntityType * _entityTypeCount, NULL if no geometry behavior - stores a boolean as 0 or 1

		// allocation data for cloth, to avoid GLMC_REALLOCation each frame if memoryspace is sufficient (not serialized)
		uint32_t _clothAllocatedEntities;
		uint32_t _clothAllocatedIndices;
		uint32_t _clothAllocatedVertices;

		uint32_t _clothEntityCount; // if 0, skip reading of everything else cloth-related (let to NULL)
		uint32_t _clothTotalIndices; // shortcut for reading
		uint32_t _clothTotalVertices; // shortcut for reading
		uint8_t* _entityUseCloth; // per entityType / Entity, size = _entityCountPerEntityType * _entityTypeCount, read in entity order
		uint32_t* _clothMeshIndexCount; // count of cloth meshes for this cloth entity, size = _clothEntityCount, read in entity order
		float(*_clothReference)[3]; // _clothEntityCount elements, gives reference vertex for quantization = this entity vertices (min+max)/2 in all dimensions
		float* _clothMaxExtent; //_clothEntityCount elements, gives maxExtent of cloth for quantization max of (_clothReference - min value) in all dimensions

		uint32_t* _clothIndices; // in serial, entity0 indices, then entity 1 indices, etc
		uint32_t* _clothMeshVertexCountPerClothIndex; //match _clothIndices, gives vertex count to read/write
		float(*_clothVertices)[3]; // vertices position, from ET0 { entity0_vertex0-n, entity1_vertex0-n, etc. }, ET1 { entity0_vertex0-n, entity1_vertex0-n, etc. }, ET2, etc. size = _entityTypeCount * _clothVertexCount * _entityCountPerEntityType;, only for entities using cloth

		// ppAttribute
		void** _ppAttributeData; // data per particle attributes, can be casted using the _ppAttributeType information, array size = _ppAttributeCount
	} GlmFrameData_v0;
	typedef GlmFrameData_v0 GlmFrameData;

	// Simulation cache status codes----------------------------------------------
	typedef enum
	{
		GSC_SUCCESS,
		GSC_FILE_OPEN_FAILED,
		GSC_FILE_MAGIC_NUMBER_ERROR,
		GSC_FILE_VERSION_ERROR,
		GSC_FILE_FORMAT_ERROR,
		GSC_SIMULATION_FILE_DOES_NOT_MATCH
	} GlmSimulationCacheStatus;

	const char* glmConvertSimulationCacheStatus(GlmSimulationCacheStatus status);

	// Simulation cache functions-------------------------------------------------

	// allocate *simulationData
	void glmCreateSimulationData(
		GlmSimulationData** simulationData, // *simulationData will be allocated by this function
		uint32_t entityCount, // number of entities, range = 0..4,294,967,295
		uint16_t entityTypeCount, // number of different entity types, range = 1..65,535
		uint8_t ppAttributeCount); // number of per particle attributes, range = 0..255

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
	extern void glmCreateClothData(const GlmSimulationData* simuData, GlmFrameData* frameData, unsigned int clothEntityCount, unsigned int clothIndices, unsigned int clothVertices);

	// read a .gscf file in the previously allocated *frameData
	// return GSC_SUCCESS || GSC_FILE_OPEN_FAILED || GSC_FILE_MAGIC_NUMBER_ERROR || GSC_FILE_VERSION_ERROR || GSC_FILE_FORMAT_ERROR
	extern GlmSimulationCacheStatus glmReadFrameData(GlmFrameData* frameData, const GlmSimulationData* simulationData, const char* file);

	// write *frameData in a .gscf file
	// return GSC_SUCCESS || GSC_FILE_OPEN_FAILED
	extern GlmSimulationCacheStatus glmWriteFrameData(const char* file, const GlmFrameData* frameData, const GlmSimulationData* simulationData);

	// deallocate *frameData and set it to NULL
	extern void glmDestroyFrameData(GlmFrameData** frameData, const GlmSimulationData* simulationData);

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
#ifndef GLMC_ASSERT
#include <assert.h>
#define GLMC_ASSERT(x) assert(x)
#endif

#ifndef GLMC_NOT_INCLUDE_MINIZ
#include "miniz.c"
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

#if defined(GLMC_MALLOC) && defined(GLMC_FREE) && defined(GLMC_REALLOC)
// ok
#elif !defined(GLMC_MALLOC) && !defined(GLMC_FREE) && !defined(GLMC_REALLOC)
// ok
#else
#error "Must define all or none of GLMC_MALLOC, GLMC_FREE, and GLMC_REALLOC."
#endif

#ifndef GLMC_MALLOC
#define GLMC_MALLOC(sz)    malloc(sz)
#define GLMC_REALLOC(p,sz) realloc(p,sz)
#define GLMC_FREE(p)       free(p)
#endif

#define GLMC_PI 3.14159265358979323846f
#define GLMC_PI_DIV_2 1.57079632679489661923f
#define GLMC_1_DIV_SQRT_2 0.7071067811865475f

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
static void glmFileRead(void* data, unsigned long elementSize, unsigned long count, FILE* fp)
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
static void glmFileReadUInt16(uint16_t* data, unsigned int count, FILE* fp)
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
static void glmFileReadUInt32(uint32_t* data, unsigned int count, FILE* fp)
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
static void glmFileReadUInt64(uint64_t* data, unsigned int count, FILE* fp)
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
static void glmFileWrite(const void* data, unsigned long elementSize, unsigned long count, FILE* fp)
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
static void glmFileWriteUInt16(uint16_t* data, unsigned int count, FILE* fp)
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
static void glmFileWriteUInt32(uint32_t* data, unsigned int count, FILE* fp)
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
static void glmFileWriteUInt64(uint64_t* data, unsigned int count, FILE* fp)
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

#define GSC_VERSION 0x00
#define GSCS_MAGIC_NUMBER 0x65C5
#define GSCF_MAGIC_NUMBER 0x65CF

const char partioFrameExtension[] = "pdb32.gz"; // need to be declared lowercase for comparison
const char golaemFrameExtension[] = "gscf"; // need to be declared lowercase for comparison
const char golaemSimulationExtension[] = "gscs"; // need to be declared lowercase for comparison

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
	default:
		return "Golaem simulation cache: unkown error code";
	}
}

//----------------------------------------------------------------------------
void glmCreateSimulationData(
	GlmSimulationData** simulationData,
	uint32_t entityCount,
	uint16_t entityTypeCount,
	uint8_t ppAttributeCount)
{
	GlmSimulationData* data;

	// input validation before allocation
#ifndef NDEBUG
	GLMC_ASSERT((entityCount >= 1) && "entityCount limit exceeded, range = 1..4,294,967,295");
	GLMC_ASSERT((entityTypeCount >= 1) && "entityTypeCount limit exceeded, range = 1..65,535");
#endif

	// allocate simulation data
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
	data->_blendShapeCount = (uint16_t*)GLMC_MALLOC(data->_entityTypeCount * sizeof(uint16_t));
	data->_iBlendShapeOffsetPerEntityType = (uint32_t*)GLMC_MALLOC(data->_entityTypeCount * sizeof(uint32_t));
	data->_hasGeoBehavior = (uint8_t*)GLMC_MALLOC(data->_entityTypeCount * sizeof(uint8_t));
	data->_iGeoBehaviorOffsetPerEntityType = (uint32_t*)GLMC_MALLOC(data->_entityTypeCount * sizeof(uint32_t));
	data->_snsCountPerEntityType = (uint16_t*)GLMC_MALLOC(data->_entityTypeCount * sizeof(uint16_t));
	data->_snsOffsetPerEntityType = (uint32_t*)GLMC_MALLOC(data->_entityTypeCount * sizeof(uint32_t));

	// ppAttribute
	data->_ppAttributeCount = ppAttributeCount;
	data->_ppAttributeNames = (char(*)[GSC_PP_MAX_NAME_LENGTH])GLMC_MALLOC(data->_ppAttributeCount * GSC_PP_MAX_NAME_LENGTH * sizeof(char));
	data->_ppAttributeTypes = (uint8_t*)GLMC_MALLOC(data->_ppAttributeCount * sizeof(uint8_t));
}

//----------------------------------------------------------------------------
GlmSimulationCacheStatus glmCreateAndReadSimulationData(GlmSimulationData** simulationData, const char* file)
{
	uint16_t magicNumber;
	uint8_t version;
	uint32_t entityCount;
	uint16_t entityTypeCount;
	uint8_t ppAttributeCount;
	uint32_t contentHashKey;
	GlmSimulationData* data;

#ifdef _MSC_VER				
	FILE* fp;
	fopen_s(&fp, file, "rb");
#else
	FILE* fp = fopen(file, "rb");
#endif

	if (fp == NULL) return GSC_FILE_OPEN_FAILED;

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
	glmFileRead(&ppAttributeCount, sizeof(uint8_t), 1, fp);

	// create
	glmCreateSimulationData(simulationData, entityCount, entityTypeCount, ppAttributeCount);
	data = *simulationData;

	data->_contentHashKey = contentHashKey;

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
	glmFileReadUInt16(data->_blendShapeCount, data->_entityTypeCount, fp);
	glmFileReadUInt32(data->_iBlendShapeOffsetPerEntityType, data->_entityTypeCount, fp);
	glmFileRead(data->_hasGeoBehavior, sizeof(uint8_t), data->_entityTypeCount, fp);
	glmFileReadUInt32(data->_iGeoBehaviorOffsetPerEntityType, data->_entityTypeCount, fp);
	glmFileReadUInt16(data->_snsCountPerEntityType, data->_entityTypeCount, fp);
	glmFileReadUInt32(data->_snsOffsetPerEntityType, data->_entityTypeCount, fp);

	// ppAttribute
	glmFileRead(data->_ppAttributeNames, sizeof(char), data->_ppAttributeCount * GSC_PP_MAX_NAME_LENGTH, fp);
	glmFileRead(data->_ppAttributeTypes, sizeof(uint8_t), data->_ppAttributeCount, fp);

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
	fopen_s(&fp, file, "wb");
#else
	FILE* fp = fopen(file, "wb");
#endif

	if (fp == NULL) return GSC_FILE_OPEN_FAILED;

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
		glmCumulativeHash16(data->_blendShapeCount[i], &contentHashValue);
		glmCumulativeHash16(data->_snsCountPerEntityType[i], &contentHashValue);
	}
	glmCumulativeHash8(data->_ppAttributeCount, &contentHashValue);
	for (i = 0; i < data->_ppAttributeCount; i++)
	{
		glmCumulativeHash8(data->_ppAttributeTypes[i], &contentHashValue);
	}

	// overwrite const pointer to write back contentHashKey 
	((GlmSimulationData*)data)->_contentHashKey = contentHashValue;

	// finally write hash key
	glmFileWriteUInt32(&contentHashValue, 1, fp); // cannot set value on data, const , but write it

	glmFileWriteUInt32((uint32_t*)&data->_entityCount, 1, fp);
	glmFileWriteUInt16((uint16_t*)&data->_entityTypeCount, 1, fp);
	glmFileWrite(&data->_ppAttributeCount, sizeof(uint8_t), 1, fp);

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
	glmFileWriteUInt16(data->_blendShapeCount, data->_entityTypeCount, fp);
	glmFileWriteUInt32(data->_iBlendShapeOffsetPerEntityType, data->_entityTypeCount, fp);
	glmFileWrite(data->_hasGeoBehavior, sizeof(uint8_t), data->_entityTypeCount, fp);
	glmFileWriteUInt32(data->_iGeoBehaviorOffsetPerEntityType, data->_entityTypeCount, fp);
	glmFileWriteUInt16(data->_snsCountPerEntityType, data->_entityTypeCount, fp);
	glmFileWriteUInt32(data->_snsOffsetPerEntityType, data->_entityTypeCount, fp);

	// ppAttribute
	glmFileWrite(data->_ppAttributeNames, sizeof(char), data->_ppAttributeCount * GSC_PP_MAX_NAME_LENGTH, fp);
	glmFileWrite(data->_ppAttributeTypes, sizeof(uint8_t), data->_ppAttributeCount, fp);

	fclose(fp);

	return GSC_SUCCESS;
}

//----------------------------------------------------------------------------
void glmDestroySimulationData(GlmSimulationData** simulationData)
{
	GlmSimulationData* data = *simulationData;
	GLMC_ASSERT((data != NULL) && "Simulation data must be created before being destroyed");
	GLMC_FREE(data->_ppAttributeTypes);
	GLMC_FREE(data->_ppAttributeNames);
	GLMC_FREE(data->_snsOffsetPerEntityType);
	GLMC_FREE(data->_snsCountPerEntityType);
	GLMC_FREE(data->_iGeoBehaviorOffsetPerEntityType);
	GLMC_FREE(data->_hasGeoBehavior);
	GLMC_FREE(data->_iBlendShapeOffsetPerEntityType);
	GLMC_FREE(data->_blendShapeCount);
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
	unsigned int totalBlendShapeCount = 0;
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
	for (i = 0; i < simulationData->_ppAttributeCount; ++i)
	{
		GLMC_ASSERT(((simulationData->_ppAttributeTypes[i] == GSC_PP_INT)
			|| (simulationData->_ppAttributeTypes[i] == GSC_PP_FLOAT)
			|| (simulationData->_ppAttributeTypes[i] == GSC_PP_VECTOR)) && "ppAttributeTypes type unknown");
	}
#endif

	// allocate frame data
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
		totalBlendShapeCount += simulationData->_blendShapeCount[i] * simulationData->_entityCountPerEntityType[i];
		if (simulationData->_hasGeoBehavior[i])
		{
			totalGeoBehaviorCount += simulationData->_entityCountPerEntityType[i];
		}
	}

	data->_bonePositions = (float(*)[3])GLMC_MALLOC(totalBoneCount * 3 * sizeof(float));
	data->_boneOrientations = (float(*)[4])GLMC_MALLOC(totalBoneCount * 4 * sizeof(float));
	data->_snsValues = (float(*)[3])GLMC_MALLOC(totalSnSCount * 3 * sizeof(float));
	data->_blendShapes = (float*)GLMC_MALLOC(totalBlendShapeCount * sizeof(float));
	if (totalGeoBehaviorCount > 0)
	{
		data->_geoBehaviorGeometryIds = (uint16_t*)GLMC_MALLOC(totalGeoBehaviorCount * sizeof(uint16_t));
		data->_geoBehaviorAnimFrameInfo = (float(*)[3])GLMC_MALLOC(totalGeoBehaviorCount * 3 * sizeof(float));
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
	data->_clothTotalIndices = 0;
	data->_clothTotalVertices = 0;
	data->_clothAllocatedEntities = 0;
	data->_clothAllocatedIndices = 0;
	data->_clothAllocatedVertices = 0;
	data->_entityUseCloth = NULL;
	data->_clothMeshIndexCount = NULL;
	data->_clothReference = NULL;
	data->_clothMaxExtent = NULL;
	data->_clothMeshVertexCountPerClothIndex = NULL;
	data->_clothIndices = NULL;
	data->_clothVertices = NULL;
	// ppAttribute
	data->_ppAttributeData = (void**)GLMC_MALLOC(simulationData->_ppAttributeCount * sizeof(void*));
	for (i = 0; i < simulationData->_ppAttributeCount; ++i)
	{
		switch (simulationData->_ppAttributeTypes[i])
		{
		case GSC_PP_INT:
			data->_ppAttributeData[i] = (int32_t*)GLMC_MALLOC(simulationData->_entityCount * sizeof(int32_t));
			break;
		case GSC_PP_FLOAT:
			data->_ppAttributeData[i] = (float*)GLMC_MALLOC(simulationData->_entityCount * sizeof(float));
			break;
		case GSC_PP_VECTOR:
			data->_ppAttributeData[i] = (float*)GLMC_MALLOC(simulationData->_entityCount * 3 * sizeof(float));
			break;
		default:
			data->_ppAttributeData[i] = NULL;
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

		rootBonePositions = (float(*)[3])GLMC_MALLOC(validEntityCount * 3 * sizeof(float));
		compressedBonePositions = (uint16_t(*)[3])GLMC_MALLOC((totalBoneCount - validEntityCount) * 3 * sizeof(uint16_t));
		glmFileRead(rootBonePositions, sizeof(float), validEntityCount * 3, fp);
		glmFileReadUInt16(compressedBonePositions[0], (totalBoneCount - validEntityCount) * 3, fp);
		for (iEntityType = 0; iEntityType < data->_entityTypeCount; ++iEntityType)
		{
			unsigned int iEntity;
			for (iEntity = 0; iEntity < data->_entityCountPerEntityType[iEntityType]; ++iEntity)
			{
				unsigned int iBone;
				unsigned int iBoneOffset = data->_iBoneOffsetPerEntityType[iEntityType] + iEntity * data->_boneCount[iEntityType];
				memcpy(bonesPositions[iBoneOffset], rootBonePositions[iRootBone], 3 * sizeof(float));
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
		uint16_t(*compressedClothVertices)[3] = (uint16_t(*)[3])GLMC_MALLOC(frameData->_clothTotalVertices * 3 * sizeof(uint16_t));

		glmFileReadUInt16(compressedClothVertices[0], frameData->_clothTotalVertices * 3, fp);

		// iteration per cloth entity:
		for (iClothEntity = 0; iClothEntity < frameData->_clothEntityCount; iClothEntity++)
		{
			// cloth max extent and reference must be read priori to calling this function
			float *referencePosition = frameData->_clothReference[iClothEntity];
			float maxExtent = frameData->_clothMaxExtent[iClothEntity];

			for (iClothEntityMesh = 0; iClothEntityMesh < frameData->_clothMeshIndexCount[iClothEntity]; iClothEntityMesh++)
			{
				for (iClothVertex = 0; iClothVertex < frameData->_clothMeshVertexCountPerClothIndex[iAbsoluteClothMesh]; iClothVertex++)
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
		if (data->_entityUseCloth == NULL)
		{
			data->_entityUseCloth = (uint8_t*)GLMC_MALLOC(simuData->_entityCount * sizeof(uint8_t));
		}

		if (clothEntityCount > data->_clothAllocatedEntities)
		{
			GLMC_FREE(data->_clothMeshIndexCount);
			GLMC_FREE(data->_clothReference);
			GLMC_FREE(data->_clothMaxExtent);

			data->_clothMeshIndexCount = (uint32_t*)GLMC_MALLOC(clothEntityCount * sizeof(uint32_t));
			data->_clothReference = (float(*)[3])GLMC_MALLOC(clothEntityCount * sizeof(float) * 3);
			data->_clothMaxExtent = (float*)GLMC_MALLOC(clothEntityCount * sizeof(float));

			data->_clothAllocatedEntities = clothEntityCount;
		}

		if (clothIndices > data->_clothAllocatedIndices)
		{
			GLMC_FREE(data->_clothIndices);
			GLMC_FREE(data->_clothMeshVertexCountPerClothIndex);

			data->_clothIndices = (uint32_t*)GLMC_MALLOC(clothIndices * sizeof(uint32_t));
			data->_clothMeshVertexCountPerClothIndex = (uint32_t*)GLMC_MALLOC(clothIndices * sizeof(uint32_t));

			data->_clothAllocatedIndices = clothIndices;
		}
		if (clothVertices > data->_clothAllocatedVertices)
		{
			GLMC_FREE(data->_clothVertices);
			data->_clothVertices = (float(*)[3])GLMC_MALLOC(clothVertices * sizeof(float) * 3);
			data->_clothAllocatedVertices = clothVertices;
		}

		data->_clothEntityCount = clothEntityCount;
		data->_clothTotalIndices = clothIndices;
		data->_clothTotalVertices = clothVertices;
	}
}

//----------------------------------------------------------------------------
GlmSimulationCacheStatus glmReadFrameData(GlmFrameData* data, const GlmSimulationData* simulationData, const char* file)
{
	uint16_t magicNumber;
	uint8_t version;
	uint8_t format;
	unsigned int totalBoneCount = 0;
	unsigned int totalSnSCount = 0;
	unsigned int totalBlendShapeCount = 0;
	unsigned int totalGeoBehaviorCount = 0;
	unsigned int i;

#ifdef _MSC_VER				
	FILE* fp;
	fopen_s(&fp, file, "rb");
#else
	FILE* fp = fopen(file, "rb");
#endif

	if (fp == NULL) return GSC_FILE_OPEN_FAILED;

	// header
	glmFileReadUInt16(&magicNumber, 1, fp);
	if (magicNumber != GSCF_MAGIC_NUMBER) return GSC_FILE_MAGIC_NUMBER_ERROR;
	glmFileRead(&version, sizeof(uint8_t), 1, fp);
	if (version > GSC_VERSION) return GSC_FILE_VERSION_ERROR;

	glmFileRead(&format, sizeof(uint8_t), 1, fp);
	data->_cacheFormat = format;
	if ((data->_cacheFormat <= GSC_PARTIO) || (data->_cacheFormat > GSC_O32_P48)) return GSC_FILE_FORMAT_ERROR;

	glmFileReadUInt32(&data->_simulationContentHashKey, 1, fp); // read simulation content hash key, check that it matches the simulation :

	if (data->_simulationContentHashKey != simulationData->_contentHashKey)
	{
		return GSC_SIMULATION_FILE_DOES_NOT_MATCH;
	}

	// entityType
	for (i = 0; i < simulationData->_entityTypeCount; ++i)
	{
		totalBoneCount += simulationData->_boneCount[i] * simulationData->_entityCountPerEntityType[i];
		totalSnSCount += simulationData->_snsCountPerEntityType[i] * simulationData->_entityCountPerEntityType[i];
		totalBlendShapeCount += simulationData->_blendShapeCount[i] * simulationData->_entityCountPerEntityType[i];
		if (simulationData->_hasGeoBehavior[i])
		{
			totalGeoBehaviorCount += simulationData->_entityCountPerEntityType[i];
		}
	}
	glmFileReadPositions(data->_bonePositions, totalBoneCount, simulationData, fp, (GlmSimulationCacheFormat)data->_cacheFormat);
	glmFileReadOrientations(data->_boneOrientations, totalBoneCount, fp, (GlmSimulationCacheFormat)data->_cacheFormat);
	glmFileRead(data->_snsValues, sizeof(float), totalSnSCount * 3, fp);
	glmFileRead(data->_blendShapes, sizeof(float), totalBlendShapeCount, fp);
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
		glmFileReadUInt32(&data->_clothTotalIndices, 1, fp);
		glmFileReadUInt32(&data->_clothTotalVertices, 1, fp);

		if (data->_clothTotalIndices != 0)
		{
			glmCreateClothData(simulationData, data, data->_clothEntityCount, data->_clothTotalIndices, data->_clothTotalVertices);
			glmFileRead(data->_entityUseCloth, sizeof(uint8_t), simulationData->_entityCount, fp);
			glmFileReadUInt32(data->_clothMeshIndexCount, data->_clothEntityCount, fp);
			glmFileRead(data->_clothReference, sizeof(float), data->_clothEntityCount * 3, fp);
			glmFileRead(data->_clothMaxExtent, sizeof(float), data->_clothEntityCount, fp);
			//glmFileRead(data->_clothMeshIndexOffsetPerEntityInUse, sizeof(uint32_t), data->_clothEntityCount, fp);
			//glmFileRead(data->_clothMeshVertexOffsetPerEntityInUse, sizeof(uint32_t), data->_clothEntityCount, fp);
			glmFileReadUInt32(data->_clothIndices, data->_clothTotalIndices, fp);
			glmFileReadUInt32(data->_clothMeshVertexCountPerClothIndex, data->_clothTotalIndices, fp);
			if (data->_clothTotalVertices > 0)
			{
				glmFileReadClothVertices(data, fp, (GlmSimulationCacheFormat)data->_cacheFormat);
			}
		}
	}


	// ppAttribute
	for (i = 0; i < simulationData->_ppAttributeCount; ++i)
	{
		switch (simulationData->_ppAttributeTypes[i])
		{
		case GSC_PP_INT:
			glmFileReadUInt32((uint32_t*)data->_ppAttributeData[i], simulationData->_entityCount, fp);
			break;
		case GSC_PP_FLOAT:
			glmFileRead(data->_ppAttributeData[i], sizeof(float), simulationData->_entityCount, fp);
			break;
		case GSC_PP_VECTOR:
			glmFileRead(data->_ppAttributeData[i], sizeof(float), simulationData->_entityCount * 3, fp);
			break;
		default:
			data->_ppAttributeData[i] = NULL;
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

		rootBonePositions = (float(*)[3])GLMC_MALLOC(validEntityCount * 3 * sizeof(float));
		compressedBonePositions = (uint16_t(*)[3])GLMC_MALLOC((totalBoneCount - validEntityCount) * 3 * sizeof(uint16_t));

		for (iEntityType = 0; iEntityType < data->_entityTypeCount; ++iEntityType)
		{
			unsigned int iEntity;
			for (iEntity = 0; iEntity < data->_entityCountPerEntityType[iEntityType]; ++iEntity)
			{
				unsigned int iBone;
				unsigned int iBoneOffset = data->_iBoneOffsetPerEntityType[iEntityType] + iEntity * data->_boneCount[iEntityType];
				memcpy(rootBonePositions[iRootBone], bonesPositions[iBoneOffset], 3 * sizeof(float));
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

		uint16_t(*compressedVertices)[3] = (uint16_t(*)[3])GLMC_MALLOC(frameData->_clothTotalVertices * 3 * sizeof(uint16_t));

		int iAbsoluteClothMesh = 0;
		int iAbsoluteMeshVertex = 0;

		// iteration per cloth entity:
		for (iClothEntity = 0; iClothEntity < frameData->_clothEntityCount; iClothEntity++)
		{
			float *referencePosition = frameData->_clothReference[iClothEntity];
			float maxExtent = frameData->_clothMaxExtent[iClothEntity];

			for (iClothEntityMesh = 0; iClothEntityMesh < frameData->_clothMeshIndexCount[iClothEntity]; iClothEntityMesh++)
			{
				for (iClothVertex = 0; iClothVertex < frameData->_clothMeshVertexCountPerClothIndex[iAbsoluteClothMesh]; iClothVertex++)
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
	unsigned int totalBlendShapeCount = 0;
	unsigned int totalGeoBehaviorCount = 0;
	unsigned int i;

#ifdef _MSC_VER				
	FILE* fp;
	fopen_s(&fp, file, "wb");
#else
	FILE* fp = fopen(file, "wb");
#endif

	if (fp == NULL) return GSC_FILE_OPEN_FAILED;

	// entityType
	for (i = 0; i < simulationData->_entityTypeCount; ++i)
	{
		totalBoneCount += simulationData->_boneCount[i] * simulationData->_entityCountPerEntityType[i];
		totalSnSCount += simulationData->_snsCountPerEntityType[i] * simulationData->_entityCountPerEntityType[i];
		totalBlendShapeCount += simulationData->_blendShapeCount[i] * simulationData->_entityCountPerEntityType[i];
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
	if (version > GSC_VERSION) return GSC_FILE_VERSION_ERROR;

	glmFileWrite(&(data->_cacheFormat), sizeof(uint8_t), 1, fp);

	glmFileWriteUInt32((uint32_t*)&data->_simulationContentHashKey, 1, fp);

	glmFileWritePositions(data->_bonePositions, totalBoneCount, simulationData, fp, (GlmSimulationCacheFormat)data->_cacheFormat);
	glmFileWriteOrientations(data->_boneOrientations, totalBoneCount, fp, (GlmSimulationCacheFormat)data->_cacheFormat);
	glmFileWrite(data->_snsValues, sizeof(float), totalSnSCount * 3, fp);
	glmFileWrite(data->_blendShapes, sizeof(float), totalBlendShapeCount, fp);
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
		glmFileWriteUInt32((uint32_t*)&data->_clothTotalIndices, 1, fp);
		glmFileWriteUInt32((uint32_t*)&data->_clothTotalVertices, 1, fp);

		if (data->_clothTotalIndices > 0)
		{
			glmFileWrite(data->_entityUseCloth, sizeof(uint8_t), simulationData->_entityCount, fp);
			glmFileWriteUInt32(data->_clothMeshIndexCount, data->_clothEntityCount, fp);
			glmFileWrite(data->_clothReference, sizeof(float), data->_clothEntityCount * 3, fp);
			glmFileWrite(data->_clothMaxExtent, sizeof(float), data->_clothEntityCount, fp);

			glmFileWriteUInt32(data->_clothIndices, data->_clothTotalIndices, fp);
			glmFileWriteUInt32(data->_clothMeshVertexCountPerClothIndex, data->_clothTotalIndices, fp);

			if (data->_clothTotalVertices > 0)
			{
				glmFileWriteClothVertices(data, fp, (GlmSimulationCacheFormat)data->_cacheFormat);
			}
		}
	}

	// ppAttribute
	for (i = 0; i < simulationData->_ppAttributeCount; ++i)
	{
		switch (simulationData->_ppAttributeTypes[i])
		{
		case GSC_PP_INT:
			glmFileWriteUInt32((uint32_t*)data->_ppAttributeData[i], simulationData->_entityCount, fp);
			break;
		case GSC_PP_FLOAT:
			glmFileWrite(data->_ppAttributeData[i], sizeof(float), simulationData->_entityCount, fp);
			break;
		case GSC_PP_VECTOR:
			glmFileWrite(data->_ppAttributeData[i], sizeof(float), simulationData->_entityCount * 3, fp);
			break;
		default:
			data->_ppAttributeData[i] = NULL;
		}
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
	GLMC_FREE(data->_blendShapes);
	GLMC_FREE(data->_geoBehaviorGeometryIds);
	GLMC_FREE(data->_geoBehaviorAnimFrameInfo);
	GLMC_FREE(data->_geoBehaviorBlendModes);

	GLMC_FREE(data->_entityUseCloth);
	GLMC_FREE(data->_clothMeshIndexCount);
	GLMC_FREE(data->_clothReference);
	GLMC_FREE(data->_clothMaxExtent);
	GLMC_FREE(data->_clothIndices);
	GLMC_FREE(data->_clothMeshVertexCountPerClothIndex);

	GLMC_FREE(data->_clothVertices);

	for (i = 0; i < simulationData->_ppAttributeCount; ++i)
	{
		GLMC_FREE(data->_ppAttributeData[i]);
	}
	GLMC_FREE(data->_ppAttributeData);
	GLMC_FREE(data);
	*frameData = NULL;
}

#undef GLMC_IMPLEMENTATION
#endif // GLMC_IMPLEMENTATION
