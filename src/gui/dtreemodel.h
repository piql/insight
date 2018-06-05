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
**  Copyright (c) 2017 Piql AS. All rights reserved.
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

//============================================================================
// CLASS: DTreeModel

class DTreeModel : public QAbstractItemModel 
{
    Q_OBJECT

public:

    explicit DTreeModel(const DRegExps& nodeRegExp, const DRegExps& labelRegExp);
    virtual ~DTreeModel();

    virtual QModelIndex     index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex     parent(const QModelIndex &child) const;
    virtual int             rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int             columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant        data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual Qt::ItemFlags   flags(const QModelIndex &index) const;
    virtual bool            setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    virtual bool            hasChildren(const QModelIndex &parent = QModelIndex()) const;

    DTreeRootItem*          createDocumentRoot( const QString& fileName );
    void                    deleteDocumentRoot( DTreeItem* root, bool onlyChildren );
    unsigned int            rootCount();
    DTreeItem*              documentRoot( unsigned int index );
    void                    setVisible( QTreeView* view, DTreeItem* node, bool show );

    DTreeItem*              createItem( DTreeRootItem* root, DTreeItem* parent, const char* text );
    DLeafNode*              createLeaf( DTreeRootItem* root, const char* key, const char* value );
    DLeafNode*              createLeaf( DTreeRootItem* root, const QString& key, const QString& value );

    QModelIndex             index( DTreeItem* item );

    void                    beginInsert(int row, int count, const QModelIndex &parent );
    void                    endInsert();

private:
    DTreeItem* m_Root;
    DRegExps   m_NodeRegExp;
    DRegExps   m_LabelRegExp;
    friend class DXmlContext;
};


#endif // DTREEMODEL_H
