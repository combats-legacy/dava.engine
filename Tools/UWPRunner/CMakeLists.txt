cmake_minimum_required (VERSION 3.0)
project ( UWPRunner )

set    ( DAVA_NO_RENDER true )
set    ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../Sources/CMake/Modules/" )
include( CMake-common )

find_package( QT5           REQUIRED )
find_package( DavaFramework REQUIRED )

if ( NOT WIN32 OR WINDOWS_UAP )
    message( FATAL_ERROR "This util can be built only on Windows Desktop platform" )
endif()

add_definitions( 
-DQT_NETWORK_LIB
-DQT_CORE_LIB
-DQT_HAVE_MMX
-DQT_HAVE_3DNOW
-DQT_HAVE_SSE
-DQT_HAVE_MMXEXT
-DQT_HAVE_SSE2
-DQT_THREAD_SUPPORT
-D_CRT_SECURE_NO_DEPRECATE
-DDAVA_QT
-DDAVA_FMOD
)

file              ( GLOB_RECURSE QT_LIST "Classes/Qt/*.h"  )
qt5_wrap_cpp      ( MOC_APP_SRCS ${QT_LIST} )

file              ( GLOB UI_LIST "DataQt/*.ui" )
qt5_wrap_ui       ( UI_APP_HDRS  ${UI_LIST} )

file              ( GLOB RCC_LIST "DataQt/*.qrc" )
qt5_add_resources ( RCC_APP_SRCS  ${RCC_LIST} )

include_directories   ( "Classes" )
include_directories   ( "Classes/Qt" )
include_directories   ( "Classes/Qt/corecon" )

set ( Generated          ${MOC_APP_SRCS} ${UI_APP_HDRS} ${RCC_APP_SRCS} )

define_source_folders ( SRC_ROOT "Classes" ${PLATFORM_SPECIFIC_FOLDER}  )
generate_source_groups_project( SRC_ROOT "Classes" )
list ( APPEND UNIFIED_IGNORE_LIST "Classes/Qt/corecon/corecon.cpp" )

set( ADDED_SRC            ${MOC_APP_SRCS} ${UI_APP_HDRS} ${RCC_APP_SRCS}  )
set( LIBRARIES            ${QT_LIBRARIES} runtimeobject.lib XmlLite.lib wsclient.lib )
set( ADDED_BINARY_DIR     ${QT5_PATH_WIN}/bin )

setup_main_executable()

set_target_properties ( ${PROJECT_NAME} PROPERTIES LINK_FLAGS_DEBUG   "/SUBSYSTEM:CONSOLE" )
set_target_properties ( ${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:CONSOLE" )