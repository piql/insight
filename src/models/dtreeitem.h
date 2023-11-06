#ifndef DTREEITEM_H
#define DTREEITEM_H

/*****************************************************************************
**  
**  Definition of the DTreeItem class
**
**  Creation date:  2017/09/08
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
#include    "dtreeitem.h"
#include    "dregexp.h"

//  QT INCLUDES
//
#include    <QString>

//  SYSTEM INCLUDES
//
#include    <list>
#include    <vector>

//  FORWARD DECLARATIONS
//
class DTreeItem;
class DTreeRootItem;
class DImportFormat;
class DJournal;

//============================================================================
// CLASS: DLeafNode

class DLeafNode
{
private:
    DLeafNode(); 
    DLeafNode( const char* key, const char* value  );
    DLeafNode( const DLeafNode& node ); 
   ~DLeafNode();

    DLeafNode& operator=( const DLeafNode& node );

    friend class DTreeRootItem;

public:
    const char* m_Key;
    const char* m_Value;
    int match( const char* text ) const;
    int match( const QString& text ) const;
};


//============================================================================
// CLASS: DLeafNodes

typedef std::list<DLeafNode*>       DLeafNodes;
typedef DLeafNodes::iterator        DLeafNodesIterator;
typedef DLeafNodes::const_iterator  DLeafNodesConstIterator;


//============================================================================
// CLASS: DTreeItems

typedef std::list<DTreeItem*>       DTreeItems;
typedef DTreeItems::const_iterator  DTreeItemsIterator;


//============================================================================
// CLASS: DTreeItem

class DTreeItem 
{
protected:
    DTreeItem( DTreeItem* parent, const QString& text );
    DTreeItem( DTreeItem* parent, const char* text );
    virtual ~DTreeItem();

    friend class DTreeModel;
    friend class DTreeRootItem;

public:
    virtual void addNode( DLeafNode* node );
    void addChild( DTreeItem* child );
    int  row();
    bool checked() const;
    void check();
    void uncheck();    
    void setChecked( bool checked );
    bool hasChildren() const;
    DLeafNode* findLeaf( const char* key );
    const DLeafNode* findLeaf( const char* key ) const;
    DTreeItem* findChild( const char* text );
    const DTreeRootItem* findRootItem() const;
    QString              findRootPath() const;
    const DImportFormat* format() const;
    const DRegExps&      nodeRegExp() const;
    const DRegExps&      labelRegExp() const;
    const DRegExps&      autoSelectRegExp() const;
    const DRegExps&      autoCollapseRegExp() const;


private:
    int  rowSlow();

public:
    static DTreeItems Anchestors( DTreeItem* item );

public:    

    enum { STATE_CHECKED = 0x1 };
    typedef std::vector<DTreeItem*> DChildren;
    typedef DChildren::const_iterator DChildrenIterator;

    DTreeItem*              m_Parent;
    DChildren               m_Children;
    const char*             m_Text;
    int                     m_State;
    DLeafNodes              m_Nodes;
    int                     m_Row;
    DJournal*               m_Journal;
};

//============================================================================
// CLASS: DTreeRootItem

class DTreeRootItem : public DTreeItem
{
private:
    DTreeRootItem( DTreeItem* parent, const QString& text, const DImportFormat* format );
    DTreeRootItem( DTreeItem* parent, const char* text, const DImportFormat* format );
    virtual ~DTreeRootItem();

    friend class DTreeModel;

public:
    virtual void        addNode( DLeafNode* node );
    void                updateNode( DLeafNode* node );
    void                removeNode( const QString& key );
    void                deleteChildren();
    const DRegExps&     nodeRegExp() const;
    const DRegExps&     labelRegExp() const;
    const DImportFormat* format() const;
    bool                isToplevelRoot() const;
    void                setImportFormat( const DImportFormat* format );

private:
    DTreeItem*          createItem( DTreeItem* parent, const char* text );
    DLeafNode*          createLeaf( const char* key, const char* value );
    DLeafNode*          createLeaf( const QString& key, const QString& value );

private:
    enum { TREE_ITEM_BLOCK_SIZE = 100000 };
    DTreeItem*          m_CurrentTreeItemBlockEnd;
    DTreeItem*          m_CurrentTreeItemBlockPos;
    std::vector<void*>  m_TreeItemBlocks;

    enum { LEAF_NODE_BLOCK_SIZE = 100000 };
    DLeafNode*          m_CurrentLeafNodeBlockEnd;
    DLeafNode*          m_CurrentLeafNodeBlockPos;
    std::vector<void*>  m_LeafNodeBlocks;

    const DImportFormat* m_Format;
};

#endif // DTREEITEM_H
