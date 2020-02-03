/***************************************************************************
**
**  Implementation of the DInsightMainWindow class
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
#include    "dinsightmainwindow.h"
#include    "dinsightreportwindow.h"
#include    "dinsightconfig.h"
#include    "dinsightsearchexpression.h"
#include    "dtreeitem.h"
#include    "dtreemodel.h"
#include    "dxmlparser.h"
#include    "dattachmentparser.h"
#include    "dattachmentindexer.h"
#include    "dsearchthread.h"
#include    "dwaitcursor.h"
#include    "qpersistantfiledialog.h"
#include    "qaboutdialog.h"
#include    "dimportformat.h"

//  QT INCLUDES
//
#include    <QFileDialog>
#include    <QMessageBox>
#include    <QDir>
#include    <QXmlStreamReader>
#include    <QLabel>
#include    <QDesktopServices>
#include    <QDateTime>
#include    <QMenu>
#include    <QProcess>
#include    <QStatusBar>
#include    <QProgressBar>
#include    <QPainter>
#include    <QSqlDatabase>
#include    <QDirIterator>
#include    <QTemporaryFile>
#include    <QSqlError>
#include    <QSpacerItem>
#include    <QCryptographicHash>
#include    <QPixmap>

//#define ENABLE_MODELTEST
#if defined ( ENABLE_MODELTEST )
#include    <modeltest.h>
#endif

//  SYSTEM INCLUDES
//
#include    <cassert>

//  PLATFORM INCLUDES
//
#if defined WIN32
#include    <Windows.h>
#endif

#if defined WIN32
extern HANDLE ghJob;
#endif


static void clearLayout( QLayout *layout );

Q_DECLARE_METATYPE( const char * );

/****************************************************************************/
/*! \class DTableSearchResultCell dinsightmainwindow.h
 *  \ingroup Insight
 *  \brief Search result table cell.
 * 
 */

//----------------------------------------------------------------------------
/*! 
 *  Constructor.
 */

DTableSearchResultCell::DTableSearchResultCell( const QString& text )
    : QLabel( text )
{
    
}


//----------------------------------------------------------------------------
/*! 
 *  Double click event on cell.
 */

void DTableSearchResultCell::mouseDoubleClickEvent(QMouseEvent * /*event*/)
{
    emit doubleClicked( this );
}


/****************************************************************************/
/*! \class DInsightMainWindow dinsightmainwindow.h
 *  \ingroup Insight
 *  \brief Main Window dialog
 * 
 *  The main window has a tree widget showing the XML parent nodes on the left
 *  side of the dialog. On the right side it has a detailed info view.
 *  
 */
 
//===================================
//  P U B L I C   I N T E R F A C E
//===================================

//----------------------------------------------------------------------------
/*! 
 *  Constructor.
 */

DInsightMainWindow::DInsightMainWindow( DImportFormats* formats)
  : m_CurrentImport( nullptr ),
    m_Model( nullptr ),
    m_SearchThread( nullptr ),
    m_SearchDeamonProcess( nullptr ),
    m_ImportFormats( formats )
{
    m_Ui.setupUi( this );

    // Set banner logo
    QString bannerFile = DInsightConfig::Get("WINDOW_BANNER","");
    if ( !bannerFile.isEmpty() )
    {
        m_Ui.bannerLabel->setPixmap( QPixmap( bannerFile ) );
    }

    // Signals and slots. The GUI components emits signals that are handled by the slots.
    qRegisterMetaType<QVector<int> >("QVector<int>");
    QObject::connect( m_Ui.aboutButton, SIGNAL(clicked()), this, SLOT(aboutButtonClicked()) );
    QObject::connect( m_Ui.importButton, SIGNAL(clicked()), this, SLOT(importButtonClicked()) );
    QObject::connect( m_Ui.exportButton, SIGNAL(clicked()), this, SLOT(exportButtonClicked()) );
    QObject::connect( m_Ui.selectButton, SIGNAL(clicked()), this, SLOT(selectButtonClicked()) );
    QObject::connect( m_Ui.searchEdit, SIGNAL(textChanged(const QString &)), this, SLOT(searchEditChanged(const QString &)));
    
    // Tree view and mode setup
    m_Model = new DTreeModel( m_ImportFormats );
    QObject::connect( m_Model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &, const QVector<int> &)), this, SLOT(dataChanged(const QModelIndex &, const QModelIndex &, const QVector<int> &)));

#if defined ENABLE_MODELTEST
    new ModelTest(m_Model, this);
#endif

    m_Ui.treeView->setModel( m_Model );
    QObject::connect( m_Ui.treeView, SIGNAL(activated(const QModelIndex &)), this, SLOT(activatedTreeItem(const QModelIndex &)));
    QObject::connect( m_Ui.treeView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(activatedTreeItem(const QModelIndex &)));
    QObject::connect( m_Ui.treeView, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(contextMenuTreeItem(const QPoint&)));

    // Search result setup
    m_SearchResultMax = DInsightConfig::GetInt( "SEARCH_RESULT_MAX_COUNT", 100 );
    m_Ui.searchResult->setSortingEnabled( false );
    m_Ui.searchResult->setColumnCount( 2 );
    m_Ui.searchResult->setHorizontalHeaderItem( 0, new QTableWidgetItem( tr("Location") ) );
    m_Ui.searchResult->setHorizontalHeaderItem( 1, new QTableWidgetItem( tr("Search Result") ) );
    m_Ui.searchResult->horizontalHeader()->setDefaultAlignment( Qt::AlignLeft );
    m_Ui.searchResult->horizontalHeader()->setStretchLastSection( true );
    m_Ui.searchResult->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    m_Ui.searchResult->verticalHeader()->hide();    
    m_Ui.searchResult->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_Ui.searchResult->setItemDelegateForColumn( 1, new DTableSearchResultDelegate() );
    m_Ui.searchResult->setItemDelegateForColumn( 0, new DTableSearchResultDelegate() );
    QObject::connect( m_Ui.searchResult, SIGNAL(itemDoubleClicked(QTableWidgetItem *)), this, SLOT(searchResultDoubleClicked(QTableWidgetItem *)));
    QObject::connect( m_Ui.searchResult->horizontalHeader(), &QHeaderView::sectionClicked, this, &DInsightMainWindow::searchResultHeaderClicked);
    m_Ui.searchNavigationFrame->setVisible( false );
    m_Ui.searchResultLabel->setVisible( false ); // Not used yet    
    QObject::connect( m_Ui.previousSearchPageButton, SIGNAL(clicked()), this, SLOT(prevSearchResultPageClicked()));
    QObject::connect( m_Ui.nextSearchPageButton, SIGNAL(clicked()), this, SLOT(nextSearchResultPageClicked()));

    // Search controls
    m_Ui.searchEdit->setEnabled( false );
    m_Ui.searchAttachmentsCheckBox->hide();
    QObject::connect( m_Ui.searchAttachmentsCheckBox, &QCheckBox::clicked, this, &DInsightMainWindow::searchAttachmentClicked );
    m_Ui.selectButton->setEnabled( false );
    m_Ui.selectButton->setText( tr("Select") );
    QObject::connect( m_Ui.searchOptionsButton, &QPushButton::clicked, this, &DInsightMainWindow::searchOptionsToggled );
    QObject::connect( m_Ui.caseSensitiveCheckBox , &QCheckBox::toggled, this, &DInsightMainWindow::treeNodesSearchFilterChanged );
    m_Ui.searchOptionsGroupBox->setVisible( false );
    m_Ui.searchOptionsButton->setText( "+" );

    // Info view layout
    //m_Ui.infoView->setRowWrapPolicy( QFormLayout::WrapLongRows );
    m_Ui.infoView->setFieldGrowthPolicy( QFormLayout::ExpandingFieldsGrow );
    m_Ui.infoView->setFormAlignment( Qt::AlignHCenter | Qt::AlignTop );
    m_Ui.infoView->setLabelAlignment( Qt::AlignRight );
    m_Ui.infoView->setVerticalSpacing(3);

    // Tree view layout
    m_Ui.treeView->setContextMenuPolicy( Qt::CustomContextMenu );
    m_Ui.treeView->setSortingEnabled( false );
    m_Ui.treeView->setUniformRowHeights( true );

    // Status bar
    m_StatusBar = statusBar();
    QWidget* frame = new QWidget();
    frame->show();
    QHBoxLayout* layout = new QHBoxLayout( frame );
    layout->setContentsMargins(0,0,0,0);
    m_ProgressBar = new QProgressBar(frame);
    m_ProgressBar->setMaximumSize( 100, 15 );
    m_ProgressBar->setVisible( false );
    m_ProgressBar->setContentsMargins(0,0,0,0);
    m_ProgressBarInfo = new QLabel();    
    m_ProgressBarInfo->setContentsMargins(0,0,0,0);
    layout->addWidget( m_ProgressBar );
    layout->addWidget( m_ProgressBarInfo );
    layout->addStretch(100);
    m_StatusBar->addWidget( frame );
    m_StatusBar->show();

    m_Ui.exportButton->setEnabled( false );
    m_Ui.searchResult->setVisible( false );

    // Enumerate all project files
    enumerateProjects( DInsightReport::getReportsRootDir() );

    // Testing
    QString autostartFile = DInsightConfig::Get( "STARTUP_LOAD_FILE", "" );
    if ( autostartFile.length() )
    {
        importFile( autostartFile, DInsightConfig::Get( "STARTUP_LOAD_FILE_FORMAT", "" ) );
    }
    else
    {
        //updateInfo( m_Model->firstDocumentRoot() );
        startSearchDeamon();
    }
}


//----------------------------------------------------------------------------
/*! 
 *  Destructor.
 */

DInsightMainWindow::~DInsightMainWindow()
{
    cancelSearch();
    cancelSearchDeamon();

    delete m_Model;
}


//----------------------------------------------------------------------------
/*! 
 *  About button pressed
 */

void DInsightMainWindow::aboutButtonClicked()
{
    QString infoText;
    infoText += "Version: " + QCoreApplication::applicationVersion() + "<br />";
    infoText += "Copyright Piql 2018";
    //infoText += "<p>Support: <a href=\"mailto:support@piql.com\">support@piql.com</a></p>";
    infoText += "<p><a href=\"http://www.piql.com\">www.piql.com</a></p>";

    QAboutDialog about( qApp->applicationName(), infoText, QString(), this );
    about.exec();
}

//----------------------------------------------------------------------------
/*! 
 *  Import AIP button pressed
 */

