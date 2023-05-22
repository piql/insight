/***************************************************************************
**
**  Implementation of the DSearchThread class
**
**  Creation date:  2017/12/14
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
***************************************************************************/

//  PROJECT INCLUDES
//
#include    "dsearchthread.h"
#include "QtCore/qlocale.h"
#include    "dtreemodel.h"
#include    "dtreeitem.h"
#include    "dattachmentindexer.h"
#include    "dinsightconfig.h"

//  QT INCLUDES
//
#include    <QSqlDatabase>
#include    <QSqlQuery>
#include    <QSqlError>
#include    <QStringConverter>

//  SYSTEM INCLUDES
//
#include    <string>
#include    <cctype>
#include    <locale>
#include    <iostream>

//  NAMESPACES
//
using namespace std;


/****************************************************************************/
/*! \class DSearchThread dsearchthread.cpp
 *  \ingroup Insight
 *  \brief Thread that searches through DTreeModel data model.
 */

//----------------------------------------------------------------------------
/*! 
 *  Constructor
 */

DSearchThread::DSearchThread( 
    DTreeModel* model, 
    DImports* imports, 
    const QString& searchString, 
    unsigned int maxMatchCount, 
    unsigned int searchModes,
    DXmlParser::StringHash& nodesToExclude )
    : m_Model( model ),
      m_Imports( imports ),
      m_SearchString( searchString ),
      m_NodeCount( 0 ),
      m_MatchCount( 0 ),
      m_MaxMatchCount( maxMatchCount ),
      m_SearchModes( searchModes ),
      m_CurrentPage( 0 ),
      m_SearchResult( SEARCH_UNDEFINED ),
      m_NodesToExclude( nodesToExclude )
{
}


//----------------------------------------------------------------------------
/*! 
 *  Return number of matching items.
 */

unsigned int DSearchThread::matchCount()
{
    return m_MatchCount;
}


//----------------------------------------------------------------------------
/*! 
 *  Return number of pages.
 */

unsigned int DSearchThread::pageCount()
{
    return (unsigned int)m_Pages.size();
}


//----------------------------------------------------------------------------
/*! 
 *  Return current page.
 */

unsigned int DSearchThread::currentPage()
{
    return m_CurrentPage;
}


//----------------------------------------------------------------------------
/*! 
 *  Return true if search result has more pages.
 */

bool DSearchThread::hasMorePages()
{
    return m_SearchResult == SEARCH_MAX_FOUND;
}


//----------------------------------------------------------------------------
/*! 
 *  Go to next page.
 */

void DSearchThread::gotoNextPage()
{
    if ( m_CurrentPage < m_Pages.size() - 1 )
    {
        m_CurrentPage++;
    }
    else
    {
        if ( hasMorePages() )
        {
            m_CurrentPage++;
        }
    }
}
    

//----------------------------------------------------------------------------
/*! 
 *  Go to previous page.
 */

void DSearchThread::gotoPrevPage()
{
    if ( m_CurrentPage > 0 )
    {
        m_CurrentPage--;
    }
}


//----------------------------------------------------------------------------
/*! 
 *  Go to first page.
 */

void DSearchThread::gotoFirstPage()
{
    m_CurrentPage = 0;
}


//----------------------------------------------------------------------------
/*! 
 *  Thread entry point
 */

