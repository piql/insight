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
**  Copyright (c) 2020 Piql AS.
**  
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 3 of the License, or
**  any later version.
**  
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**  
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
#include   <QSettings>

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
    static QDebug&       Log();
private:
    DInsightConfig();
    static DInsightConfig& AppConfig();

private:
    QString         m_FileName;
    QSettings       m_Settings;
};


#endif // DINSIGHTCONFIG_H
