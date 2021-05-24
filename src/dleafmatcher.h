#ifndef DLEAFMATCHER_H
#define DLEAFMATCHER_H

/*****************************************************************************
**  
**  Definition of the DLeafMatcher class
**
**  Creation date:  2019/10/16
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


//  QT INCLUDES
//
#include   <QRegularExpression>
#include   <QVector>

//  FORWARD DECLARATIONS
//
class DLeafMatcher;

//============================================================================
// CLASS: DLeafMatcher
class DLeafMatcher
{
public:
    QRegularExpression      m_LeafMatch;
    QRegularExpression      m_ContentMatch;

    bool isMatch( const QString& key, const QString& value ) const;

public:
    static bool CreateFromString( DLeafMatcher& matcher, const QString& str );
    static bool IsMatch( const QVector<DLeafMatcher>& matchers, const QString& key, const QString& value );
};

//============================================================================
// CLASS: DLeafMatchers
typedef QVector<DLeafMatcher>   DLeafMatchers;


#endif // DLEAFMATCHER_H
