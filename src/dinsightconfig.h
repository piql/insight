#ifndef DINSIGHTCONFIG_H
#define DINSIGHTCONFIG_H

/*****************************************************************************
**  
**  Definition of the DInsightConfig class
**
**  Creation date:  2017/08/16
**  Created by:     Ole Liabo
**
**
**  Copyright (c) 2017 Piql AS. All rights reserved.
**
*****************************************************************************/

//  PROJECT INCLUDES
//
#include   "dregexp.h"

//  QT INCLUDES
//
#include   <QString>
#include   <QObject>
#include   <QVector>
#include   <QDebug>

//  FORWARD DECLARATIONS
//

//============================================================================
// CLASS: DInsightConfig

class DInsightConfig : public QObject
{
    Q_OBJECT
public:

    static QString  defaultFileName();
    static QString  get( const QString& key, const QString& def = "" );
    static int      getInt( const QString& key, int def = -1 );
    static bool     getBool( const QString& key, bool def = false );
    static DRegExps getRegExps( const QString& key, const QString& def = "" );
    static void     set( const QString& key, const QString& value );
    static QString  getLocalizedKey( const QString& key );

    static QDebug&  log();
};


#endif // DINSIGHTCONFIG_H
