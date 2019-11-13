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
**  Copyright (c) 2009 Piql. All rights reserved.
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
};

//----------------------------------------------------------------------------
// INLINE: QPersistantFileDialog


//============================================================================

//CV_NAMESPACE_END

//====================================EOF=====================================

#endif /* QPERSISTANTFILEDIALOG_H */