void DInsightMainWindow::importButtonClicked()
{
    if ( m_CurrentImport == nullptr || m_CurrentImport->state() == DImport::IMPORT_STATE_DONE )
    {
        QString filters;
        //tr("NOARK-5 (arkivstruktur.xml);;NOARK-5 (arkivuttrekk.xml);;AIPs (*.xml)")
        foreach (const DImportFormat& format, *m_ImportFormats )
        {
            foreach( const DRegExp& pattern, format.patterns() )
            {
                if ( filters.length() )
                {
                    filters += ";;";
                }
                filters += QString("%1 (%2)").arg(format.name()).arg(pattern.pattern());
            }
        }
        
        QString selectedFilter;
        QString fileName = QPersistantFileDialog::getOpenFileName( "import", this, tr("Select files to import"), ".", filters, &selectedFilter );
        if ( !fileName.length() )
        {
            return;
        }

        // Get selected format
        QString importFormatName;
        foreach (const DImportFormat& format, *m_ImportFormats )
        {
            if (selectedFilter.startsWith( format.name() + " ("))
            {
                importFormatName = format.name();
                break;
            }
        }

        importFile( fileName, importFormatName );
    }
    else if ( m_CurrentImport->state() == DImport::IMPORT_STATE_IMPORTING ) 
    {
        cancelImport();
    }
    else if ( m_CurrentImport->state() == DImport::IMPORT_STATE_INDEXING ) 
    {
        cancelIndexer();
    }
}


//----------------------------------------------------------------------------
/*! 
 *  Load all project files found under root folder.
 */

void DInsightMainWindow::enumerateProjects( const QString& rootDir )
{
    QStringList nameFilters;
    nameFilters << DInsightReport::getXmlReportName();
    QDirIterator dirIt( rootDir, nameFilters, QDir::NoFilter, QDirIterator::Subdirectories );

    while ( dirIt.hasNext() )    
    {
        QString fileName = dirIt.next();
        DImport* import = DImport::CreateFromReport( fileName, m_Model, this, m_ImportFormats );
        if ( import ) 
        {
            m_Imports.push_back( import );
        }
        else
        {
            DInsightConfig::Log() << "Import failed: " << fileName << endl;
        }
    }

    if ( m_Imports.size() )
    {
        updateInfo( m_Imports.at(0)->root() );
        m_Ui.treeView->setCurrentIndex( m_Model->index( m_Imports.at(0)->root() ) );
    }
}


//----------------------------------------------------------------------------
/*! 
 *  Import AIP
 */

void DInsightMainWindow::importFile(
        const QString& fileNameRelative,
        const QString& importFormatName,
        DTreeItem* parent )
{
    // Cancel search
    cancelSearch();
    cancelSearchDeamon();

    // Convert to full path
    QFileInfo info( fileNameRelative );
    if ( !info.exists() )
    {
        DInsightConfig::Log() << "Importing failed, file does not exist: " << fileNameRelative << endl;
        QMessageBox::information( this, tr("Failed to open file"), tr("Failed to open '%1'.").arg( fileNameRelative ), QMessageBox::Ok );
        return;
    }
    
    QString fileName = info.absoluteFilePath();
    DInsightConfig::Log() << "Importing file: " << fileName << endl;

    // First check that file is not loaded already
    DImportsIterator it = m_Imports.begin();
    DImportsIterator itEnd = m_Imports.end();
    while ( it != itEnd )
    {
        if ( (*it)->fileName() == fileName )
        {
            if ( (*it)->hasChildren() )
            {
                QMessageBox::information( this, tr("Already loaded"), tr("Project '%1' is already loaded. Please unload first if you want to re-load it.").arg( fileName ) );
                return;
            }
            else
            {
                DInsightConfig::Log() << "Already loaded, reloading" << endl;
                m_CurrentImport = *it;
                (*it)->load( m_ImportFormats->find( (*it)->formatName() ) );
                return;
            }
        }

        it++;
    }    

    // Determine what type of import - could replace with factory pattern
    DImport* parentImport = nullptr;
    if ( parent )
    {
        parentImport = findImport( parent->findRootItem() );
    }
    
    DImport* import = nullptr;
    QString suffix = info.suffix();
    const DImportFormat* format = m_ImportFormats->find(importFormatName);
    if ( format->parser() == "xml" )
    {
        import = DImport::CreateFromXml( fileName, m_Model, this, format, parent, parentImport );
    }
    else if ( format->parser() == "tar" )
    {
        import = DImport::CreateFromTar( fileName, m_Model, this, format, parent, parentImport );
    }
    else if ( format->parser() == "dir" )
    {
        import = DImport::CreateFromExtract( fileName, m_Model, this, format, parent, parentImport );
    }
    else
    {
        QMessageBox::information( this, tr("Unknown parser type"), tr("No import parser registered for %1.").arg( format->parser() ) );
        return;
    }
    
    if ( import ) 
    {
        m_Imports.push_back( import );
        m_CurrentImport = import;
        setupUiForImport();
    }
}


//----------------------------------------------------------------------------
/*! 
 *  Setup UI in "importing" state.
 */

void DInsightMainWindow::setupUiForImport()
{
    m_StatusBar->clearMessage();
    m_ProgressBar->setRange( 0, 100 );
    m_ProgressBar->setValue( 0 );
    m_ProgressBar->setFormat( QString( "%p%" ) );
    m_ProgressBar->show();
    m_ProgressBarInfo->setText( tr("read") );
    m_ProgressBarInfo->show();
    m_Ui.importButton->setText( tr("Cancel Import") );
    m_Ui.searchAttachmentsCheckBox->hide();
    m_Ui.searchEdit->setEnabled( false );
    m_Ui.searchOptionsGroupBox->setVisible( false );
    m_Ui.searchOptionsButton->setText( "+" );
    m_Ui.searchOptionsButton->setEnabled( false );
}


//----------------------------------------------------------------------------
/*! 
 *  Cancel import.
 */

void DInsightMainWindow::cancelImport()
{
    if ( m_CurrentImport )
    {
        m_CurrentImport->unload();
        m_Ui.importButton->setText( tr( "Canceling" ) );
        m_Ui.importButton->setDisabled(true);
        //m_Ui.treeView->setModel( nullptr );
        updateInfo( m_Model->firstDocumentRoot() );
    }
}


//----------------------------------------------------------------------------
/*! 
 *  Cancel indexer.
 */

void DInsightMainWindow::cancelIndexer()
{
    if ( m_CurrentImport )
    {
        m_CurrentImport->unload();
        m_Ui.importButton->setText( tr( "Canceling" ) );
        m_Ui.importButton->setDisabled(true);
    }
}


//----------------------------------------------------------------------------
/*! 
 *  Slot called by DImport when XML import is complete 
 */

void DInsightMainWindow::importFileFinished( bool ok )
{
    bool parsingAttachments = false;

    DImport* import = (DImport*)QObject::sender();

    if ( !import->fromReport() )
    {
        if ( ok )
        {
            unsigned int attachmentsNotFound = import->attachmentsNotFound();
            unsigned int attachmentsFound = import->attachmentsFound();
            if ( attachmentsNotFound )
            {
                QMessageBox::warning( this, tr("Attachments not found"), tr("%1 attachments referenced by the AIP where not found during import.").arg( attachmentsNotFound ) );
            }

            bool searchableAttachments = true;
            if ( searchableAttachments && attachmentsFound )
            {
                bool forceAttachmentParsing = DInsightConfig::GetBool( "ATTACHMENT_PARSING_DONT_ASK_USER", false );
                if ( forceAttachmentParsing )
                {
                    parsingAttachments = true;
                }
                else
                {
                    QString sizeString;
                    if ( import->attachmentsSizeInBytes() / (qint64)(1024*1024) )
                    {
                        sizeString = QString( "%1 MB" ).arg( import->attachmentsSizeInBytes() / (qint64)(1024*1024) );
                    }
                    else
                    {
                        sizeString = QString( "%1 KB" ).arg( import->attachmentsSizeInBytes() / (qint64)(1024) );
                    }

                    QString message = 
                        tr( "%1 attachments found (total %2). Should they be made searchable?\n\nPlease note that this operation might take a long time, depending on the amount and size of the attachments.")
                        .arg( attachmentsFound )
                        .arg( sizeString );

                    QMessageBox messageBox(QMessageBox::Question,
                        tr("Searchable attachments?"),
                        message,
                        QMessageBox::Yes | QMessageBox::No,
                        this);

                    // Temp translation to norwegian
                    messageBox.setButtonText(QMessageBox::Yes, tr("Ja"));
                    messageBox.setButtonText(QMessageBox::No, tr("Nei"));
                    parsingAttachments = messageBox.exec() == QMessageBox::Yes;
                }

                if ( parsingAttachments )
                {
                    // Launch attachment extractor thread
                    import->index();

                    m_ProgressBar->setVisible( true );
                    m_ProgressBar->setRange( 0, 100 );
                    m_ProgressBar->setValue( 0 );
                    m_ProgressBar->setFormat( QString( "%p%" ) );
                    m_ProgressBarInfo->setText( tr( "of attachments converted to text" ) );
                    m_ProgressBar->setTextVisible(true);
                    m_StatusBar->showMessage( "" );
                }
            }

            //// Select document root
            //m_Ui.treeView->setCurrentIndex( m_Model->index(0,0) );
            //activatedTreeItem( m_Model->index(0,0) );
        }

        // Create import report filenames. If this is a child import, the reports
        // should be created in the parent import folder.
        QString report = import->reportsDir();
        QDir reportsDir;
        reportsDir.cd( report );

        QString pdfFileName;
        QString xmlFileName;
        if ( import->root()->isToplevelRoot() )
        {
            pdfFileName = tr("import.pdf");
            xmlFileName = DInsightReport::getXmlReportName();
        }
        else
        {
            pdfFileName = tr("child") + "-" + tr("import.pdf");
            xmlFileName = tr("child") + "-" + DInsightReport::getXmlReportName();
        }
        
        // Create import report - PDF
        QStringList attachmentsIgnore;
        DInsightReport importReportPdf;
        createReport( tr("Import Report"), import->root(), importReportPdf, 0, 0, attachmentsIgnore, false );
        importReportPdf.save( report + pdfFileName );

        // Create import report - XML
        DInsightReport importReportXml( DInsightReport::REPORT_FORMAT_XML );
        createReport( "", import->root(), importReportXml, 0, 0, attachmentsIgnore, false, false );
        importReportXml.save( report + xmlFileName );
    }

    QString infoMessage;
    if ( !ok )
    {
        if ( !import->fromReport() )
        {
            infoMessage = tr("Import canceled!");
        }
        else
        {
            infoMessage = tr("Unloaded: %1").arg( import->fileName() );
        }
    }
    else if ( parsingAttachments ) 
    {
        infoMessage = tr("Indexing");
    }
    else
    {
        infoMessage = tr("Import complete!");
    }
    m_Ui.treeView->expandAll();
    int nodeCount = treeNodeCount( false );
    m_Ui.treeView->setItemsExpandable( nodeCount < 200000 ); // Can be really slow, disable on large trees
    int nodeCountSelected = treeNodeCount( true );
    m_Ui.exportButton->setEnabled( nodeCountSelected > 0 );
    m_Ui.searchEdit->setEnabled( m_Imports.size() );
    m_Ui.searchOptionsButton->setEnabled( m_Imports.size() );

    const QObjectList& children = m_Ui.treeNodesSearchFilterGroupBox->children();
    foreach( QObject* child, children )
    {
        delete child;
    }
    DXmlParser::StringHashIterator it = DXmlParser::NodeHashMapBegin();
    DXmlParser::StringHashIterator itEnd = DXmlParser::NodeHashMapEnd();

    QGridLayout* gridLayout = new QGridLayout( m_Ui.treeNodesSearchFilterGroupBox );

    int count = 0;
    while ( it != itEnd )
    {
        QString label = *it;
        ReplaceString( label, m_ImportFormats->defaultFormat()->infoViewLabelRegExp() );
        
        QCheckBox* check = new QCheckBox( label, m_Ui.treeNodesSearchFilterGroupBox );        
        check->setChecked( true );
        QVariant variant;
        variant.setValue<const char*>( *it );
        check->setProperty( "node", variant );
        gridLayout->addWidget( check, count/2, count%2 );
        it++;
        count++;        
        QObject::connect( check, &QCheckBox::clicked, this, &DInsightMainWindow::treeNodesSearchFilterChanged );
    }
    
    m_Ui.treeNodesSearchFilterGroupBox->setVisible( count > 0 );

    if ( !parsingAttachments ) 
    {
        m_ProgressBar->hide();
        m_ProgressBarInfo->hide();
        m_StatusBar->showMessage( infoMessage );
        m_Ui.importButton->setText( tr( "Import" ) );
        m_Ui.importButton->setEnabled( true );

        startSearchDeamon();
    }
    else
    {
        m_Ui.importButton->setText( tr( "Cancel Indexing" ) );
    }

    m_Ui.treeView->setCurrentIndex( m_Model->index( m_CurrentImport->root() ) );
    updateInfo( import->root() );

    if ( !parsingAttachments ) 
    {
        m_CurrentImport = nullptr;
    }
}


