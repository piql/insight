/***************************************************************************
**
**  Implementation of the DTreeView class
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
