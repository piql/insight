/*****************************************************************************
**  $Id$
**
**  Implementation of QAboutDialog class.
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
**  This file is part of the "Base" application.
**
*****************************************************************************/

//  PROJECT INCLUDES
//
#include "qaboutdialog.h"

//  SYSTEM INCLUDES
//

//  NAMESPACE
//

/****************************************************************************/
/*! \class QAboutDialog about_dialog.h
 *  \ingroup Insight
 *  \brief Class for displaying application information
 * 
 *  This generic dialog will display the application name, info and logo 
 *  bitmap as well as a Piql logo. The info text can be HTML formatted.
 *
 *  Note: Applications using this dialog must declare Q_INIT_RESOURCE(qaboutdialog)
 *  somwhere after the QApplication object is created, this ensures that the resources
 *  needed by the QAboutDialog are avaliable to the application. Example:
 *  \code
 *  QApplication app(argc, argv);
 *  Q_INIT_RESOURCE(qaboutdialog)  
 *  \endcode
 */

//============================================================================
//  P U B L I C   I N T E R F A C E
//============================================================================
/**
 * Constructor. Sets up ui.
 * 
 * \param applicationName Name of application, will be displayed in title bar.
 * \param infoText        Info text for the application, can be HTML formatted.
 * \param bitmapFilename  Application logo.
 * \param parent          Parent widget.
 * 
 * Example info string:
 * \code
 * QString infoText;
 * infoText += "Version: " + QCoreApplication::applicationVersion() + "<br />";
 * infoText += "Copyright Piql 2010";
 * infoText += "<p>Support: <a href=\"mailto:support@cinevation.net\">support@conevation.net</a></p>";
 * infoText += "<p><a href=\"www.cinevation.net\">www.cinevation.net</a></p>";
 * \endcode
 */
QAboutDialog::QAboutDialog(const QString& applicationName, const QString& infoText, const QString& bitmapFilename, QWidget* parent /*=nullptr*/ )
: QDialog(parent)
{
    // Create user interface
    m_Ui.setupUi(this);

    // Set info label
    m_Ui.infoTextLabel->setText( infoText );
    m_Ui.infoTextLabel->setTextInteractionFlags( Qt::TextBrowserInteraction );
    m_Ui.infoTextLabel->setOpenExternalLinks( true );

    // Put application name in title bar
    this->setWindowTitle( "About " + applicationName );

    // Set bitmap for application
    if ( bitmapFilename.length() )
    {
        QPixmap pixmap( bitmapFilename );
        m_Ui.applicationLogoBitmap->setPixmap( pixmap );  
    }
    else
    {
        m_Ui.applicationLogoBitmap->setScaledContents(true);
    }
}

/**
 * Constructor. Sets up ui with default info text.
 * 
 * \param applicationName Name of application, will be displayed in title bar.
 * \param bitmapFilename  Application logo.
 * \param parent          Parent widget.
 * 
 * The application info will default to:
 * \code
 * QString infoText;
 * infoText += "Version: " + QCoreApplication::applicationVersion() + "<br />";
 * infoText += "Copyright Piql 2010";
 * infoText += "<p>Support: <a href=\"mailto:support@cinevation.net\">support@conevation.net</a></p>";
 * infoText += "<p><a href=\"www.cinevation.net\">www.cinevation.net</a></p>";
 * \endcode
 */
QAboutDialog::QAboutDialog(const QString& applicationName, const QString& bitmapFilename, QWidget* parent /*=nullptr*/ )
: QDialog(parent)
{
    // Create user interface
    m_Ui.setupUi(this);

    // Set info label
    QString infoText;
    infoText += "Version: " + QCoreApplication::applicationVersion() + "<br />";
    infoText += "Copyright Piql 2015";
    infoText += "<p>Support: <a href=\"mailto:support@piql.com\">support@piql.com</a></p>";
    infoText += "<p><a href=\"www.piql.com\">www.piql.com</a></p>";
    m_Ui.infoTextLabel->setText( infoText );
    m_Ui.infoTextLabel->setTextInteractionFlags( Qt::TextBrowserInteraction );
    m_Ui.infoTextLabel->setOpenExternalLinks( true );

    // Put application name in title bar
    this->setWindowTitle( "About " + applicationName );

    // Set bitmap for application
    if ( bitmapFilename.length() )
    {
        QPixmap pixmap( bitmapFilename );
        m_Ui.applicationLogoBitmap->setPixmap( pixmap );  
    }
}


/**
 * Destructor
 */
QAboutDialog::~QAboutDialog()
{
}

