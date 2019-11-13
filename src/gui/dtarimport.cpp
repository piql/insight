/***************************************************************************
**
**  Implementation of the DTarImport class
**
**  Creation date:  2019/02/20
**  Created by:     Ole Liabo
**
**
**  Copyright (c) 2019 Piql AS. All rights reserved.
**
***************************************************************************/

//  PROJECT INCLUDES
//
#include    "dtarimport.h"


/****************************************************************************/
/*! \class DTarImport dtarimport.h
 *  \ingroup Insight
 *  \brief Handles import of tar files
 */
 

//----------------------------------------------------------------------------
/*! 
 *  Start loading (parsing) of TAR.
 */

void DImport::load()
{
    m_ImportState = IMPORT_STATE_IMPORTING;

    // Cleanup connections
    disconnect();

    // Connect importer to main window
    connect( this, &DImport::imported, m_Window, &DInsightMainWindow::importFileFinished );

    // Start parsing XML
    assert( m_XmlParser == nullptr );
    m_XmlParser = new DXmlParser( &m_TreeItems, m_FileName, m_Model, m_RootItem );
    connect( m_XmlParser, &DXmlParser::nodesReady, m_Window, &DInsightMainWindow::loadXmlProgress );
    connect( m_XmlParser, &DXmlParser::finished, this, &DImport::loadXmlFinished );

    // Start parsing attachments
    assert( m_AttachmentParser == nullptr );
    m_AttachmentParser = new DAttachmentParser( &m_TreeItems, m_FileNameDir, m_DocumentTypeRegExp );
    connect( m_XmlParser, &DXmlParser::nodesReady, m_AttachmentParser, &DAttachmentParser::nodesReady );
    connect( m_AttachmentParser, &DAttachmentParser::finished, this, &DImport::attachmentParserFinished);

    m_XmlFinished = false;
    m_AttachmentFinished = false;

    m_XmlParser->start();
    m_AttachmentParser->start();
}


//----------------------------------------------------------------------------
/*! 
 *  Unload of XML. This will remove all children from root node.
 */

void DImport::unload()
{
    // Cleanup connections
    disconnect();

    // Connect importer to main window
    connect( this, &DImport::imported, m_Window, &DInsightMainWindow::unloadFinished );

    m_ImportState = IMPORT_STATE_CANCELING;

    // Abort import
    if ( m_XmlParser )
    {    
        m_XmlParser->requestInterruption();
        m_XmlParser->quit();
    }

    if ( m_AttachmentParser )
    {
        m_AttachmentParser->requestInterruption();
        m_AttachmentParser->quit();
    }

    importFinished();
}


//----------------------------------------------------------------------------
/*! 
 *  Index all attachment found for this import.
 */

void DImport::index()
{
    // Cleanup connections
    disconnect();

    m_ImportState = IMPORT_STATE_INDEXING;

    if ( m_AttachmentIndexer )
    {
        m_AttachmentIndexer->wait();
        delete m_AttachmentIndexer;
        m_AttachmentIndexer = nullptr;
    }

    assert( m_AttachmentIndexer == nullptr );
    m_AttachmentIndexer = new DAttachmentIndexer(
        m_FileNameDir, 
        m_AttachmentParser, 
        m_ReportsDir,  
        attachmentsDir(),
        m_FileName );
    connect( m_AttachmentIndexer, &DAttachmentIndexer::finished, this, &DImport::indexingFinished);
    connect( m_AttachmentIndexer, &DAttachmentIndexer::progress, m_Window, &DInsightMainWindow::indexingProgress);
    connect( m_AttachmentIndexer, &DAttachmentIndexer::indexerStarted, this, &DImport::indexingIndexerStarted);
    connect( this, &DImport::indexed, m_Window, &DInsightMainWindow::indexingFinished);

    // Launch attachment extractor thread
    m_AttachmentIndexer->start();
}


//----------------------------------------------------------------------------
/*! 
 *  Does the root node have any children?
 */

