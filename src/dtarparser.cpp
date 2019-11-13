/***************************************************************************
**
**  Implementation of the DTarParser class
**
**  Creation date:  2019/02/25
**  Created by:     Ole Liabo
**
**
**  Copyright (c) 2019 Piql AS. All rights reserved.
**
***************************************************************************/

//  PROJECT INCLUDES
//
#include    "dtarparser.h"
#include    "dtreeitem.h"
#include    "dtreemodel.h"

//  TAR INCLUDES
//
#include    "dtartools.h"

using namespace std;


/****************************************************************************/
/*! \class DTarParser dtarparser.cpp
 *  \ingroup Insight
 *  \brief TAR parser
 *
 *  Responsble for loading TAR file pointed to by fileName into model under 
 *  node rootNode.
 * 
 *  TAR parsing is done in a thread.
 */


//----------------------------------------------------------------------------
/*! 
 *  Constructor.
 */

DTarParser::DTarParser( DTreeItems* treeItems, const QString& fileName, DTreeModel* model, DTreeRootItem* rootNode, const DImportFormat* importFormat )
    : DXmlParser( treeItems, fileName, model, rootNode, importFormat )
{
}

DTarParser::~DTarParser()
{
}

void DTarParser::incItems( DTreeItem* item, bool finalItem )
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

void DTarParser::run()
{
    DLFR::DTarTools tar( m_Filename.toStdString() );

    if ( !tar.open() )
    {
        DInsightConfig::Log() << "Opening failed, file does not exist: " << m_Filename << endl;
        return;
    }

    //bool                getItems( DTarItemInfoVector& items, bool includeDirectories = true, bool includeChildTars=false );
    //bool                traverse( DTarToolsItemCallback& callback, bool includeDirectories = true, bool includeChildTars=false );

    DLFR::DTarTools::DTarItemInfoVector items;
    if ( !tar.getItems( items ) )
    {
        DInsightConfig::Log() << "Opening failed, file does not exist: " << m_Filename << endl;
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
        //m_Model->beginInsert( 0, 1, m_Model->index( m_RootNode->m_Parent ) );
        //const char* fileName = DXmlParser::AddToHashMap( m_Filename.toStdString().c_str() ); 
        //tarNode = m_Model->createItem( m_RootNode, m_RootNode, fileName );
        //m_Model->endInsert();
        tarNode = m_RootNode;
    }
    
    m_MaxItems = items.size();
    m_Items = 0;
    m_ItemsLastReport = 0;
    DLFR::DTarTools::DTarItemInfoVector::iterator it = items.begin();

    const char* fileName = DXmlParser::AddToHashMap( "filename" );
    const char* dirName  = DXmlParser::AddToHashMap( "directory" );
    const char* dateName = DXmlParser::AddToHashMap( "date" );
    const char* sizeName = DXmlParser::AddToHashMap( "size" );

    DTreeItem* currentNode = nullptr;
    while ( it != items.end() )
    {
        currentNode = tarNode;
        QStringList parts = QString(it->filename().c_str()).split('/');

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
        const string& filename = it->filename();
        char* fileString = strdup(filename.c_str());

        // Size
        int sizeBytes = 1 + snprintf( nullptr, 0, "%llu", it->fileSize() );
        char* sizeString = (char*)malloc( sizeBytes );
        snprintf(sizeString, sizeBytes, "%llu", it->fileSize());

        // Date
        char* dateString = (char*)malloc(32);
        time_t time = it->time();
        // Format: 15.06.2009 20:20:00
        std::tm tm;
        localtime_r(&time, &tm);
        strftime(dateString, 32, "%d.%m.%Y %H:%M:%S", &tm);         

        if ( it->filename().back() == '/' )
        {
            currentNode->addNode( m_Model->createLeaf( m_RootNode, dirName, fileString ) );
        }
        else
        {
            currentNode->addNode( m_Model->createLeaf( m_RootNode, fileName, fileString ) );
        }
        
        currentNode->addNode( m_Model->createLeaf( m_RootNode, sizeName, sizeString ) );
        currentNode->addNode( m_Model->createLeaf( m_RootNode, dateName, dateString ) );
        incItems( currentNode, false );

        it++;
    }

    m_NodeCount = items.size();
    incItems( currentNode, !isInterruptionRequested() );
    
}
