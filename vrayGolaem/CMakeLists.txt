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
include_directories( ${GOLAEMDEVKIT_INCDIR} )

# Link
target_link_libraries( ${VRAYGOLAEMLIB} "${SYSTEM_LIBS}")
target_link_libraries( ${VRAYGOLAEMLIB} "${3DSMAXSDK_LIBS}" )
target_link_libraries( ${VRAYGOLAEMLIB} "${VRAYFOR3DSMAX_LIBS}" )
target_link_libraries( ${VRAYGOLAEMLIB} "${GOLAEMDEVKIT_LIBS}" )

# Extension dlo
set_target_properties(${VRAYGOLAEMLIB} PROPERTIES SUFFIX ".dlo")

# Flags
add_definitions(-DUNICODE -D_UNICODE)

# Install
install (TARGETS ${VRAYGOLAEMLIB} RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/vrayplugins/")
install (FILES "vraygolaem.py" DESTINATION "${CMAKE_INSTALL_PREFIX}/scripts/")

############################################################
# END Project
############################################################
