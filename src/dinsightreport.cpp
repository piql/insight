/***************************************************************************
**
**  Implementation of the DInsightReport class
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
#include    "dinsightreport.h"
#include    "dinsightconfig.h"
#include    "dwaitcursor.h"

//  QT INCLUDES
//
#include    <QDir>
#include    <QDateTime>
#include    <QPrinter>
#include    <QTextDocument>

/****************************************************************************/
/*! \class DInsightReport dinsightreport.h
 *  \ingroup Insight
 *  \brief Report generated from XML subset
 *
 *  Create a HTML report.
 */

//----------------------------------------------------------------------------
/*! 
 *  Construct report object. Reports can be gererated on two formats:
 *  XML or HTML.
 */

DInsightReport::DInsightReport( const ReportFormat& format )
  : m_Format( format )
{
}


//----------------------------------------------------------------------------
/*! 
 *  Add report header.
 */

void DInsightReport::addHeader( const QString& label, int level )
{
    m_HeaderStack.push(label);

    if ( isHtml() )
    {
        // Add report heading - based on level
        QStringList levelBorder;
        levelBorder << "hr" << "hr";
        QList<bool> levelUpperCase;
        levelUpperCase << true << false;
        QStringList levelTag;
        levelTag << "h1" << "h2" << "h3" << "h4" << "h4" << "h4" << "h4" << "h4" << "h4" << "h4" << "h4";

        QString header = label;
        if ( level < levelUpperCase.size() )
        {
            if ( levelUpperCase[level] )
            {
                header = header.toUpper();
            }
        }

        QString levelStartTag;
        QString levelEndTag;
        if ( level < levelTag.size() )
        {
            levelStartTag = "<" + levelTag[level] + ">";
            levelEndTag = "</" + levelTag[level] + ">";
        }

        m_Text += levelStartTag + header + levelEndTag + "\n";
        if ( level < levelBorder.size() )
        {
            m_Text += "<" + levelBorder[level] + ">";
        }
    }
    else
    {
        m_Text += "<" + label + ">" + "\n";
    }
}


//----------------------------------------------------------------------------
/*! 
 *  Start table section.
 */

void DInsightReport::startTable( int /*level*/ )
{
    if ( isHtml() )
    {
        m_Text += "<table cellspacing='3'>\n";
    }
}


//----------------------------------------------------------------------------
/*! 
 *  Add table row.
 */

void DInsightReport::addRow( const QString& label, const QString& value )
{
    if ( isHtml() )
    {
        m_Text += "<tr><td>" + label + "</td><td>" + value + "<td></tr>" + "\n";
    }
    else
    {
        m_Text += "<" + label + ">" + value + "</" + label + ">" + "\n";
    }
}


//----------------------------------------------------------------------------
/*! 
 *  Add table row.
 */

void DInsightReport::addRow( const QString& label )
{
    if ( isHtml() )
    {
        m_Text += "<tr><td>" + label + "</td></tr>" + "\n";
    }
}


//----------------------------------------------------------------------------
/*! 
 *  End table section.
 */

void DInsightReport::endTable( int /*level*/ )
{
    if ( isHtml() )
    {
        m_Text += "</table><br>\n";
        m_Text += "\n";
    }
    else
    {
        QString l = m_HeaderStack.pop();
        m_Text += "</" + l + ">\n";
    }
}


//----------------------------------------------------------------------------
/*! 
 *  Return report as text.
 */

QString& DInsightReport::text()
{
    return m_Text;
}


//----------------------------------------------------------------------------
/*! 
 *  Get name of XML reports.
 */

QString DInsightReport::getXmlReportName()
{
    return tr( "import.xml" );
}


//----------------------------------------------------------------------------
/*! 
 *  Return name of report output dir.
 */

QString DInsightReport::getReportsRootDir()
{
    QString root = DInsightConfig::Get( "REPORTS_DIR", QString(".") + QDir::separator() + tr("reports") );
    //root = QDir::fromNativeSeparators( root );
    root.replace('\\','/');
    return root;
}


//----------------------------------------------------------------------------
/*! 
 *  Get reports dir on format: REPORTS_DIR/YYYY/MM/DD/HHMMSS/
 *
 *  Note: Date comes from QDateTime::currentDateTime, so this function shold only
 *  be used once per file import to ensure consistent file location.
 */

QString DInsightReport::getReportsDir()
{
    QString root = getReportsRootDir();
    if ( !root.endsWith( QDir::separator() ) )
    {
        root.append( QDir::separator() );
    }

    return getReportsDir( root );
}

QString DInsightReport::getReportsDir( const QString& r )
{
    QString root = r;
    if ( !root.endsWith( QDir::separator() ) )
    {
        root.append( QDir::separator() );
    }

    QDateTime date = QDateTime::currentDateTime();

    QString folder = QString("%1/%2/%3/%4/")
        .arg( date.date().year() )
        .arg( date.date().month() ) 
        .arg( date.date().day() ) 
        .arg( date.time().toString("HHmmss") );

    return QDir::fromNativeSeparators( root + folder );
}


//----------------------------------------------------------------------------
/*! 
 *  Save as PDF.
 */

bool DInsightReport::save( const QString& fileName )
{
    if ( isHtml() )
    {
        QPrinter printer( QPrinter::PrinterResolution );
        printer.setOutputFormat( QPrinter::PdfFormat );
        QPageLayout pageLayout = printer.pageLayout();
        pageLayout.setPageSize(QPageSize::A4);
        printer.setPageLayout( pageLayout );
        printer.setOutputFileName( fileName );

        return print( printer );
    }
    else
    {
        QFile out( fileName );
        if ( !out.open( QIODevice::WriteOnly | QIODevice::Text ) )
        {
            return false;
        }
        QTextStream stream( &out );
        stream << m_Text << Qt::endl;
        return out.flush();
    }
}


//----------------------------------------------------------------------------
/*! 
 *  Print report.
 */

bool DInsightReport::print( QPrinter& printer )
{
    DWaitCursor wait;

    QTextDocument doc;
    doc.setHtml( m_Text );
    doc.setPageSize( printer.pageRect(QPrinter::Unit::Point).size() ); // This is necessary if you want to hide the page number
    doc.print( &printer );

    return true;
}


//----------------------------------------------------------------------------
/*! 
 *  Return true if report is in HTML format.
 */

bool DInsightReport::isHtml() const
{
    return m_Format == REPORT_FORMAT_HTML;
}



