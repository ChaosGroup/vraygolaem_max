############################################################
# BEGIN Project
############################################################

project(${TARGET_CROWD3DSMAXVRAY})

# List Files
list_files( "." "h;cpp;c;def;rc;py" )

# Create a library
add_library( ${TARGET_CROWD3DSMAXVRAY} SHARED ${LIST_FILES} )

# Flags
add_definitions(-DUNICODE -D_UNICODE)

if(NOT VRAYGOLAEM_STANDALONE_BUILD)
    crowd_label_project( ${TARGET_CROWD3DSMAXVRAY} )
    
    crowd_load_dependencies( ${TARGET_CROWD3DSMAXVRAY} )
    set_target_prefix( ${TARGET_CROWD3DSMAXVRAY} PLUGIN )
    set_target_postfix( ${TARGET_CROWD3DSMAXVRAY} PLUGIN )

    set_target_extension( ${TARGET_CROWD3DSMAXVRAY} CUSTOM ".dlo" )

	target_compile_options(${TARGET_CROWD3DSMAXVRAY} PUBLIC -MD -U_DEBUG)

    CROWD_INSTALL_RUNTIME_LIBRARY( ${TARGET_CROWD3DSMAXVRAY} "crowd3dsMaxVray" "vrayGolaem${MAYA_VERSION}")
    crowd_install_dependencies()
    install( FILES "${CMAKE_CURRENT_SOURCE_DIR}/vraygolaem.py" CONFIGURATIONS Debug DESTINATION "${CROWD_INSTALL_PATH_Debug}/crowd3dsMaxVray/scripts" )
    install( FILES "${CMAKE_CURRENT_SOURCE_DIR}/vraygolaem.py" CONFIGURATIONS Release DESTINATION "${CROWD_INSTALL_PATH_Release}/crowd3dsMaxVray/scripts" )

else()
    # Include
    include_directories( ${3DSMAXSDK_INCDIR} )
    include_directories( ${VRAYFOR3DSMAX_INCDIR} )
    include_directories( ${GOLAEMDEVKIT_INCDIR} )

    # Link
    target_link_libraries( ${TARGET_CROWD3DSMAXVRAY} "${SYSTEM_LIBS}")
    target_link_libraries( ${TARGET_CROWD3DSMAXVRAY} "${3DSMAXSDK_LIBS}" )
    target_link_libraries( ${TARGET_CROWD3DSMAXVRAY} "${VRAYFOR3DSMAX_LIBS}" )
    target_link_libraries( ${TARGET_CROWD3DSMAXVRAY} "${GOLAEMDEVKIT_LIBS}" )

    # Extension dlo
    set_target_properties(${TARGET_CROWD3DSMAXVRAY} PROPERTIES SUFFIX ".dlo")

    # Install
    install (TARGETS ${TARGET_CROWD3DSMAXVRAY} RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/vrayplugins/")
    install (FILES "vraygolaem.py" DESTINATION "${CMAKE_INSTALL_PREFIX}/scripts/")
endif()

############################################################
# END Project
############################################################
