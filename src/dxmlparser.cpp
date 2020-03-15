/***************************************************************************
**
**  Implementation of the DXmlParser class
**
**  Creation date:  2017/11/08
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
#include    "dxmlparser.h"
#include    "dtreeitem.h"
#include    "dtreemodel.h"

//  SYSTEM INCLUDES
//
#include    <fstream>
#include    <vector>
#include    <iostream>
#include    <sys/types.h>
#include    <sys/stat.h>
#include    <fcntl.h>
#include    <stdio.h>
#include    <string>

//  PLATFORM INCLUDES
//
#if defined (WIN32)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#include    <io.h>
#else
#include    <unistd.h>
#endif

//  XML INCLUDES
//
#include    "yxml.h"

using namespace std;


class DXmlStream
{
public:
    virtual ~DXmlStream() {}
    virtual bool open() = 0;
    virtual ssize_t read(char* buffer, size_t bufferSize) = 0;
    virtual off_t pos() = 0;
    virtual size_t size() = 0;
};

class DFileStream : public DXmlStream
{
public:
    DFileStream(const QString& fileName)
    {
        m_FileName = fileName;
    }
    ~DFileStream()
    {
#if defined (WIN32)
        _close( m_FileHandle );
#else
        close( m_FileHandle );
#endif
    }

    bool open()
    {
#if defined (WIN32)
        m_FileHandle = _open( m_FileName.toStdString().c_str(), _O_RDONLY | _O_SEQUENTIAL );
#else
        m_FileHandle = ::open( m_FileName.toStdString().c_str(), O_RDONLY );
#endif
        return m_FileHandle != -1;
    }

    off_t pos()
    {
#if defined (WIN32)
        return _telli64(m_FileHandle);
#else
        return lseek(m_FileHandle,0,SEEK_CUR);
#endif
    }

    size_t size()
    {
        size_t s;
#if defined (WIN32)
        _lseeki64( m_FileHandle, 0, SEEK_END );
        s = _telli64( m_FileHandle );
        _lseeki64( m_FileHandle, 0, SEEK_SET );
#else
        s = (size_t)lseek( m_FileHandle, 0, SEEK_END );
        lseek( m_FileHandle, 0, SEEK_SET );
#endif
        return s;
    }

    ssize_t read(char* buffer, size_t bufferSize)
    {
#if defined (WIN32)
        return _read( m_FileHandle,  buffer, sizeof(buffer));
#else
        return ::read( m_FileHandle,  buffer, sizeof(buffer));
#endif
    }

private:
    QString m_FileName;
    int     m_FileHandle;

};

/****************************************************************************/
/*! \class RandomStream dxmlparser.cpp
 *  \ingroup Insight
 *  \brief Create random stream of XML, for testing
 */

class DRandomStream : public DXmlStream
{
public:
    DRandomStream(unsigned int maxNodeCount )
    {
        m_MaxNodeCount = maxNodeCount;
        m_NodeCount = 0;
        m_MaxDepth = 10;
        m_LastDown = false;
    }

    bool open()
    {
        return true;
    }

    string getRandomString()
    {

        int randomLength = rand() % 20 + 1;
        string rnd;
        for ( int i = 0; i < randomLength; i++ )
        {
            rnd.push_back( 'a' + rand() % 10 );
        }
        rnd.push_back('>');
        rnd.push_back('\0');

        return rnd;
    }

