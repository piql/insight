#ifndef DINSIGHTSEARCHEXPRESSION_H
#define DINSIGHTSEARCHEXPRESSION_H

/*****************************************************************************
**  
**  Definition of the DInsightSearchExpression class
**
**  Creation date:  2017/09/08
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
#include   <QString>
#include   <QRegularExpression>

//  FORWARD DECLARATIONS
//
class DTreeItem;

//============================================================================
// CLASS: DInsightSearchExpression

class DInsightSearchExpression
{
public:
    DInsightSearchExpression();

    void updateText( const QString& text );
    bool match( DTreeItem* node );

private:
    enum { PLAIN, REGEXP, XPATH };
    int                 m_Type;
    QString             m_Text;
    QRegularExpression  m_RegExp;
    QStringList         m_Words;
};


#endif // DINSIGHTSEARCHEXPRESSION_H
