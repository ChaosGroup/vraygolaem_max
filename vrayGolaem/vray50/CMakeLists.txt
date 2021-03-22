############################################################
# BEGIN Project
############################################################

project("${TARGET_CROWD3DSMAX${3DSMAX_VERSION}VRAY50}")

# List Files
list_files( "." "h;cpp;c;def;rc;py" "${CMAKE_CURRENT_SOURCE_DIR}/../_shared")

# Create a library
add_library( ${PROJECT_NAME} SHARED ${LIST_FILES} )

# Flags
string( REGEX MATCHALL "[0-9]+" VERSIONS ${PROJECT_NAME}) # Extract numbers from project name (3;2018;40)
list(GET VERSIONS -1 VERSION) # Get last element
target_compile_options(${PROJECT_NAME} PRIVATE "-DGLM_USE_VRAY${VERSION}")
target_compile_options(${PROJECT_NAME} PRIVATE "-DUNICODE" "-D_UNICODE")
target_compile_options(${PROJECT_NAME} PRIVATE "-DWITH_DR1")

if(NOT VRAYGOLAEM_STANDALONE_BUILD)
    crowd_label_project( ${PROJECT_NAME} )
    
    crowd_load_dependencies( ${PROJECT_NAME} )
    set_target_prefix( ${PROJECT_NAME} PLUGIN )
    set_target_postfix( ${PROJECT_NAME} PLUGIN )

    set_target_extension( ${PROJECT_NAME} CUSTOM ".dlo" )

	target_compile_options(${PROJECT_NAME} PRIVATE -MD -U_DEBUG)

    #CROWD_INSTALL_RUNTIME_LIBRARY( ${PROJECT_NAME} "procedurals/3dsmax" "vrayGolaem${3DSMAX_VERSION}")
    CROWD_INSTALL_PLUGIN( ${PROJECT_NAME} "procedurals/3dsmax/${VERSION}" )
    crowd_install_dependencies()
    install( FILES "${CMAKE_CURRENT_SOURCE_DIR}/../_shared/vraygolaem.py" CONFIGURATIONS Debug DESTINATION "${CROWD_INSTALL_PATH_Debug}/procedurals/3dsmax/${VERSION}/scripts" )
    install( FILES "${CMAKE_CURRENT_SOURCE_DIR}/../_shared/vraygolaem.py" CONFIGURATIONS Release DESTINATION "${CROWD_INSTALL_PATH_Release}/procedurals/3dsmax/${VERSION}/scripts" )

else()
    # Include
    target_include_directories(${PROJECT_NAME} PRIVATE ${3DSMAXSDK_INCDIR_${3DSMAX_VERSION}} )
    target_include_directories(${PROJECT_NAME} PRIVATE ${VRAYFOR3DSMAX_INCDIR_${3DSMAX_VERSION}} )
    target_include_directories(${PROJECT_NAME} PRIVATE ${GOLAEMDEVKIT_INCDIR} )

    # Link
    target_link_libraries( ${PROJECT_NAME} "${SYSTEM_LIBS}")
    target_link_libraries( ${PROJECT_NAME} "${3DSMAXSDK_LIBS_${3DSMAX_VERSION}}" )
    target_link_libraries( ${PROJECT_NAME} "${VRAYFOR3DSMAX_LIBS_${3DSMAX_VERSION}}" )
    target_link_libraries( ${PROJECT_NAME} "${GOLAEMDEVKIT_LIBS}" )

    # Extension dlo
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".dlo")

    # Install
    install (TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/vrayplugins")
    #install (TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "${3DSMAXSDK_ROOTDIR}/Plugins/vrayplugins")

    install (FILES "../_shared/vraygolaem.py" DESTINATION "${CMAKE_INSTALL_PREFIX}/scripts")
    #install (FILES "vraygolaem.py" DESTINATION "${3DSMAXSDK_ROOTDIR}/scripts/Python")
endif()

############################################################
# END Project
############################################################
