# Need to find both Qt4 and QGLViewer if the QQL support is to be built
FIND_PACKAGE(Qt4 COMPONENTS QtCore QtXml QtOpenGL QtGui)

FIND_PATH(QGLVIEWER_INCLUDE_DIR qglviewer.h
        /usr/include/QGLViewer
        /opt/local/include/QGLViewer
        /usr/local/include/QGLViewer
        /sw/include/QGLViewer
        ENV QGLVIEWERROOT
        )

find_library(QGLVIEWER_LIBRARY_RELEASE
        NAMES qglviewer-qt4 QGLViewer-qt4 qglviewer QGLViewer QGLViewer2
        PATHS /usr/lib
        /usr/local/lib
        /usr/lib/x86_64-linux-gnu
        /opt/local/lib
        /sw/lib
        ENV QGLVIEWERROOT
        ENV LD_LIBRARY_PATH
        ENV LIBRARY_PATH
        PATH_SUFFIXES QGLViewer QGLViewer/release
        )
find_library(QGLVIEWER_LIBRARY_DEBUG
        NAMES dqglviewer dQGLViewer dQGLViewer2 QGLViewerd2
        PATHS /usr/lib
        /usr/local/lib
        /usr/lib/x86_64-linux-gnu
        /opt/local/lib
        /sw/lib
        ENV QGLVIEWERROOT
        ENV LD_LIBRARY_PATH
        ENV LIBRARY_PATH
        PATH_SUFFIXES QGLViewer QGLViewer/release
        )

if(QGLVIEWER_LIBRARY_RELEASE)
    if(QGLVIEWER_LIBRARY_DEBUG)
        set(QGLVIEWER_LIBRARY optimized ${QGLVIEWER_LIBRARY_RELEASE} debug ${QGLVIEWER_LIBRARY_DEBUG})
    else()
        set(QGLVIEWER_LIBRARY ${QGLVIEWER_LIBRARY_RELEASE})
    endif()
endif()


IF(QGLVIEWER_INCLUDE_DIR AND QGLVIEWER_LIBRARY)
  SET(QGLVIEWER_FOUND TRUE)
ELSE(QGLVIEWER_INCLUDE_DIR AND QGLVIEWER_LIBRARY)
  SET(QGLVIEWER_FOUND FALSE)
ENDIF(QGLVIEWER_INCLUDE_DIR AND QGLVIEWER_LIBRARY)
