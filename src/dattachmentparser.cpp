/***************************************************************************
**
**  Implementation of the DAttachmentParser class
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
#include    "dattachmentparser.h"

//  QT INCLUDES
//
#include    <QFileInfo>
#include    <QDir>
#include    <QDirIterator>

/****************************************************************************/
/*! \class DJournalPage dattachmentparser.h
 *  \ingroup Insight
 *  \brief Points to journal attachments for one page
 *
 *  m_Checked: Include in export? Default is true.
 */

//----------------------------------------------------------------------------
/*!
 *  Constructor.
 */

DJournalPage::DJournalPage()
    : m_Checked( true )
{
}

bool DJournal::hasCheckedPages() const
{
    foreach(const DJournalPage& p, m_Pages)
    {
        if (p.m_Checked) 
        {
            return true;
        }    
    }
    return false;
}

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

DAttachmentParser::DAttachmentParser( 
    DTreeItems* treeItems, 
    const QString& rootDir, 
    const DLeafMatchers& attachmentTypeRegExp,
    const DJournalMatchers& journalMatchers )
    : m_MaxNodeCount( treeItems->size() ),
      m_FinalNodeReached( false ),
      m_TreeItems( treeItems ),
      m_RootDir( rootDir ),
      m_AttachmentTypeRegExp( attachmentTypeRegExp ),
      m_AttachmentsSizeBytes( 0 ),
      m_JournalMatchers( journalMatchers )
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

    foreach( DJournal* j, m_Journals )
    {
        delete j;
    }
}


//----------------------------------------------------------------------------
/*! 
 *  Return attachment nodes found.
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
 *  Return journal nodes found.
 */

DJournals& DAttachmentParser::journals()
{
    return m_Journals;
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


        // Journal?
        DJournalMatcher* journalMatcher = isJournalNode( (*currentNode)->m_Text );
        if ( journalMatcher )
        {
            // Find matching pages
            QString wildcardPages = DJournalMatcher::CreateWildcard( journalMatcher->m_PageWildcard, *currentNode );
            QString wildcardOcr = DJournalMatcher::CreateWildcard( journalMatcher->m_OcrWildcard, *currentNode );
            QFileInfo filePages( wildcardPages );
            QFileInfo fileOcr( wildcardOcr );

            QDir dirPages( filePages.absolutePath() );
            QDir dirOcr( fileOcr.absolutePath() );
            QFileInfoList pages = dirPages.entryInfoList( QStringList() << filePages.fileName(), QDir::NoFilter, QDir::SortFlag::Name);
            QFileInfoList ocrs = dirOcr.entryInfoList( QStringList() << fileOcr.fileName(), QDir::NoFilter, QDir::SortFlag::Name );
            
            DJournal* journal = new DJournal;
            journal->m_TreeItem = *currentNode;
            (*currentNode)->m_Journal = journal;
            for (int i = 0; i < pages.size(); i++ )
            {
                DJournalPage page;
                page.m_PageFileName = pages.at(i).absoluteFilePath();
                if ( ocrs.length() > i )
                {
                    page.m_OcrFileName = ocrs.at(i).absoluteFilePath();
                }
                journal->m_Pages.push_back( page );
            }

            m_Journals.push_back( journal );
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

//----------------------------------------------------------------------------
/*!
 *  Return true if current node points to a journal.
 */

DJournalMatcher* DAttachmentParser::isJournalNode( const char* text )
{
    QString key( text );

    foreach( const DJournalMatcher& matcher, m_JournalMatchers )
    {
        if ( matcher.m_NodeMatch.match( key ).hasMatch() )
        {
            return (DJournalMatcher*)&matcher;
        }
    }

    return NULL;
}
