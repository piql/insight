/*****************************************************************************
**
**  Creation date:  2017-06-12
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
#include    "gui/dinsightmainwindow.h"
#include    "dinsightconfig.h"
#include    "drunguard.h"
#include    "dimportformat.h"
#include    "dosxtools.h"

//  QT INCLUDES
//
#include    <QApplication>
#include    <QTranslator>
#include    <QSettings>
#include    <QMessageBox>
#include    <QDir>
#include    <QSqlDatabase>
#include    <QCommandLineParser>

//  PLATFORM INCLUDES
//
#if defined WIN32
#include    <Windows.h>
#include    <Dbghelp.h>
#endif
#include    <iostream>

#if defined WIN32
HANDLE ghJob = nullptr;
#endif

/****************************************************************************/
/*! \defgroup Insight AIP inspection tool
 *  \ingroup ClientTools
 *
 *  Load AIP archives. Search for data and create reports.
 */


#if defined WIN32
//----------------------------------------------------------------------------
/*! 
 *  Create core dump on crash.
 */

LONG WINAPI unhandledExceptionFilter( _In_ struct _EXCEPTION_POINTERS *pep )
{
    // Open the file 
    typedef BOOL (__stdcall *PDUMPFN)( 
      HANDLE hProcess, 
      DWORD ProcessId, 
      HANDLE hFile, 
      MINIDUMP_TYPE DumpType, 
      PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, 
      PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam, 
      PMINIDUMP_CALLBACK_INFORMATION CallbackParam
    );


    HANDLE hFile = CreateFileA( "insight.dmp", GENERIC_READ | GENERIC_WRITE, 
    0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr );

    HMODULE h = ::LoadLibrary(L"DbgHelp.dll");
    PDUMPFN pFn = (PDUMPFN)GetProcAddress(h, "MiniDumpWriteDump");

    if( ( hFile != nullptr ) && ( hFile != INVALID_HANDLE_VALUE ) )
    {
        // Create the minidump 

        MINIDUMP_EXCEPTION_INFORMATION mdei; 

        mdei.ThreadId           = GetCurrentThreadId(); 
        mdei.ExceptionPointers  = pep; 
        mdei.ClientPointers     = TRUE; 

        MINIDUMP_TYPE mdt       = MiniDumpNormal; 

        /*BOOL rv =*/ (*pFn)( GetCurrentProcess(), GetCurrentProcessId(), 
          hFile, mdt, (pep != 0) ? &mdei : 0, 0, 0 ); 

        // Close the file 

        CloseHandle( hFile ); 
    }
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif


//============================================================================

int  main( int argc, char* argv[] )
{
#if defined WIN32
    // Create core dump
    SetUnhandledExceptionFilter( unhandledExceptionFilter );

    // Code to make sure child processes created byt his application is killed 
    // when the application terminates (or crashes)
    ghJob = CreateJobObject( nullptr, nullptr); // GLOBAL
    if( ghJob == nullptr)
    {
    }
    else
    {
        JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = { 0 };
    
        // Configure all child processes associated with the job to terminate when the
        jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        if ( 0 == SetInformationJobObject( ghJob, JobObjectExtendedLimitInformation, &jeli, sizeof( jeli ) ) )
        {
        }
    }
#endif

    // Create application object
    QApplication qtApp( argc, argv );
    qtApp.setOrganizationName( "Piql" );
    qtApp.setOrganizationDomain("piql.com");
    qtApp.setApplicationName( "insight" );
    qtApp.setApplicationVersion( "v1.3.0" );

    QCommandLineParser cmdLine;
    cmdLine.setApplicationDescription( "Archival Package Inspector and Dissimination tool" );
    QCommandLineOption helpOption = cmdLine.addHelpOption();
    QCommandLineOption versionOption = cmdLine.addVersionOption();

    // Add options
    QCommandLineOption fileOption( QStringList() << "f" << "file", "<file> to open", "file" );
    QCommandLineOption fileFormatOption( QStringList() << "file-format", "<format> of file to open", "format" );
    QCommandLineOption autoExportOption( QStringList() << "a" << "auto-export", "Auto export to <file>", "file" );
    QCommandLineOption serverOption(QStringList() << "s" << "server", "Run in server mode", "port");
    cmdLine.addOption( fileOption );
    cmdLine.addOption( fileFormatOption );
    cmdLine.addOption( autoExportOption );
    //cmdLine.addOption( serverOption );


    QString language = DInsightConfig::Get( "LANGUAGE", "en" );
    QString languageFile = QString( "insight_%1.qm" ).arg( language );
    languageFile = DOsXTools::GetBundleResource( languageFile.toStdString() ).c_str();

    DInsightConfig::Log() << QSqlDatabase::drivers() << Qt::endl;
    DInsightConfig::Log() << QCoreApplication::libraryPaths() << Qt::endl;

    // Localization support
    QTranslator translator;
    bool ok = translator.load( languageFile );
    if ( !ok )
    {
        QMessageBox::warning(
            nullptr, "Failed to load translation file",
            QString( "Failed to load translation file: %1" ).arg( languageFile ) );
        return 1;
    }

    qtApp.installTranslator(&translator);

    DRunGuard guard( "insight_unique_key" );
    if ( !guard.tryToRun() )
    {
        QMessageBox::warning( 
            nullptr, "Application already started",
            QString( "Only one instance of the application can run at the same time." ) );
        return 1;
    }

    // Load the import formats
    DImportFormats formats;
    QString formatsDefaultDir = DOsXTools::GetBundleResource( "./formats" ).c_str();
    if ( !DImportFormats::Load( formats, formatsDefaultDir ) )
    {
        QMessageBox::warning( 
            nullptr, "Load error",
            QString( "Failed to load import formats from %1." ).arg(formatsDefaultDir) );
        return 1;
    }

    // Default format must exist
    if ( !formats.defaultFormat() )
    {
        QMessageBox::warning( 
            nullptr, "Missing format",
            QString( "The format ./formats/default.conf must exist." ) );
        return 1;
    }

    // Import report format must exist
    const DImportFormat* reportFormat = formats.find( "Report" );
    if ( !reportFormat )
    {
        QMessageBox::warning(
            nullptr, "Missing import report format",
            QString( "The format ./formats/report.conf must exist." ) );
        return 1;
    }
    DImport::SetReportFormat( reportFormat );

    // Parse command line
    cmdLine.process( qtApp );

    if ( !cmdLine.parse( QCoreApplication::arguments() ) ) {
        std::cerr << cmdLine.errorText().toStdString() << std::endl;
        return 1;
    }

    if ( cmdLine.isSet( versionOption ) )
    {
        printf( "%s %s\n", qPrintable( QCoreApplication::applicationName() ),
            qPrintable( QCoreApplication::applicationVersion() ) );
        return 0;
    }

    if ( cmdLine.isSet( helpOption ) )
    {
        cmdLine.showHelp();
        return 0;
    }

    if ( cmdLine.isSet( serverOption) ) 
    {
        // Run as server
    }
    else 
    {
        // Start GUI mode
        QString attachmentParsing = DInsightConfig::Get( "ATTACHMENT_PARSING", "ASK" );
        if ( cmdLine.isSet( autoExportOption ) )
        {
            attachmentParsing = "YES";
        }

        // Create app main window
        DInsightMainWindow window( &formats, attachmentParsing );
        window.setWindowTitle( DInsightConfig::Get( "WINDOW_TITLE", "KDRS Innsyn" ) );

        if ( cmdLine.isSet( fileOption ) )
        {
            window.importFile( cmdLine.value( fileOption ), cmdLine.value( fileFormatOption ), cmdLine.value( autoExportOption ) );
        }

        if ( !cmdLine.isSet( autoExportOption ) )
        {
            // Start GUI
            window.show();
        }
    }

    return qtApp.exec();
}
