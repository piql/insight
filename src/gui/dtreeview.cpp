/***************************************************************************
**
**  Implementation of the DTreeView class
**
**  Creation date:  2017/12/18
**  Created by:     Ole Liabo
**
**
**  Copyright (c) 2017 Piql AS. All rights reserved.
**
***************************************************************************/

//  PROJECT INCLUDES
//
#include    "dtreeview.h"

//  QT INCLUDES
//


/****************************************************************************/
/*! \class DTreeView dtreeview.h
 *  \ingroup Insight
 *  \brief Tree widget 
 */


//----------------------------------------------------------------------------
/*! 
 *  Constructor.
 */

DTreeView::DTreeView(QWidget* parent/*=nullptr*/)
  : QTreeView( parent )
{

}


//----------------------------------------------------------------------------
/*! 
 *  Key pressed in tree view.
 */

void DTreeView::keyPressEvent(QKeyEvent *event)
{
    // Get current item
    QModelIndex before = currentIndex();
    QTreeView::keyPressEvent( event );
    QModelIndex after = currentIndex();
    if ( before != after )
    {
        // Set focus on new items
        activated( after );
    }
}


//----------------------------------------------------------------------------
/*! 
 *  Received rowsInserted signal.
 */

void DTreeView::rowsInserted(const QModelIndex & parent, int start, int end)
{
    QTreeView::rowsInserted(parent, start, end);
}
