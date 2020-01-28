#ifndef QABOUTDIALOG_H
#define QABOUTDIALOG_H

/*****************************************************************************
**  $Id$
**
**  
**
**  Creation date:  2010-12-21
**  Created by:     Matti Wennberg
**
**  Platform(s):    Intel, Linux 2.4.x, g++ 3.0
**                  Intel, Windows 2K/XP, VC++ 6.0
**
**  Dependencies:   -
**  Restrictions:   -
**  ToDo:           -
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
**  This file is part of the "Base" library.
**
*****************************************************************************/

//  PROJECT INCLUDES
//
#include "ui_qaboutdialog.h"

// CV INCLUDES

//  SYSTEM INCLUDES
//

//  NAMESPACE
//

//  FORWARD DECLARATIONS
//

//============================================================================
// CLASS: QAboutDialog
class QAboutDialog : public QDialog
{
    Q_OBJECT
public:
    QAboutDialog( const QString& applicationName, const QString& infoText, const QString& bitmapFilename, QWidget* parent=nullptr );
    QAboutDialog( const QString& applicationName, const QString& bitmapFilename, QWidget* parent=nullptr );
    ~QAboutDialog();

private:
    Ui::AboutDialog m_Ui;
};
//----------------------------------------------------------------------------
// INLINE: 

//===================================EOF======================================

#endif /* QABOUTDIALOG_H */
