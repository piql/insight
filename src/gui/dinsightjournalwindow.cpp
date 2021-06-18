/***************************************************************************
**
**  Implementation of the DInsightJournalWindow class
**
**  Creation date:  2021/05/09
**  Created by:     Ole Liabo
**
**
**  Copyright (c) 2021 Piql AS.
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
#include    "dinsightjournalwindow.h"
#include    "dattachmentparser.h"
#include    "dimportformat.h"
#include    "dimport.h"
#include    "dinsightconfig.h"

//  QT INCLUDES
//
#include    <QDialogButtonBox>
#include    <QProcess>
#include    <QMessageBox>
#include    <QFile>
#include    <QTemporaryFile>
#include    <QDesktopServices>
#include    <QUrl>
#include    <QFileInfo>
#include    <QDir>
#include    <QDateTime>
#include    <QProgressDialog>

/****************************************************************************/
/*! \class DInsightJournalWindow dinsighjournalwindow.h
 *  \ingroup Insight
 *  \brief Journal Window dialog
 * 
 *  The journal window shows the attachments (pages) connected to a journal 
 *  node. Typically this will be JPGs or similar. The window allows to browse the
 *  pages and make selection for export.
 */
 
//===================================
//  P U B L I C   I N T E R F A C E
//===================================


//----------------------------------------------------------------------------
/*! 
 *  Constructor.
 */

DInsightJournalWindow::DInsightJournalWindow( DJournal* journal, DImport* import )
  : m_Journal( journal ),
    m_Import( import ),
    m_CurrentPage( 0 )
{
    m_Ui.setupUi( this );    

    // Set window title
    setWindowTitle( tr("Journal") );

    // Signals and slots. The GUI components emits signals that are handled by the slots.
    connect( m_Ui.buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept );
    QObject::connect( m_Ui.nextButton, SIGNAL(clicked()), this, SLOT(nextButtonClicked()) );
    QObject::connect( m_Ui.prevButton, SIGNAL(clicked()), this, SLOT(prevButtonClicked()) );
    QObject::connect( m_Ui.selectPageButton, SIGNAL(clicked()), this, SLOT(selectButtonClicked()) );
    QObject::connect( m_Ui.selectAllButton, SIGNAL( clicked() ), this, SLOT( selectAllButtonClicked() ) );
    QObject::connect( m_Ui.unselectAllButton, SIGNAL( clicked() ), this, SLOT( unselectAllButtonClicked() ) );
    QObject::connect( m_Ui.viewPdfButton, SIGNAL( clicked() ), this, SLOT( viewPdfButtonClicked() ) );

    m_Ui.nextButton->setEnabled( m_Journal->m_Pages.size() > 1 );
    m_Ui.prevButton->setEnabled( false );

    m_Ui.page->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );
    m_Ui.page->setScaledContents( true );

    setPage( m_CurrentPage );
}


//----------------------------------------------------------------------------
/*!                                                                              
 *  Destructor.
 */

DInsightJournalWindow::~DInsightJournalWindow()
{
}

void DInsightJournalWindow::nextButtonClicked()
{
    m_CurrentPage++;
    setPage( m_CurrentPage );
}

void DInsightJournalWindow::prevButtonClicked()
{
    m_CurrentPage--;
    setPage( m_CurrentPage );
}

void DInsightJournalWindow::selectButtonClicked()
{
    DJournalPage& p = m_Journal->m_Pages.at( m_CurrentPage );
    p.m_Checked = !p.m_Checked;
}

void DInsightJournalWindow::selectAllButtonClicked()
{
    for ( size_t i = 0; i < m_Journal->m_Pages.size(); i++) 
    {
        m_Journal->m_Pages.at( i ).m_Checked = true;
    }
    m_Ui.selectPageButton->setChecked( true );
}

void DInsightJournalWindow::unselectAllButtonClicked()
{
    for ( size_t i = 0; i < m_Journal->m_Pages.size(); i++ )
    {
        m_Journal->m_Pages.at( i ).m_Checked = false;
    }
    m_Ui.selectPageButton->setChecked( false );
}

