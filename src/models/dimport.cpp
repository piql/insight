/***************************************************************************
**
**  Implementation of the DImport class
**
**  Creation date:  2017/06/12
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
#include    "dimport.h"
#include    "dxmlparser.h"
#include    "ddirparser.h"
#include    "dattachmentparser.h"
#include    "dattachmentindexer.h"
#include    "dtreemodel.h"
#include    "dinsightreport.h"
#include    "dimportformat.h"

//  QT INCLUDES
//
#include    <QDir>
#include    <QDateTime>
#include    <QMessageBox>
#include    <QtGlobal>
#include    <QProcess>
#include    <QUuid>

//  SYSTEM INCLUDES
//
#include    <cassert>

//  STATICS
//
static const DImportFormat* g_ImportFormat;

/****************************************************************************/
/*! \class DImport dimport.h
 *  \ingroup Insight
 *  \brief Handles import sessions
 * 
 * States
 *   
 *      IMPORT_STATE_DONE
 *               |                    
 *               |
 *               V
 *   IMPORT_STATE_IMPORTING --> IMPORT_STATE_DONE --> IMPORT_STATE_INDEXING --> IMPORT_STATE_DONE
 *               |                                          |
 *               |                                          |
 *               V                                          |
 *   IMPORT_STATE_CANCELING <-------------------------------|
 *               |
 *               |
 *               V
 *      IMPORT_STATE_DONE
 * 
 *       |-----------------------------------------------|
 *       |                                               V
 *       |  |---> UNDEFINED -> IMPORTING_REPORT -> REPORT_IMPORTED
 *       |  |         |                                  |
 *       |  |         V                                  |
 *   CANCELING <-- IMPORTING <---------------------------|
 *                    |
 *                    V
 *                 IMPORTED <------|
 *                    |            |
 *                    V            |
 *                 INDEXING -> CANCELING
 *                    |
 *                    V
 *                  DONE
 */
 
//----------------------------------------------------------------------------
/*! 
 *  Constructor.
 */

DImport::DImport( DTreeModel* model )
  : m_ImportState( IMPORT_STATE_UNDEFINED ),
    m_Model( model ),
    m_RootItem( nullptr ),
    m_XmlParser( nullptr ),
    m_AttachmentParser( nullptr ),
    m_AttachmentIndexer( nullptr ),
    m_Uuid(QUuid::createUuid())
{
}


//----------------------------------------------------------------------------
/*! 
 *  Destructor.
 */

DImport::~DImport()
{
}


//----------------------------------------------------------------------------
/*! 
 *  Loading report XML.
 */

bool DImport::loadReport()
{
    assert( m_XmlParser == nullptr );
    assert(m_ImportState == IMPORT_STATE_UNDEFINED);
    m_FromReport = true;
    m_ImportState = IMPORT_STATE_IMPORTING_REPORT;
    m_XmlFinished = false;
    m_AttachmentFinished = false;
    m_FileNameDir = QFileInfo( m_FileName ).path();
    m_RootItem = nullptr;


    m_XmlParser = new DXmlParser( &m_TreeItems, m_FileName, m_Model, m_RootItem, GetReportFormat() );
    m_XmlParser->start();
    m_XmlParser->wait();

    bool loadOk = m_XmlParser->loadedOK();
    if ( loadOk )
    {
        m_RootItem = m_XmlParser->root();
        m_ImportState = IMPORT_STATE_REPORT_IMPORTED;
    }
    else 
    {
        m_ImportState = IMPORT_STATE_UNDEFINED;
    }

    delete m_XmlParser;
    m_XmlParser = nullptr;

    return loadOk;
}


//----------------------------------------------------------------------------
/*! 
 *  Start loading (parsing) of file.
 */

