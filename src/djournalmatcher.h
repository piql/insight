#ifndef DJOURNALMATCHER_H
#define DJOURNALMATCHER_H

/*****************************************************************************
**  
**  Definition of the DJournalMatcher class
**
**  Creation date:  2021/05/08
**  Created by:     Ole Liabo
**
**
**  Copyright (c) 2021 Piql AS.
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


//  QT INCLUDES
//
#include   <QRegularExpression>
#include   <QVector>

//  FORWARD DECLARATIONS
//
class DTreeItem;
class DJournalMatcher;

//============================================================================
// CLASS: DJournalMatchers

typedef QVector<DJournalMatcher>   DJournalMatchers;


//============================================================================
// CLASS: DJournalMatcher

class DJournalMatcher
{
public:
    QRegularExpression      m_NodeMatch;
    QString                 m_PageWildcard;
    QString                 m_OcrWildcard;

public:
    static bool CreateFromString( DJournalMatchers& matchers, const QString& str );
    static QString CreateWildcard( const QString& wildcard, DTreeItem* item );
};


#endif // DJOURNALMATCHER_H
