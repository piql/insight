##############################################################################
#
#   Creation date:  2015-06-12
#   Created by:     Morgun Dmytro
#
#   Copyright (c) 2015 Piql AS, Norway. All rights reserved.
#
##############################################################################

##  INCLUDES
!include ($$(CV_BASE_ROOT)/base.pri) {
    error( "No base.pri file found" )
}

##  CONFIG
TEMPLATE            =  lib
CONFIG             +=  staticlib create_prl
unix:QMAKE_CFLAGS  +=  -std=c99

##  OUTPUT
TARGET                =  posixtar


##  INCLUDES
INCLUDEPATH         =  inc \
                       ../common/inc \
                       $$INCLUDE_ZLIB()
DEPENDPATH         +=  $$INCLUDEPATH


##  FILES
HEADERS             =  inc/posixtar.h

SOURCES             =  src/posixtar.c

unix:LIBS          +=  -lz