//----------------------------------------------------------------------------
/*! 
 *  Clear layout content.
 */

static void clearLayout( QLayout *layout )
{
    if ( !layout->count() )
    {
        return;
    }

    QLayoutItem *item;
    while( layout->count() && (item = layout->takeAt(0))) 
    {
        if (item->widget()) 
        {
            delete item->widget();
        }
        if (item->layout()) 
        {
            clearLayout(item->layout());
            delete item->layout();
        }
    }
}


//----------------------------------------------------------------------------
/*! 
 *  Use regular expression search / repalce to update content of key.
 */

void DInsightMainWindow::ReplaceString( QString& key, const DRegExps& regExps )
{
    const QRegularExpression ueFix( "\\\\u.\\\\e" );    

    for ( int i = 0; i < regExps.length(); i += 2 )
    {
        key.replace( regExps[i], regExps[i+1].pattern() );

        // Manually fix uppercase reg-exp for now, \u \e not yet supported by Qt
        int index = 0;
        while ( ( index = key.indexOf( ueFix, index ) ) != -1 )
        {
            key.replace( index, 5, key[index+2].toUpper() );
        }
    }
}


//----------------------------------------------------------------------------
/*! 
 *  Update info view. Also highlights content based on search string in search
 *  edit control.
 *
 *  If parentNode is nullptr, the view is cleared.
 */

void DInsightMainWindow::updateInfo( Node* parentNode )
{
    // Get search text from edit control, only use it if we are searching the nodes
    QString search;
    if ( !m_Ui.searchAttachmentsCheckBox->isChecked() )
    {
        search = m_Ui.searchEdit->text();
    }
    QWidget* firstMatchingWidget = nullptr;

    clearLayout( m_Ui.infoView );

    if ( !parentNode )
    {
        return;
    }

    DLeafNodes::const_iterator leaf = parentNode->m_Nodes.begin();
    const DImportFormat* format = parentNode->format();
        
    for ( ; leaf != parentNode->m_Nodes.end(); leaf++ )
    {
        int matchPos = (*leaf)->match( search );
        bool matchingLeaf = matchPos != -1;

        QLayout* layout = nullptr;

        QString key = QString( (*leaf)->m_Key );
        QString value = QString( (*leaf)->m_Value );
        if ( isDocumentNode( format, key, value ) ||
             isFolderNode( format, key ) ||
             isDeleteNode( format, key ) || 
             isImportNode( format, key, value ) ||
             isChecksumNode( format, key ))
        {
            QHBoxLayout* hBox = new QHBoxLayout;
            hBox->setSpacing(3);

            QTextEdit* edit = new QTextEdit( (*leaf)->m_Value );
            int height = 20; // m_Ui.searchEdit->height();
            edit->setFixedHeight( height + edit->height() - edit->viewport()->height() + 1 );
            edit->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
            edit->setLineWrapMode(QTextEdit::NoWrap);
            edit->setToolTip((*leaf)->m_Value);
            

            if ( matchingLeaf )
            {
                QTextCursor cursor = edit->textCursor();
                cursor.setPosition( matchPos );
                cursor.movePosition( QTextCursor::Left, QTextCursor::KeepAnchor, (int)search.length() );
                edit->setTextCursor( cursor );
                if ( firstMatchingWidget == nullptr )
                {
                    firstMatchingWidget = edit;
                }
            }
            edit->setReadOnly( true );
            hBox->addWidget( edit );
            if ( isDocumentNode( format, key, value ) || isFolderNode( format, key ) )
            {
                QPushButton* button = new QPushButton( tr( "View" ) );
                button->setProperty( "doc", (*leaf)->m_Value );
                button->setProperty( "node", m_Model->index( parentNode ) );
                //button->setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed ) ); 
                //button->setMinimumSize( button->fontMetrics().size( Qt::TextSingleLine,  button->text() ) + QSize(10,0) );
                hBox->addWidget( button );
                QObject::connect( button, SIGNAL(clicked()), this, SLOT( viewDocumentClicked() ));
            }
            if ( isDeleteNode( format, key ) )
            {
                QPushButton* button = new QPushButton( tr( "Delete" ) );
                button->setProperty( "doc", (*leaf)->m_Value );
                button->setProperty( "node", m_Model->index( parentNode ) );
                //button->setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed ) ); 
                //button->setMinimumSize( button->fontMetrics().size( Qt::TextSingleLine,  button->text() ) + QSize(10,0) );
                hBox->addWidget( button );
                QObject::connect( button, SIGNAL(clicked()), this, SLOT( deleteFolderClicked() ));
            }
            if ( isImportNode( format, key, value ) )
            {
                QPushButton* button = new QPushButton( parentNode->hasChildren() ? tr( "Unload" ) : tr( "Load" ) );
                button->setProperty( "doc", (*leaf)->m_Value );
                button->setProperty( "node", m_Model->index( parentNode ) );
                //button->setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed ) ); 
                //button->setMinimumSize( button->fontMetrics().size( Qt::TextSingleLine,  button->text() ) + QSize(10,0) );
                //button->setFixedSize(button->size());
                hBox->addWidget( button );
                QObject::connect( button, SIGNAL(clicked()), this, SLOT( importDocumentClicked() ));
            }
            if ( isChecksumNode( format, key ) )
            {
                QString sourceFile;
                DLeafNodes::const_iterator l = parentNode->m_Nodes.begin();
                for ( ; l != parentNode->m_Nodes.end(); l++ )
                {
                    QString key = QString( (*l)->m_Key );
                    if ( isChecksumSourceNode( format, key ) )
                    {
                        sourceFile = (*l)->m_Value;
                    }
                }

                if ( sourceFile.length() )
                {
                    QPushButton* button = new QPushButton( tr( "Validate" ) );
                    button->setProperty( "doc", sourceFile );
                    button->setProperty( "checksum", (*leaf)->m_Value );
                    button->setProperty( "node", m_Model->index( parentNode ) );
                    //button->setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed ) ); 
                    //button->setMinimumSize( button->fontMetrics().size( Qt::TextSingleLine,  button->text() ) + QSize(10,0) );
                    hBox->addWidget( button );
                    QObject::connect( button, SIGNAL(clicked()), this, SLOT( checksumClicked() ));
                }
            }
            layout = hBox;
        }

        key = getInfoViewLabel( format, (*leaf)->m_Key );

        if ( layout == nullptr )
        {
            QTextEdit* edit = new QTextEdit( (*leaf)->m_Value );
            if ( matchingLeaf )
            {
                QTextCursor cursor = edit->textCursor();
                cursor.setPosition( matchPos );
                cursor.movePosition( QTextCursor::Right, QTextCursor::KeepAnchor, (int)search.length() );
                edit->setTextCursor( cursor );
                edit->setStyleSheet("selection-color:blue");
                if ( firstMatchingWidget == nullptr )
                {
                    firstMatchingWidget = edit;
                }
            }
            edit->setReadOnly( true );

            //int height = m_Ui.searchEdit->height();
            //edit->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding );
            //edit->setMinimumSize( 50, height + edit->height() - edit->viewport()->height() + 1 );
            int height = 20; // m_Ui.searchEdit->height();
            edit->setFixedHeight( height + edit->height() - edit->viewport()->height() + 1 );
            edit->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
            edit->setLineWrapMode(QTextEdit::NoWrap);
            edit->setToolTip((*leaf)->m_Value);
            m_Ui.infoView->addRow( key, edit );
        }
        else
        {
            m_Ui.infoView->addRow( key, layout );
        }
    }
}


//----------------------------------------------------------------------------
/*! 
 *  Slot called when a tree item is selected.
 */

void DInsightMainWindow::activatedTreeItem( const QModelIndex &index )
{
    if ( index.isValid() ) 
    {
        updateInfo( (DTreeItem*)index.internalPointer() );
    }
}


//----------------------------------------------------------------------------
/*! 
 *  Slot called when a tree item is selected.
 */

