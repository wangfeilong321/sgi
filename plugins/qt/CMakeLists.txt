SET(TARGET_SRC
    stdafx.cpp
    sgi_qt_plugin.cpp
    writeHTMLQt.cpp
    writeHTMLQtMeta.cpp
    writeHTMLQtPaintDevice.cpp
    getObjectInfoQt.cpp
    ObjectTreeQt.cpp
    ContextMenuQt.cpp
    MenuActionQt.cpp
    ImagePreviewDialog.cpp
    SettingsDialogQt.cpp
    )

SET(TARGET_H
    SGIItemQt
	stdafx.h
    writeHTMLQt.h
    getObjectInfoQt.h
    ObjectTreeQt.h
    ContextMenuQt.h
    MenuActionQt.h
    SettingsDialogQt.h
    )

configure_file(SGIItemQt ${SGI_INCLUDE_DIR}/sgi/plugins/SGIItemQt COPYONLY)

SET(TARGET_MOC_H
    ImagePreviewDialog.h
)

SET(TARGET_UI
    ImagePreviewDialog.ui
)

SET(TARGET_LIBRARIES_VARS
    OSG_LIBRARY
    OSGDB_LIBRARY
    SGI_PLUGIN_IMPL_LIBRARY
)

IF((UNIX AND FAST_PLATFORM_BITS EQUAL 32) OR WIN32)
    LIST(APPEND TARGET_LIBRARIES_VARS OPENTHREADS_LIBRARY)
ENDIF()

IF ( NOT Qt5Widgets_FOUND )
    LIST(APPEND TARGET_LIBRARIES_VARS QT_QTCORE_LIBRARY QT_QTGUI_LIBRARY QT_QTOPENGL_LIBRARY)
ENDIF(  )

SGI_PLUGIN(qt)

IF ( Qt5Widgets_FOUND )
    qt5_use_modules( osgdb_sgi_qt_plugin Gui Widgets OpenGL)
ENDIF( Qt5Widgets_FOUND )
