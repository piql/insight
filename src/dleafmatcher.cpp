/*****************************************************************************
**
**  Definition of DOsXTools class
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
**
*****************************************************************************/

//  PROJECT INCLUDES
//
#include    "dleafmatcher.h"
#include    "dregexp.h"

//  SYSTEM INCLUDES
//


/****************************************************************************/
/*! \class  DLeafMatcher  dleafmatcher.h
 *  \brief  Class to match leaf name and leaf content.
 *
 * Uses quasi XQuery syntax: <name-regexp>([<content-reg-exp>])
 *
 */


//============================================================================
//  P U B L I C   I N T E R F A C E
//============================================================================

//----------------------------------------------------------------------------
/*! 
 *
 */

bool DLeafMatcher::CreateFromString( DLeafMatcher& matcher, const QString& str )
{
    // Extract content between [], if any
    QRegularExpression splitTokens("[\\[\\]]");
    QStringList items = str.split(splitTokens, QString::SplitBehavior::SkipEmptyParts);
    if ( items.size() != 1 && items.size() != 2 )
    {
        return false;
    }

    // Create matcher
    matcher.m_LeafMatch = DRegExp( items.at(0) );
    if ( items.size() == 2 )
    {
        matcher.m_ContentMatch = DRegExp( items.at(1) );
    }
    return true;
}
