#ifndef DINSIGHTREPORTWINDOW_H
#define DINSIGHTREPORTWINDOW_H

/*****************************************************************************
**  
**  Definition of the DInsightReportWindow class
**
**  Creation date:  2017/08/16
**  Created by:     Ole Liabo
**
**
**  Copyright (c) 2017 Piql AS. All rights reserved.
**
*****************************************************************************/

//  PROJECT INCLUDES
//
#include    "ui_dinsightreportwindow.h"
#include    "dinsightreport.h"


//  QT INCLUDES
//


//  FORWARD DECLARATIONS
//
class QPrinter;

//============================================================================
// CLASS: DInsightReportWindow

class DInsightReportWindow : public QDialog
{
    Q_OBJECT
public:
    DInsightReportWindow( DInsightReport& report, QStringList& attachments );
    virtual ~DInsightReportWindow();


private slots:
    void okButtonClicked();
    void saveButtonClicked();
    void emailButtonClicked();
    void printButtonClicked();

private:
    bool createPdfReport( const QString& fileName );
    bool createAttachmentArchive( const QString& fileName, const QString& reportFileName );
    bool printReport( QPrinter& printer );
    bool printAttachments( QPrinter& printer );
    bool printPdfAttachment( QPainter& painter, QPrinter& printer, const QString& attachment );
    bool printImageAttachment( QPainter& painter, QPrinter& printer, const QString& attachment );
    bool printTextAttachment( QPainter& painter, QPrinter& printer, const QString& attachment );

private:
    Ui::IngestToolReportWindow m_Ui;
    DInsightReport&            m_Report;
    QStringList                m_Attachments;
};


#endif // DINSIGHTREPORTWINDOW_H
