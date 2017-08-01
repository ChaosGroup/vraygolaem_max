############################################################
# BEGIN Project
############################################################

# List Files
list_files( "." "h;cpp;c;def;rc" )

# Create a library
set( VRAYGOLAEMHSLLIB "vraygolaemhsl${3DSMAXSDK_VERSION}" )
add_library( ${VRAYGOLAEMHSLLIB} SHARED ${LIST_FILES} )

# Include
include_directories( ${3DSMAXSDK_INCDIR} )
include_directories( ${VRAYFOR3DSMAX_INCDIR} )

# Link
target_link_libraries( ${VRAYGOLAEMHSLLIB} "${SYSTEM_LIBS}")
target_link_libraries( ${VRAYGOLAEMHSLLIB} "${3DSMAXSDK_LIBS}" )
target_link_libraries( ${VRAYGOLAEMHSLLIB} "${VRAYFOR3DSMAX_LIBS}" )

# Extension dlt
set_target_properties(${VRAYGOLAEMHSLLIB} PROPERTIES SUFFIX ".dlt")

# Flags
add_definitions(-DUNICODE -D_UNICODE)

#Install
install (TARGETS ${VRAYGOLAEMHSLLIB} RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/vrayplugins/")

############################################################
# END Project
############################################################