void DSearchThread::run()
{
    bool searchAttachments = m_SearchModes & SEARCH_MODE_ATTACHMENTS;
    m_MatchCount = 0;
    if ( searchAttachments )
    {
        QSqlDatabase db;
        if ( QSqlDatabase::contains( "insight" ) )
        {
            // Database must be created in same thread..
            QSqlDatabase::removeDatabase("insight");
        }
        db = QSqlDatabase::addDatabase( "QMYSQL", "insight" );

        if (!db.isValid())
        {
            DInsightConfig::Log() << "Could not create database connection" << Qt::endl;
        }
        db.setHostName( "127.0.0.1" );
        //db.setConnectOptions( "CLIENT_INTERACTIVE=TRUE" );
        db.setPort( 9306 );
        //db.setUserName("root");
        //db.setPassword("");
        //db.setDatabaseName((*it)->databaseName());
        if (!db.isOpen())
        {
           if (!db.open())
           {
              DInsightConfig::Log()
                  << "Failed to open database: "
                  << db.lastError().text()
                  << " port: " << db.port()
                  << Qt::endl;
              m_SearchResult = SEARCH_ABORTED;
              return;
            }
        }

        DImportsIterator it = m_Imports->begin();
        DImportsIterator itEnd = m_Imports->end();
        int importIndex = 0;
        while ( it != itEnd )
        {
            if ( (*it)->hasChildren() )
            {
                QString dbName = (*it)->databaseName();
                QString queryString = QString("SELECT i FROM %1 WHERE MATCH('%2');").arg( dbName ).arg(m_SearchString);
                QSqlQuery query( queryString, db );
                bool ok = query.exec();
                if ( !ok )
                {
                    QString dbError = query.lastError().text();
                    DInsightConfig::Log() << "Search deamon query failed: " << queryString << Qt::endl;
                    DInsightConfig::Log() << "Search deamon error: " << dbError << Qt::endl;
                }
                else
                {
                    while (query.next()) 
                    {
                        // Index to attachment
                        QVariant value = query.value(0);

                        int attachmentIndex = value.toInt();
                        emit attachmentMatch( importIndex, attachmentIndex );
                        m_MatchCount++;
                    }        
                }
            }
            importIndex++;
            it++;
        }
    }
    else
    {
        std::string t;
    
        if ( m_SearchModes & SEARCH_MODE_CASE_INSENSITIVE )
        {
            t = QLocale::system().toLower(m_SearchString).toStdString();
        }
        else
        {
            t = m_SearchString.toStdString();
        }

        m_SearchStringLength = (unsigned int)t.length();
        bool startFound = m_CurrentPage == 0;

        // Could consider launching one thread per root node here...
        SearchResult res = SEARCH_NOT_FOUND;
        unsigned int c = m_Model->rootCount();
        for ( unsigned int i = 0; i < c; i++ )
        {
            if ( m_Pages.size() == 0 )
            {
                m_Pages.push_back( m_Model->documentRoot( i ) );
            }

            res = searchTree( m_Model->documentRoot( i ), t.c_str(), m_Model->index(i,0), startFound );
            if ( res == SEARCH_ABORTED || res == SEARCH_MAX_FOUND )
            {
                break;
            }
        }
        m_SearchResult = res;
    }
}


//----------------------------------------------------------------------------
/*! 
 *  Recursively search for tree leaf nodes that has data matching the text.
 *
 *  \param startFound Used to provide "paged" search result, where the search 
 *                    result is grouped into pages. The user is presented a 
 *                    'prev' / 'next' button depending on where in the search  
 *                    result he/she currently is.
 */

DSearchThread::SearchResult DSearchThread::searchTree( DTreeItem* node, const char* text, const QModelIndex& index, bool& startFound )
{    
    if ( m_MatchCount >= m_MaxMatchCount )
    {
        if ( m_Pages.size() == m_CurrentPage + 1 )
        {
            m_Pages.push_back( node );
        }
        return SEARCH_MAX_FOUND;
    }

    if ( m_NodeCount % 5000 == 0 )
    {
        emit searchProgress( 0.5f );

        if ( isInterruptionRequested() )
        {
            return SEARCH_ABORTED;
        }
    }
    m_NodeCount++;

    // Have we found the start of the page?
    if ( !startFound )
    {
        if ( m_Pages[m_CurrentPage] == node )
        {
            startFound = true;
        }
    }

    // Search through data attached to this node
    bool leafMatch = false;
    if ( startFound )
    {
        if ( m_SearchStringLength )
        {
            if ( m_NodesToExclude.find( node->m_Text ) == m_NodesToExclude.end() )
            {
                for ( const DLeafNode* leaf: node->m_Nodes )
                {
                    if ( m_SearchModes & SEARCH_MODE_CASE_INSENSITIVE )
                    {
                        size_t valueLength = strlen(leaf->m_Value);
                        char* value = (char*)malloc(valueLength+1);
                        for ( size_t i = 0; i < valueLength; i++ )
                        {
                            value[i] = std::tolower(leaf->m_Value[i], std::locale());
                        }
                        value[valueLength] = '\0';
                        
                        leafMatch = strstr( value, text ) != nullptr;
                        free(value);
                    }
                    else
                    {
                        leafMatch = strstr( node->m_Text, text ) != nullptr;
                    }

                    if ( leafMatch )
                    {
                        break;
                    }
                }
            }
        }
        else
        {
            leafMatch = true;
        }
    
        // If any of the leaf nodes match, emit signal. This will update the search result UI.
        if ( leafMatch )
        {
            m_MatchCount++;
            emit searchMatch( index );
        }
    }

    // Search through all child nodes for matching text
    bool childMatch = false;
    for ( unsigned int i = 0; i < node->m_Children.size(); i++ )
    {
        SearchResult res = searchTree( (DTreeItem*)node->m_Children[ i ], text, m_Model->index( i, 0, index ), startFound );
        if ( res == SEARCH_FOUND ) 
        {
            childMatch = true;
        }
        else if ( res == SEARCH_MAX_FOUND ) 
        {
            return res;
        }            
    }

    bool match = leafMatch || childMatch;

    return match ? SEARCH_FOUND : SEARCH_NOT_FOUND;    
}
