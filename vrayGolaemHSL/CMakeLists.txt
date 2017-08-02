############################################################
# BEGIN Project
############################################################

project(${TARGET_CROWD3DSMAXVRAYHSL})

# List Files
list_files( "." "h;cpp;c;def;rc" )

# Create a library
add_library( ${TARGET_CROWD3DSMAXVRAYHSL} SHARED ${LIST_FILES} )

# Flags
add_definitions(-DUNICODE -D_UNICODE)

if(NOT VRAYGOLAEM_STANDALONE_BUILD)
    crowd_label_project( ${TARGET_CROWD3DSMAXVRAYHSL} )

    crowd_load_dependencies( ${TARGET_CROWD3DSMAXVRAYHSL} )
    set_target_prefix( ${TARGET_CROWD3DSMAXVRAYHSL} PLUGIN )
    set_target_postfix( ${TARGET_CROWD3DSMAXVRAYHSL} PLUGIN )

    set_target_extension( ${TARGET_CROWD3DSMAXVRAYHSL} CUSTOM ".dlt" )

    CROWD_INSTALL_RUNTIME_LIBRARY( ${TARGET_CROWD3DSMAXVRAYHSL} "crowd3dsMaxVray" "vrayGolaemHSL${MAYA_VERSION}")
    crowd_install_dependencies()
else()
    # Include
    include_directories( ${3DSMAXSDK_INCDIR} )
    include_directories( ${VRAYFOR3DSMAX_INCDIR} )

    # Link
    target_link_libraries( ${TARGET_CROWD3DSMAXVRAYHSL} "${SYSTEM_LIBS}")
    target_link_libraries( ${TARGET_CROWD3DSMAXVRAYHSL} "${3DSMAXSDK_LIBS}" )
    target_link_libraries( ${TARGET_CROWD3DSMAXVRAYHSL} "${VRAYFOR3DSMAX_LIBS}" )

    # Extension dlt
    set_target_properties(${TARGET_CROWD3DSMAXVRAYHSL} PROPERTIES SUFFIX ".dlt")

    #Install
    install (TARGETS ${TARGET_CROWD3DSMAXVRAYHSL} RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/vrayplugins/")
endif()

############################################################
# END Project
############################################################
