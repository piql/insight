/***************************************************************************
**
**  Implementation of the DDirParser class
**
**  Creation date:  2019/07/01
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
#include    "ddirparser.h"
#include    "dtreeitem.h"
#include    "dtreemodel.h"

//  QT INCLUDES
//
#include    <QDir>
#include    <QDirIterator>
#include    <QDateTime>

//  PLATFORM INCLUDES
//
#if defined (WIN32)
#include <ctime>
#endif

using namespace std;


/****************************************************************************/
/*! \class DDirParser ddirparser.cpp
 *  \ingroup Insight
 *  \brief Directory parser
 *
 *  Responsible for loading all files and folders below given directory.
 * 
 *  Parsing is done in a thread.
 */


//----------------------------------------------------------------------------
/*! 
 *  Constructor.
 */

DDirParser::DDirParser( DTreeItems* treeItems, const QString& dir, DTreeModel* model, DTreeRootItem* rootNode, const DImportFormat* importFormat )
    : DXmlParser( treeItems, dir, model, rootNode, importFormat ),
      m_MaxItems( 0 ),
      m_Items( 0 ),
      m_ItemsLastReport( 0 )
{
}

DDirParser::~DDirParser()
{
}

void DDirParser::incItems( DTreeItem* item, bool finalItem )
{
    if ( item )
    {
        m_Items++;
        m_TreeItems->push_back( item );
    }

    const unsigned long reportInterval = 1;
    if ( finalItem )
    {
        reportProgress( m_Items, 1.0f );
    }
    else if ( m_Items == m_ItemsLastReport + reportInterval )
    {
        reportProgress( m_Items, m_MaxItems );
        m_ItemsLastReport = m_Items;
    }
}


//----------------------------------------------------------------------------
/*! 
 *  Thread entry point
 */

void DDirParser::run()
{
    QDir dir( m_Filename );
    if ( !dir.exists() )
    {
        DInsightConfig::Log() << "Opening failed, dir does not exist: " << m_Filename << endl;
        incItems( NULL, true );   
        return;
    }

    // Create root node
    DTreeItem* tarNode;
    if ( m_RootNode == nullptr )
    {
        m_Model->beginInsert( 0, 1, QModelIndex() );
        m_RootNode = m_Model->createDocumentRoot( m_Filename, nullptr, m_ImportFormat );
        tarNode = m_RootNode;
        m_Model->endInsert();
    }
    else
    {
        tarNode = m_RootNode;
    }
    
    QDirIterator itCount(dir, QDirIterator::Subdirectories);
    m_MaxItems = 0;
    while (itCount.hasNext())
    {
        m_MaxItems++;
        itCount.next();
    }
    m_Items = 0;
    m_ItemsLastReport = 0;

    const char* fileName = DXmlParser::AddToHashMap( "filename" );
    const char* dirName  = DXmlParser::AddToHashMap( "directory" );
    const char* dateName = DXmlParser::AddToHashMap( "date" );
    const char* sizeName = DXmlParser::AddToHashMap( "size" );

    DTreeItem* currentNode = nullptr;

    m_NodeCount = 0;
    QDirIterator it(dir, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        it.next();

        if ( it.fileName() == ".." || it.fileName() == "." )
        {
            continue;
        }

        currentNode = tarNode;
        QDir absDir( dir.absoluteFilePath( m_Filename ) );
        QString absFile = dir.absoluteFilePath( it.filePath() );
        QString relativeFilename = absDir.relativeFilePath( absFile );
        QStringList parts = relativeFilename.split('/');

        foreach( const QString& part, parts )
        {
            if ( part == "." || part.length() == 0 )
            {
                continue;
            }

            QByteArray ba = part.toLocal8Bit();

            DTreeItem* child = currentNode->findChild(ba.data());
            if ( child )
            {
                currentNode = child;
            }
            else
            {
                m_Model->beginInsert( 0, 1, m_Model->index( currentNode->m_Parent ) );
                const char* fileNameNode = DXmlParser::AddToHashMap( ba.data() ); 
                currentNode = m_Model->createItem( m_RootNode, currentNode, fileNameNode );
                m_Model->endInsert();
            }
        }
        
        // Add leafs
        // Filename
        const QString filename = it.fileName();
        char* fileString = strdup(filename.toUtf8().constData());

        QFileInfo fileInfo = it.fileInfo();
        // Size
        int sizeBytes = 1 + snprintf( nullptr, 0, "%llu", fileInfo.size() );
        char* sizeString = (char*)malloc( sizeBytes );
        snprintf(sizeString, sizeBytes, "%llu", fileInfo.size());

        // Date
        char* dateString = (char*)malloc(32);
        time_t time = fileInfo.created().toTime_t();
        // Format: 15.06.2009 20:20:00

#if defined (WIN32)
		std::tm* tm = localtime(&time);
		strftime(dateString, 32, "%d.%m.%Y %H:%M:%S", tm);
#else
        std::tm tm;
        localtime_r(&time, &tm);
        strftime(dateString, 32, "%d.%m.%Y %H:%M:%S", &tm);         
#endif

        if ( it.fileName().back() == '/' )
        {
            currentNode->addNode( m_Model->createLeaf( m_RootNode, dirName, fileString ) );
        }
        else
        {
            currentNode->addNode( m_Model->createLeaf( m_RootNode, fileName, relativeFilename ) );
        }
        
        currentNode->addNode( m_Model->createLeaf( m_RootNode, sizeName, sizeString ) );
        currentNode->addNode( m_Model->createLeaf( m_RootNode, dateName, dateString ) );
        incItems( currentNode, false );
        m_NodeCount++;
    }

    incItems( currentNode, !isInterruptionRequested() );   
}
