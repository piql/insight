##############################################################################
#
#   Creation date:  2017-07-12
#   Created by:     Ole Liabo
#
#   Copyright (c) 2017 Piql AS, Norway. All rights reserved.
#
##############################################################################

QT               +=   xml widgets gui core printsupport sql 
#debug:QT         +=   testlib
#QT               +=   xmlpatterns
TEMPLATE          =   app
CONFIG           +=   qt debug_and_release 
macx:CONFIG      +=   app_bundle
TARGET            =   insight


##  SUPPORT DEBUG AND RELEASE BUILDS  ##
!debug_and_release|build_pass {
    CONFIG(debug, debug|release) {
        CURBUILD = debug
    }
    CONFIG(release, debug|release) {
        CURBUILD = release
        DEFINES += NDEBUG
    }
}

##  LNX PLATFORM  ##
PLATFORM                    =   LNX
linux-g++-64:PLATFORM       =   LNX64

##  WIN PLATFORM  ##
win32 {
    contains(QT_ARCH, i386) {
        message("Windows 32-bit")
        PLATFORM              =   W32
    } else {
        message("Windows 64-bit")
        PLATFORM              =   W64
    }
}


OBJECTS_DIR                    =  obj/$$PLATFORM/$$CURBUILD
MOC_DIR                        =  obj/$$PLATFORM/$$CURBUILD
QMAKE_CXXFLAGS                +=  -DQUAZIP_STATIC 
win32:QMAKE_CXXFLAGS_RELEASE  +=  /O2 /Ob2 /Oi /Ot /GL
win32:QMAKE_LFLAGS_RELEASE    +=  /LTCG /DEBUG


INCLUDEPATH       =   src \
                      src/gui \
                      src/thirdparty/quazip-0.7.3/quazip/ \
                      src/thirdparty/minixml/inc \
                      $$[QT_INSTALL_PREFIX]/include/QtZlib 
win32:INCLUDEPATH+=   $$(CV_THIRDPARTY)/w32/poppler-0.57.0/qt5/src
macx:INCLUDEPATH +=   /usr/local/Cellar/poppler/0.63.0_1/include/poppler \
                      /usr/local/Cellar/poppler/0.63.0_1/include/poppler/qt5/

DEPENDPATH       +=   $$INCLUDEPATH

win32:contains(QT_ARCH, i386):release:LIBS += $$(CV_THIRDPARTY)/w32/poppler-0.57.0/qt5/src/Release/poppler-qt5.lib \
                      $$(CV_THIRDPARTY)/w32/poppler-0.57.0/Release/poppler.lib
win32:!contains(QT_ARCH, i386):release:LIBS += $$(CV_THIRDPARTY)/w64/poppler-0.57.0/qt5/src/Release/poppler-qt5.lib \
                      $$(CV_THIRDPARTY)/w64/poppler-0.57.0/Release/poppler.lib

# Library dependency checking
PRE_TARGETDEPS   +=   $$LIBS

macx:LIBS        +=   -L/usr/local/Cellar/poppler/0.63.0_1/lib -lpoppler -lpoppler-qt5
macx:LIBS        +=   -L/usr/local/Cellar/zlib/1.2.11/lib  -lz

# Tested on Debian GNU/Linux using distribution
# libraries (libpoppler-qt5-dev and libquazip5-dev)
unix {
INCLUDEPATH      +=   /usr/include/quazip5 /usr/include/poppler/qt5
LIBS             +=   -lquazip5 -lz -lpoppler -lpoppler-qt5
target.path       =   /usr/bin
INSTALLS         +=   target
}

                      
GUI_SOURCES       =   src/gui/dinsightmainwindow.cpp \
                      src/gui/dinsightreportwindow.cpp \
                      src/gui/dtreeview.cpp \
                      src/gui/dtreeitem.cpp \
                      src/gui/dtreemodel.cpp \
                      src/gui/dimport.cpp \
                      src/gui/dwaitcursor.cpp \
                      src/gui/qpersistantfiledialog.cpp \
                      src/gui/qaboutdialog.cpp
GUI_HEADERS       =   src/gui/dinsightmainwindow.h \
                      src/gui/dinsightreportwindow.h \
                      src/gui/dtreeview.h \
                      src/gui/dtreeitem.h \
                      src/gui/dtreemodel.h \
                      src/gui/dimport.h \
                      src/gui/dwaitcursor.h \
                      src/gui/qpersistantfiledialog.h \
                      src/gui/qaboutdialog.h

