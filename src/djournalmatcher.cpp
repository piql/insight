/*****************************************************************************
**
**  Definition of DJournalMatcher class
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
**
*****************************************************************************/

//  PROJECT INCLUDES
//
#include    "djournalmatcher.h"
#include    "dregexp.h"
#include    "dtreeitem.h"
#include    "dimport.h"

//  SYSTEM INCLUDES
//
#include    <QFileInfo>

/****************************************************************************/
/*! \class  DJournalMatcher  djournalmatcher.h
 *  \brief  Class to detect journal node.
 *
 * Node: <name-regexp>
 * Pages: <wildcard>
 */


//============================================================================
//  P U B L I C   I N T E R F A C E
//============================================================================

//----------------------------------------------------------------------------
/*! 
 * Parse string
 */

bool DJournalMatcher::CreateFromString( DJournalMatchers& matchers, const QString& str )
{
    // Empty string is OK
    if ( str.length() == 0 )
    {
        return true;
    }

    QStringList items = str.split( "@" );
    if ( items.size() % 3 != 0 )
    {
        return false;
    }

    for ( int i = 0; i < items.size(); i += 3 ) 
    {
        DJournalMatcher matcher;

        // Node matcher
        matcher.m_NodeMatch = QRegularExpression( items.at(i) );
        matcher.m_NodeMatch.optimize();

        // Wildcards
        matcher.m_PageWildcard = items.at( i + 1 );
        matcher.m_OcrWildcard = items.at( i + 2 );

        matchers.push_back( matcher );
    }
    return true;
}

QString DJournalMatcher::CreateWildcard( const QString& wildcardIn, DTreeItem* item )
{
    QString wildcard = wildcardIn;
    QString nodeName = QString( item->m_Text );

    static QRegularExpression getVariables( "%(.*?)%" );
    static QString nodeVariable( "%node%" );
    static QString rootDirVariable( "%rootdir%" );

    // extract all variable names
    QRegularExpressionMatchIterator it = getVariables.globalMatch( wildcard );
    while ( it.hasNext() )
    {
        QRegularExpressionMatch match = it.next();
        QString word = match.captured( 1 );

        if ( word == "node" )
        {
            wildcard.replace( nodeVariable, nodeName );
        }
        else if ( word == "rootdir" )
        {
            const DTreeRootItem* root = item->findRootItem();
            if ( root )
            {
                const DLeafNode* fileNameLeaf = root->findLeaf( DImport::FileNameKey().toStdString().c_str() );
                if ( fileNameLeaf )
                {
                    QFileInfo f( fileNameLeaf->m_Value );
                    wildcard.replace( rootDirVariable, f.absolutePath() );
                }
            }
        }
        else
        {
            // Search in nodes
            DLeafNodesIterator nodeIt = item->m_Nodes.begin();
            bool nodeFound = false;
            while ( nodeIt != item->m_Nodes.end() )
            {
                if ( QString( (*nodeIt)->m_Key ) == word )
                {
                    wildcard.replace( QString( "%%1%" ).arg( word ), (*nodeIt)->m_Value );
                    nodeFound = true;
                    break;
                }
                nodeIt++;
            }
        }
    }

    return wildcard;
}