void DInsightJournalWindow::setPage( unsigned int page )
{
    DJournalPage& p = m_Journal->m_Pages.at( page );
    QPixmap pixmap( p.m_PageFileName );
    m_Ui.page->setPixmap( pixmap );

    m_Ui.pageInfo->setText(QString(tr("Page %1 of %2")).arg(m_CurrentPage+1).arg(m_Journal->m_Pages.size()));
    m_Ui.nextButton->setEnabled( m_Journal->m_Pages.size() - 1 > m_CurrentPage );
    m_Ui.prevButton->setEnabled( m_CurrentPage > 0 );
    m_Ui.selectPageButton->setChecked( p.m_Checked );

    if ( p.m_OcrFileName.length() )
    {
        QFileInfo ocr( p.m_OcrFileName );
        m_Ui.ocrInfo->setText( QString( tr( "OCR: %1" ) ).arg( ocr.fileName() ) );
        m_Ui.ocrInfo->setToolTip( p.m_OcrFileName );
    }
    else
    {
        m_Ui.ocrInfo->setText( QString() );
        m_Ui.ocrInfo->setToolTip( QString() );
    }
}


void DGenerateThread::run() 
    {
        m_Ok = DInsightJournalWindow::GeneratePdf( m_OutFileName, m_Journal, m_TempDir );
    }

void DInsightJournalWindow::viewPdfButtonClicked()
{
    QDir reportsDir(m_Import->reportsDir());
    QString journalFileName = tr("journal") + "-" + DInsightConfig::FileNameDatePart() + ".pdf";
    QString out = reportsDir.absoluteFilePath( journalFileName );

    QProgressDialog progress( tr("Genererer journal"), QString(), 0, 100, this );
    progress.setWindowModality( Qt::WindowModal );

    DGenerateThread thread;
    thread.m_Journal = m_Journal;
    thread.m_Ok = false;
    thread.m_OutFileName = out;
    thread.m_TempDir = m_Import->reportsDir();
    thread.start();
    while (thread.isRunning()) 
    {
        thread.wait(1000);
        progress.setValue(progress.value() == 99 ? 0 : progress.value() + 1 );
    }
    progress.hide();

    if ( !thread.m_Ok )
    {
        QMessageBox::warning(
            this, tr( "Failed to convert" ), tr( "Failed to convert to PDF, check PDF tool config in format file." ), QMessageBox::Ok );
    }
    else 
    {
        bool ok = QDesktopServices::openUrl( QUrl::fromLocalFile( out ) );
        if ( !ok ) 
        {
            QMessageBox::warning(
                this, tr( "Failed to open" ), tr( "Failed to open PDF:\n%1\n\nCheck PDF tool config in format file." ).arg(out), QMessageBox::Ok );
        }
    }
}


bool DInsightJournalWindow::GeneratePdf( const QString& outFileName, const DJournal* journal, const QString& tempDir )
{
    // Create list of selected pages
    QStringList pages;
    QStringList ocrs;

    for ( size_t i = 0; i < journal->m_Pages.size(); i++ )
    {
        const DJournalPage & p = journal->m_Pages.at( i );
        if ( p.m_Checked )
        {
            pages.push_back( p.m_PageFileName );
            if ( p.m_OcrFileName.length() )
            {
                ocrs.push_back( p.m_OcrFileName );
            }
        }
    }

    QDir reportsDir( tempDir );
    QString filesFileName = tr( "journal-files" ) + "-" + DInsightConfig::FileNameDatePart() + ".txt";
    filesFileName = reportsDir.absoluteFilePath( filesFileName );
    QString out = outFileName;

    // Create list of files
    QFile filesFile( filesFileName );
    if ( filesFile.open( QIODevice::ReadWrite ) ) {
        QTextStream stream( &filesFile );
        for ( int i = 0; i < pages.size(); i++ )
        {
            stream << QDir::toNativeSeparators( pages.at( i ) );
            if ( i < ocrs.size() )
            {
                stream << ";" << QDir::toNativeSeparators( ocrs.at( i ) );
            }
            stream << endl;
        }
        filesFile.close();
    }


    // Call tool that converts attachments to PDF
    const DImportFormat* format = journal->m_TreeItem->format();
    QString tool = format->convertToPdfTool( filesFileName, out, reportsDir.absolutePath() );
    return QProcess::execute( tool ) == 0;
}