bool DImport::hasChildren()
{
    if ( m_RootItem == nullptr )
    {
        return false;
    }
    
    return m_RootItem->hasChildren();   
}


//----------------------------------------------------------------------------
/*! 
 *  Loading (parsing) of XML complete  - signal sendt from DXmlParser
 */

void DImport::loadXmlFinished()
{
    m_XmlFinished = true;
    importFinished();
}


//----------------------------------------------------------------------------
/*! 
 *  Analyzing attachments complete - signal sendt from DAttachmentParser
 */

void DImport::attachmentParserFinished()
{
    m_AttachmentFinished = true;
    importFinished();
}


//----------------------------------------------------------------------------
/*! 
 *  Slot called when import is finished.
 */

void DImport::importFinished()
{
    if ( !m_AttachmentFinished || !m_XmlFinished )
    {
        return;
    }

    if ( m_ImportState == IMPORT_STATE_IMPORTING )
    {
        DAttachments& attachments               = m_AttachmentParser->attachments();
        DAttachmentIndexes& attachmentsNotFound = m_AttachmentParser->attachmentsNotFound();

        m_RootItem->updateNode( m_Model->createLeaf( m_RootItem, tr("nodeCount"), QString("%1").arg( m_XmlParser->nodeCount() ) ) );
        m_RootItem->updateNode( m_Model->createLeaf( m_RootItem, tr("attachments"), QString("%1").arg( attachments.size() ) ) );
        m_RootItem->updateNode( m_Model->createLeaf( m_RootItem, tr("attachmentsNotFound"), QString("%1").arg( attachmentsNotFound.size() ) ) );
        m_RootItem->removeNode( tr("importCanceled") );
    }
    else
    {
        delete m_AttachmentParser;
        m_AttachmentParser = nullptr;        

        // Create dummy node for report
        m_RootItem->updateNode( m_Model->createLeaf( m_RootItem, tr("importCanceled"), tr("Import canceled by user") ) );

        // Remove all child nodes
        m_Model->deleteDocumentRoot( m_RootItem, true );

        m_TreeItems.clear();
    }

    // Create import report folder
    QString report = m_ReportsDir;
    QDir reportsDir;
    reportsDir.cd( report );

    m_RootItem->updateNode( m_Model->createLeaf( m_RootItem, reportsDirKey(), QDir::toNativeSeparators(report) ) );

    delete m_XmlParser;
    m_XmlParser = nullptr;

    bool ok = m_ImportState == IMPORT_STATE_IMPORTING;
    m_ImportState = IMPORT_STATE_DONE;
    emit imported( ok );
}


//----------------------------------------------------------------------------
/*! 
 *  Slot called when iindexing is finished.
 */

void DImport::indexingFinished()
{
    emit indexed( m_ImportState == IMPORT_STATE_INDEXING );
    m_ImportState = IMPORT_STATE_DONE;
}

//----------------------------------------------------------------------------
/*! 
 *  Create DImport instance from file.
 */

