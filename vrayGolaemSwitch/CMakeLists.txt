############################################################
# BEGIN Project
############################################################

project(${TARGET_CROWD3DSMAXVRAYSWITCH})

# List Files
list_files( "." "h;cpp;c;def;rc" )

# Create a library
add_library( ${TARGET_CROWD3DSMAXVRAYSWITCH} SHARED ${LIST_FILES} )

# Flags
add_definitions(-DUNICODE -D_UNICODE)

if(NOT VRAYGOLAEM_STANDALONE_BUILD)
    crowd_label_project( ${TARGET_CROWD3DSMAXVRAYSWITCH} )

    crowd_load_dependencies( ${TARGET_CROWD3DSMAXVRAYSWITCH} )
    set_target_prefix( ${TARGET_CROWD3DSMAXVRAYSWITCH} PLUGIN )
    set_target_postfix( ${TARGET_CROWD3DSMAXVRAYSWITCH} PLUGIN )

    set_target_extension( ${TARGET_CROWD3DSMAXVRAYSWITCH} CUSTOM ".dlt" )

	if( MAYA_VERSION GREATER_EQUAL 2018 )
		target_compile_options(${TARGET_CROWD3DSMAXVRAYSWITCH} PUBLIC -MD -U_DEBUG)
    endif()

    CROWD_INSTALL_RUNTIME_LIBRARY( ${TARGET_CROWD3DSMAXVRAYSWITCH} "crowd3dsMaxVray" "vrayGolaemSwitch${MAYA_VERSION}")
    crowd_install_dependencies()
else()
    # Include
    include_directories( ${3DSMAXSDK_INCDIR} )
    include_directories( ${VRAYFOR3DSMAX_INCDIR} )

    # Link
    target_link_libraries( ${TARGET_CROWD3DSMAXVRAYSWITCH} "${SYSTEM_LIBS}")
    target_link_libraries( ${TARGET_CROWD3DSMAXVRAYSWITCH} "${3DSMAXSDK_LIBS}" )
    target_link_libraries( ${TARGET_CROWD3DSMAXVRAYSWITCH} "${VRAYFOR3DSMAX_LIBS}" )

    # Extension dlt
    set_target_properties(${TARGET_CROWD3DSMAXVRAYSWITCH} PROPERTIES SUFFIX ".dlt")

    #Install
    install (TARGETS ${TARGET_CROWD3DSMAXVRAYSWITCH} RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/vrayplugins/")
endif()

############################################################
# END Project
############################################################
