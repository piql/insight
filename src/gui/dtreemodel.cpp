/***************************************************************************
**
**  Implementation of the DTreeModel class
**
**  Creation date:  2017/10/28
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
#include    "dtreemodel.h"
#include    "dtreeitem.h"
#include    "dinsightmainwindow.h"
#include    "dxmlparser.h"
#include    "dimportformat.h"

//  QT INCLUDES
//
#include    <QTreeView>

/****************************************************************************/
/*! \class DTreeModel dtreemodel.h
 *  \ingroup Insight
 *  \brief Tree widget data model
 */


//----------------------------------------------------------------------------
/*! 
 *  Constructor.
 */

DTreeModel::DTreeModel( const DImportFormats* formats )
    : m_ImportFormats( formats )
{
    m_Root = new DTreeItem( nullptr, "hidden root" );
}


//----------------------------------------------------------------------------
/*! 
 *  Destructor.
 */

DTreeModel::~DTreeModel()
{
    // Delete all doc roots
    DTreeItem::DChildrenIterator it = m_Root->m_Children.begin();
    DTreeItem::DChildrenIterator itEnd = m_Root->m_Children.end();
    
    while ( it != itEnd )
    {
        delete (DTreeRootItem*)*it;
        it++;
    }

    delete m_Root;
}


//----------------------------------------------------------------------------
/*! 
 *  Create item with given parent and text as description.
 */

DTreeItem* DTreeModel::createItem( DTreeRootItem* root, DTreeItem* parent, const char* text )
{
    return root->createItem( parent, text );
}


//----------------------------------------------------------------------------
/*! 
 *  Create leaf item.
 */

DLeafNode* DTreeModel::createLeaf( DTreeRootItem* root, const QString& key, const QString& value )
{
    return root->createLeaf( key, value );
}


//----------------------------------------------------------------------------
/*! 
 *  Create leaf item.
 */

DLeafNode* DTreeModel::createLeaf( DTreeRootItem* root, const char* key, const char* value )
{
    return root->createLeaf( key, value );
}


//----------------------------------------------------------------------------
/*! 
 *  Cteate model index.
 */

QModelIndex DTreeModel::index(int row, int column, const QModelIndex &parent /*= QModelIndex()*/) const
{
    if ( !hasIndex(row, column, parent) )
    {
        return QModelIndex();
    }

    DTreeItem *parentItem;
    if ( !parent.isValid() )
    {
        parentItem = m_Root;
    }
    else
    {
        parentItem = static_cast<DTreeItem*>( parent.internalPointer() );
    }

    DTreeItem *childItem;
    childItem = parentItem->m_Children[ row ];

    return createIndex( row, column, childItem );
}


//----------------------------------------------------------------------------
/*! 
 *  Create parent model index.
 */

QModelIndex DTreeModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
    {
        return QModelIndex();
    }

    DTreeItem *childItem = static_cast<DTreeItem*>(child.internalPointer());
    DTreeItem *parentItem = childItem->m_Parent;

    if (parentItem == m_Root || parentItem == nullptr)
    {
        return QModelIndex();
    }

    return createIndex(parentItem->row(), 0, parentItem);
}


//----------------------------------------------------------------------------
/*! 
 *  Return number of children for parent.
 */

int DTreeModel::rowCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    DTreeItem *parentItem;
    if ( parent.column() > 0 )
    {
        return 0;
    }

    if (!parent.isValid())
    {
        parentItem = m_Root;
    }
    else
    {
        parentItem = static_cast<DTreeItem*>(parent.internalPointer());
    }

    return (int)parentItem->m_Children.size();
}


//----------------------------------------------------------------------------
/*! 
 *  Return number of columns (always 1)
 */

int DTreeModel::columnCount(const QModelIndex &/*parent*/ /*= QModelIndex()*/) const
{
    return 1;
}


//----------------------------------------------------------------------------
/*! 
 *  Return item data, depending on role flag.
 */

QVariant DTreeModel::data(const QModelIndex &index, int role /*= Qt::DisplayRole*/) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    if ( role == Qt::CheckStateRole )
    {
        DTreeItem *item = static_cast<DTreeItem*>(index.internalPointer());

        return QVariant( item->checked() ? Qt::Checked : Qt::Unchecked );
    }
    else if (role == Qt::DisplayRole)
    {
        DTreeItem *item = static_cast<DTreeItem*>(index.internalPointer());

        QString label = DInsightMainWindow::GetTreeItemLabel(item);

        return QVariant( label );
    }

    return QVariant();
}


//----------------------------------------------------------------------------
/*! 
 *  Get root item from index.
 */

DTreeItem* DTreeModel::documentRoot( unsigned int index )
{
    return m_Root->m_Children[index];
}


