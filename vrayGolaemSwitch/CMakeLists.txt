############################################################
# BEGIN Project
############################################################

# List Files
list_files( "." "h;cpp;c;def;rc" )

# Create a library
set( VRAYGOLAEMSWITCHLIB "vraygolaemswitch${3DSMAXSDK_VERSION}" )
add_library( ${VRAYGOLAEMSWITCHLIB} SHARED ${LIST_FILES} )

# Include
include_directories( ${3DSMAXSDK_INCDIR} )
include_directories( ${VRAYFOR3DSMAX_INCDIR} )

# Link
target_link_libraries( ${VRAYGOLAEMSWITCHLIB} "${SYSTEM_LIBS}")
target_link_libraries( ${VRAYGOLAEMSWITCHLIB} "${3DSMAXSDK_LIBS}" )
target_link_libraries( ${VRAYGOLAEMSWITCHLIB} "${VRAYFOR3DSMAX_LIBS}" )

# Extension dlt
set_target_properties(${VRAYGOLAEMSWITCHLIB} PROPERTIES SUFFIX ".dlt")

# Flags
add_definitions(-DUNICODE -D_UNICODE)

#Install
install (TARGETS ${VRAYGOLAEMSWITCHLIB} RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/vrayplugins/")

############################################################
# END Project
############################################################
