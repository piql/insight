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
#include "qpersistantfiledialog.h"
#include "dfixedfolderdialog.h"

//  SYSTEM INCLUDES
//

//  QT INCLUDES
//
#include <QSettings>
#include <QFileSystemModel>
#include <QTreeView>
#include <QScreen>

//  NAMESPACE
//

//  FORWARD DECLARATIONS
//

// QCoreApplication::setOrganizationName() and QCoreApplication::setApplicationName() must be set for functions to work..

QString QPersistantFileDialog::getOpenFileName ( 
    const QString & uniqueID,
    QWidget * parent /*= 0*/, 
    const QString & caption /*= QString()*/, 
    const QString & dir /*= QString()*/, 
    const QString & filter /*= QString()*/, 
    QString * selectedFilter /*= 0*/, 
    QFileDialog::Options options /*= 0*/ )
{
    QStringList files = getOpenFileNames(
        uniqueID,
        parent,
        caption,
        dir,
        filter,
        selectedFilter,
        options,
        false );
        
    if ( files.size() == 0 )
    {
        return QString();
    }
    return files[0];
}

QStringList QPersistantFileDialog::getOpenFileNames ( 
    const QString & uniqueID,
    QWidget * parent /*= 0*/, 
    const QString & caption /*= QString()*/, 
    const QString & dir /*= QString()*/, 
    const QString & filter /*= QString()*/, 
    QString * selectedFilter /*= 0*/, 
    QFileDialog::Options options /*= 0*/,
    bool multiSelect /*= true*/ )
{
    // If a registered key exists for this folder, use it, otherwize use the provided folder
    QSettings settings;
    QString key = QString("persistant_file_dialog/get_open_file_name/%1").arg(uniqueID);
    QString actualDir = settings.value(key, dir).toString();

    // Present the dialog
    QStringList files;
    if ( multiSelect )
    {
        files = QFileDialog::getOpenFileNames(
            parent, caption, actualDir, filter, selectedFilter, options );
    }
    else
    {
        QString file = QFileDialog::getOpenFileName(
            parent, caption, actualDir, filter, selectedFilter, options );
        if ( file.length() )
        {
            files.append( file );
        }
    }

    // If the user did not press cancel, store the path
    if ( files.size() )
    {
        settings.setValue( key, QDir(files[0]).absolutePath() );
    }

    // Return result
    return files;
}

QString QPersistantFileDialog::getSaveFileName( 
    const QString & uniqueID,
    QWidget * parent /*= 0*/, 
    const QString & caption /*= QString()*/, 
    const QString & dir /*= QString()*/, 
    const QString & filter /*= QString()*/, 
    QString * selectedFilter /*= 0*/, 
    QFileDialog::Options options /*= 0*/,
    bool onlyDirPersistant /*= false*/ )
{
    // If a registered key exists for this folder, use it, otherwize use the provided folder
    QSettings settings;
    QString key = QString("persistant_file_dialog/get_open_file_name/%1").arg(uniqueID);
    QString actualDir = settings.value(key, dir).toString();

    if ( onlyDirPersistant )
    {
        // Only use folder, take file from caller
        QFileInfo newDir(actualDir);
        QFileInfo oldDir(dir);

        actualDir = QFileInfo(newDir.path(), oldDir.fileName()).filePath();
    }

    // Present the dialog
    QString file = QFileDialog::getSaveFileName(
        parent, caption, actualDir, filter, selectedFilter, options );

    // If the user did not press cancel, store the path
    if (file.length())
    {
        settings.setValue( key, QDir(file).absolutePath() );
    }

    // Return result
    return file;
}

QString QPersistantFileDialog::getExistingDirectory ( 
    const QString & uniqueID,
    QWidget * parent /*= 0*/, 
    const QString & caption /*= QString()*/, 
    const QString & dir /*= QString()*/ )
{
    // If a registered key exists for this folder, use it, otherwize use the provided folder
    QSettings settings;
    QString key = QString("persistant_file_dialog/get_open_file_name/%1").arg(uniqueID);
    QString actualDir = settings.value(key, dir).toString();

    // Present the dialog
    QString dirName = QFileDialog::getExistingDirectory( parent, caption, actualDir, QFileDialog::ShowDirsOnly|QFileDialog::DontUseNativeDialog );

    // If the user did not press cancel, store the path
    if (dirName.length())
    {
        settings.setValue( key, QDir(dirName).absolutePath() );
    }

    // Return result
    return dirName;
}

//----------------------------------------------------------------------------
/*!
 *  Get a filename below a fixed root. Users are not allowed to navigate up from
 *  the fixed root. This is to simplify the user interface so users by accident 
 *  save files outside output folder.
 */

QString QPersistantFileDialog::getFixedRootSaveFileName(
    QWidget * /*parent*/,
    const QString & caption,
    const QString & fileName,
    const QString & rootDir )
{
    DFixedFolderDialog dialog(rootDir, fileName, caption);
    
    //tree.show();

    // Return result
    if ( dialog.exec() == QDialog::Accepted ) 
    {
        return dialog.fileName();
    }

    return QString();
}