//----------------------------------------------------------------------------
/*! 
 *  Create a new root item. Root items are special since they own the 
 *  data pools all childs are allocated in. The data pools are there to speed
 *  up allocation.
 */

DTreeRootItem* DTreeModel::createDocumentRoot( const QString& fileName, DTreeItem* parent, const DImportFormat* format )
{
    if ( parent == nullptr )
    {
        parent = m_Root;
    }

    DTreeRootItem* docRoot = new DTreeRootItem( parent, strdup( fileName.toStdString().c_str() ), format );
    return docRoot;
}


//----------------------------------------------------------------------------
/*! 
 *  Delete item.
 */

void DTreeModel::deleteDocumentRoot( DTreeItem* root, bool onlyChildren )
{
    // TODO: Deletes only children of top level root
    //DTreeItem::DChildrenIterator it = m_Root->m_Children.begin();
    //DTreeItem::DChildrenIterator itEnd = m_Root->m_Children.end();
    DTreeItem* parent = root->m_Parent;
    DTreeItem::DChildrenIterator it = parent->m_Children.begin();
    DTreeItem::DChildrenIterator itEnd = parent->m_Children.end();
    
    int row = 0;
    while ( it != itEnd )
    {
        if ( *it == root )
        {
            if ( onlyChildren )
            {
                if ( root->m_Children.size() )
                {
                    beginRemoveRows( index( root ), 0, (int)root->m_Children.size()-1 );                
                    DTreeRootItem* rootItem = (DTreeRootItem*)root;
                    rootItem->deleteChildren();
                    endRemoveRows();
                }
            }
            else
            {
                beginRemoveRows( QModelIndex(), row, row );
                delete *it;
                parent->m_Children.erase( it );
                endRemoveRows();
            }

            break;
        }
        it++;
        row++;
    }
}


//----------------------------------------------------------------------------
/*! 
 *  Return number of root items.
 */

unsigned int DTreeModel::rootCount()
{
    return (unsigned int)m_Root->m_Children.size();
}


//----------------------------------------------------------------------------
/*!
 *  Return first root
 */

DTreeItem* DTreeModel::firstDocumentRoot()
{
    if (rootCount() == 0)
    {
        return nullptr;
    }
    return m_Root->m_Children[0];
}


//----------------------------------------------------------------------------
/*! 
 *  Return display hint flags for the item.
 */

Qt::ItemFlags DTreeModel::flags(const QModelIndex & index) const
{
    if ( !index.isValid() )
    {
        return 0;
    }

    static Qt::ItemFlags flags = Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled; 

    DTreeItem *item = static_cast<DTreeItem*>(index.internalPointer());
    if ( item->m_Children.size() == 0 )
    {
        return flags | Qt::ItemNeverHasChildren;
    }
    return flags;
}


//----------------------------------------------------------------------------
/*! 
 *  Set data for item.
 */

bool DTreeModel::setData(const QModelIndex &index, const QVariant &value, int role /*= Qt::EditRole*/)
{
    if ( !index.isValid() )
    {
        return false;
    }

    if ( role == Qt::CheckStateRole )
    {
        DTreeItem *item = static_cast<DTreeItem*>(index.internalPointer());
        int v = value.toInt();
        if ( v == Qt::Checked )
        {
            item->check();
        }
        else 
        {
            item->uncheck();
        }

        QVector<int> roles;
        roles.push_back( role );
        emit dataChanged( index, index, roles );
    }
    else
    {
        return false;
    }
    
    return true;
}


//----------------------------------------------------------------------------
/*! 
 *  Signal to connected GUI components (tree views) that we are about to start 
 *  inserting rows.
 */

void DTreeModel::beginInsert(int row, int count, const QModelIndex &parent /*= QModelIndex()*/)
{
    beginInsertRows(parent, row, row + count-1 );
}


//----------------------------------------------------------------------------
/*! 
 *  Signal to connected GUI components (tree views) that the row insertion 
 *  operation is finished.
 */

void DTreeModel::endInsert()
{
    endInsertRows();
}


//----------------------------------------------------------------------------
/*! 
 *  Return true if parent has children.
 */

bool DTreeModel::hasChildren(const QModelIndex &parent /*= QModelIndex()*/) const
{
    if ( !parent.isValid() )
    {
        return true;
    }

    DTreeItem *item = static_cast<DTreeItem*>(parent.internalPointer());
    return item->m_Children.size() != 0;
}


//----------------------------------------------------------------------------
/*! 
 *  Create model index for item.
 */

QModelIndex DTreeModel::index( DTreeItem* item )
{
    return createIndex( item->row(), 0, item );
}