ZIP_HEADERS       =   src/thirdparty/quazip-0.7.3/quazip/crypt.h \
                      src/thirdparty/quazip-0.7.3/quazip/ioapi.h \
                      src/thirdparty/quazip-0.7.3/quazip/JlCompress.h \
                      src/thirdparty/quazip-0.7.3/quazip/quaadler32.h \
                      src/thirdparty/quazip-0.7.3/quazip/quachecksum32.h \
                      src/thirdparty/quazip-0.7.3/quazip/quacrc32.h \
                      src/thirdparty/quazip-0.7.3/quazip/quagzipfile.h \
                      src/thirdparty/quazip-0.7.3/quazip/quaziodevice.h \
                      src/thirdparty/quazip-0.7.3/quazip/quazip.h \
                      src/thirdparty/quazip-0.7.3/quazip/quazipdir.h \
                      src/thirdparty/quazip-0.7.3/quazip/quazipfile.h \
                      src/thirdparty/quazip-0.7.3/quazip/quazipfileinfo.h \
                      src/thirdparty/quazip-0.7.3/quazip/quazipnewinfo.h \
                      src/thirdparty/quazip-0.7.3/quazip/quazip_global.h \
                      src/thirdparty/quazip-0.7.3/quazip/unzip.h \
                      src/thirdparty/quazip-0.7.3/quazip/zip.h

ZIP_SOURCES       =   src/thirdparty/quazip-0.7.3/quazip/unzip.c \
                      src/thirdparty/quazip-0.7.3/quazip/zip.c \
                      src/thirdparty/quazip-0.7.3/quazip/JlCompress.cpp \
                      src/thirdparty/quazip-0.7.3/quazip/qioapi.cpp \
                      src/thirdparty/quazip-0.7.3/quazip/quaadler32.cpp \
                      src/thirdparty/quazip-0.7.3/quazip/quacrc32.cpp \
                      src/thirdparty/quazip-0.7.3/quazip/quagzipfile.cpp \
                      src/thirdparty/quazip-0.7.3/quazip/quaziodevice.cpp \
                      src/thirdparty/quazip-0.7.3/quazip/quazip.cpp \
                      src/thirdparty/quazip-0.7.3/quazip/quazipdir.cpp \
                      src/thirdparty/quazip-0.7.3/quazip/quazipfile.cpp \
                      src/thirdparty/quazip-0.7.3/quazip/quazipfileinfo.cpp \
                      src/thirdparty/quazip-0.7.3/quazip/quazipnewinfo.cpp

FORMS             =   src/gui/dinsightmainwindow.ui \
                      src/gui/dinsightreportwindow.ui \
                      src/gui/qaboutdialog.ui

RESOURCES         =   resources.qrc

include( src/thirdparty/yxml/yxml.pri )


SOURCES          +=   src/main.cpp \
                      src/drunguard.cpp \
                      src/dxmlparser.cpp \
                      src/dattachmentindexer.cpp \
                      src/dattachmentparser.cpp \
                      src/dsearchthread.cpp \
                      src/dinsightconfig.cpp \
                      src/dinsightreport.cpp \
                      $$GUI_SOURCES

HEADERS          +=   src/drunguard.h \
                      src/dxmlparser.h \
                      src/dattachmentindexer.h \
                      src/dattachmentparser.h \
                      src/dsearchthread.h \
                      src/dinsightconfig.h \
                      src/dinsightreport.h \
                      $$GUI_HEADERS

win32 {
SOURCES          +=   $$ZIP_SOURCES
HEADERS          +=   $$ZIP_HEADERS
}

macx {
SOURCES          +=   $$ZIP_SOURCES
HEADERS          +=   $$ZIP_HEADERS
}

TRANSLATIONS      =   insight_no.ts \
                      insight_nn.ts \
                      insight_en.ts

# Tool for testing qt models
#win32:debug:SOURCES     +=  $$(QTDIR)/tests/auto/other/modeltest/modeltest.cpp
#win32:debug:HEADERS     +=  $$(QTDIR)/tests/auto/other/modeltest/modeltest.h
#win32:debug:INCLUDEPATH +=  $$(QTDIR)/tests/auto/other/modeltest/

