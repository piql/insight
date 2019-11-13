#ifndef DINSIGHTREPORT_H
#define DINSIGHTREPORT_H

/*****************************************************************************
**  
**  Definition of the DInsightReport class
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


//  QT INCLUDES
//
#include   <QString>
#include   <QObject>
#include   <QStringList>
#include   <QStack>

//  FORWARD DECLARATIONS
//
class QPrinter;

//============================================================================
// CLASS: DInsightConfig

class DInsightReport : public QObject
{
    Q_OBJECT
public:
    enum ReportFormat { REPORT_FORMAT_HTML, REPORT_FORMAT_XML };

public:

    DInsightReport( const ReportFormat& format = REPORT_FORMAT_HTML );

    void addHeader( const QString& label, int level );
    void startTable( int level );
    void addRow( const QString& label, const QString& value );
    void addRow( const QString& label );
    void endTable( int level );
    QString& text();
    bool save( const QString& fileName );
    bool print( QPrinter& printer );

public:
    static QString getReportsRootDir();
    static QString getReportsDir();
    static QString getReportsDir( const QString& root );
    static QString getXmlReportName();

private:
    bool            isHtml() const;

private:
    QString         m_Text;
    ReportFormat    m_Format;
    QStack<QString> m_HeaderStack;
};


#endif // DINSIGHTREPORT_H
