############################################################
# BEGIN Project
############################################################

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

# Extension dlo
set_target_properties(${VRAYGOLAEMLIB} PROPERTIES SUFFIX ".dlo")

# Flags
add_definitions(-DUNICODE -D_UNICODE)

############################################################
# END Project
############################################################
