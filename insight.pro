##############################################################################
#
#   Creation date:  2017-07-12
#   Created by:     Ole Liabo
#
#   Copyright (c) 2017 Piql AS, Norway. All rights reserved.
#
##############################################################################

QT               +=   xml widgets gui core printsupport sql 
TEMPLATE          =   app
CONFIG           +=   qt debug_and_release
CONFIG           +=   lrelease 
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
macx:POPPLER_VER               =  0.77.0

OBJECTS_DIR                    =  obj/$$PLATFORM/$$CURBUILD
MOC_DIR                        =  obj/$$PLATFORM/$$CURBUILD
QMAKE_CXXFLAGS                +=  -DQUAZIP_STATIC 
win32:QMAKE_CXXFLAGS_RELEASE  +=  /O2 /Ob2 /Oi /Ot /GL
win32:QMAKE_LFLAGS_RELEASE    +=  /LTCG /DEBUG
macx:QMAKE_LFLAGS             +=  -stdlib=libc++ -liconv -Wno-c++11-narrowing -framework CoreFoundation


CMU112_BASE       =   ../../../base

INCLUDEPATH       =   src \
                      src/gui \
                      src/thirdparty/quazip-0.7.3/quazip/ \
                      src/thirdparty/minixml/inc \
                      src/thirdparty/tools/inc \
                      src/thirdparty/posixtar/inc \
                      $$[QT_INSTALL_PREFIX]/include/QtZlib 
win32:INCLUDEPATH+=   $$(CV_THIRDPARTY)/w32/poppler-0.57.0/qt5/src
macx:INCLUDEPATH +=   /usr/local/Cellar/poppler/$$POPPLER_VER/include/poppler \
                      /usr/local/Cellar/poppler/$$POPPLER_VER/include/poppler/qt5/ \
                      $$(CV_BOOST_INCLUDE)
                      
DEPENDPATH       +=   $$INCLUDEPATH

win32:contains(QT_ARCH, i386):release:LIBS += \
                      $$(CV_THIRDPARTY)/w32/poppler-0.57.0/qt5/src/Release/poppler-qt5.lib \
                      $$(CV_THIRDPARTY)/w32/poppler-0.57.0/Release/poppler.lib
win32:!contains(QT_ARCH, i386):release:LIBS += \
                      $$(CV_THIRDPARTY)/w64/poppler-0.57.0/qt5/src/Release/poppler-qt5.lib \
                      $$(CV_THIRDPARTY)/w64/poppler-0.57.0/Release/poppler.lib

CV_BOOST_LIBS    =    /Users/ole/dev/cmu_112/trunk/../thirdparty/mac/boost_1_60_0/stage/x64/lib                      
unix:LIBS       +=    $$CV_BOOST_LIBS/libboost_thread.a \
                      $$CV_BOOST_LIBS/libboost_date_time.a \
                      $$CV_BOOST_LIBS/libboost_regex.a \
                      $$CV_BOOST_LIBS/libboost_system.a \
                      $$CV_BOOST_LIBS/libboost_chrono.a
   
# Library dependency checking
PRE_TARGETDEPS   +=   $$LIBS

macx:LIBS        +=   -L/usr/local/Cellar/poppler/$$POPPLER_VER/lib -lpoppler -lpoppler-qt5
macx:LIBS        +=   -L/usr/local/Cellar/zlib/1.2.11/lib  -lz

TOOLS_SOURCES     =   src/thirdparty/tools/src/derror.cpp \
                      src/thirdparty/tools/src/derrorman.cpp \
                      src/thirdparty/tools/src/dpath.cpp \
                      src/thirdparty/tools/src/dtartools.cpp \
                      src/thirdparty/tools/src/dtimeval.cpp \
                      src/thirdparty/tools/src/dstringtools.cpp \
                      src/thirdparty/tools/src/dtimetools.cpp \
                      src/thirdparty/tools/src/dsystemtools.cpp \
                      src/thirdparty/tools/src/dfilesystemtools.cpp \
                      src/thirdparty/tools/src/dexception.cpp \
                      src/thirdparty/tools/src/dbase.cpp \
                      src/thirdparty/tools/src/dbaseio.cpp \
                      src/thirdparty/tools/src/dbasefile.cpp \
                      src/thirdparty/tools/src/dfile.cpp
                      
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
                      src/ddirparser.cpp \
                      src/dattachmentindexer.cpp \
                      src/dattachmentparser.cpp \
                      src/dsearchthread.cpp \
                      src/dinsightconfig.cpp \
                      src/dinsightreport.cpp \
                      src/dimportformat.cpp \
                      src/dosxtools.cpp \
                      src/dleafmatcher.cpp \
                      $$GUI_SOURCES \
                      $$ZIP_SOURCES \
                      $$POSIXTAR_SOURCES

HEADERS          +=   src/drunguard.h \
                      src/dxmlparser.h \
                      src/ddirparser.h \
                      src/dattachmentindexer.h \
                      src/dattachmentparser.h \
                      src/dsearchthread.h \
                      src/dinsightconfig.h \
                      src/dinsightreport.h \
                      src/dimportformat.h \
                      src/dosxtools.h \
                      src/dleafmatcher.h \
                      $$GUI_HEADERS \
                      $$ZIP_HEADERS \
                      $$TOOLS_INCLUDES \
                      $$POSIXTAR_INCLUDES   

TRANSLATIONS      =   insight_no.ts \
                      insight_nn.ts \
                      insight_en.ts

# OS-X create icons
createicons.depends = src/gui/resources/icon_32x32.png
createicons.target = src/gui/resources/icon_32x32.icns
createicons.commands = \
    /bin/zsh -c                                            \'; \
    src=src/gui/resources/icon_32x32.png                     ; \
                                                             ; \
    NAME=\$$\(basename \$$src .png); DIR="\$$NAME.iconset"   ; \
    mkdir -pv \$$DIR                                     \'\"; \
    for m r in \'n\' \'\' \'((n+1))\' \'@2x\'; do        \"\'; \
        for n in \$$\(seq 4 9 | grep -v 6); do               ; \
            p=\$$\((2**\$$m)); q=\$$\((2**\$$n))             ; \
            OUT="\$$DIR/icon_\$${q}x\$${q}\$${r}.png"        ; \
            sips -z \$$p \$$p \$$src --out \$$OUT            ; \
        done                                                 ; \
    done                                                     ; \
    iconutil -c icns \$$DIR                                  ; \
    rm -frv \$$DIR                                           ; \
    mv icon_32x32.icns src/gui/resources/.                 \' 


LANGUAGE_FILES.files = insight_en.qm insight_no.qm insight_nn.qm
LANGUAGE_FILES.path  = Contents/Resources    
CONFIG_FILES.files   = insight.conf formats testdata
CONFIG_FILES.path    = Contents/Resources
QMAKE_BUNDLE_DATA   += CONFIG_FILES LANGUAGE_FILES

#macx:ICON                 = src/gui/resources/icon_32x32.icns
#!macx:ICON                = src/gui/resources/icon_32x32.png
#macx:QMAKE_EXTRA_TARGETS += createicons
#macx:PRE_TARGETDEPS      += src/gui/resources/icon_32x32.icns
