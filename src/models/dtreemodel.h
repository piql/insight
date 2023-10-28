#ifndef DTREEMODEL_H
#define DTREEMODEL_H

/*****************************************************************************
**  
**  Definition of the DTreeModel class
**
**  Creation date:  2017/10/26
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
*****************************************************************************/

//  PROJECT INCLUDES
//
#include   "dinsightconfig.h"

//  QT INCLUDES
//
#include   <QAbstractItemModel>
#include   <QSortFilterProxyModel>

//  FORWARD DECLARATIONS
//
class DTreeItem;
class QTreeView;
class DLeafNode;
class DTreeRootItem;
class DImportFormat;
class DImportFormats;

//============================================================================
// CLASS: DTreeModel

class DTreeModel : public QAbstractItemModel 
{
    Q_OBJECT

public:

    explicit DTreeModel( const DImportFormats* formats );
    virtual ~DTreeModel();

    virtual QModelIndex     index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex     parent(const QModelIndex &child) const;
    virtual int             rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int             columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant        data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual Qt::ItemFlags   flags(const QModelIndex &index) const;
    virtual bool            setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    virtual bool            hasChildren(const QModelIndex &parent = QModelIndex()) const;

    DTreeRootItem*          createDocumentRoot( const QString& fileName, DTreeItem* parent, const DImportFormat* format );

    void                    deleteDocumentRoot( DTreeItem* root, bool onlyChildren );
    unsigned int            rootCount();
    DTreeItem*              documentRoot( unsigned int index );
    DTreeItem*              firstDocumentRoot();
    void                    setVisible( QTreeView* view, DTreeItem* node, bool show );

    DTreeItem*              createItem( DTreeRootItem* root, DTreeItem* parent, const char* text );
    DLeafNode*              createLeaf( DTreeRootItem* root, const char* key, const char* value );
    DLeafNode*              createLeaf( DTreeRootItem* root, const QString& key, const QString& value );

    QModelIndex             index( DTreeItem* item );

    void                    beginInsert(int row, int count, const QModelIndex &parent );
    void                    endInsert();

private:
    DTreeItem* m_Root;
    const DImportFormats* m_ImportFormats;
    friend class DXmlContext;
};


#endif // DTREEMODEL_H