void DInsightMainWindow::activatedTreeItem()
{
    activatedTreeItem( m_Ui.treeView->currentIndex() );
}


//----------------------------------------------------------------------------
/*! 
 *  Slot called when right mouse button is clicked within tree view.
 */

void DInsightMainWindow::contextMenuTreeItem( const QPoint& /*pos*/ )
{
    if ( !m_Model )
    {
        return;
    }

    QMenu menu;
    QModelIndex index = m_Ui.treeView->currentIndex();
    if ( index.isValid() )
    {
        DTreeRootItem* root = dynamic_cast<DTreeRootItem*>( (DTreeItem*)index.internalPointer() );
        if ( root )
        {
            DLeafNodes::const_iterator leaf = root->m_Nodes.begin();
            for ( ; leaf != root->m_Nodes.end(); leaf++ )
            {
                QString key = QString( (*leaf)->m_Key );
                QString value = QString( (*leaf)->m_Value );
                if ( isImportNode( root->format(), key, value ) )
                {
                    QAction* load = menu.addAction( root->hasChildren() ? tr( "Unload" ) : tr( "Load" ) );
                    load->setProperty( "doc", (*leaf)->m_Value );
                    load->setProperty( "node", m_Model->index( root ) );
                    QObject::connect( load, SIGNAL(triggered(bool)), this, SLOT( loadMenuClicked(bool) ));
                }
                else if ( isDeleteNode( root->format(), key ) )
                {
                    QAction* load = menu.addAction( tr( "Delete" ) );
                    load->setProperty( "doc", (*leaf)->m_Value );
                    load->setProperty( "node", m_Model->index( root ) );
                    QObject::connect( load, SIGNAL(triggered(bool)), this, SLOT( deleteFolderClicked() ));
                }
            }
        }

        QAction* selectChildren = menu.addAction( tr("Select All Children") );
        QAction* deselectChildren = menu.addAction( tr("Unselect All Children") );
        QObject::connect( selectChildren, SIGNAL(triggered(bool)), this, SLOT(selectChildrenMenuClicked(bool)));
        QObject::connect( deselectChildren, SIGNAL(triggered(bool)), this, SLOT(deselectChildrenMenuClicked(bool)));
    }

    QAction* select = menu.addAction( tr("Select All") );
    QAction* deselect = menu.addAction( tr("Unselect All") );
    QAction* invert = menu.addAction( tr("Invert selection") );

    QObject::connect( select, SIGNAL(triggered(bool)), this, SLOT(selectAllMenuClicked(bool)));
    QObject::connect( deselect, SIGNAL(triggered(bool)), this, SLOT(deselectAllMenuClicked(bool)));
    QObject::connect( invert, SIGNAL(triggered(bool)), this, SLOT(invertSelectionMenuClicked(bool)));

    menu.exec( QCursor::pos() );
}


//----------------------------------------------------------------------------
/*! 
 *  Paths in the nodes can be relative to the documents root, if so, fix it 
 *  here.
 */

void DInsightMainWindow::makeAbsolute( QString& filename, const QModelIndex& index )
{
    if ( !QFile::exists( filename ) )
    {
        DTreeItem* item = (DTreeItem*)index.internalPointer();
        const DTreeRootItem* root = item->findRootItem();
        const DImportFormat* format = root->format();
        DImport* import = findImport( root );

        if ( format->parser() == "dir" )
        {
            if ( format->extractTool("a","b").length() != 0 )
            {
                // The content is relative to the "extract" folder under the import folder
                QDir dir( import->reportsDir() );
                dir.cd(tr("extract"));
                QString path = item->findRootPath();
                dir.cd(path);
                filename = dir.filePath( filename );
            }
            else
            {
                // The content is relative to the import filename root
            }
        }
        else
        {
            QDir dir( import->fileNameRoot() );
            filename = dir.filePath( filename );
        }
    }
}


//----------------------------------------------------------------------------
/*! 
 *  Slot called when view document push button is called for an info element.
 */

void DInsightMainWindow::viewDocumentClicked()
{
    QPushButton* sender = (QPushButton*)QObject::sender();    
    QString document = sender->property( "doc" ).toString();
    QModelIndex index = sender->property( "node" ).toModelIndex();

    makeAbsolute( document, index );
      
    if ( !QFile::exists( document ) )
    {
        QMessageBox::warning( this, tr("File not found"), tr("Could not locate file: %1").arg( document ) );
    }
    else
    {
        QDesktopServices::openUrl( QUrl::fromLocalFile( document ) );
    }
}


//----------------------------------------------------------------------------
/*! 
 *  Slot called when delete document push button is called for an info element.
 */

void DInsightMainWindow::deleteFolderClicked()
{
    QPushButton* sender = (QPushButton*)QObject::sender();    
    QString document = sender->property( "doc" ).toString();
    QModelIndex index = sender->property( "node" ).toModelIndex();

    deleteImportFolder( document, index );
}


//----------------------------------------------------------------------------
/*! 
 *  Delete import context menu clicked.
 */

void DInsightMainWindow::deleteMenuClicked( bool /*checked*/ )
{
    QAction* sender = (QAction*)QObject::sender();
    QString document = sender->property( "doc" ).toString();
    QModelIndex index = sender->property( "node" ).toModelIndex();

    deleteImportFolder( document, index );
}


//----------------------------------------------------------------------------
/*! 
 *  Delete import folder.
 */

void DInsightMainWindow::deleteImportFolder( const QString& document, QModelIndex& index )
{
    DTreeItem* item = (DTreeItem*)index.internalPointer();
    
    cancelSearchDeamon();
    cancelSearch();

    if ( !QFile::exists( document ) )
    {
        QMessageBox::warning( this, tr("Folder not found"), tr("Could not locate folder: %1").arg( document ) );
    }
    else
    {
        int answer = QMessageBox::question( this, tr("Delete Import"), tr("This will delete the report folder %1 and all files and folders below it. It will also remove this archive from the tree view. This operation can't be reverted. Are you sure you want to continue?").arg( document) );

        if ( answer == QMessageBox::Yes )
        {
            QDir d( document );
            d.removeRecursively();

            m_Ui.treeView->clearSelection();
            m_Ui.treeView->setCurrentIndex( QModelIndex() );
            m_Model->deleteDocumentRoot( item, false );
            updateInfo( m_Model->firstDocumentRoot() );

            DImportsIterator it = m_Imports.begin();
            DImportsIterator itEnd = m_Imports.end();
            while ( it != itEnd )
            {
                if ( (*it)->root() == item )
                {
                    delete *it;
                    m_Imports.erase( it );
                    break;
                }
                it++;
            }

            m_Ui.exportButton->setEnabled( treeNodeCount( true ) > 0 );
            m_Ui.searchEdit->setEnabled( m_Imports.size() );
            m_Ui.searchOptionsButton->setEnabled( m_Imports.size() );

            if ( m_Imports.size() == 0 )
            {
                m_Ui.searchOptionsGroupBox->setVisible( false );
                m_Ui.searchOptionsButton->setText( "+" );
            }
        }
    }

    startSearchDeamon();
}


//----------------------------------------------------------------------------
/*! 
 *  Slot called when import document push button is called for an info element.
 */

void DInsightMainWindow::importDocumentClicked()
{
    QPushButton* sender = (QPushButton*)QObject::sender();
    QString document = sender->property( "doc" ).toString();
    QModelIndex index = sender->property( "node" ).toModelIndex();

    importDocumentClicked( document, index );
}


//----------------------------------------------------------------------------
/*! 
 *  Slot called when Validate checksum button is clicked for an info element.
 */

void DInsightMainWindow::checksumClicked()
{
    QPushButton* sender = (QPushButton*)QObject::sender();
    QString document = sender->property( "doc" ).toString();
    QString checksum = sender->property( "checksum" ).toString();
    QString method = sender->property( "method" ).toString();
    QModelIndex index = sender->property( "node" ).toModelIndex();

    makeAbsolute( document, index );

    struct
    {
        QCryptographicHash::Algorithm m_Method;
        const char* m_Name;
        int m_Length;
    } methods[] =
    {
        { QCryptographicHash::Sha256, "sha256", 256/8*2 },
        { QCryptographicHash::Sha512, "sha512", 512/8*2 },
        { QCryptographicHash::Sha1, "sha1", 20*2 },
    };

    // String length determines method
    QCryptographicHash::Algorithm algorithm = QCryptographicHash::Sha1;
    int methodLength = checksum.length();
    if ( !method.length() )
    {
        for ( size_t i = 0; i < sizeof(methods)/sizeof(method[0]); i++ )
        {
            // Last method is default;
            algorithm = methods[i].m_Method;
            method = methods[i].m_Name;
            if (methodLength == methods[i].m_Length/*QCryptographicHash::hashLength(algorithm)*2*/)
            {
                break;
            }
        }
    }
    else
    {
        // Map name to method
    }

    QFile file(document);
    if ( !file.open( QFile::ReadOnly ) )
    {
        DInsightConfig::Log() << "Failed to open: " << document << endl;
        QMessageBox::information( this, tr("Failed to open file"), tr("Failed to open '%1'.").arg( document ), QMessageBox::Ok );
        return;
    }

    QCryptographicHash hash( algorithm );
    hash.addData( &file );

    QString calculatedHash = hash.result().toHex();
    bool ok = calculatedHash.toLower() == checksum.toLower();
    if ( !ok )
    {
        DInsightConfig::Log() << "Checksum mismatch: " << checksum << " vs " << calculatedHash << " for " << document << endl;
        QMessageBox::information( this, tr("Checksum mismatch"), tr("Checksum not equal for file '%1' using %2 calculation. Got '%3', expected '%4' using method %5.")
                                  .arg( document )
                                  .arg( method )
                                  .arg( calculatedHash )
                                  .arg( checksum )
                                  .arg( method ),
                                  QMessageBox::Ok );
        return;
    }
    else
    {
        QMessageBox::information( this, tr("Checksum OK"), tr("Checksum validated OK!"),
                                  QMessageBox::Ok );
    }
}


//----------------------------------------------------------------------------
/*! 
 *  Import AIP button clicked.
 */