DImport* DImport::CreateFromFile(DImport* import, const QString& fileName, DTreeModel* model, DInsightMainWindow* window, const DRegExps &documentTypeRegExp )
{
    DInsightConfig::log() << "Importing " << fileName << endl;
    
    import->m_FromReport = false;
    import->m_ImportState = IMPORT_STATE_IMPORTING;
    import->m_XmlFinished = false;
    import->m_AttachmentFinished = false;
    import->m_Window = window;
    import->m_FileName = fileName;
    import->m_FileNameDir = QFileInfo( import->m_FileName ).path();
    import->m_ReportsDir = DInsightReport::getReportsDir();
    import->m_DocumentTypeRegExp = documentTypeRegExp;

    QDir reportsDir;
    if ( !reportsDir.mkpath( import->m_ReportsDir ) )
    {
        QMessageBox::warning( window, tr("Directory creation failed"), tr("Failed to create directory '%1'.").arg(import->m_ReportsDir) );
        delete import;
        return nullptr;
    } 
    
    // Open file
    QFile file( import->m_FileName );
    if ( !file.open(QIODevice::ReadOnly) )
    {
        QMessageBox::information( window, tr("Failed to open file"), tr("Failed to open '%1'.").arg( fileName ), QMessageBox::Ok );
        delete import;
        return nullptr;
    }

    // Create root node
    model->beginInsert( 0, 1, QModelIndex() );
    import->m_RootItem = model->createDocumentRoot( tr("import") );
    model->endInsert();

    // Add a couple of info nodes for the root node
    QDateTime dateTime = QDateTime::currentDateTime().toLocalTime();
    QString importDate = QString( "%1 %2" ).arg( dateTime.date().toString( "dddd dd.MM.yyyy" ) ).arg( dateTime.time().toString() );

    import->m_RootItem->addNode( model->createLeaf( import->m_RootItem, tr("importDate"), importDate ) );
    import->m_RootItem->addNode( model->createLeaf( import->m_RootItem, fileNameKey(), fileName ) );
    import->m_RootItem->addNode( model->createLeaf( import->m_RootItem, tr("sizeBytes"), QString("%1").arg( file.size() ) ) );
    import->m_RootItem->addNode( model->createLeaf( import->m_RootItem, tr("fileDate"), QString("%1").arg( QFileInfo(file).created().toString() ) ) );
    file.close();

    import->load();    
    return import;
}


//----------------------------------------------------------------------------
/*! 
 *  Create DImport instance from XML file.
 */

DImport* DImport::CreateFromXml( const QString& fileName, DTreeModel* model, DInsightMainWindow* window, const DRegExps &documentTypeRegExp )
{
    DInsightConfig::log() << "Importing " << fileName << endl;
    
    DImport* import = new DImport( model );

    return CreateFromFile( import, fileName, model, window, documentTypeRegExp );
}

//----------------------------------------------------------------------------
/*! 
 *  Create DImport instance from tar file.
 */

DImport* DImport::CreateFromTar( const QString& fileName, DTreeModel* model, DInsightMainWindow* window, const DRegExps &documentTypeRegExp )
{
    DInsightConfig::log() << "Importing " << fileName << endl;
    
    DImport* import = new DTarImport( model );

    return CreateFromFile( import, fileName, model, window, documentTypeRegExp );
}


//----------------------------------------------------------------------------
/*! 
 *  Create DImport instance from XML import report.
 */

DImport* DImport::CreateFromReport( const QString& fileName, DTreeModel* model, DInsightMainWindow* window, const DRegExps& documentTypeRegExp )
{
    DInsightConfig::log() << "Importing: " << fileName << endl;
    
    DImport* import = new DImport( model );

    import->m_FromReport = true;
    import->m_ImportState = IMPORT_STATE_IMPORTING;
    import->m_XmlFinished = false;
    import->m_AttachmentFinished = false;
    import->m_Window = window;
    import->m_FileName = fileName;
    import->m_FileNameDir = QFileInfo( import->m_FileName ).path();
    import->m_DocumentTypeRegExp = documentTypeRegExp;
    import->m_RootItem = nullptr;
    import->loadReport();

    DLeafNode* fileNameLeaf = import->m_RootItem->findLeaf( fileNameKey().toStdString().c_str() );
    if ( fileNameLeaf )
    {
        import->m_FileName = QString( fileNameLeaf->m_Value );
        import->m_FileNameDir = QFileInfo( import->m_FileName ).path();
    }
    else
    {
        DInsightConfig::log() << "Import failed: " << fileName << " missing key: " << fileNameKey() << endl;
        delete import;
        return nullptr;
    }

    DLeafNode* reportDirLeaf = import->m_RootItem->findLeaf( reportsDirKey().toStdString().c_str() );
    if ( reportDirLeaf )
    {
        import->m_ReportsDir = QString( reportDirLeaf->m_Value );
    }
    else
    {
        DInsightConfig::log() << "Import failed: " << fileName << " missing key: " << reportsDirKey() << endl;
        delete import;
        return nullptr;
    }

    return import;
}


