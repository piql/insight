/***************************************************************************
**
**  Implementation of the DAttachmentIndexer class
**
**  Creation date:  2017/12/04
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
#include    "dattachmentindexer.h"
#include    "dinsightconfig.h"

//  QT INCLUDES
//
#include    <QDir>
#include    <QFileInfo>
#include    <QProcess>
#include    <QMessageBox>
#include    <QTextStream>
#include    <QByteArray>
#include    <QRunnable>
#include    <QThreadPool>
#include    <QCryptographicHash>


/****************************************************************************/
/*! \class DPdf2Txt dattachmentindexer.cpp
 *  \ingroup Insight
 *  \brief Runnable executing PDF to text conversion.
 */

class DPdf2Txt : public QRunnable
{
public:
    DPdf2Txt( DAttachmentIndexer* indexer, const QString& command, const QString& outFile, const QString& attachment, unsigned int index );

private:
    void run();

    QString             m_Command;
    QString             m_OutFile;
    DAttachmentIndexer* m_Indexer;
    QString             m_Attachment;
    unsigned int        m_Index;
};


//----------------------------------------------------------------------------
/*! 
 *  Constructor.
 */

DPdf2Txt::DPdf2Txt( DAttachmentIndexer* indexer, const QString& command, const QString& outFile, const QString& attachment, unsigned int index )
  : m_Command( command ),
    m_OutFile( outFile ),
    m_Indexer( indexer ),
    m_Attachment( attachment ),
    m_Index( index )
{
}


//----------------------------------------------------------------------------
/*! 
 *  Runnable thread function.
 */

void DPdf2Txt::run()
{
    QProcess toolExe;

    // Execute
    toolExe.start( m_Command );
    toolExe.waitForFinished();

    // Check size of out file
    bool emptyFile = false;
    QFile file( m_OutFile );
    if ( !file.open(QIODevice::ReadOnly | QIODevice::Text) )
    {
        emptyFile = true;
    }
    else
    {
        QString fileContent =  QString( file.readAll() );
        QString fileContentTrimmed = fileContent.trimmed();
        emptyFile = fileContentTrimmed.length() == 0;
    }
    
    
    m_Indexer->toolFinished( &toolExe, emptyFile, m_Attachment, m_Index, m_Command );
}


/****************************************************************************/
/*! \class DAttachmentIndexer dattachmentindexer.h
 *  \ingroup Insight
 *  \brief Indexing of attachments
 */

//----------------------------------------------------------------------------
/*! 
 *  Constructor.
 */

DAttachmentIndexer::DAttachmentIndexer( 
    const QString& rootDir,  
    DAttachmentParser* attachmentParser,
    const QString& reportFolder, 
    const QString& attachmentsFolder,
    const QString& xmlFileName )
    : m_RootDir( rootDir ),
      m_AttachmentParser( attachmentParser ),
      m_ReportFolder( reportFolder ),
      m_AttachmentsFolder( attachmentsFolder ),
      m_XmlFileName( xmlFileName ),
      m_AttachmentsSinceLastProgress( 0 ),
      m_AttachmentConvertCount( 0 ),
      m_Error( false )
{
    QString convertLogFileName = m_ReportFolder + tr( "attachments.log" );
    m_ConvertLogFile.setFileName( convertLogFileName );
    m_ConvertLogFile.open( QFile::WriteOnly | QIODevice::Text );
    m_ConvertLog = new QTextStream( &m_ConvertLogFile );
}


//----------------------------------------------------------------------------
/*! 
 *  Destructor.
 */

DAttachmentIndexer::~DAttachmentIndexer()
{
    delete m_ConvertLog;
}


//----------------------------------------------------------------------------
/*! 
 *  Thread entry point.
 */

void DAttachmentIndexer::run()
{
    // Run conversion tool on attachments
    QString defaultPdfTool = "pdf2text -enc UTF-8 %INFILE% %OUTFILE%";
    QString pdfToolOriginal = DInsightConfig::Get( "PDF_TO_TEXT_TOOL", defaultPdfTool );

    DAttachments& attachments = m_AttachmentParser->attachments();
    DAttachmentIndexes& attachmentsFound = m_AttachmentParser->attachmentsFound();

    foreach( unsigned int i, attachmentsFound )
    {
        DAttachment* a = attachments.at( i );
        QString attachment = a->m_FileName;

        attachment = QDir::fromNativeSeparators(attachment).trimmed();
        QFileInfo info( attachment );
        if ( info.suffix().toLower() == "pdf" )
        {
            QString infile = DAttachmentParser::AttachmentPath( attachment, m_RootDir );
            QString tool = pdfToolOriginal;
            tool.replace( "%INFILE%", QDir::toNativeSeparators(infile) );
            QString outFile = DAttachmentIndexer::attachmentConvertFileName( m_AttachmentsFolder, i );

            QDir dir;
            dir.mkpath( QFileInfo(outFile).dir().path() );
            tool.replace( "%OUTFILE%", QDir::toNativeSeparators(outFile) );

            // Execute
            DPdf2Txt *pdfRunner = new DPdf2Txt( this, tool, outFile, attachment, i );

            // QThreadPool takes ownership and deletes 'pdfRunner' automatically
            QThreadPool::globalInstance()->start( pdfRunner );
        }

        if (isInterruptionRequested())
        {
            QThreadPool::globalInstance()->clear();
            break;
        }
    }



    // Wait for all threads to complete
    while ( !QThreadPool::globalInstance()->waitForDone( 1000 ) )
    {
        if ( isInterruptionRequested() )
        {
            QThreadPool::globalInstance()->clear();
        }
    }

    if ( isInterruptionRequested() )
    {
        cleanupOnCancel();
        return;
    }
    
    // Next step is to launch indexer...
    QString defaultIndexerTool = "index.cmd %REPORTS_DIR% %ATTACHMENT_DIR% %NAME%";
    QString indexerTool = DInsightConfig::Get( "INDEXER_TOOL", defaultIndexerTool );
    QProcess indexerExe;

    indexerTool = indexerTool.replace( "%REPORTS_DIR%", QDir::toNativeSeparators( m_ReportFolder ) );
    indexerTool = indexerTool.replace( "%ATTACHMENT_DIR%", QDir::toNativeSeparators( m_AttachmentsFolder ) );
    indexerTool = indexerTool.replace( "%NAME%", indexName( m_XmlFileName ) );
    
   
    *m_ConvertLog << tr("Starting indexer") << ": " << indexerTool << endl;

    indexerExe.start( indexerTool );
    if ( indexerExe.waitForStarted() )
    {
        emit indexerStarted();

        while ( !indexerExe.waitForFinished( 1000 ) )
        {
            if (isInterruptionRequested())
            {
                *m_ConvertLog << tr("Indexer interrupted") << endl;
                indexerExe.kill();
                indexerExe.waitForFinished();
                break;
            }
        }
    }

    if ( indexerExe.error() == QProcess::FailedToStart )
    {
        *m_ConvertLog << tr("Failed to start") << ": " << indexerTool << endl;
        m_Error = true;
    }
    else if ( indexerExe.exitCode() != 0 )
    {
        *m_ConvertLog << tr("Indexer failed") << ": " << endl;
        QByteArray output = indexerExe.readAllStandardError();
        *m_ConvertLog << QString(output).trimmed() << endl;
        m_Error = true;
    }

    if ( m_Error )
    {
        cleanupOnCancel();
    }
}


