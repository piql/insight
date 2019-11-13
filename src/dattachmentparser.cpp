/***************************************************************************
**
**  Implementation of the DAttachmentParser class
**
**  Creation date:  2017/11/08
**  Created by:     Ole Liabo
**
**
**  Copyright (c) 2017 Piql AS. All rights reserved.
**
***************************************************************************/

//  PROJECT INCLUDES
//
#include    "dattachmentparser.h"

//  QT INCLUDES
//
#include    <QFileInfo>
#include    <QDir>

/****************************************************************************/
/*! \class DAttachmentParser dattachmentparser.h
 *  \ingroup Insight
 *  \brief Parsing of attachments
 *
 *  Runs in a separate thread.
 */

//----------------------------------------------------------------------------
/*! 
 *  Constructor.
 */

DAttachmentParser::DAttachmentParser( DTreeItems* treeItems, const QString& rootDir, const DLeafMatchers& attachmentTypeRegExp )
    : m_MaxNodeCount( treeItems->size() ),
      m_FinalNodeReached( false ),
      m_TreeItems( treeItems ),
      m_RootDir( rootDir ),
      m_AttachmentTypeRegExp( attachmentTypeRegExp ),
      m_AttachmentsSizeBytes( 0 )
{
}


//----------------------------------------------------------------------------
/*! 
 *  Destructor.
 */

DAttachmentParser::~DAttachmentParser()
{
    foreach( DAttachment* a, m_Attachments )
    {
        delete a;
    }
}


//----------------------------------------------------------------------------
/*! 
 *  Return number of attachment nodes found.
 */

DAttachments& DAttachmentParser::attachments()
{
    return m_Attachments;
}


//----------------------------------------------------------------------------
/*! 
 *  Return number of attachments found.
 */

DAttachmentIndexes& DAttachmentParser::attachmentsFound()
{
    return m_AttachmentsFound;
}


//----------------------------------------------------------------------------
/*! 
 *  Return numner of attachments not found.
 */

DAttachmentIndexes& DAttachmentParser::attachmentsNotFound()
{
    return m_AttachmentsNotFound;
}


//----------------------------------------------------------------------------
/*! 
 *  Return total size of attachments in bytes.
 */

qint64 DAttachmentParser::attachmentsSizeInBytes()
{
    return m_AttachmentsSizeBytes;
}


//----------------------------------------------------------------------------
/*! 
 *  Slot called when new nodes are ready for processing.
 */

void DAttachmentParser::nodesReady( unsigned long count, float progress )
{
    QMutexLocker lock( &m_Mutex );

    m_MaxNodeCount = count;
    m_FinalNodeReached = progress == 1.0f;

    m_Wait.wakeAll();
}


//----------------------------------------------------------------------------
/*! 
 *  Return full path to attachment.
 */

QString DAttachmentParser::AttachmentPath( const QString& fileName, const QString& documentRoot )
{
    QFileInfo info( fileName );
    if ( !info.isAbsolute() )
    {
        QDir dir( documentRoot );
        return dir.filePath( fileName );
    }

    return fileName;
}


//----------------------------------------------------------------------------
/*! 
 *  Return true if attachment exists.
 */

bool DAttachmentParser::AttachmentExists( const QString& fileName, const QString& documentRoot, qint64& size )
{
    QFileInfo info( fileName );
    if ( !info.isAbsolute() )
    {
        QDir dir( documentRoot );
        info = QFileInfo( dir.filePath( fileName ) );
    }

    bool exists = info.exists();
    if ( exists )
    {
        size = info.size();
    }
    return exists;
 }


//----------------------------------------------------------------------------
/*! 
 *  Thread entry point.
 */

void DAttachmentParser::run()
{
    unsigned int count = 0;
    DTreeItemsIterator currentNode = m_TreeItems->end();
    while ( !isInterruptionRequested() )
    {
        {
            QMutexLocker lock( &m_Mutex );

            if ( count == m_MaxNodeCount || currentNode ==  m_TreeItems->end() )
            {
                if ( m_FinalNodeReached )
                {
                    break;
                }

                while ( !m_Wait.wait(&m_Mutex, 1000 ) )
                {
                    if ( isInterruptionRequested() )
                    {
                        break;
                    }
                }        

                if ( count == 0 )
                {
                    currentNode = m_TreeItems->begin();
                }
            } 

            if ( currentNode == m_TreeItems->end() )
            {
                continue;
            }
        }

        DLeafNodesIterator it = (*currentNode)->m_Nodes.begin();
        while ( it != (*currentNode)->m_Nodes.end() )
        {
            // Attachment?
            if ( isAttachmentNode( (*it)->m_Key, (*it)->m_Value ) )
            {
                DAttachment* attachment = new DAttachment;
                attachment->m_FileName = (*it)->m_Value;
                attachment->m_TreeItem = *currentNode;
                m_Attachments.push_back( attachment );

                qint64 size;
                if ( AttachmentExists((*it)->m_Value, m_RootDir, size ) )
                {
                    m_AttachmentsFound.push_back( (unsigned int)m_Attachments.size()-1 );
                    m_AttachmentsSizeBytes += size;
                }
                else
                {
                    m_AttachmentsNotFound.push_back( (unsigned int)m_Attachments.size()-1 );
                }

            }
            
            it++;
        }
        currentNode++;
        count++;
    }    
}


//----------------------------------------------------------------------------
/*! 
 *  Return true if current node points to an attachment.
 */

bool DAttachmentParser::isAttachmentNode( const char* text, const char* content )
{
    QString key( text );
    QString value( content );

    foreach( const DLeafMatcher& matcher, m_AttachmentTypeRegExp )
    {
        if ( matcher.m_LeafMatch.match( key ).hasMatch() )
        {
            if ( matcher.m_ContentMatch.pattern().length() != 0 )
            {
                if ( matcher.m_ContentMatch.match( value ).hasMatch() )
                {
                    return true;
                }
            }
            else
            {
                return true;
            }
        }
    }

    return false;
}
