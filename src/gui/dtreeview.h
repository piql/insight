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
**  Copyright (c) 2017 Piql AS. All rights reserved.
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