void DInsightMainWindow::importDocumentClicked( const QString& document, QModelIndex& index )
{
    if ( m_CurrentImport )
    {
        QMessageBox::information( this, tr("Import already in progress"), tr("Please wait untill ongoing import is complete before importing more documents.") );
        return;
    }

    DTreeItem* item = (DTreeItem*)index.internalPointer();

    // Stop search
    cancelSearch();

    // Stop search deamon
    cancelSearchDeamon();

    if ( item->hasChildren() )
    {
        m_StatusBar->showMessage( tr("Unloaded file: %1").arg( document ) );
    
        m_Ui.treeView->clearSelection();
        m_Ui.treeView->setCurrentIndex( QModelIndex() );

        // Remove all nodes below root item
        DImport* import = findImport( item );
        m_CurrentImport = import;
        import->setFromReport( true );
        import->unload();
    }
    else
    {
        // Import file
        DImport* import = findImport( item );

        if ( import )
        {        
            DInsightConfig::Log() << "Re-importing: " << import->fileName() << endl;
            QFileInfo info( import->fileName() );
            if ( !info.exists() )
            {
                DInsightConfig::Log() << "Opening failed, file does not exist: " << import->fileName() << endl;
                QMessageBox::information( this, tr("Failed to open file"), tr("Failed to open '%1'.").arg( import->fileName() ), QMessageBox::Ok );
                return;
            }

            const DImportFormat* format = m_ImportFormats->find(import->formatName());
            if ( format == nullptr )
            {
                DInsightConfig::Log() << "Import format not found: " << import->formatName() << endl;
                QMessageBox::information( this, tr("Format not found"), tr("Import format not found: '%1'.").arg( import->formatName() ), QMessageBox::Ok );
                return;
            }

            m_CurrentImport = import;
        
            setupUiForImport();

            import->setFromReport( true );
            import->load( format );
        }
        else
        {
            QString absDocument = document;
            makeAbsolute( absDocument, index );

            const DImportFormat* importFormat = m_ImportFormats->findMatching(absDocument);

            DInsightConfig::Log() << "Importing doc: " << absDocument << endl;
            importFile( absDocument, importFormat->name(), item );
        }
    }
}


//----------------------------------------------------------------------------
/*! 
 *  Slot called when unload of root node is finished.
 */

void DInsightMainWindow::unloadFinished( bool /*ok*/ )
{
    DImport* import = (DImport*)sender();

    // Update info view
    m_Ui.treeView->setCurrentIndex( m_Model->index( import->root() ) );
    updateInfo( import->root() );
    m_CurrentImport = nullptr;

    m_Ui.importButton->setText( tr("Import") );
    m_Ui.importButton->setEnabled( true );
    m_ProgressBar->hide();
    m_ProgressBarInfo->hide();

    startSearchDeamon();
}


//----------------------------------------------------------------------------
/*! 
 *  Slot called when text inside search edit changes.
 */

void DInsightMainWindow::searchEditChanged( const QString & text )
{
    bool search = text.length() > 0;
    m_Ui.selectButton->setEnabled( search );
    m_Ui.selectButton->setText( tr("Select") );

    cancelSearchThread();

    // Create search thread here...
    if ( search )
    {
        assert( m_SearchThread == nullptr );
        
        int searchModes = m_Ui.searchAttachmentsCheckBox->isChecked() ? DSearchThread::SEARCH_MODE_ATTACHMENTS : DSearchThread::SEARCH_MODE_TREE;
        if ( !m_Ui.caseSensitiveCheckBox->isChecked() )
        {
            searchModes |= DSearchThread::SEARCH_MODE_CASE_INSENSITIVE;
        }

        DXmlParser::StringHash nodesToExclude;
        getNodesToExcludeFromSearch( nodesToExclude );
        m_SearchThread = new DSearchThread( m_Model, &m_Imports, text, m_SearchResultMax, searchModes, nodesToExclude );
        connect(m_SearchThread, &DSearchThread::finished, this, &DInsightMainWindow::searchFinished);
        connect(m_SearchThread, &DSearchThread::searchProgress, this, &DInsightMainWindow::searchProgress);
        connect(m_SearchThread, &DSearchThread::searchMatch, this, &DInsightMainWindow::searchMatch);
        connect(m_SearchThread, &DSearchThread::attachmentMatch, this, &DInsightMainWindow::attachmentMatch);

        startSearch();
    }
    else
    {
        // Update info view
        searchInfo( m_Ui.searchEdit->text() );

        cancelSearch();
    }
}


//----------------------------------------------------------------------------
/*! 
 *  Start search thread, search string comes from search edit box.
 */

void DInsightMainWindow::startSearch()
{
    m_Ui.searchResult->setRowCount( 0 );
    m_Ui.searchResult->setVisible( true );
    
    m_SearchThread->start();

    m_ProgressBar->setVisible( true );
    m_ProgressBar->setRange( 0, 0 );
    m_ProgressBar->setFormat( "" );
    m_ProgressBar->setTextVisible( false );
    m_ProgressBarInfo->setVisible( true );
    m_ProgressBarInfo->setText( tr("search in progress") );
    m_StatusBar->showMessage( "" );
}


//----------------------------------------------------------------------------
/*! 
 *  Slot called when search is finished.
 *  \note Not called if search string is empty
 */

void DInsightMainWindow::searchFinished()
{
    // Update info view
    searchInfo( m_Ui.searchEdit->text() );

    unsigned int matchCount = 0;
    if ( m_SearchThread )
    {
        matchCount = m_SearchThread->matchCount();
    }

    // Did we find anything?
    if ( matchCount == 0 )
    {
        m_Ui.searchResult->setRowCount( 1 );

        QTableWidgetItem* locationItem = new QTableWidgetItem( tr( "No match." ) );
        locationItem->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
        QTableWidgetItem* contentItem = new QTableWidgetItem( tr( "" ) );
        contentItem->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );

        m_Ui.searchResult->setItem( 0, 0, locationItem );
        m_Ui.searchResult->setItem( 0, 1, contentItem );
    }

    // Prev/next buttons?
    m_Ui.previousSearchPageButton->setEnabled( m_SearchThread->currentPage() != 0 );
    m_Ui.nextSearchPageButton->setEnabled( m_SearchThread->currentPage() < m_SearchThread->pageCount() - 1 || m_SearchThread->hasMorePages() );
    m_Ui.nextSearchPageButton->setVisible( m_SearchThread->pageCount() > 1 );
    m_Ui.previousSearchPageButton->setVisible( m_SearchThread->pageCount() > 1 );
    m_Ui.searchNavigationFrame->setVisible( m_SearchThread->pageCount() > 1 );

    m_Ui.searchResult->resizeColumnToContents( 0 );
    m_Ui.searchResult->setVisible( m_Ui.searchEdit->text().length() );

    QString resultMessage;
    if ( matchCount && matchCount < m_SearchResultMax )
    {
        resultMessage = tr("Search complete! %1 matches found.").arg( matchCount );
    }
    else if ( matchCount >= m_SearchResultMax )
    {
        resultMessage = tr("Search complete! Over %1 matches found, displaying first %1 matches.").arg( matchCount );
    }
    m_StatusBar->showMessage( resultMessage, 120*1000 ); // ms
    m_ProgressBar->setVisible( m_CurrentImport && m_CurrentImport->state() != DImport::IMPORT_STATE_DONE );
    m_ProgressBarInfo->setVisible( false );
}


//----------------------------------------------------------------------------
/*! 
 *  Slot for search progress updates.
 */

void DInsightMainWindow::searchProgress( float /*progress*/ )
{
    m_ProgressBar->update();    
}


//----------------------------------------------------------------------------
/*! 
 *  Slot for search match found.
 */

void DInsightMainWindow::searchMatch( const QModelIndex& matchingIndex )
{
    // Get match
    DTreeItem* item = (DTreeItem*)matchingIndex.internalPointer();
    QString matchString = getTreeItemMatchString( item, m_Ui.searchEdit->text() );

    QString location = QString( item->m_Text );
    DInsightMainWindow::ReplaceString( location, item->nodeRegExp() );

    addSearchResult( location, matchString, matchingIndex );
}


//----------------------------------------------------------------------------
/*! 
 *  Slot for attachment match found.
 */

void DInsightMainWindow::attachmentMatch( int importIndex, int attachmentIndex )
{
    DImport* import = m_Imports.at( importIndex );

    DAttachment* attachment = import->getAttachment( attachmentIndex );
    if ( attachment )
    {
        QString attachmentTextFileName = DAttachmentIndexer::attachmentConvertFileName( import->attachmentsDir(), attachmentIndex );

        QString matchString = getAttachmentMatchString( attachmentTextFileName );
        addSearchResult( attachment->m_FileName, matchString, m_Model->index( attachment->m_TreeItem ) );
    }
}


//----------------------------------------------------------------------------
/*! 
 *  Add search result to result table.
 */

void DInsightMainWindow::addSearchResult( const QString& location, const QString& matchString, const QModelIndex& matchingIndex )
{
    // Add row to search result window
    int row = m_Ui.searchResult->rowCount();
    m_Ui.searchResult->setRowCount( row + 1 );

    // Location column
    QTableWidgetItem* locationItem = new QTableWidgetItem( location );
    locationItem->setData( Qt::UserRole, matchingIndex );
    locationItem->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
    m_Ui.searchResult->setItem( row, 0, locationItem );

    // Match column
    QTableWidgetItem* matchItem = new QTableWidgetItem( matchString );
    matchItem->setData( Qt::UserRole, matchingIndex );
    matchItem->setFlags( Qt::ItemIsEnabled );
    m_Ui.searchResult->setItem( row, 1, matchItem );
}


//----------------------------------------------------------------------------
/*! 
 *  Find DImport object from DTreeItem.
 */

DImport* DInsightMainWindow::findImport( const DTreeItem* item )
{
    DImportsIterator it = m_Imports.begin();
    DImportsIterator itEnd = m_Imports.end();

    while ( it != itEnd )
    {
        if ( (*it)->root() == item )
        {
            return *it;
        }
        it++;
    }
    return nullptr;
}


//----------------------------------------------------------------------------
/*! 
 *  Set focus on tree item
 */

void DInsightMainWindow::searchResultDoubleClicked(QTableWidgetItem *item)
{
    QVariant d = item->data( Qt::UserRole );
    QModelIndex i = d.toModelIndex();
    
    if ( i.internalPointer() )
    {
        m_Ui.treeView->scrollTo( i );
        m_Ui.treeView->setCurrentIndex( i );
        updateInfo( (DTreeItem*)i.internalPointer() );
    }
}

//----------------------------------------------------------------------------
/*! 
 *  Set focus on tree item
 */

void DInsightMainWindow::searchResultDoubleClicked(DTableSearchResultCell *item)
{
    QVariant d = item->property( "item" );
    QModelIndex i = d.toModelIndex();
    
    m_Ui.treeView->scrollTo( i );
    m_Ui.treeView->setCurrentIndex( i );
    updateInfo( (DTreeItem*)i.internalPointer() );
}


