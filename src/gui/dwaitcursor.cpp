/***************************************************************************
**
**  Implementation of the DWaitCursor class
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
#include    "dwaitcursor.h"

//  QT INCLUDES
//
#include    <QApplication>

/****************************************************************************/
/*! \class DWaitCursor dwaitcursor.h
 *  \ingroup Insight
 *  \brief Display hour glass cursor during slow operations.
 */

//----------------------------------------------------------------------------
/*! 
 *  Constructor.
 */

DWaitCursor::DWaitCursor()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
}


//----------------------------------------------------------------------------
/*! 
 *  Destructor.
 */

DWaitCursor::~DWaitCursor()
{
    QApplication::restoreOverrideCursor();
}
