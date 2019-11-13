/*****************************************************************************
**
**  Creation date:  2017-06-12
**  Created by:     Ole Liabo
**
**
**  Copyright (c) 2017 Piql AS. All rights reserved.
**
*****************************************************************************/

//  PROJECT INCLUDES
//
#include    "gui/dinsightmainwindow.h"
#include    "dinsightconfig.h"
#include    "drunguard.h"

//  QT INCLUDES
//
#include    <QApplication>
#include    <QTranslator>
#include    <QSettings>
#include    <QMessageBox>

//  PLATFORM INCLUDES
//
#if defined WIN32
#include    <Windows.h>
#include    <Dbghelp.h>
#endif

#if defined WIN32
HANDLE ghJob = NULL;
#endif

/****************************************************************************/
/*! \defgroup Insight NOARK-5 inspection tool
 *  \ingroup ClientTools
 *
 *  Load NOARK-5 archives. Search for data and create reports.
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
    0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL ); 

    HMODULE h = ::LoadLibrary(L"DbgHelp.dll");
    PDUMPFN pFn = (PDUMPFN)GetProcAddress(h, "MiniDumpWriteDump");

    if( ( hFile != NULL ) && ( hFile != INVALID_HANDLE_VALUE ) ) 
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
    ghJob = CreateJobObject( NULL, NULL); // GLOBAL
    if( ghJob == NULL)
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
    qtApp.setApplicationVersion( "v1.0.0-rc1" );

    QString language = DInsightConfig::get( "LANGUAGE", "en" );
    QString languageFile = QString("insight_%1").arg( language );

    // Localization support
    QTranslator translator;
    translator.load( languageFile );
    /*
    if ( !ok )
    {
        QMessageBox::warning( NULL, "Failed to load translation file", QString("Failed to load translation file: %1.").arg( languageFile ) );
        return 1;
    }
    */

    qtApp.installTranslator(&translator);

    DRunGuard guard( "insight_unique_key" );
    if ( !guard.tryToRun() )
    {
        QMessageBox::warning( 
            NULL, "Application already started", 
            QString( "Only one instance of the application can run at the same time.") );
        return 1;
    }

    // Start GUI
    DInsightMainWindow window;
    window.setWindowTitle( DInsightConfig::get( "WINDOW_TITLE", "KDRS Innsyn" ) );
    window.show();
    return qtApp.exec();
}
