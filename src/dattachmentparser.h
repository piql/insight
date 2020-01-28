#ifndef DATTACHMENTPARSER_H
#define DATTACHMENTPARSER_H

/*****************************************************************************
**  
**  Definition of the DAttachmentParser class
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
*****************************************************************************/

//  PROJECT INCLUDES
//
#include    "dregexp.h"
#include    "dleafmatcher.h"
#include    "dtreeitem.h"

//  QT INCLUDES
//
#include    <QThread>
#include    <QString>
#include    <QMutex>
#include    <QWaitCondition>

//  FORWARD DECLARATIONS
//
class DTreeItem;

//============================================================================
// CLASS: DAttachment

class DAttachment
{
public:
    const char* m_FileName;
    DTreeItem*  m_TreeItem;
};

typedef unsigned int DAttachmentIndex;

//============================================================================
// CLASS: DAttachments

typedef std::vector<DAttachment*> DAttachments;

//============================================================================
// CLASS: DAttachmentIndexes

typedef std::vector<DAttachmentIndex> DAttachmentIndexes;


//============================================================================
// CLASS: DAttachmentParser

class DAttachmentParser : public QThread
{
    Q_OBJECT
public:

    DAttachmentParser( DTreeItems* treeItems, const QString& rootDir, const DLeafMatchers& attachmentTypeRegExp );
   ~DAttachmentParser();

    DAttachments&       attachments();
    DAttachmentIndexes& attachmentsFound();
    DAttachmentIndexes& attachmentsNotFound();
    qint64              attachmentsSizeInBytes();

public:
    static QString      AttachmentPath( const QString& fileName, const QString& documentRoot );
    static bool         AttachmentExists( const QString& fileName, const QString& documentRoot, qint64& size );

public slots: 
    void                nodesReady( unsigned long count, float progress );

private:
    void                run();
    bool                isAttachmentNode(  const char* text, const char* content );

private:
    unsigned int        m_MaxNodeCount;
    bool                m_FinalNodeReached;

    DTreeItems*         m_TreeItems;
    QString             m_RootDir;
    DAttachments        m_Attachments;
    DAttachmentIndexes  m_AttachmentsFound;
    DAttachmentIndexes  m_AttachmentsNotFound;
    DLeafMatchers       m_AttachmentTypeRegExp;
    QMutex              m_Mutex;
    QWaitCondition      m_Wait;
    qint64              m_AttachmentsSizeBytes;
};


#endif // DATTACHMENTPARSER_H
