include_directories(${LOG4CPLUS_INCLUDE_DIR})

SET(TARGET_SRC
    stdafx.cpp
    sgi_log4cplus_plugin.cpp
    ObjectTreeLog4cplus.cpp
    writeHTMLLog4cplus.cpp
    getObjectInfoLog4cplus.cpp
    ContextMenuLog4cplus.cpp
    MenuActionLog4cplus.cpp
    )

SET(TARGET_H
    stdafx.h
    SGIItemLog4cplus
    ObjectTreeLog4cplus.h
    writeHTMLLog4cplus.h
    getObjectInfoLog4cplus.h
    ContextMenuLog4cplus.h
    MenuActionLog4cplus.h
    )
configure_file(SGIItemLog4cplus ${SGI_INCLUDE_DIR}/sgi/plugins/SGIItemLog4cplus COPYONLY)

SET(TARGET_LIBRARIES_VARS
    OSG_LIBRARY
    OSGDB_LIBRARY
    LOG4CPLUS_LIBRARY
)

IF((UNIX AND FAST_PLATFORM_BITS EQUAL 32) OR WIN32)
    LIST(APPEND TARGET_LIBRARIES_VARS OPENTHREADS_LIBRARY)
ENDIF()

SGI_PLUGIN(log4cplus)
