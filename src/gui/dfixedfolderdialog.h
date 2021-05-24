#ifndef DFIXEDFOLDERDIALOG_H
#define DFIXEDFOLDERDIALOG_H

/*****************************************************************************
**  
**  Definition of the DInsightReportWindow class
**
**  Creation date:  2021/05/07
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
#include    "ui_fixedfolderdialog.h"


//  QT INCLUDES
//


//  FORWARD DECLARATIONS
//

//============================================================================
// CLASS: DInsightReportWindow

class DFixedFolderDialog : public QDialog
{
    Q_OBJECT
public:
    DFixedFolderDialog( const QString& rootFolder, const QString& fileName, const QString& title );
    virtual ~DFixedFolderDialog();

    QString fileName();

private slots:
    void selectionChanged( const QItemSelection&, const QItemSelection& );


private:
    Ui::FixedFolderDialog m_Ui;
    QString               m_RootFolder;
         
};


#endif // DFIXEDFOLDERDIALOG_H
