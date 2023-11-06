#ifndef DIMPORT_H
#define DIMPORT_H

/*****************************************************************************
**  
**  Definition of the DImport class
**
**  Creation date:  2017/03/29
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
#include    "djournalmatcher.h"
#include    "dpendingimport.h"

//  QT INCLUDES
//
#include    <QString>
#include    <QUuid>

//  FORWARD DECLARATIONS
//
class DTreeRootItem;
class DTreeModel;
class DXmlParser;
class DAttachmentParser;
class DAttachmentIndexer;
class DAttachment;
class DImportFormats;

//============================================================================
// CLASS: DImport

class DImport : public QObject
{
    Q_OBJECT
public:
    enum DImportState { 
        IMPORT_STATE_UNDEFINED,
        IMPORT_STATE_IMPORTING_REPORT,
        IMPORT_STATE_REPORT_IMPORTED,
        IMPORT_STATE_IMPORTING, 
        IMPORT_STATE_CANCELING, 
        IMPORT_STATE_INDEXING, 
        IMPORT_STATE_DONE };
    enum DIndexingState { 
        INDEXING_STATE_OK, 
        INDEXING_STATE_CANCELED, 
        INDEXING_STATE_SKIP, 
        INDEXING_STATE_ERROR };

private:
    DImport( DTreeModel* model );

public:
    ~DImport();

public:
    bool            loadReport();
    void            load( DXmlParser* parser );
    void            load( const DImportFormat* format );
    void            unload();
    void            unloadChildren();
    void            index();
    bool            hasChildren();
    DImportState    state() const;
    DTreeRootItem*  root() const;
    QString         fileName() const;
    QString         fileNameRoot();
    QString         formatName() const;
    QString         databaseName();
    QString         reportsDir();
    QString         attachmentsDir();
    QString         searchConfig();
    QString         extractDir();
    unsigned int    attachmentsNotFound();
    unsigned int    attachmentsFound();
    unsigned int    attachmentsFailedToConvert();
    unsigned int    attachmentsEmpty();
    DAttachment*    getAttachment( int index );
    void            setFromReport( bool fromReport );
    bool            fromReport();
    qint64          attachmentsSizeInBytes();
    const DPendingImports& pendingImports();
    const QUuid&    uuid() const;

public:
    static QString              FileNameKey();
    static QString              ReportsDirKey();
    static QString              ExtractDirKey();
    static void                 SetReportFormat( const DImportFormat* importFormat );
    static const DImportFormat* GetReportFormat();

signals:
    void            imported( bool ok );
    void            indexed( DIndexingState state );
    void            removed( bool ok );
    void            nodesReady( unsigned long count, float progress );
    void            indexingProgress( float progress );

public slots:
    void            loadXmlFinished();
    void            attachmentParserFinished();
    void            indexingIndexerStarted();
    void            indexingFinished();
    void            nodesReadySlot(unsigned long count, float progress);
    void            indexingProgressSlot(float progress);

private:
    void            importFinished();


public:
    static DImport* CreateFromExtract( const QString& fileName, DTreeModel* model, QWidget* window, const DImportFormat* format, DTreeItem* parent, DImport* parentImport );
    static DImport* CreateFromTar( const QString& fileName, DTreeModel* model, QWidget* window, const DImportFormat* format, DTreeItem* parent, DImport* parentImport );
    static DImport* CreateFromXml( const QString& fileName, DTreeModel* model, QWidget* window, const DImportFormat* format, DTreeItem* parent, DImport* parentImport );
    static DImport* CreateFromReport( const QString& fileName, DTreeModel* model, const DImportFormats* formats );

private:
    static DImport* CreateFromFile(
        const QString& fileName,
        DTreeModel* model,
        QWidget* window,
        const DImportFormat* format,
        DTreeItem* parent,
        DImport* parentImport );
    
private:
    DImportState        m_ImportState;
    DTreeModel*         m_Model;
    DTreeRootItem*      m_RootItem;
    DXmlParser*         m_XmlParser;
    DAttachmentParser*  m_AttachmentParser;
    DAttachmentIndexer* m_AttachmentIndexer;
    bool                m_XmlFinished;
    bool                m_AttachmentFinished;
    QString             m_FileName;
    QString             m_FileNameDir;
    QString             m_ImportFormat;
    QString             m_ReportsDir;
    DTreeItems          m_TreeItems;
    DLeafMatchers       m_DocumentTypeRegExp;
    bool                m_FromReport;
    QString             m_ExtractDir;
    DJournalMatchers    m_JournalMatchers;
    DPendingImports     m_PendingImports;
    QUuid               m_Uuid;
};


//============================================================================
// CLASS: DImports

typedef std::vector<DImport*>       DImports;
typedef DImports::iterator          DImportsIterator;
typedef DImports::const_iterator    DImportsConstIterator;

DImport* GetImport(const DImports& imports, const QString& uuid);

#endif // DIMPORT_H