//----------------------------------------------------------------------------
/*! 
 *  Return number of attachments found.
 */

unsigned int DAttachmentIndexer::attachmentsCount()
{
    return (unsigned int)m_AttachmentParser->attachmentsFound().size();
}


//----------------------------------------------------------------------------
/*! 
 *  Return list of attachments that failed to convert.
 */

const QStringList& DAttachmentIndexer::attachmentsFailedToConvert()
{
    return m_AttachmentsFailedToConvert;
}


//----------------------------------------------------------------------------
/*! 
 *  Return list of attachments that where empty.
 */

const QStringList& DAttachmentIndexer::attachmentsEmpty()
{
    return m_AttachmentsEmpty;
}


//----------------------------------------------------------------------------
/*! 
 *  Returns true if error where found during convert.
 */

bool DAttachmentIndexer::error()
{
    return m_Error;
}


//----------------------------------------------------------------------------
/*! 
 *  Get convert filename from index.
 */

QString DAttachmentIndexer::attachmentConvertFileName( const QString& attachmentsFolder, unsigned int index )
{
    QString outFile = attachmentsFolder;

    if ( outFile.at(outFile.length() - 1) != QDir::separator() )
    {
        outFile.append( QDir::separator() );
    }
    outFile += QString("%1").arg( index, 10, 10, QLatin1Char('0') ) + ".txt";

    return outFile;
}


//----------------------------------------------------------------------------
/*! 
 *  Callback function from runner when convert is complete (or failed with 
 *  error).
 */

void DAttachmentIndexer::toolFinished( QProcess* toolExe, bool emptyFile, const QString& attachment, unsigned int index, const QString& command )
{
    QMutexLocker lock( &m_Mutex );

    if ( toolExe->error() == QProcess::FailedToStart )
    {
        m_AttachmentsFailedToConvert.push_back( attachment );
        *m_ConvertLog << command << ": " << tr("ERROR") << " " << tr("Failed to start") << endl;
    }
    else if ( toolExe->exitCode() != 0 )
    {
        m_AttachmentsFailedToConvert.push_back( attachment );
        *m_ConvertLog << command << ": " << tr("ERROR") << " (" << toolExe->exitCode() << ")" << endl;
        *m_ConvertLog << tr( "OUTPUT" ) << ": ";

        QByteArray output = toolExe->readAllStandardError();
        *m_ConvertLog << QString(output).trimmed() << endl;
    }
    else
    {
        *m_ConvertLog << command << ": " << tr("OK") << endl;
        m_ConvertOK.push_back( index );
    }

    if ( emptyFile && toolExe->error() != QProcess::FailedToStart)
    {
        *m_ConvertLog << attachment << ": " << tr("Convert result is empty (has no text)") << endl;
        m_AttachmentsEmpty.push_back( attachment );
    }

    DAttachmentIndexes& attachmentsFound = m_AttachmentParser->attachmentsFound();
    int progressFrequency = (int)attachmentsFound.size() / 100; 

    m_AttachmentConvertCount++;
    if ( m_AttachmentsSinceLastProgress == progressFrequency )
    {
        m_AttachmentsSinceLastProgress = 0;
        emit progress( m_AttachmentConvertCount / (float)attachmentsFound.size() );
    }
    else
    {
        m_AttachmentsSinceLastProgress++;
    }
}


//----------------------------------------------------------------------------
/*! 
 *  Cleanup after user cancel.
 */

void DAttachmentIndexer::cleanupOnCancel()
{
    QDir d( m_ReportFolder);
    d.remove( "sphinx_index.conf" );
}


//----------------------------------------------------------------------------
/*! 
 *  Get name of index, used by search deamon to get unique name for index.
 */

QString DAttachmentIndexer::indexName( const QString& xmlFileName )
{
    return QString("i") + QCryptographicHash::hash( xmlFileName.toUtf8(), QCryptographicHash::Md5 ).toHex();
}