    ssize_t read(char* buffer, size_t /*bufferSize*/)
    {
        buffer[0] = '\0';

        if (m_NodeCount > m_MaxNodeCount && m_Stack.size() == 0)
        {
            return 0;
        }

        if (m_Stack.size() == 0)
        {
            m_Stack.push_back("rot>");
            strcat(buffer, "<");
            strcat(buffer, m_Stack.back().c_str() );
            m_NodeCount++;
        }
        else if (m_NodeCount > m_MaxNodeCount )
        {
            strcat(buffer, "</");
            strcat(buffer, m_Stack.back().c_str());
            m_Stack.pop_back();
        }
        else
        {
            int updown = rand() % 3;
            if (updown == 0 && m_LastDown != true && m_Stack.size() > 1)
            {
                // Up
                strcat(buffer, "</");
                strcat(buffer, m_Stack.back().c_str());
                m_Stack.pop_back();
                m_LastDown = false;
            }
            else if (updown == 1 && m_Stack.size() < m_MaxDepth)
            {
                // Down
                strcat(buffer, "<");
                m_Stack.push_back(getRandomString());
                strcat(buffer, m_Stack.back().c_str() );
                m_NodeCount++;
                m_LastDown = true;
            }
            else
            {
                strcat(buffer, "<node>");
                // Leaf
                strcat(buffer, "<leaf>");
                int attributes = rand() % 20 + 1;
                for (int i = 0; i < attributes; i++)
                {
                    strcat(buffer, "<attribute>data</attribute>");
                }
                strcat(buffer, "</leaf>");

                strcat(buffer, "</node>");
                m_NodeCount += 1 + attributes;
                m_LastDown = false;
            }
        }

        return strlen(buffer);
    }

    off_t pos() { return m_NodeCount; }
    size_t size() { return m_MaxNodeCount; }

private:
    unsigned int m_NodeCount;
    unsigned int m_MaxNodeCount;
    unsigned int m_MaxDepth;
    std::vector<std::string> m_Stack;
    bool m_LastDown;
};


/****************************************************************************/
/*! \class UnorderedSetEqualString dxmlparser.cpp
 *  \ingroup Insight
 *  \brief Unordered set item compare
 */

/****************************************************************************/
/*! \class UnorderedSetDereference dxmlparser.cpp
 *  \ingroup Insight
 *  \brief Unordered set hash function
 */

/****************************************************************************/
/*! \class StringHash dxmlparser.cpp
 *  \ingroup Insight
 *  \brief String hashmap
 */

static const char* AddToHashMapInternal( const char* text, DXmlParser::StringHash& map )
{
    DXmlParser::StringHashIterator it = map.find( text );
    if ( it == map.end() )
    {
        char* s = strdup( text );
        it = map.insert( s ).first; 
    }

    return *it;
}

// Global node name hash map
static DXmlParser::StringHash nodeMap;
static DXmlParser::StringHash stringMap;

/****************************************************************************/
/*! \class DXmlContext dxmlparser.cpp
 *  \ingroup Insight
 *  \brief Context for XML parser
 */

class DXmlContext
{
public:
    DXmlContext();

    void incItems( DTreeItem* item, bool finalItem );

    // Processing
    DTreeItem*          m_CurrentNode;
    DTreeItem*          m_LastUpdateNode;
    char                m_Data[64*1024];    
    char*               m_DataPos;    
    const char*         m_Name;    
    bool                m_LastClose; // Was last instruction a close node?

    // Thread
    DXmlParser*         m_Thread;
    DTreeItems*         m_TreeItems;
    DTreeModel*         m_Model;
    DTreeRootItem*      m_RootNode;
    const DImportFormat*m_ImportFormat;
    DXmlStream*         m_Stream;

    // Stats
    unsigned long long  m_Items;
    unsigned long long  m_ItemsLastReport;
    unsigned long long  m_FileSize;
    unsigned long long  m_FilePos;
};


//----------------------------------------------------------------------------
/*! 
 *  Constructor.
 */

DXmlContext::DXmlContext()
    : m_LastUpdateNode( nullptr ),
      m_Items( 0ULL ),
      m_ItemsLastReport( 0ULL )
{
}


//----------------------------------------------------------------------------
/*! 
 *  Called for each item added to the tree data model. Responsible for 
 *  reporting progress to GUI.
 */