//----------------------------------------------------------------------------
/*! 
 *  Change sort order.
 */

void DInsightMainWindow::searchResultHeaderClicked(int logicalIndex)
{
    m_Ui.searchResult->sortByColumn( logicalIndex );
}


//----------------------------------------------------------------------------
/*! 
 *  Attachment check box clicked.
 */

void DInsightMainWindow::searchAttachmentClicked(bool /*checked*/ )
{
    // The search should be reset
    searchEditChanged( m_Ui.searchEdit->text() );
}


//----------------------------------------------------------------------------
/*! 
 *  Cancel the ongoing search operation.
 */

void DInsightMainWindow::cancelSearch()
{
    cancelSearchThread();

    m_Ui.searchResult->setRowCount( 0 );
    m_Ui.searchResult->setVisible( false );
    m_Ui.selectButton->setEnabled( false );
    m_Ui.previousSearchPageButton->setVisible( false );
    m_Ui.nextSearchPageButton->setVisible( false );
    m_Ui.searchNavigationFrame->setVisible( false );

    // Disable signal from search edit to avoid starting a new search in the cleanup case
    QObject::disconnect( m_Ui.searchEdit, SIGNAL(textChanged(const QString &)), this, SLOT(searchEditChanged(const QString &)));
    m_Ui.searchEdit->setText( "" );
    QObject::connect( m_Ui.searchEdit, SIGNAL(textChanged(const QString &)), this, SLOT(searchEditChanged(const QString &)));

    m_StatusBar->clearMessage();
    m_ProgressBar->setVisible( m_CurrentImport && m_CurrentImport->state() != DImport::IMPORT_STATE_DONE );
    m_ProgressBarInfo->setVisible( false );
}


//----------------------------------------------------------------------------
/*! 
 *  Cancel the ongoing search operation.
 */

void DInsightMainWindow::cancelSearchThread()
{
    if ( m_SearchThread )
    {
        QObject::disconnect(m_SearchThread, &DSearchThread::finished, this, &DInsightMainWindow::searchFinished);
        QObject::disconnect(m_SearchThread, &DSearchThread::searchProgress, this, &DInsightMainWindow::searchProgress);
        QObject::disconnect(m_SearchThread, &DSearchThread::searchMatch, this, &DInsightMainWindow::searchMatch);
        QObject::disconnect(m_SearchThread, &DSearchThread::attachmentMatch, this, &DInsightMainWindow::attachmentMatch);
        
        m_SearchThread->requestInterruption();
        m_SearchThread->quit();
        m_SearchThread->wait();

        delete m_SearchThread;
        m_SearchThread = nullptr;
    }
}

//----------------------------------------------------------------------------
/*! 
 *  Cancel the search deamon process.
 */

void DInsightMainWindow::cancelSearchDeamon()
{
    if ( m_SearchDeamonProcess )
    {
        m_SearchDeamonProcess->kill();
        m_SearchDeamonProcess->waitForFinished();
        delete m_SearchDeamonProcess;
        m_SearchDeamonProcess = nullptr;
    }
}


//----------------------------------------------------------------------------
/*! 
 *  Slot called when attachment indexer is finished
 */

void DInsightMainWindow::indexingFinished( DImport::DIndexingState state )
{
    QString message;
    DImport* import = (DImport*)sender();

    if ( state == DImport::INDEXING_STATE_OK )
    {
        QString failedMessage;
        unsigned int failCount = import->attachmentsFailedToConvert();
        if ( failCount )
        {
            failedMessage = tr( "Warning: %1 attachments failed to convert. See report log for details." ).arg( failCount );
        }
        unsigned int emptyCount = import->attachmentsEmpty();
        if ( emptyCount )
        {
            failedMessage += tr( " Warning: %1 attachments are empty. See report log for details." ).arg( emptyCount );
        }

        message = QString( tr( "Indexing of %1 attachments complete! %2" ) ).arg( import->attachmentsFound() ).arg( failedMessage );
    }
    else if ( state == DImport::INDEXING_STATE_ERROR )
    {
        message = QString( tr( "Failed to start indexer! See import log for more details." ) );
    }
    else
    {
        message = QString( tr( "Indexing canceled!" ) );
    }
    
    m_Ui.importButton->setText( tr( "Import" ) );
    m_Ui.importButton->setEnabled( true );
    m_StatusBar->showMessage( message );
    m_ProgressBar->setVisible( false );
    m_ProgressBarInfo->setVisible( false );
    m_CurrentImport = nullptr;

    startSearchDeamon();
}


//----------------------------------------------------------------------------
/*! 
 *  Progress event from indexer.
 */

void DInsightMainWindow::indexingProgress( float progress )
{
    m_ProgressBar->setValue( 100 * progress );
}


//----------------------------------------------------------------------------
/*! 
 *  Indexer has started.
 */

void DInsightMainWindow::indexingIndexerStated()
{
    m_ProgressBar->setRange( 0, 0 );
    m_ProgressBar->setTextVisible( true );
    m_ProgressBarInfo->setVisible( true );
    m_ProgressBarInfo->setText( tr("Indexing attachments, might take a long time") );
}


//----------------------------------------------------------------------------
/*! 
 *  Update info view and high-light items that match search text.
 */

bool DInsightMainWindow::searchInfo( const QString& /*text*/ )
{
    if ( !m_Ui.treeView->currentIndex().isValid() )
    {
        return false;
    }

    QModelIndex index = m_Ui.treeView->currentIndex();
    if ( index.isValid() && index.internalPointer() )
    {
        updateInfo( (DTreeItem*) index.internalPointer() );
    }
    return true;
}


//----------------------------------------------------------------------------
/*! 
 *  Called when check box in tree is clicked
 */

void DInsightMainWindow::dataChanged(const QModelIndex &, const QModelIndex &, const QVector<int> & roles)
{
    foreach( int r, roles )
    {
        if ( r == Qt::CheckStateRole )
        {
            treeNodeSelectionCountChanged();
        }
    }
}


//----------------------------------------------------------------------------
/*! 
 *  Load document context menu.
 */

void DInsightMainWindow::loadMenuClicked( bool /*checked*/ )
{
    QAction* sender = (QAction*)QObject::sender();
    QString document = sender->property( "doc" ).toString();
    QModelIndex index = sender->property( "node" ).toModelIndex();

    importDocumentClicked( document, index );
}


//----------------------------------------------------------------------------
/*! 
 *  Select children context menu.
 */

void DInsightMainWindow::selectChildrenMenuClicked( bool /*checked*/ )
{
    QModelIndex index = m_Ui.treeView->currentIndex();
    if ( !index.isValid() )
    {
        return;
    }

    setCheckStateRecursive( (DTreeItem*)index.internalPointer(), Qt::Checked );

    treeNodeSelectionCountChanged();
}


//----------------------------------------------------------------------------
/*! 
 *  Deselect children context menu.
 */

void DInsightMainWindow::deselectChildrenMenuClicked( bool /*checked*/ )
{
    QModelIndex index = m_Ui.treeView->currentIndex();
    if ( !index.isValid() )
    {
        return;
    }

    setCheckStateRecursive( (DTreeItem*)index.internalPointer(), Qt::Unchecked );

    treeNodeSelectionCountChanged();
}


//----------------------------------------------------------------------------
/*! 
 *  This function should be called whenever the node selection count changes.
 */

void DInsightMainWindow::treeNodeSelectionCountChanged()
{
    int selectionCount = treeNodeCount( true );
    m_Ui.exportButton->setEnabled( selectionCount > 0 );

    QString infoMessage;
    if ( selectionCount > 1)
    {
        infoMessage = QString( tr("%1 nodes selected.") ).arg( selectionCount );
    }
    else if ( selectionCount == 1)
    {
        infoMessage = QString( tr("%1 node selected.") ).arg( selectionCount );
    }

    m_StatusBar->showMessage( infoMessage );
}


//----------------------------------------------------------------------------
/*! 
 *  Select all context menu.
 */

void DInsightMainWindow::selectAllMenuClicked( bool /*checked*/ )
{
    unsigned int rootCount = m_Model->rootCount();
    for ( unsigned int i = 0; i < rootCount; i++ )
    {
        setCheckStateRecursive( m_Model->documentRoot( i ), Qt::Checked );
    }

    treeNodeSelectionCountChanged();
}


//----------------------------------------------------------------------------
/*! 
 *  Deselect all context menu.
 */

void DInsightMainWindow::deselectAllMenuClicked( bool /*checked*/ )
{
    unsigned int rootCount = m_Model->rootCount();
    for ( unsigned int i = 0; i < rootCount; i++ )
    {
        setCheckStateRecursive( m_Model->documentRoot( i ), Qt::Unchecked );
    }
    treeNodeSelectionCountChanged();
}


//----------------------------------------------------------------------------
/*! 
 *  Invert selection context menu.
 */

void DInsightMainWindow::invertSelectionMenuClicked( bool /*checked*/ )
{
    unsigned int rootCount = m_Model->rootCount();
    for ( unsigned int i = 0; i < rootCount; i++ )
    {
        invertCheckStateRecursive( m_Model->documentRoot( i ) );
    }
    treeNodeSelectionCountChanged();
}


//----------------------------------------------------------------------------
/*! 
 *  Set tree item check state recursively.
 */

void DInsightMainWindow::setCheckStateRecursive( DTreeItem* parent, Qt::CheckState checkState )
{
    if ( checkState == Qt::Checked )
    {
        parent->check();
    }
    else
    {
        parent->uncheck();
    }

    DTreeItem::DChildrenIterator it = parent->m_Children.begin();
    while ( it != parent->m_Children.end() )    
    { 
        setCheckStateRecursive( *it, checkState );
        ++it;
    }   
}


//----------------------------------------------------------------------------
/*! 
 *  Invert tree item check state recursively.
 */

void DInsightMainWindow::invertCheckStateRecursive( DTreeItem* parent )
{
    if ( parent->checked() )
    {
        parent->uncheck();
    }
    else
    {
        parent->check();
    }

    DTreeItem::DChildrenIterator it = parent->m_Children.begin();
    while ( it != parent->m_Children.end() )    
    { 
        invertCheckStateRecursive( *it );
        ++it;
    }   
}


//----------------------------------------------------------------------------
/*! 
 *  Progress from XML loader
 */

void DInsightMainWindow::loadXmlProgress( unsigned long /*count*/, float progress )
{
    m_ProgressBar->setValue( 100 * progress );
}


//----------------------------------------------------------------------------
/*! 
 *  Export DIP
 */

