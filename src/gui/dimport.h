#ifndef DIMOPRT_H
#define DIMOPRT_H

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
    void            loadReport();
    void            load();
    void            unload();
    void            unloadChildren();
    void            index();
    bool            hasChildren();
    DImportState    state();
    DTreeRootItem*  root();
    QString         fileName();
    QString         fileNameRoot();
    QString         databaseName();
    QString         reportsDir();
    QString         attachmentsDir();
    QString         searchConfig();
    unsigned int    attachmentsNotFound();
    unsigned int    attachmentsFound();
    unsigned int    attachmentsFailedToConvert();
    unsigned int    attachmentsEmpty();
    DAttachment*    getAttachment( int index );
    void            setFromReport( bool fromReport );
    bool            fromReport();
    qint64          attachmentsSizeInBytes();

public:
    static QString  fileNameKey();
    static QString  reportsDirKey();

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
    static DImport* CreateFromXml( const QString& fileName, DTreeModel* model, DInsightMainWindow* window, const DRegExps& documentTypeRegExp );
    static DImport* CreateFromReport( const QString& fileName, DTreeModel* model, DInsightMainWindow* window, const DRegExps& documentTypeRegExp );

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
    QString             m_ReportsDir;
    DTreeItems          m_TreeItems;
    DInsightMainWindow* m_Window;
    DRegExps            m_DocumentTypeRegExp;
    bool                m_FromReport;
};


//============================================================================
// CLASS: DImports

typedef std::vector<DImport*> DImports;
typedef DImports::iterator    DImportsIterator;


#endif // DIMOPRT_H
