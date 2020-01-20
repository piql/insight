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
**  Copyright (c) 2017 Piql AS. All rights reserved.
**
*****************************************************************************/

//  PROJECT INCLUDES
//
#include    "dregexp.h"
#include    "dleafmatcher.h"
#include    "dtreeitem.h"

//  QT INCLUDES
//


//  FORWARD DECLARATIONS
//
class DTreeRootItem;
class DTreeModel;
class DInsightMainWindow;
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
    enum DImportState { IMPORT_STATE_IMPORTING, IMPORT_STATE_CANCELING, IMPORT_STATE_INDEXING, IMPORT_STATE_DONE };

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
    DImportState    state();
    DTreeRootItem*  root();
    QString         fileName();
    QString         fileNameRoot();
    QString         formatName();
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

public:
    static QString              FileNameKey();
    static QString              ReportsDirKey();
    static QString              ExtractDirKey();
    static void                 SetReportFormat( const DImportFormat* importFormat );
    static const DImportFormat* GetReportFormat();

signals:
    void            imported( bool ok );
    void            indexed( bool ok );
    void            removed( bool ok );

public slots:
    void            loadXmlFinished();
    void            attachmentParserFinished();
    void            indexingIndexerStarted();
    void            indexingFinished();

private:
    void            importFinished();


public:
    static DImport* CreateFromExtract( const QString& fileName, DTreeModel* model, DInsightMainWindow* window, const DImportFormat* format, DTreeItem* parent, DImport* parentImport );
    static DImport* CreateFromTar( const QString& fileName, DTreeModel* model, DInsightMainWindow* window, const DImportFormat* format, DTreeItem* parent, DImport* parentImport );
    static DImport* CreateFromXml( const QString& fileName, DTreeModel* model, DInsightMainWindow* window, const DImportFormat* format, DTreeItem* parent, DImport* parentImport );
    static DImport* CreateFromReport( const QString& fileName, DTreeModel* model, DInsightMainWindow* window, const DImportFormats* formats );

private:
    static DImport* CreateFromFile(
        const QString& fileName,
        DTreeModel* model,
        DInsightMainWindow* window,
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
    DInsightMainWindow* m_Window;
    DLeafMatchers       m_DocumentTypeRegExp;
    bool                m_FromReport;
    QString             m_ExtractDir;
};


//============================================================================
// CLASS: DImports

typedef std::vector<DImport*> DImports;
typedef DImports::iterator    DImportsIterator;


#endif // DIMPORT_H