void DInsightMainWindow::exportButtonClicked()
{
    DInsightReport report;
    QStringList existingAttachmentsFullPath;

    // Could take some time:
    {
        DWaitCursor wait;

        DImportsIterator it = m_Imports.begin();
        DImportsIterator itEnd = m_Imports.end();
        while ( it != itEnd )
        {
            // Exclude child imports
            if ( !(*it)->root()->isToplevelRoot() )
            {
                it++;
                continue;
            }

            if ( treeNodeCountRecursive( (*it)->root(), true ) )
            {
                QStringList attachments;
                createReport( tr("Export Report"), (*it)->root(), report, 0, INT_MAX, attachments );

                QString attachmentRoot = (*it)->fileNameRoot();
                if ( (*it)->root()->format()->parser() == "dir" )
                {
                    attachmentRoot = (*it)->extractDir();
                }

                foreach( QString attachment, attachments )
                {
                    QFileInfo info( attachment );

                    existingAttachmentsFullPath.push_back( DAttachmentParser::AttachmentPath( attachment, attachmentRoot ) );
                }

                // Add attachments to report
                report.addHeader( tr( "Attachments" ), 1 );
                report.startTable( 0 );
                foreach( QString attachment, attachments )
                {
                    report.addRow( attachment );        
                }    
                report.endTable(0);
            }
            it++;
        }                    
    }

    // Display export dialog with report 
    DInsightReportWindow reportDialog( report, existingAttachmentsFullPath );
    reportDialog.exec();
}


//----------------------------------------------------------------------------
/*! 
 *  Select all nodes matching current search
 */

void DInsightMainWindow::selectButtonClicked()
{
    Qt::CheckState checkState = m_Ui.selectButton->text() == tr("Deselect") ? Qt::Unchecked : Qt::Checked;
    m_Ui.selectButton->setText( checkState == Qt::Checked ? tr("Deselect") : tr("Select") );

    // Select/deselect current search result
    //setCheckStateRecursive( m_Model->documentRoot(), checkState, true );
    
    // Iterate through match result
    int matchCount = m_Ui.searchResult->rowCount();
    for ( int i = 0; i < matchCount; i++ )
    {
        QTableWidgetItem* item = m_Ui.searchResult->item( i, 0 );
        
        QVariant d = item->data( Qt::UserRole );
        QModelIndex index = d.toModelIndex();
    
        DTreeItem* treeItem = (DTreeItem*)index.internalPointer();
        while ( treeItem )
        {
            treeItem->setChecked( checkState == Qt::Checked );
            treeItem = treeItem->m_Parent;
        }
    }

    QVector<int> roles;
    roles.push_back( Qt::CheckStateRole );
    m_Model->dataChanged( m_Model->index(0,0), m_Model->index(m_Model->rowCount(),0), roles );
}


//----------------------------------------------------------------------------
/*! 
 *  Create report as text
 */

void DInsightMainWindow::createReport( 
    const QString& reportHeading, 
    Node* parent, 
    DInsightReport& report, 
    int level, 
    int maxLevel, 
    QStringList& attachments, 
    bool onlyChecked /*=true*/,
    bool replaceLabels /*=true*/ )
{
    if ( reportHeading.length() )
    {
        report.addHeader( reportHeading, 0 );
    }

    if ( !onlyChecked || parent->checked() )
    {
        // Add report heading - based on level
        QString header;
        if ( replaceLabels )
        {
            header = DInsightMainWindow::GetTreeItemLabel( parent );
        }
        else
        {
            header = parent->m_Text;
        }
        report.addHeader( header, level );

        // Search through data attached to this node
        report.startTable( level );
        DLeafNodesIterator it = parent->m_Nodes.begin();
        while ( it != parent->m_Nodes.end() )
        {
            QString key = (*it)->m_Key;
            if (replaceLabels)
            {
                ReplaceString( key, parent->nodeRegExp() );
            }
            report.addRow( key, (*it)->m_Value );

            if ( isDocumentNode( parent->format(), (*it)->m_Key, (*it)->m_Value ) )
            {
                attachments.push_back( (*it)->m_Value );
            }
            
            ++it;
        }
        report.endTable( level );
    }

    // Search through all child nodes for matching text
    if ( level < maxLevel )
    {
        DTreeItem::DChildrenIterator it = parent->m_Children.begin();
        while ( it != parent->m_Children.end() )
        {
            createReport( QString(), *it, report, level + 1, maxLevel, attachments, onlyChecked );

            ++it;
        }
    }
}


//----------------------------------------------------------------------------
/*! 
 *  Return number of nodes in tree view
 */

int DInsightMainWindow::treeNodeCount( bool onlyChecked )
{
    if ( !m_Model || !m_Model->rootCount() )
    {
        return 0;
    }

    int count = 0;

    // Select/deselect current search result
    unsigned int rootCount = m_Model->rootCount();
    for ( unsigned int i = 0; i < rootCount; i++ )
    {
        count += treeNodeCountRecursive( m_Model->documentRoot( i ), onlyChecked );
    }

    return count;
}


//----------------------------------------------------------------------------
/*! 
 *  Return number of child nodes for tree itrem.
 */

int DInsightMainWindow::treeNodeCountRecursive( Node* parent, bool onlyChecked )
{
    int count = onlyChecked ? (parent->checked() ? 1 : 0) : 1;

    // Search through all child nodes for matching text
    for ( unsigned int i = 0; i < parent->m_Children.size(); i++ )
    {
        count += treeNodeCountRecursive( parent->m_Children[i], onlyChecked );
    }

    return count;
}


//----------------------------------------------------------------------------
/*! 
 *  Return true if key matches one of the regular expressions.
 */

bool DInsightMainWindow::isNode( const QString& key, const DRegExps& regExps )
{
    foreach( const QRegularExpression& regExp, regExps )
    {
        if ( regExp.match( key ).hasMatch() )
        {
            return true;
        }
    }

    return false;
}

//----------------------------------------------------------------------------
/*!
 *  Return true if key matches one of the leaf matchers.
 */