void DXmlContext::incItems( DTreeItem* item, bool finalItem )
{
    if ( item )
    {
        m_Items++;
        m_TreeItems->push_back( item );
    }

    const unsigned long reportInterval = 10000;
    if ( finalItem )
    {
        m_Thread->reportProgress( m_Items, 1.0f );
    }
    else if ( m_Items == m_ItemsLastReport + reportInterval )
    {
        m_FilePos = m_Stream->pos();
        m_Thread->reportProgress( m_Items, (m_FilePos / (float)m_FileSize) );
        m_ItemsLastReport = m_Items;
    }

    m_LastUpdateNode = item; // Todo: NOT USED?
}


//----------------------------------------------------------------------------
/*! 
 *  XML processor: Builds tree from XML
 */

#define yxml_isSP(c) (c == 0x20 || c == 0x09 || c == 0x0a)
inline static void sax_cb(yxml_t *x, yxml_ret_t r, DXmlContext* context ) 
{
    switch(r) 
    {
    case YXML_OK:
        break;
    case YXML_ELEMSTART:
            // Create new node?
            if ( context->m_Name )
            {
                // If root node is nullptr, first element found should be root
                if ( context->m_RootNode == nullptr )
                {
                    context->m_Model->beginInsert( 0, 1, QModelIndex() );
                    context->m_RootNode = context->m_Model->createDocumentRoot( context->m_Name, nullptr, context->m_ImportFormat );
                    context->m_CurrentNode = context->m_RootNode;
                    context->m_Model->endInsert();
                    DXmlParser::AddToNodeHashMap( context->m_Name );
                }
                else
                {
                    context->m_Model->beginInsert( 0, 1, context->m_Model->index( context->m_CurrentNode->m_Parent ) );
                    context->m_CurrentNode = context->m_Model->createItem( context->m_RootNode, context->m_CurrentNode, context->m_Name );
                    context->m_Model->endInsert();
                    DXmlParser::AddToNodeHashMap( context->m_Name );
                }
            }

            context->m_LastClose = false;
            context->m_Name = DXmlParser::AddToHashMap( x->elem );
        break;
    case YXML_ELEMEND:
            if ( !context->m_LastClose /*context->m_DataPos != context->m_Data*/ )
            {
                *context->m_DataPos = '\0';
                int s = (context->m_DataPos + 1) - context->m_Data;
                char* d = (char*)malloc( s );
                memcpy( d, context->m_Data, s);
                context->m_CurrentNode->addNode( context->m_Model->createLeaf( context->m_RootNode, context->m_Name, d ) );
            }
            else
            {
                context->incItems( context->m_CurrentNode, false );
                context->m_CurrentNode = context->m_CurrentNode->m_Parent;
            }
            context->m_DataPos = context->m_Data;
            *context->m_DataPos = '\0';
            context->m_Name = nullptr;
            context->m_LastClose = true;
        break;
    case YXML_ATTRSTART:
    case YXML_ATTREND:
        break;
    case YXML_PICONTENT:
    case YXML_CONTENT:
        {
            const char *text = x->data;
    
            if ( context->m_DataPos == context->m_Data )
            {
                while ( *text )
                {
                    if ( !yxml_isSP( *text ) )
                    {
                        break;    
                    }
                    text++;            
                }
            }
            
            while( *text )
                *(context->m_DataPos++) = *(text++);

            break;
        }
    case YXML_ATTRVAL:
        break;
    case YXML_PISTART:
        break;
    case YXML_PIEND:
        break;
    default:
        exit(0);
    }
}


/****************************************************************************/
/*! \class DXmlParser dxmlparser.cpp
 *  \ingroup Insight
 *  \brief XML parser
 *
 *  Responsble for loading XML file pointed to by fileName into model under 
 *  node rootNode.
 * 
 *  XML parsing is done in a thread.
 */


//----------------------------------------------------------------------------
/*! 
 *  Constructor.
 */

