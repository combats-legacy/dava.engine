include ( GlobalVariables )
include ( CMake-common )

if( WIN32 )
	if( X64_MODE )
		set (QT_ACTUAL_PATH ${QT5_PATH_WIN64})
	else ()
		set (QT_ACTUAL_PATH ${QT5_PATH_WIN})
	endif ()	
elseif ( MACOS )
	set (QT_ACTUAL_PATH ${QT5_PATH_MAC})
endif ()

macro ( qt_deploy )
    if ( NOT QT5_FOUND )
        return ()
    endif ()

    set(DEPLOY_SCRIPT_PATH ${DAVA_SCRIPTS_FILES_PATH}/deployQt.py)
    set(DEPLOY_ROOT_FOLDER ${DEPLOY_DIR})

    if( WIN32 )
        get_qt5_deploy_list(BINARY_ITEMS)

        if (QML_SCAN_DIR)
            set(QML_SCAN_FLAG "--qmldir ${QML_SCAN_DIR}")
        endif()

        set(DEPLOY_PLATFORM "WIN")
        set(DEPLOY_QT_FOLDER ${QT_ACTUAL_PATH})
        set(DEPLOY_ARGUMENTS "$<$<CONFIG:Debug>:--debug> $<$<NOT:$<CONFIG:Debug>>:--release>")
        set(DEPLOY_ARGUMENTS "${DEPLOY_ARGUMENTS} --dir ${DEPLOY_DIR}")
        set(DEPLOY_ARGUMENTS "${DEPLOY_ARGUMENTS} ${QML_SCAN_FLAG}  $<TARGET_FILE:${PROJECT_NAME}>")
        foreach(ITEM ${BINARY_ITEMS})
            string(TOLOWER ${ITEM} ITEM)
            if (EXISTS ${QT_ACTUAL_PATH}/bin/Qt5${ITEM}.dll)
                set(DEPLOY_ARGUMENTS "${DEPLOY_ARGUMENTS} --${ITEM}")
            endif()
        endforeach()

    elseif( MACOS )

        if (QML_SCAN_DIR)
            set(QML_SCAN_FLAG "-qmldir=${QML_SCAN_DIR}")
        endif()

        set(DEPLOY_PLATFORM "MAC")
        set(DEPLOY_QT_FOLDER ${QT_ACTUAL_PATH})
        set(DEPLOY_ARGUMENTS "${PROJECT_NAME}.app -always-overwrite ${QML_SCAN_FLAG}")

    endif()

    is_deploy_qt_delayed(IS_DELAYED_DEPLOY)
    if (${IS_DELAYED_DEPLOY})
        set(DEPLOY_PROJECT "QT_DEPLOY_${PROJECT_NAME}")
        ADD_CUSTOM_TARGET ( ${DEPLOY_PROJECT} ALL)

        get_deploy_dependencies(DEPENDENCIES_LIST)
        foreach(DEPENDENCY ${DEPENDENCIES_LIST})
            add_dependencies( ${DEPLOY_PROJECT} ${DEPENDENCY} )
        endforeach()
    else()
        set(DEPLOY_PROJECT ${PROJECT_NAME})
    endif()

    ADD_CUSTOM_COMMAND( TARGET ${DEPLOY_PROJECT}  POST_BUILD
            COMMAND "python"
                    ${DEPLOY_SCRIPT_PATH}
                    "-p" "${DEPLOY_PLATFORM}"
                    "-q" "${DEPLOY_QT_FOLDER}"
                    "-d" "${DEPLOY_ROOT_FOLDER}"
                    "-a" "${DEPLOY_ARGUMENTS}"
        )

endmacro ()

macro(resolve_qt_pathes)
    if ( NOT QT5_LIB_PATH)

        if( WIN32 )
            set ( QT_CORE_LIB Qt5Core.lib )
        elseif( MACOS )
            set ( QT_CORE_LIB QtCore.la )
        endif()

        find_path( QT5_LIB_PATH NAMES ${QT_CORE_LIB}
                          PATHS ${QT_ACTUAL_PATH}
                          PATH_SUFFIXES lib)

        ASSERT(QT5_LIB_PATH "Please set the correct path to QT5 in file DavaConfig.in")
    endif()
    set ( CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "${QT5_LIB_PATH}/cmake")
endmacro()

#################################################################

# Find includes in corresponding build directories
set ( CMAKE_INCLUDE_CURRENT_DIR ON )
# Instruct CMake to run moc automatically when needed.
set ( CMAKE_AUTOMOC ON )

list( APPEND QT5_FIND_COMPONENTS ${QT5_FIND_COMPONENTS} Core Gui Widgets Concurrent Qml Quick Network)
list( REMOVE_DUPLICATES QT5_FIND_COMPONENTS)

resolve_qt_pathes()

foreach(COMPONENT ${QT5_FIND_COMPONENTS})
    if (NOT Qt5${COMPONENT}_FOUND)
        find_package("Qt5${COMPONENT}")
    endif()

    ASSERT(Qt5${COMPONENT}_FOUND "Can't find Qt5 component : ${COMPONENT}")
    LIST(APPEND DEPLOY_LIST "${COMPONENT}")
    LIST(APPEND LINKAGE_LIST "Qt5::${COMPONENT}")
endforeach()

append_qt5_deploy(DEPLOY_LIST)
set_linkage_qt5_modules(LINKAGE_LIST)
set ( DAVA_EXTRA_ENVIRONMENT QT_QPA_PLATFORM_PLUGIN_PATH=$ENV{QT_QPA_PLATFORM_PLUGIN_PATH} )

set(QT5_FOUND 1)

if( NOT QT5_FOUND )
    message( FATAL_ERROR "Please set the correct path to QT5 in file DavaConfig.in"  )
endif()