bool DInsightMainWindow::isNode( const QString& key, const QString& value, const DLeafMatchers& matchers )
{
    foreach( const DLeafMatcher& matcher, matchers )
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
 *  Return true is this is a document node.
 */

bool DInsightMainWindow::isDocumentNode( const DImportFormat* format, const QString& key, const QString& value )
{
    return isNode( key, value, format->documentTypeRegExp() );
}


//----------------------------------------------------------------------------
/*! 
 *  Return true if this is a folder node.
 */

bool DInsightMainWindow::isFolderNode( const DImportFormat* format, const QString& key )
{
    return isNode( key, format->folderTypeRegExp() );
}


//----------------------------------------------------------------------------
/*! 
 *  Return true if this node should have the delete operation.
 */

bool DInsightMainWindow::isDeleteNode( const DImportFormat* format, const QString& key )
{
    return isNode( key, format->deleteTypeRegExp() );
}


//----------------------------------------------------------------------------
/*! 
 *  Return true if this node should have the load/unload operation.
 */

bool DInsightMainWindow::isImportNode( const DImportFormat* format, const QString& key, const QString& value )
{
    return isNode( key, value, format->importTypeRegExp() );
}


//----------------------------------------------------------------------------
/*! 
 *  Return true if this node should have the validate operation.
 */

bool DInsightMainWindow::isChecksumNode( const DImportFormat* format, const QString& key )
{
    return isNode( key, format->checksumTypeRegExp() );
}


//----------------------------------------------------------------------------
/*! 
 *  Return true if this node is has a file that is checksum protected.
 */

bool DInsightMainWindow::isChecksumSourceNode( const DImportFormat* format, const QString& key )
{
    return isNode( key, format->checksumSourceTypeRegExp() );
}


//----------------------------------------------------------------------------
/*! 
 *  Return prettyfied tree view item text.
 */

QString DInsightMainWindow::getInfoViewLabel( const DImportFormat* format, const char* key )
{
    QString k( key );
    ReplaceString( k, format->infoViewLabelRegExp() );

    return k;
}


//----------------------------------------------------------------------------
/*! 
 *  Convert tree item nodes to a string, highliting the match.
 */

QString DInsightMainWindow::getTreeItemMatchString( DTreeItem* item, const QString& searchText )
{
    QString text;

    DLeafNodesIterator it = item->m_Nodes.begin();

    while ( it != item->m_Nodes.end() )
    {
        QString v( (*it)->m_Value );

        QString key = getInfoViewLabel( item->format(), (*it)->m_Key );

        bool match;
        text += getHighlighMatchString( searchText, key, v, match );
        ++it;
    }

    return text;
}


//----------------------------------------------------------------------------
/*! 
 *  Hilight the match using HTML tags.
 */

QString DInsightMainWindow::getHighlighMatchString( 
    const QString& searchText, 
    const QString& key, 
    const QString& text, 
    bool &match )
{
    bool ignoreCase = !m_Ui.caseSensitiveCheckBox->isChecked();
    
    int i = text.indexOf( searchText, 0, ignoreCase ? Qt::CaseInsensitive : Qt::CaseSensitive );
    match = i != -1;
    if ( match )
    {
        QString a = text.mid( 0, i );
        QString b = text.mid( i, searchText.length() );
        QString c = text.mid( i + searchText.length() );

        return QString( "<b>%1</b> %2<font color='#0000FF'>%3</font>%4 " )
            .arg( key )
            .arg( a )
            .arg( b )
            .arg( c );
    }

    return QString( "<b>%1</b> %2 " ).arg( key ).arg( text );
}


//----------------------------------------------------------------------------
/*! 
 *  Hilight the attachment match.
 */

QString DInsightMainWindow::getAttachmentMatchString( const QString& attacmentTextFileName )
{
    QString searchText = m_Ui.searchEdit->text();

    QFile file( attacmentTextFileName );
    if ( !file.open( QFile::ReadOnly | QFile::Text ) )
    {
        DInsightConfig::Log() << "Failed to open: " << attacmentTextFileName << endl;
        return QString();
    }
    QTextStream stream( &file );

    QString matchString;
    QString line;
    unsigned int lineCount = 0;
    while ( stream.readLineInto( &line ) )
    {
        QString key( tr("Line %1").arg( lineCount ) );

        bool match;
        QString text = getHighlighMatchString( searchText, key, line, match );
        if ( match )
        {
            if ( !matchString.isEmpty() )
            {
                matchString += "<br>";
            }
            matchString += text;
        }
        lineCount++;
    }

    return matchString;
}
            

//----------------------------------------------------------------------------
/*! 
 *  Called if the search deamon process reports an error.
 */

void DInsightMainWindow::searchDeamonError(QProcess::ProcessError error)
{
    DInsightConfig::Log() << "Search deamon error: " << error << endl;
    QByteArray errorContent = m_SearchDeamonProcess->readAllStandardError();
    DInsightConfig::Log() << "Error: " << QString(errorContent).trimmed() << endl;    
}


//----------------------------------------------------------------------------
/*! 
 *  Go to previous search result page
 */

void DInsightMainWindow::prevSearchResultPageClicked()
{
    m_SearchThread->gotoPrevPage();

    // Restart search
    startSearch();
}


//----------------------------------------------------------------------------
/*! 
 *  Go to next search result page
 */

void DInsightMainWindow::nextSearchResultPageClicked()
{
    m_SearchThread->gotoNextPage();

    // Restart search
    startSearch();
}


//----------------------------------------------------------------------------
/*! 
 *  Show/hide advanced search options
 */

void DInsightMainWindow::searchOptionsToggled( bool /*on*/ )
{
    m_Ui.searchOptionsGroupBox->setVisible( !m_Ui.searchOptionsGroupBox->isVisible() );
    m_Ui.searchOptionsButton->setText( m_Ui.searchOptionsGroupBox->isVisible() ? "-" : "+" );
}


//----------------------------------------------------------------------------
/*! 
 *  Node filters changed.
 */

void DInsightMainWindow::treeNodesSearchFilterChanged( bool /*checked*/ )
{
    searchEditChanged( m_Ui.searchEdit->text() );
}


//----------------------------------------------------------------------------
/*! 
 *  Combine all indexes loaded into one indexer config file.
 */

QString DInsightMainWindow::createCombinedSearchConfigFile()
{
    //QTemporaryFile tempFile;
    //tempFile.setAutoRemove( false );
    QFile tempFile( "sphinx_temp.conf" );
    if ( tempFile.open( QIODevice::Text | QIODevice::WriteOnly ) )
    {
        // Write all indexes to config
        int importCount = 0;
        DImportsIterator it = m_Imports.begin();
        DImportsIterator itEnd = m_Imports.end();
        while ( it != itEnd )
        {
            // Only start indexer for imports that are loaded
            if ( (*it)->hasChildren() )
            {
                QFile config( (*it)->searchConfig() );
                if ( config.open( QIODevice::ReadOnly ) )
                {
                    QByteArray data = config.readAll();
                    tempFile.write( data );                
                    importCount++;
                }
            }
            it++;
        }

        if ( importCount == 0 )
        {
            return QString();
        }

        // At the end write the global config
        QFile config( "sphinx.conf" );
        if ( config.open( QIODevice::ReadOnly ) )
        {
            QByteArray data = config.readAll();
            tempFile.write( data );                
        }
	else
        {
            DInsightConfig::Log() << "Failed to open: " << "sphinx.conf" << endl;
            return QString();
        }
        return tempFile.fileName();    
    }
    DInsightConfig::Log() << "Failed to open: " << "sphinx_temp.conf" << endl;
    
    return QString();
}


//----------------------------------------------------------------------------
/*! 
 *  Start the search deamon.
 */

void DInsightMainWindow::startSearchDeamon()
{
    QString message;

    // TODO: Create combined config file for all open imports..
    QString combinedConfigFile = createCombinedSearchConfigFile();
    if ( !combinedConfigFile.length() )
    {
        m_Ui.searchAttachmentsCheckBox->setVisible( false );
        return;
    }

    QString defaultsSearchTool = "searchd.exe --config %CONFIG_FILE%";
    QString searchTool = DInsightConfig::Get( "SEARCH_DEAMON_TOOL", defaultsSearchTool );

    searchTool = searchTool.replace( "%CONFIG_FILE%", combinedConfigFile );

    DInsightConfig::Log() << "Starting search deamon: " << searchTool << endl;

    assert( m_SearchDeamonProcess == nullptr );
    m_SearchDeamonProcess = new QProcess( this );
    QObject::connect( m_SearchDeamonProcess, &QProcess::errorOccurred, this, &DInsightMainWindow::searchDeamonError );
    QObject::connect( QCoreApplication::instance(), SIGNAL(aboutToQuit()), m_SearchDeamonProcess, SLOT(kill()));

    m_SearchDeamonProcess->start( searchTool );

#if defined WIN32
    // Make sure deamon is killed if application crashes
    if(ghJob)
    {
        if(0 == AssignProcessToJobObject( ghJob, m_SearchDeamonProcess->pid()->hProcess) )
        {
        }
    }
#endif

    int startCount = 0;
    if ( m_SearchDeamonProcess->waitForStarted() )
    {
        DImportsIterator it = m_Imports.begin();
        DImportsIterator itEnd = m_Imports.end();
        bool allOk = true;
        while ( it != itEnd )
        {
            if ( (*it)->hasChildren() )
            {
                QSqlDatabase db = QSqlDatabase::addDatabase( "QMYSQL", (*it)->databaseName() );
                if (!db.isValid())
		{
                   DInsightConfig::Log() << "Could not create database connection for: " << (*it)->databaseName() << endl;
                }
                db.setHostName( "localhost" );
                //db.setConnectOptions( "CLIENT_INTERACTIVE=TRUE" );
                db.setPort( 9306 );
		db.setUserName("root");
		db.setPassword("");
		db.setDatabaseName((*it)->databaseName());
                /* Disabled since we dont know how long time the index needs to load..
                if ( !db.open() )
                {
                    QSqlError sqlError = db.lastError();
                    DInsightConfig::log() << tr("Failed to connect to database") << ": " << sqlError.text() << endl;
                    allOk = false;
                }
                else*/
                {
                    startCount++;
                }
            }
            it++;
        }

        if ( allOk && startCount )
        {
            message = QString( tr( "Indexer started!" ) );
        }
        else if ( startCount != 0 )
        {
            message = QString( tr( "Failed to connect to indexer!" ) );
        }
    }
    else
    {
        message = QString( tr( "Failed to start indexer!" ) );
    }

    DInsightConfig::Log() << "Search deamon: " << message << endl;

    m_Ui.searchAttachmentsCheckBox->setVisible( startCount );
    m_StatusBar->showMessage( message );
}


void DInsightMainWindow::getNodesToExcludeFromSearch( DXmlParser::StringHash& nodesToExclude )
{
    const QObjectList& children = m_Ui.treeNodesSearchFilterGroupBox->children();
    foreach( QObject* child, children )
    {
        if ( child->isWidgetType() )
        {
            QCheckBox* check = (QCheckBox*)child;
            if ( !check->isChecked() )
            {
                nodesToExclude.insert( check->property( "node" ).value<const char*>() );
            }
        }
    }
}


//----------------------------------------------------------------------------
/*!
 *  The tree item label is derived from the XML tag name.
 *  Two regular expressions can change the name. The regular expressions are
 *  defined in the import format config file, using the keys:
 *   - TREEVIEW_LABEL_REGEXP: Composes label that can include child node data
 *   - TREEVIEW_NODE_REGEXP: Replaces XML tag name
 */

QString DInsightMainWindow::GetTreeItemLabel( DTreeItem *item )
{
    QString nodeName = QString( item->m_Text );
    const DRegExps& labelRegExp = item->labelRegExp();
    const DRegExps& nodeRegExp = item->nodeRegExp();

    static QRegularExpression getVariables("%(.*?)%");
    static QString nodeVariable( "%node%" );

    bool foundLabelMatch = false;
    QString label;
    for ( int i = 0; i < labelRegExp.length(); i += 2 )
    {
        QRegularExpressionMatch match = labelRegExp.at(i).match( nodeName );
        if ( match.hasMatch() )
        {
            label = labelRegExp.at(i+1).pattern();

            // extract all varable names
            QRegularExpressionMatchIterator it = getVariables.globalMatch( label );
            while ( it.hasNext() )
            {
                QRegularExpressionMatch match = it.next();
                QString word = match.captured(1);

                if ( word == "node" )
                {
                    DInsightMainWindow::ReplaceString( nodeName, nodeRegExp );
                    label.replace( nodeVariable, nodeName );
                }
                else
                {
                    // Search in nodes
                    DLeafNodesIterator nodeIt = item->m_Nodes.begin();
                    bool nodeFound = false;
                    while ( nodeIt != item->m_Nodes.end() )
                    {
                        if ( QString((*nodeIt)->m_Key) == word )
                        {
                            label.replace( QString("%%1%").arg( word ), (*nodeIt)->m_Value );
                            nodeFound = true;
                            break;
                        }
                        nodeIt++;
                    }

                    if ( !nodeFound )
                    {
                        label = item->m_Text; // Default to text from XML if not found
                    }
                }

            }
            foundLabelMatch = true;
            break;
        }
    }

    if ( !foundLabelMatch )
    {
        label = nodeName;
        DInsightMainWindow::ReplaceString( label, nodeRegExp );
    }

    return label;
}


/****************************************************************************/
/*! \class DTableSearchResultDelegate dinsightmainwindow.cpp
 *  \ingroup Insight
 *  \brief Controls search result table items look'n'feel
 * 
 */

//----------------------------------------------------------------------------
/*! 
 *  Constructor.
 */

DTableSearchResultDelegate::DTableSearchResultDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{}


//----------------------------------------------------------------------------
/*! 
 *  Draw the search result item.
 */

void DTableSearchResultDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const 
{
    auto options = option;
    initStyleOption(&options, index);

    painter->save();

    QTextDocument doc;
    doc.setHtml(options.text);
    doc.setTextWidth(options.rect.width());
    
    //QTextOption textOption = doc.defaultTextOption();
    //textOption.setWrapMode( QTextOption::WrapAtWordBoundaryOrAnywhere );    
    //doc.setDefaultTextOption( textOption );

    options.text = "";
    options.widget->style()->drawControl(QStyle::CE_ItemViewItem, &option, painter);

    painter->translate(options.rect.left(), options.rect.top());
    QRect clip(0, 0, options.rect.width(), options.rect.height());
    doc.drawContents(painter, clip);

    painter->restore();
}


//----------------------------------------------------------------------------
/*! 
 *  Get size hint.
 */

QSize DTableSearchResultDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const 
{
    QStyleOptionViewItemV4 options = option;
    initStyleOption(&options, index);

    QTextDocument doc;
    doc.setHtml(options.text);
    // Dont wrap first column
    doc.setTextWidth( index.column() == 0 ? -1 : options.rect.width() );
    return QSize(doc.idealWidth(), doc.size().height());
}
