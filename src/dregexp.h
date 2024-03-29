#ifndef DREGEXP_H
#define DREGEXP_H

/*****************************************************************************
**  
**  Definition of the DRegExp class
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


//  QT INCLUDES
//
#include   <QRegularExpression>
#include   <QVector>

//  FORWARD DECLARATIONS
//

//============================================================================
// CLASS: DRegExps
typedef QRegularExpression DRegExp;
typedef QVector<DRegExp>   DRegExps;

class DRegExpUtils
{
public:
    static bool Match( const DRegExps& regExps, const char* text );
};

#endif // DREGEXP_H