//----------------------------------------------------------------------------
/*! 
 *  Return import state.
 */

DImport::DImportState DImport::state()
{
    return m_ImportState;
}


//----------------------------------------------------------------------------
/*! 
 *  Get root item.
 */

DTreeRootItem* DImport::root()
{
    return m_RootItem;
}


//----------------------------------------------------------------------------
/*! 
 *  Return XML filename.
 */

QString DImport::fileName()
{
    return m_FileName;
}


//----------------------------------------------------------------------------
/*! 
 *  Return root directory for XML file name.
 */

QString DImport::fileNameRoot()
{
    return QFileInfo( m_FileName ).path();
}


//----------------------------------------------------------------------------
/*! 
 *  Return database name. The database name is used to query the search demon 
 *  when saerching through attachments.
 */

QString DImport::databaseName()
{
    return DAttachmentIndexer::indexName( fileName() );
}


//----------------------------------------------------------------------------
/*! 
 *  Get the reports folder. In the reports folder all data related to the
 *  import is stored.
 */

QString DImport::reportsDir()
{
    return m_ReportsDir;
}


//----------------------------------------------------------------------------
/*! 
 *  Get the folder where the attachments (converted to text) are stored.
 */

QString DImport::attachmentsDir()
{
    return QDir::fromNativeSeparators( m_ReportsDir + tr("attachments") + QDir::separator() );
}


//----------------------------------------------------------------------------
/*! 
 *  Get the path to the search demon config file.
 */

QString DImport::searchConfig()
{
    return reportsDir() + QString( "sphinx_index.conf" );
}


//----------------------------------------------------------------------------
/*! 
 *  Return number of attachments not found.
 */

unsigned int DImport::attachmentsNotFound()
{
    return (unsigned int)m_AttachmentParser->attachmentsNotFound().size();
}


//----------------------------------------------------------------------------
/*! 
 *  Return number of attachments found.
 */

unsigned int DImport::attachmentsFound()
{
    return (unsigned int)m_AttachmentParser->attachmentsFound().size();
}


//----------------------------------------------------------------------------
/*! 
 *  Return number of attachments that failed to convert to text.
 */

unsigned int DImport::attachmentsFailedToConvert()
{
    return m_AttachmentIndexer->attachmentsFailedToConvert().size();
}


//----------------------------------------------------------------------------
/*! 
 *  Return number of attachments that where empty (no text).
 */

unsigned int DImport::attachmentsEmpty()
{
    return m_AttachmentIndexer->attachmentsEmpty().size();
}


//----------------------------------------------------------------------------
/*! 
 *  Get attachment from index.
 */

DAttachment* DImport::getAttachment( int index )
{
    if ( m_AttachmentParser )
    { 
        return m_AttachmentParser->attachments().at( index );
    }

    return nullptr;
}


//----------------------------------------------------------------------------
/*! 
 *  Set if this DImport is created from a report.
 */

void DImport::setFromReport( bool fromReport )
{
    m_FromReport = fromReport;
}


//----------------------------------------------------------------------------
/*! 
 *  Is this DImport instance created from a XML report file?
 */

bool DImport::fromReport()
{
    return m_FromReport;
}


//----------------------------------------------------------------------------
/*! 
 *  Total attachments file size in bytes.
 */

qint64 DImport::attachmentsSizeInBytes()
{
    if ( m_AttachmentParser )
    {
        return m_AttachmentParser->attachmentsSizeInBytes();
    }
    return 0LL;
}


//----------------------------------------------------------------------------
/*! 
 *  Called when indexing thread is started.
 */

void DImport::indexingIndexerStarted()
{
}


//----------------------------------------------------------------------------
/*! 
 *  Return node name for file name.
 */

QString DImport::fileNameKey()
{
    return tr("importFileName");
}


//----------------------------------------------------------------------------
/*! 
 *  Return node name for reports dir.
 */

QString DImport::reportsDirKey()
{
    return tr("reportsFolder");
}
