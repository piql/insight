#ifndef DATTACHMENTINDEXER_H
#define DATTACHMENTINDEXER_H

/*****************************************************************************
**  
**  Definition of the DAttachmentIndexer class
**
**  Creation date:  2017/12/04
**  Created by:     Ole Liabo
**
**
**  Copyright (c) 2017 Piql AS. All rights reserved.
**
*****************************************************************************/

//  PROJECT INCLUDES
//
#include    "dattachmentparser.h"

//  QT INCLUDES
//
#include    <QThread>
#include    <QFile>

//  FORWARD DECLARATIONS
//
class DTreeItem;
class QProcess;
class QTextStream;

//============================================================================
// CLASS: DAttachmentIndexer

class DAttachmentIndexer : public QThread
{
    Q_OBJECT
public:

    DAttachmentIndexer(
        const QString& rootDir,  
        DAttachmentParser* attachmentParser, 
        const QString& reportFolder, 
        const QString& attachmentsFolder,
        const QString& xmlFileName );
    ~DAttachmentIndexer();

    unsigned int            attachmentsCount();
    const QStringList&      attachmentsFailedToConvert();
    const QStringList&      attachmentsEmpty();
    bool                    error();

public:
    static QString          attachmentConvertFileName( const QString& attachmentsFolder, unsigned int index );
    static QString          indexName( const QString& xmlFileName );

signals:
    void                    progress( float progress );    
    void                    indexerStarted();

private:
    void                    run();
    void                    toolFinished( QProcess* toolExe, bool emptyFile, const QString& attachment, unsigned int index, const QString& command );
    void                    cleanupOnCancel();

private:
    QString                m_RootDir;
    DAttachmentParser*     m_AttachmentParser;
    QString                m_ReportFolder;
    QString                m_AttachmentsFolder;
    QString                m_XmlFileName;
    QStringList            m_AttachmentsFailedToConvert;
    QStringList            m_AttachmentsEmpty;
    DAttachmentIndexes     m_ConvertOK;
    QMutex                 m_Mutex;
    QFile                  m_ConvertLogFile;
    QTextStream*           m_ConvertLog;
    int                    m_AttachmentsSinceLastProgress;
    int                    m_AttachmentConvertCount;
    bool                   m_Error;

    friend class DPdf2Txt;

};


#endif // DATTACHMENTINDEXER_H
