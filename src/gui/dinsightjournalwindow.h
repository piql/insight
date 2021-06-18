#ifndef DINSIGHTJOURNALWINDOW_H
#define DINSIGHTJOURNALWINDOW_H

/*****************************************************************************
**  
**  Definition of the DInsightJournalWindow class
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
*****************************************************************************/

//  PROJECT INCLUDES
//
#include    "ui_dinsightjournalwindow.h"


//  QT INCLUDES
//
#include <QDialog>
#include <QThread>

//  FORWARD DECLARATIONS
//
class DJournal;
class DImport;

//============================================================================
// CLASS: DInsightJournalWindow

class DInsightJournalWindow : public QDialog
{
    Q_OBJECT
public:
    DInsightJournalWindow( DJournal* journal, DImport* import );
    virtual ~DInsightJournalWindow();

public:
    static bool GeneratePdf( const QString& outFileName, const DJournal* journal, const QString& tempDir );

private slots:
    void nextButtonClicked();
    void prevButtonClicked();
    void selectButtonClicked();
    void selectAllButtonClicked();
    void unselectAllButtonClicked();
    void viewPdfButtonClicked();

private:
    void setPage( unsigned int page );

private:
    DJournal *      m_Journal;
    DImport *       m_Import;
    unsigned int    m_CurrentPage;

private:
    Ui::InsightJournalWindow m_Ui;
};

class DGenerateThread : public QThread
{
    Q_OBJECT

    void run() override;

public:
    bool m_Ok;
    QString m_OutFileName;
    DJournal* m_Journal;
    QString  m_TempDir;
};

#endif // DINSIGHTJOURNALWINDOW_H
