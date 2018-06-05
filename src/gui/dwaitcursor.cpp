/***************************************************************************
**
**  Implementation of the DWaitCursor class
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