void DImport::load( const DImportFormat* format )
{
    assert( m_XmlParser == nullptr );

    DXmlParser* parser = nullptr;
    m_RootItem->setImportFormat( format );
    if ( format->parser() == "xml" )
    {
        parser  = new DXmlParser( &m_TreeItems, m_FileName, m_Model, m_RootItem, format );
    }
    else if ( format->parser() == "dir" )
    {
        QString importRoot = m_ExtractDir;
        parser  = new DDirParser( &m_TreeItems, importRoot, m_Model, m_RootItem, format );
    }
    else if (format->parser() == "Random")
    {
        QString importRoot = m_ExtractDir;
        parser = new DXmlParser(&m_TreeItems, "", m_Model, m_RootItem, format);
    }
    else
    {
        assert(0);
    }

    
    load( parser );
}


//----------------------------------------------------------------------------
/*! 
 *  Start loading (parsing) of file.
 */

void DImport::load( DXmlParser* parser )
{
    m_ImportState = IMPORT_STATE_IMPORTING;

    // Setup parsing
    assert( m_XmlParser == nullptr );
    m_XmlParser = parser;
    connect( m_XmlParser, &DXmlParser::nodesReady, this, &DImport::nodesReadySlot );
    connect( m_XmlParser, &DXmlParser::finished, this, &DImport::loadXmlFinished );

    // Setup parsing attachments
    QString attachmentRootDir = m_FileNameDir;
    if ( parser->importFormat()->parser() == "dir" )
    {
        attachmentRootDir = m_ExtractDir;
    }

    assert( m_AttachmentParser == nullptr );
    m_AttachmentParser = new DAttachmentParser( &m_TreeItems, attachmentRootDir, m_DocumentTypeRegExp, m_JournalMatchers );
    connect( m_XmlParser, &DXmlParser::nodesReady, m_AttachmentParser, &DAttachmentParser::nodesReady );
    connect( m_AttachmentParser, &DAttachmentParser::finished, this, &DImport::attachmentParserFinished);

    m_XmlFinished = false;
    m_AttachmentFinished = false;

    // Start parsing
    m_XmlParser->start();
    m_AttachmentParser->start();
}


//----------------------------------------------------------------------------
/*! 
 *  Unload of XML. This will remove all children from root node.
 */

