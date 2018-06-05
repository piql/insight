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
**  Copyright (c) 2010 Piql. All rights reserved.
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
    QAboutDialog( const QString& applicationName, const QString& infoText, const QString& bitmapFilename, QWidget* parent=NULL );
    QAboutDialog( const QString& applicationName, const QString& bitmapFilename, QWidget* parent=NULL );
    ~QAboutDialog();

private:
    Ui::AboutDialog m_Ui;
};
//----------------------------------------------------------------------------
// INLINE: 

//===================================EOF======================================

#endif /* QABOUTDIALOG_H */
