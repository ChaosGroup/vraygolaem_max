cmake_minimum_required (VERSION 2.8.11)

############################################################
# BEGIN Declare various macros
############################################################
macro( list_files FOLDER_NAME FILE_EXTENSIONS )

	set( SKIP_INCLUDE_DIR OFF )
	if( ( ${ARGC} GREATER 2 ) AND ( NOT "${ARGV2}" STREQUAL "" ) )
		set( DIRECTORY_REFERENCE "${ARGV2}/${FOLDER_NAME}" )
		if( ( ${ARGC} GREATER 3 ) AND ( "${ARGV3}" STREQUAL "SKIP_INCLUDE_DIR" ) )
			set( SKIP_INCLUDE_DIR ON )
		endif()
	else()
		set( DIRECTORY_REFERENCE "${CMAKE_CURRENT_SOURCE_DIR}/${FOLDER_NAME}" )
	endif()
	if( EXISTS ${DIRECTORY_REFERENCE} )
		if( NOT SKIP_INCLUDE_DIR )
			include_directories( ${DIRECTORY_REFERENCE} )
		endif()
		if( "${FOLDER_NAME}" STREQUAL "." )
			set( DIRECTORY_FILTER "Source Files" )
			set( DIRECTORY_LABEL "ROOT" )
		else()
			set( DIRECTORY_FILTER "Source Files/${FOLDER_NAME}" )
			string( REGEX REPLACE "/\\." "" DIRECTORY_FILTER ${DIRECTORY_FILTER} )
			string( REGEX REPLACE "\\./" "" DIRECTORY_FILTER ${DIRECTORY_FILTER} )
			string( REGEX REPLACE "/" "\\\\\\\\" DIRECTORY_FILTER ${DIRECTORY_FILTER} )
			set( DIRECTORY_LABEL "${FOLDER_NAME}" )
			string( REGEX REPLACE "/\\." "" DIRECTORY_LABEL ${DIRECTORY_LABEL} )
			string( REGEX REPLACE "\\./" "" DIRECTORY_LABEL ${DIRECTORY_LABEL} )
			string( REGEX REPLACE "/" "_" DIRECTORY_LABEL ${DIRECTORY_LABEL} )
			string( TOUPPER "${DIRECTORY_LABEL}" DIRECTORY_LABEL )
		endif()
		foreach( file_ext ${FILE_EXTENSIONS} )
			string( TOUPPER "${file_ext}" _FILE_EXT )
			unset( LIST_FILES_${DIRECTORY_LABEL}_${_FILE_EXT} )
			file( GLOB LIST_FILES_${DIRECTORY_LABEL}_${_FILE_EXT} ${DIRECTORY_REFERENCE}/*.${file_ext} )
			list( APPEND LIST_FILES "${LIST_FILES_${DIRECTORY_LABEL}_${_FILE_EXT}}" )
			list( APPEND LIST_FILES_${_FILE_EXT} "${LIST_FILES_${DIRECTORY_LABEL}_${_FILE_EXT}}" )
			source_group( "${DIRECTORY_FILTER}" FILES ${LIST_FILES_${DIRECTORY_LABEL}_${_FILE_EXT}} )
			# unset( LIST_FILES_${DIRECTORY_LABEL}_${_FILE_EXT} )
		endforeach()
	else()
	endif()

endmacro()

############################################################
# END Declare various macros
############################################################


############################################################
# BEGIN Project
############################################################
project (vraygolaem)

# Add dependencies
set( 3DSMAXSDK_ROOTDIR "3dsMaxSDK Root Dir" CACHE PATH "3dsMaxSDK Root directory" )
set( 3DSMAXSDK_VERSION 2014 CACHE INT "3dsMaxSDK Version" )
set( VRAYFOR3DSMAX_ROOTDIR "VRay Root Dir" CACHE PATH "VRay Root directory" )

set( 3DSMAXSDK_INCDIR "${3DSMAXSDK_ROOTDIR}/include" )
set( 3DSMAXSDK_LIBDIR "${3DSMAXSDK_ROOTDIR}/lib/x64/Release" )
set( VRAYFOR3DSMAX_INCDIR "${VRAYFOR3DSMAX_ROOTDIR}/include" )
set( VRAYFOR3DSMAX_LIBDIR "${VRAYFOR3DSMAX_ROOTDIR}/lib/x64" )
if( 3DSMAXSDK_VERSION EQUAL 2014 )
	set( VRAYFOR3DSMAX_LIBDIRVC "vc101")
else()
	set( VRAYFOR3DSMAX_LIBDIRVC "vc11")
endif()

list(APPEND 3DSMAXSDK_LIBS "${3DSMAXSDK_LIBDIR}/assetmanagement.lib")
list(APPEND 3DSMAXSDK_LIBS "${3DSMAXSDK_LIBDIR}/bmm.lib")
list(APPEND 3DSMAXSDK_LIBS "${3DSMAXSDK_LIBDIR}/core.lib")
list(APPEND 3DSMAXSDK_LIBS "${3DSMAXSDK_LIBDIR}/maxutil.lib")
list(APPEND 3DSMAXSDK_LIBS "${3DSMAXSDK_LIBDIR}/geom.lib")
list(APPEND 3DSMAXSDK_LIBS "${3DSMAXSDK_LIBDIR}/gfx.lib")
list(APPEND 3DSMAXSDK_LIBS "${3DSMAXSDK_LIBDIR}/mesh.lib")
list(APPEND 3DSMAXSDK_LIBS "${3DSMAXSDK_LIBDIR}/maxscrpt.lib")
list(APPEND 3DSMAXSDK_LIBS "${3DSMAXSDK_LIBDIR}/manipsys.lib")
list(APPEND 3DSMAXSDK_LIBS "${3DSMAXSDK_LIBDIR}/paramblk2.lib")
list(APPEND 3DSMAXSDK_LIBS "${3DSMAXSDK_LIBDIR}/MNMath.lib")

list(APPEND VRAYFOR3DSMAX_LIBS "${VRAYFOR3DSMAX_LIBDIR}/vray${3DSMAXSDK_VERSION}.lib")
list(APPEND VRAYFOR3DSMAX_LIBS "${VRAYFOR3DSMAX_LIBDIR}/vrender${3DSMAXSDK_VERSION}.lib")
list(APPEND VRAYFOR3DSMAX_LIBS "${VRAYFOR3DSMAX_LIBDIR}/zlib_s.lib")
list(APPEND VRAYFOR3DSMAX_LIBS "${VRAYFOR3DSMAX_LIBDIR}/tiff_s.lib")
list(APPEND VRAYFOR3DSMAX_LIBS "${VRAYFOR3DSMAX_LIBDIR}/${VRAYFOR3DSMAX_LIBDIRVC}/plugman_s.lib")
list(APPEND VRAYFOR3DSMAX_LIBS "${VRAYFOR3DSMAX_LIBDIR}/${VRAYFOR3DSMAX_LIBDIRVC}/vutils_s.lib")
list(APPEND VRAYFOR3DSMAX_LIBS "${VRAYFOR3DSMAX_LIBDIR}/${VRAYFOR3DSMAX_LIBDIRVC}/meshes_s.lib")
list(APPEND VRAYFOR3DSMAX_LIBS "${VRAYFOR3DSMAX_LIBDIR}/${VRAYFOR3DSMAX_LIBDIRVC}/libmmd.lib")
list(APPEND VRAYFOR3DSMAX_LIBS "${VRAYFOR3DSMAX_LIBDIR}/${VRAYFOR3DSMAX_LIBDIRVC}/libirc.lib")
list(APPEND VRAYFOR3DSMAX_LIBS "${VRAYFOR3DSMAX_LIBDIR}/${VRAYFOR3DSMAX_LIBDIRVC}/svml_dispmd.lib")
list(APPEND VRAYFOR3DSMAX_LIBS "${VRAYFOR3DSMAX_LIBDIR}/${VRAYFOR3DSMAX_LIBDIRVC}/libdecimal.lib")

#MESSAGE(${3DSMAXSDK_INCDIR})

# List Files 
list_files( "." "h;cpp;c;def;rc" )

# Create a library
set( VRAYGOLAEMLIB "vraygolaem${3DSMAXSDK_VERSION}" )
add_library( ${VRAYGOLAEMLIB} SHARED ${LIST_FILES} )

# Include
include_directories( ${3DSMAXSDK_INCDIR} )
include_directories( ${VRAYFOR3DSMAX_INCDIR} )

# Link
target_link_libraries( ${VRAYGOLAEMLIB} "comctl32.lib")
target_link_libraries( ${VRAYGOLAEMLIB} "${3DSMAXSDK_LIBS}" )
target_link_libraries( ${VRAYGOLAEMLIB} "${VRAYFOR3DSMAX_LIBS}" )

# todo extension dlo

# Flags
add_definitions(-DUNICODE -D_UNICODE)

############################################################
# END Project
############################################################
