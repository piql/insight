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
#include   "dleafmatcher.h"

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
    DInsightConfig( const QString& fileName );

    QString         get( const QString& key, const QString& def = "" );
    int             getInt( const QString& key, int def = -1 );
    bool            getBool( const QString& key, bool def = false );
    DRegExps        getRegExps( const QString& key, const QString& def = "" );
    DLeafMatchers   getLeafMatchers( const QString& key, const QString& def = "" );
    QString         getLocalizedKey( const QString& key );
    
public:

    static QString       DefaultFileName();
    static QString       Get( const QString& key, const QString& def = "" );
    static int           GetInt( const QString& key, int def = -1 );
    static bool          GetBool( const QString& key, bool def = false );
    static DRegExps      GetRegExps( const QString& key, const QString& def = "" );
    static DLeafMatchers GetLeafMatchers( const QString& key, const QString& def = "" );
    static void          Set( const QString& key, const QString& value );
    static QString       GetLocalizedKey( const QString& key );
    static DRegExps      StringToRegExps( const QString& key );
    static DLeafMatchers StringToLeafMatchers( const QString& key );

    static QDebug&  Log();

private:
    QString         m_FileName;
};


#endif // DINSIGHTCONFIG_H
