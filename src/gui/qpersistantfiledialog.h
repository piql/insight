#ifndef QPERSISTANTFILEDIALOG_H
#define QPERSISTANTFILEDIALOG_H

/*****************************************************************************
**  $Id$
**
**  Provide a file open dialog that remembers it prevoius folder
**
**  Creation date:  2009-07-17
**  Created by:     Ole Liabo
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
**  This file is part of the "Piql Qt" library.
**
*****************************************************************************/

//  PROJECT INCLUDES
//

//  SYSTEM INCLUDES
//

//  QT INCLUDES
//
#include <QFileDialog>

//  NAMESPACE
//

//  FORWARD DECLARATIONS
//

//============================================================================
// CLASS: QPersistantFileDialog
class QPersistantFileDialog
{
public:
    static QString getOpenFileName ( 
        const QString & uniqueID,
        QWidget * parent = 0, 
        const QString & caption = QString(), 
        const QString & dir = QString(), 
        const QString & filter = QString(), 
        QString * selectedFilter = 0, 
        QFileDialog::Options options = 0 );
    static QStringList getOpenFileNames ( 
        const QString & uniqueID,
        QWidget * parent = 0, 
        const QString & caption = QString(), 
        const QString & dir = QString(), 
        const QString & filter = QString(), 
        QString * selectedFilter = 0, 
        QFileDialog::Options options = 0,
        bool multiSelect = true );
    static QString getSaveFileName ( 
        const QString & uniqueID,
        QWidget * parent = 0, 
        const QString & caption = QString(), 
        const QString & dir = QString(), 
        const QString & filter = QString(), 
        QString * selectedFilter = 0, 
        QFileDialog::Options options = 0,
        bool onlyDirPersistant = false );
    static QString getExistingDirectory ( 
        const QString & uniqueID,
        QWidget * parent = 0, 
        const QString & caption = QString(), 
        const QString & dir = QString() );
    static QString getFixedRootSaveFileName(
        QWidget * parent,
        const QString & caption,
        const QString & fileName,
        const QString & rootDir );
};

//----------------------------------------------------------------------------
// INLINE: QPersistantFileDialog


//============================================================================

//CV_NAMESPACE_END

//====================================EOF=====================================

#endif /* QPERSISTANTFILEDIALOG_H */
