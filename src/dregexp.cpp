/*****************************************************************************
**  
**  Implementation of the DRegExp class
**
**  Creation date:  2021/06/16
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
#include   "dregexp.h"


bool DRegExpUtils::Match( const DRegExps& regExps, const char* text )
{
    for ( int i = 0; i < regExps.length(); i++ )
    {
        QRegularExpressionMatch match = regExps.at( i ).match( text );
        if ( match.hasMatch() )
        {
            return true;
        }
    }
    return false;
}
