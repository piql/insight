/*****************************************************************************
**
**  Definition of DOsXTools class
**
**
**  Copyright (c) 2019 Piql AS. All rights reserved.
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
