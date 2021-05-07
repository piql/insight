/***************************************************************************
**
**  Implementation of the DFixedFolderDialog class
**
**  Creation date:  2021/05/07
**  Created by:     Ole Liabo
**
**
**  Copyright (c) 2021 Piql AS.
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
#include    "dfixedfolderdialog.h"


//  QT INCLUDES
//
#include    <QDir>
#include    <QFileSystemModel>
#include    <QScreen>


/****************************************************************************/
/*! \class DFixedFolderDialog dfixedfolderdialog.h
 *  \ingroup Insight
 *  \brief Select folder from fixed root
 * 
 */
 
//===================================
//  P U B L I C   I N T E R F A C E
//===================================


//----------------------------------------------------------------------------
/*! 
 *  Constructor.
 */

DFixedFolderDialog::DFixedFolderDialog( const QString& rootFolder, const QString& title )
  : m_RootFolder( rootFolder )
{
    m_Ui.setupUi( this );


    // Root exists?
    QDir root( m_RootFolder );
    if ( !root.exists() )
    {
        root.mkdir( m_RootFolder );
    }

    QFileSystemModel model;
    model.setRootPath( rootFolder );
    model.setOption( QFileSystemModel::DontUseCustomDirectoryIcons );

    m_Ui.treeView->setModel( &model );
    if ( !m_RootFolder.isEmpty() ) {
        const QModelIndex rootIndex = model.index( QDir::cleanPath( m_RootFolder ) );
        if ( rootIndex.isValid() )
            m_Ui.treeView->setRootIndex( rootIndex );
    }

    m_Ui.treeView->setSelectionMode( QAbstractItemView::SingleSelection );
    m_Ui.treeView->setAnimated( false );
    m_Ui.treeView->setIndentation( 20 );
    m_Ui.treeView->setSortingEnabled( true );
    const QSize availableSize = m_Ui.treeView->screen()->availableGeometry().size();
    m_Ui.treeView->resize( availableSize / 2 );
    m_Ui.treeView->setColumnWidth( 0, m_Ui.treeView->width() / 3 );

    // Make it flickable on touchscreens
    //QScroller::grabGesture( &tree, QScroller::TouchGesture );

    //tree.setWindowTitle( caption );

    // Set window title
    setWindowTitle( title );

    // Signals and slots. The GUI components emits signals that are handled by the slots.
    connect( m_Ui.buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept );
}


//----------------------------------------------------------------------------
/*!                                                                              
 *  Destructor.
 */

DFixedFolderDialog::~DFixedFolderDialog()
{
}

QString DFixedFolderDialog::fileName()
{
    //m_Ui.treeView->sel
    return QString();
}