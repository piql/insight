/***************************************************************************
**
**  Implementation of the DInsightConfig class
**
**  Creation date:  2017/06/12
**  Created by:     Ole Liabo
**
**
**  Copyright (c) 2017 Piql AS. All rights reserved.
**
***************************************************************************/

//  PROJECT INCLUDES
//
#include    "dinsightconfig.h"

//  QT INCLUDES
//
#include    <QSettings>
#include    <QCoreApplication>
#include    <QFile>

/****************************************************************************/
/*! \class DInsightConfig dinsightconfig.h
 *  \ingroup Insight
 *  \brief Application config file
 * 
 *  Handles config file settings.
 *  
 */
 
//===================================
//  P U B L I C   I N T E R F A C E
//===================================


//----------------------------------------------------------------------------
/*! 
 *  Get config file value.
 */

QString DInsightConfig::get( const QString& key, const QString& def /*= ""*/ )
{
    return QSettings( defaultFileName(), QSettings::IniFormat ).value( key, def ).toString();
}


//----------------------------------------------------------------------------
/*! 
 *  Get integer config file value.
 */

int DInsightConfig::getInt( const QString& key, int def /*= -1*/ )
{
    return QSettings( defaultFileName(), QSettings::IniFormat ).value( key, def ).toInt();
}


//----------------------------------------------------------------------------
/*! 
 *  Get bool config file value.
 */

bool DInsightConfig::getBool( const QString& key, bool def /*= false*/ )
{
    return QSettings( defaultFileName(), QSettings::IniFormat ).value( key, def ).toBool();
}


//----------------------------------------------------------------------------
/*! 
 *  Get regexp list.
 */

DRegExps DInsightConfig::getRegExps( const QString& key, const QString& def /*= ""*/ )
{
    DRegExps regExps;

    QStringList regExpsString = DInsightConfig::get( key, def ).split( "@" );
    foreach( const QString& r, regExpsString )
    {
        QRegularExpression regExp( r );
        regExp.optimize();
        regExps.push_back( regExp );
    }

    return regExps;
}


//----------------------------------------------------------------------------
/*! 
 *  Get localized regexp list.
 */

QString DInsightConfig::getLocalizedKey( const QString& key )
{
    return key + "_" + DInsightConfig::get( "LANGUAGE", "en" );
}


//----------------------------------------------------------------------------
/*! 
 *  Set config file value.
 */

void    DInsightConfig::set( const QString& key, const QString& value )
{
    QSettings( defaultFileName(), QSettings::IniFormat ).setValue( key, value );
}


//----------------------------------------------------------------------------
/*! 
 *  Return application log.
 */

QDebug&  DInsightConfig::log()
{
    static QFile logFile( get( "LOGFILE", QCoreApplication::applicationName() + ".log" ) );
    static QDebug logStream( &logFile );

    if ( !logFile.isOpen() )
    {
        logFile.open( QIODevice::WriteOnly | QIODevice::Text );
    }

    return logStream;
}


//----------------------------------------------------------------------------
/*! 
 *  Get def config file name.
 */

QString DInsightConfig::defaultFileName()
{
    static QString def = QCoreApplication::applicationName() + QString( ".conf");

    return def;
}
