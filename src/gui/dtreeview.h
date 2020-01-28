#ifndef DTREEVIEW_H
#define DTREEVIEW_H

/*****************************************************************************
**  
**  Definition of the DTreeView class
**
**  Creation date:  2017/12/18
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

//  QT INCLUDES
//
#include   <QTreeView>

//  FORWARD DECLARATIONS
//

//============================================================================
// CLASS: DTreeView

class DTreeView : public QTreeView
{
    Q_OBJECT

public:
    explicit DTreeView(QWidget* parent=nullptr);

protected:
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void rowsInserted(const QModelIndex & parent, int start, int end);
};


#endif // DTREEVIEW_H