void DImport::unload()
{
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
 *  Index all attachments found for this import.
 */

void DImport::index()
{
    m_ImportState = IMPORT_STATE_INDEXING;

    if ( m_AttachmentIndexer )
    {
        m_AttachmentIndexer->wait();
        delete m_AttachmentIndexer;
        m_AttachmentIndexer = nullptr;
    }

    QString attachmentRootDir = m_FileNameDir;
    if ( m_ExtractDir.length() )
    {
        attachmentRootDir = m_ExtractDir;
    }

    assert( m_AttachmentIndexer == nullptr );
    m_AttachmentIndexer = new DAttachmentIndexer(
        attachmentRootDir,
        m_AttachmentParser, 
        m_ReportsDir,  
        attachmentsDir(),
        m_FileName );
    connect( m_AttachmentIndexer, &DAttachmentIndexer::finished, this, &DImport::indexingFinished );
    connect( m_AttachmentIndexer, &DAttachmentIndexer::progress, this, &DImport::indexingProgress );
    connect( m_AttachmentIndexer, &DAttachmentIndexer::indexerStarted, this, &DImport::indexingIndexerStarted );

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
 *  Progress - signal sendt from DXmlParser
 */

void DImport::nodesReadySlot(unsigned long count, float progress)
{
    emit nodesReady(count, progress);
}


//----------------------------------------------------------------------------
/*!
 *  Progress - signal sendt from DAttachmentParser
 */

void DImport::indexingProgressSlot(float progress)
{
    emit indexingProgress(progress);
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

    if (m_XmlParser)
    {
        m_PendingImports = m_XmlParser->pendingImports();
    }
    delete m_XmlParser;
    m_XmlParser = nullptr;

    bool ok = m_ImportState == IMPORT_STATE_IMPORTING;
    m_ImportState = IMPORT_STATE_DONE;
    emit imported( ok );
}


//----------------------------------------------------------------------------
/*! 
 *  Slot called when indexing is finished.
 */

void DImport::indexingFinished()
{
    if ( m_AttachmentIndexer && m_AttachmentIndexer->error() )
    {
        emit indexed( DImport::INDEXING_STATE_ERROR );
    }
    else if ( m_ImportState == IMPORT_STATE_INDEXING )
    {
        emit indexed( DImport::INDEXING_STATE_OK );
    }
    else
    {
        emit indexed( DImport::INDEXING_STATE_CANCELED );
    }
    m_ImportState = IMPORT_STATE_DONE;
}


//----------------------------------------------------------------------------
/*! 
 *  Create DImport instance from file.
 */

DImport* DImport::CreateFromFile(
    const QString& fileName,
    DTreeModel* model,
    QWidget* window,
    const DImportFormat* format,
    DTreeItem* parent,
    DImport* parentImport )
{
    DInsightConfig::Log() << "Importing " << fileName << " parser=" << format->parser() << " has parent=" << (parent != nullptr) << " format=" << format->name() << Qt::endl;
    
    DImport* import = new DImport( model );
    import->m_FromReport = false;
    import->m_ImportState = IMPORT_STATE_IMPORTING;
    import->m_XmlFinished = false;
    import->m_AttachmentFinished = false;
    import->m_FileName = fileName;
    import->m_FileNameDir = QFileInfo( import->m_FileName ).path();
    import->m_DocumentTypeRegExp = format->documentTypeRegExp();
    import->m_ImportFormat = format->name();
    import->m_JournalMatchers = format->journalMatchers();

    if ( parentImport )
    {
        import->m_ReportsDir = DInsightReport::getReportsDir( parentImport->reportsDir() + tr("childimports") + "/" );
    }
    else
    {
        import->m_ReportsDir = DInsightReport::getReportsDir();
    }

    QDir reportsDir;
    if ( !reportsDir.mkpath( import->m_ReportsDir ) )
    {
        QMessageBox::warning( window, tr("Directory creation failed"), tr("Failed to create directory '%1'.").arg(import->m_ReportsDir) );
        delete import;
        return nullptr;
    } 
    
    // Open file
    QFile file( import->m_FileName );
    if ( format->parser() != "Random")
    {
        if ( !file.open(QIODevice::ReadOnly) )
        {
            QMessageBox::information( window, tr("Failed to open file"), tr("Failed to open '%1'.").arg( fileName ), QMessageBox::Ok );
            delete import;
            return nullptr;
        }
    }

    // Create root node
    if ( parent == nullptr )
    {
        model->beginInsert( 0, 1, QModelIndex() );
        import->m_RootItem = model->createDocumentRoot( tr("import"), nullptr, format );
        model->endInsert();
    }
    else
    {
        model->beginInsert( 0, 1, model->index( parent ) );
        import->m_RootItem = model->createDocumentRoot( tr("import"), parent, format );
        model->endInsert();
    }

    // Add a couple of info nodes for the root node
    QDateTime dateTime = QDateTime::currentDateTime().toLocalTime();
    QString importDate = QString( "%1 %2" ).arg( dateTime.date().toString( "dddd dd.MM.yyyy" ) ).arg( dateTime.time().toString() );

    import->m_RootItem->addNode( model->createLeaf( import->m_RootItem, tr("importDate"), importDate ) );
    import->m_RootItem->addNode( model->createLeaf( import->m_RootItem, FileNameKey(), fileName ) );
    import->m_RootItem->addNode( model->createLeaf( import->m_RootItem, tr("sizeBytes"), QString("%1").arg( file.size() ) ) );
   
#if QT_VERSION >= 0x051000 // birthTime()
    import->m_RootItem->addNode( model->createLeaf( import->m_RootItem, tr("fileDate"), QString("%1").arg( QFileInfo(file).birthTime().toString() ) ) );
#else
    import->m_RootItem->addNode( model->createLeaf( import->m_RootItem, tr("fileDate"), QString("%1").arg( QFileInfo(file).lastModified().toString() ) ) );
#endif
    import->m_RootItem->addNode( model->createLeaf( import->m_RootItem, tr("importFormat"), format->name() ) );
    import->m_RootItem->addNode(model->createLeaf(import->m_RootItem, "uuid", import->m_Uuid.toString()));
    file.close();

    // Create import report folder
    QString report = import->m_ReportsDir;
    reportsDir.cd( report );

    import->m_RootItem->updateNode( model->createLeaf( import->m_RootItem, ReportsDirKey(), QDir::toNativeSeparators(report) ) );

    DXmlParser* parser;
    if ( format->parser() == "xml" )
    {
        parser = new DXmlParser( &import->m_TreeItems, import->m_FileName, import->m_Model, import->m_RootItem, format );
    }
    else if ( format->parser() == "dir" )
    {
        QString extractDir = tr("extract");
        QDir rootExtractDir( import->m_ReportsDir );
        if ( !rootExtractDir.mkdir( extractDir ) )
        {
            QMessageBox::information(
                window, tr("Failed to create directory"), tr("Failed to create '%1/%2'.").arg( rootExtractDir.path() ).arg( extractDir ), QMessageBox::Ok );
            delete import;
            return nullptr;
        }

        // Extract to import folder
        QString extractPath = QDir::toNativeSeparators( QString("%1/%2").arg( rootExtractDir.path() ).arg( extractDir ) );
        QString extractTool = format->extractTool( import->m_FileName, extractPath );

        int ok = QProcess::execute(extractTool);
        if ( ok != 0 )
        {
            QMessageBox::information(
                window, tr("Failed to extract"), tr("Failed to extract '%1'.").arg( extractTool ), QMessageBox::Ok );
            delete import;
            return nullptr;
        }

        // Store extract dir in import file
        import->m_ExtractDir = extractPath;
        import->m_RootItem->addNode( model->createLeaf( import->m_RootItem, tr("extractFolder"), extractPath ) );

        // Parse temp dir
        parser = new DDirParser( &import->m_TreeItems, extractPath, import->m_Model, import->m_RootItem, format );
    }
    else if ( format->parser() == "Random" )
    {
        parser = new DXmlParser( &import->m_TreeItems, "", import->m_Model, import->m_RootItem, format );
    }
    else
    {
        DInsightConfig::Log() << "Unknown parser: " << format->parser() << Qt::endl;
        delete import;
        return nullptr;
    }
    
    import->load(parser);    
    return import;
}


//----------------------------------------------------------------------------
/*! 
 *  Create DImport instance from XML file.
 */

DImport* DImport::CreateFromXml(
    const QString& fileName,
    DTreeModel* model,
    QWidget* window,
    const DImportFormat* format,
    DTreeItem* parent,
    DImport* parentImport )
{
    return CreateFromFile( fileName, model, window, format, parent, parentImport );
}


//----------------------------------------------------------------------------
/*! 
 *  Create DImport instance from tar file.
 */

DImport* DImport::CreateFromTar(
    const QString& fileName,
    DTreeModel* model,
    QWidget* window,
    const DImportFormat* format,
    DTreeItem* parent,
    DImport* parentImport )
{
    return CreateFromFile( fileName, model, window, format, parent, parentImport );
}

//----------------------------------------------------------------------------
/*!
 *  Create DImport instance from tar file.
 */

DImport* DImport::CreateFromExtract(
    const QString& fileName,
    DTreeModel* model,
    QWidget* window,
    const DImportFormat* format,
    DTreeItem* parent,
    DImport* parentImport )
{
    return CreateFromFile( fileName, model, window, format, parent, parentImport );
}


//----------------------------------------------------------------------------
/*! 
 *  Create DImport instance from XML import report.
 * 
 * A report is the root node of the import session, and it contains import 
 * metadata. 
 */

DImport* DImport::CreateFromReport( const QString& fileName, DTreeModel* model, const DImportFormats* formats )
{
    DInsightConfig::Log() << "Importing: " << fileName << Qt::endl;
    
    DImport* import = new DImport( model );
    import->m_FileName = fileName;
    if ( !import->loadReport() )
    {
        DInsightConfig::Log() << "Import failed: " << fileName << " XML parsing failed." << Qt::endl;
        delete import;
        return nullptr;
    }

    // Find format in report:
    DLeafNode* fileFormatNode = import->m_RootItem->findLeaf( "importFormat" );
    const DImportFormat* format = formats->defaultFormat();
    if ( fileFormatNode )
    {
        import->m_ImportFormat = fileFormatNode->m_Value;
        format = formats->find( import->m_ImportFormat );
        if ( format == nullptr )
        {
            DInsightConfig::Log() << "Warning: Format not found: " << fileFormatNode->m_Value << Qt::endl;
            format = formats->defaultFormat();
        }
        else
        {
            DInsightConfig::Log() << "Using format: " << fileFormatNode->m_Value << Qt::endl;
        }
    }
    
    import->m_DocumentTypeRegExp = format->documentTypeRegExp();
    
    DLeafNode* fileNameLeaf = import->m_RootItem->findLeaf( FileNameKey().toStdString().c_str() );
    if ( fileNameLeaf )
    {
        import->m_FileName = QString( fileNameLeaf->m_Value );
        import->m_FileNameDir = QFileInfo( import->m_FileName ).path();
    }
    else
    {
        DInsightConfig::Log() << "Import failed: " << fileName << " missing key: " << FileNameKey() << Qt::endl;
        delete import;
        return nullptr;
    }

    DLeafNode* reportsDirLeaf = import->m_RootItem->findLeaf( ReportsDirKey().toStdString().c_str() );
    if ( reportsDirLeaf )
    {
        import->m_ReportsDir = QString( reportsDirLeaf->m_Value );
    }
    else
    {
        DInsightConfig::Log() << "Import failed: " << fileName << " missing key: " << ReportsDirKey() << Qt::endl;
        delete import;
        return nullptr;
    }

    DLeafNode* extractDirLeaf = import->m_RootItem->findLeaf( ExtractDirKey().toStdString().c_str() );
    if ( extractDirLeaf )
    {
        import->m_ExtractDir = QString( extractDirLeaf->m_Value );
    }
    else
    {
        // Not all formats have an extract dir
    }

    DLeafNode* uuidLeaf = import->m_RootItem->findLeaf("uuid");
    if (uuidLeaf)
    {
        import->m_Uuid = QUuid(uuidLeaf->m_Value);
    }

    return import;
}


//----------------------------------------------------------------------------
/*! 
 *  Return import state.
 */

DImport::DImportState DImport::state() const
{
    return m_ImportState;
}


//----------------------------------------------------------------------------
/*! 
 *  Get root item.
 */

DTreeRootItem* DImport::root() const
{
    return m_RootItem;
}


//----------------------------------------------------------------------------
/*! 
 *  Return XML filename.
 */

QString DImport::fileName() const
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
 *  Return import format name, as found in the format config file.
 */

QString DImport::formatName() const
{
    return m_ImportFormat;
}


//----------------------------------------------------------------------------
/*! 
 *  Return database name. The database name is used to query the search demon 
 *  when searching through attachments.
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
 *
 */

QString DImport::extractDir()
{
    return m_ExtractDir;
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
    if (m_AttachmentParser)
    {
        return m_AttachmentParser->attachmentsSizeInBytes();
    }
    return 0LL;
}


const DPendingImports& DImport::pendingImports()
{
    return m_PendingImports;
}


const QUuid& DImport::uuid() const
{
    return m_Uuid;
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

QString DImport::FileNameKey()
{
    return tr("importFileName");
}


//----------------------------------------------------------------------------
/*! 
 *  Return node name for reports dir.
 */

QString DImport::ReportsDirKey()
{
    return tr("reportsFolder");
}


//----------------------------------------------------------------------------
/*!
 *  Return node name for extract dir. This is where import formats that are
 *  containers (TAR, ZIP, etc) are extracted during import.
 */

QString DImport::ExtractDirKey()
{
    return tr("extractFolder");
}


void DImport::SetReportFormat( const DImportFormat* importFormat )
{
    g_ImportFormat = importFormat;
}


const DImportFormat* DImport::GetReportFormat()
{
    return g_ImportFormat;
}


DImport* GetImport(const DImports& imports, const QString& uuid)
{
    DImportsConstIterator it = imports.cbegin();
    DImportsConstIterator itEnd = imports.cend();

    QUuid findMe(uuid);
    for (; it != itEnd; it++)
    {
        if ((*it)->uuid() == findMe) 
        {
            return *it;
        }
    }
    return nullptr;
}
