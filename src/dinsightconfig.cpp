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

DInsightConfig::DInsightConfig( const QString& fileName )
    : m_FileName( fileName )
{
}


QString  DInsightConfig::get( const QString& key, const QString& def /*= ""*/ )
{
    return QSettings( m_FileName, QSettings::IniFormat ).value( key, def ).toString();
}


int DInsightConfig::getInt( const QString& key, int def /*= -1*/ )
{
    return QSettings( m_FileName, QSettings::IniFormat ).value( key, def ).toInt();
}


bool DInsightConfig::getBool( const QString& key, bool def /*= false*/ )
{
    return QSettings( m_FileName, QSettings::IniFormat ).value( key, def ).toBool();
}


DRegExps DInsightConfig::getRegExps( const QString& key, const QString& def /*= ""*/ )
{
    return StringToRegExps( get( key, def ) );
}

DLeafMatchers DInsightConfig::getLeafMatchers( const QString& key, const QString& def /*= ""*/ )
{
    return StringToLeafMatchers( get( key, def ) );
}


QString DInsightConfig::getLocalizedKey( const QString& key )
{
    return key + "_" + get( "LANGUAGE", "EN" );
}

//----------------------------------------------------------------------------
/*! 
 *  Get config file value.
 */

QString DInsightConfig::Get( const QString& key, const QString& def /*= ""*/ )
{
    DInsightConfig conf( DefaultFileName() );
    return conf.get( key, def );
}


//----------------------------------------------------------------------------
/*! 
 *  Get integer config file value.
 */

int DInsightConfig::GetInt( const QString& key, int def /*= -1*/ )
{
    DInsightConfig conf( DefaultFileName() );
    return conf.getInt( key, def );
}


//----------------------------------------------------------------------------
/*! 
 *  Get bool config file value.
 */

bool DInsightConfig::GetBool( const QString& key, bool def /*= false*/ )
{
    DInsightConfig conf( DefaultFileName() );
    return conf.getBool( key, def );
}


//----------------------------------------------------------------------------
/*! 
 *  Get regexp list.
 */

DRegExps DInsightConfig::GetRegExps( const QString& key, const QString& def /*= ""*/ )
{
    DInsightConfig conf( DefaultFileName() );
    return conf.getRegExps( key, def );
}


//----------------------------------------------------------------------------
/*!
 *  Get leaf matchers list.
 */

DLeafMatchers DInsightConfig::GetLeafMatchers( const QString& key, const QString& def /*= ""*/ )
{
    DInsightConfig conf( DefaultFileName() );
    return conf.getLeafMatchers( key, def );
}


//----------------------------------------------------------------------------
/*! 
 *  Get localized regexp list.
 */

QString DInsightConfig::GetLocalizedKey( const QString& key )
{
    DInsightConfig conf( DefaultFileName() );
    return conf.getLocalizedKey( key );
}


DRegExps DInsightConfig::StringToRegExps( const QString& str )
{
    DRegExps regExps;

    QStringList regExpsString = str.split( "@" );
    foreach( const QString& r, regExpsString )
    {
        if ( r.length() )
        {
            QRegularExpression regExp( r );
            regExp.optimize();
            regExps.push_back( regExp );
        }
    }

    return regExps;    
}


//----------------------------------------------------------------------------
/*!
 *
 */

DLeafMatchers DInsightConfig::StringToLeafMatchers( const QString& str )
{
    DLeafMatchers matchers;

    QStringList strings = str.split( "@" );
    foreach( const QString& r, strings )
    {
        if ( r.length() )
        {
            DLeafMatcher matcher;
            if ( DLeafMatcher::CreateFromString( matcher, r) )
            {
                matchers.push_back( matcher );
            }
        }
    }

    return matchers;
}


//----------------------------------------------------------------------------
/*! 
 *  Set config file value.
 */

void    DInsightConfig::Set( const QString& key, const QString& value )
{
    QSettings( DefaultFileName(), QSettings::IniFormat ).setValue( key, value );
}


//----------------------------------------------------------------------------
/*! 
 *  Return application log.
 */

QDebug&  DInsightConfig::Log()
{
    static QFile logFile( Get( "LOGFILE", QCoreApplication::applicationName() + ".log" ) );
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

QString DInsightConfig::DefaultFileName()
{
    static QString def = QCoreApplication::applicationName() + QString( ".conf");

    return def;
}