DXmlParser::DXmlParser( DTreeItems* treeItems, const QString& fileName, DTreeModel* model, DTreeRootItem* rootNode, const DImportFormat* importFormat )
    : m_TreeItems( treeItems ),
      m_Filename( fileName ),
      m_Model( model ),
      m_RootNode( rootNode ),
      m_ImportFormat( importFormat ),
      m_NodeCount( 0UL )
{
}

DXmlParser::~DXmlParser()
{
}


//----------------------------------------------------------------------------
/*! 
 *  Returns number of tree items processed.
 */

unsigned long DXmlParser::nodeCount()
{
    return m_NodeCount;
}


//----------------------------------------------------------------------------
/*!
 *  Returns true if XML parsing was successful.
 */

bool DXmlParser::loadedOK() const
{
    return m_LoadedOk;
}


//----------------------------------------------------------------------------
/*! 
 *  Thread entry point
 */

void DXmlParser::run()
{
    yxml_ret_t r;
    yxml_t x[1];
    char stack[8*1024];

    yxml_init(x, stack, sizeof(stack));

    DXmlContext context;
    context.m_CurrentNode = m_RootNode;
    context.m_LastUpdateNode = m_RootNode;
    context.m_DataPos = context.m_Data;
    *context.m_DataPos = '\0';
    context.m_LastClose = false;
    context.m_Name = nullptr;
    context.m_Thread = this;
    context.m_FileSize = 0UL;
    context.m_FilePos = 0UL;
    context.m_TreeItems = m_TreeItems;
    context.m_Model = m_Model;
    context.m_RootNode = m_RootNode;
    context.m_ImportFormat = m_ImportFormat;

    m_LoadedOk = true;

    DXmlStream* stream;
    if ( m_Filename.isEmpty())
    {
        stream = new DRandomStream(1000*1000);
    }
    else
    {
        stream = new DFileStream(m_Filename);
    }
    context.m_Stream = stream;

    if ( !stream->open() )
    {
        delete stream;
        DInsightConfig::Log() << "Opening failed, file does not exist: " << m_Filename << endl;
        return;
    }
    
    context.m_FileSize = stream->size();
    
    char buffer[1024*16];

    int bytesRead = stream->read(buffer, sizeof(buffer));

    while( bytesRead > 0 && !isInterruptionRequested())
    {
        char *b = buffer;
        while ( bytesRead )
        {
            r = yxml_parse(x, *b);

            if ( r == YXML_ESYN  )
            {
                requestInterruption();
                m_LoadedOk = false;
                break;
            }

            sax_cb(x, r, &context);
            b++;
            bytesRead--;
        }
        bytesRead = stream->read(buffer, sizeof(buffer));
    }

    context.incItems( context.m_CurrentNode, !isInterruptionRequested() );
    m_NodeCount = context.m_Items;
    m_RootNode = context.m_RootNode;

    DInsightConfig::Log() << "XML loading complete" << endl;

    delete stream;
}


//----------------------------------------------------------------------------
/*! 
 *  Report progress to GUI.
 */

void DXmlParser::reportProgress( unsigned long nodeCount, float progress )
{
    emit nodesReady( nodeCount, progress );
}



//----------------------------------------------------------------------------
/*! 
 *  Add node name to hash map.
 */
const char* DXmlParser::AddToNodeHashMap( const char* text )
{
    return AddToHashMapInternal( text, nodeMap );
}

const char* DXmlParser::AddToHashMap( const char* text )
{
    return AddToHashMapInternal( text, stringMap );
}

DXmlParser::StringHashIterator DXmlParser::NodeHashMapBegin()
{
    return nodeMap.begin();
}

DXmlParser::StringHashIterator DXmlParser::NodeHashMapEnd()
{
    return nodeMap.end();
}


//----------------------------------------------------------------------------
/*! 
 *  Return XML root item. 
 */

DTreeRootItem* DXmlParser::root()
{
    return m_RootNode;
}

const DImportFormat* DXmlParser::importFormat() const
{
    return m_ImportFormat;
}

