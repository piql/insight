/***************************************************************************
**
**  Implementation of the DTreeItem class
**
**  Creation date:  2017/09/08
**  Created by:     Ole Liabo
**
**
**  Copyright (c) 2017 Piql AS. All rights reserved.
**
***************************************************************************/

//  PROJECT INCLUDES
//
#include    "dtreeitem.h"
#include    "dxmlparser.h"

//  SYSTEM INCLUDES
//
#include    <cassert>

//  QT INCLUDES
//
#include    <Qt>

/****************************************************************************/
/*! \class DLeafNode dtreeitem.h
 *  \ingroup Insight
 *  \brief Leaf node in XML tree
 */


//----------------------------------------------------------------------------
/*! 
 *  Constructor.
 */

DLeafNode::DLeafNode()
    : m_Key( NULL ),
      m_Value( NULL )
{
}


//----------------------------------------------------------------------------
/*! 
 *  Destructor.
 */

DLeafNode::DLeafNode( const char* key, const char* value )
  : m_Key( key ),
    m_Value( value )
{
}


//----------------------------------------------------------------------------
/*! 
 *  Copy constructor.
 */

DLeafNode::DLeafNode( const DLeafNode& node )
  : m_Key( node.m_Key ),
    m_Value( strdup(node.m_Value) )
{
}


//----------------------------------------------------------------------------
/*! 
 *  Destructor.
 */

DLeafNode::~DLeafNode()
{
    free( (void*)m_Value );
}


//----------------------------------------------------------------------------
/*! 
 *  Assignment operator.
 */

DLeafNode& DLeafNode::operator=( const DLeafNode& node )
{
    if ( m_Value )
    {
        free( (void*)m_Value );
    }

    m_Key = node.m_Key;
    m_Value = strdup( node.m_Value );

    return *this;
}


//----------------------------------------------------------------------------
/*! 
 *  Return position of start of text in node text, -1 if not found.
 */

int DLeafNode::match( const char* text ) const
{
    if ( text == NULL || strlen(text) == 0 )
    {
        return -1;
    }

    QString v( m_Value );

    return v.indexOf( QString( text ) );
/*
    const char* p = strstr( m_Value, text );
    if ( p )
    {
        return p - m_Value;
    }
    return -1;
*/
}


//----------------------------------------------------------------------------
/*! 
 *  Return position of start of text in node text, -1 if not found.
 */

int DLeafNode::match( const QString& text ) const
{
    if ( text.length() == 0 )
    {
        return -1;
    }

    QString v( m_Value );

    return v.indexOf( text );
}


/****************************************************************************/
/*! \class DTreeItem dtreeitem.h
 *  \ingroup Insight
 *  \brief Node in tree view widget
 */

//----------------------------------------------------------------------------
/*! 
 *  Constructor.
 */

DTreeItem::DTreeItem( DTreeItem* parent, const QString& text )
    : m_Parent( parent ),
      m_Text( strdup( text.toStdString().c_str() ) ),
      m_State( 0 )
{
    if ( parent )
    {
        parent->addChild( this );
    }

    m_Row = rowSlow();
}


//----------------------------------------------------------------------------
/*! 
 *  Constructor.
 */

DTreeItem::DTreeItem( DTreeItem* parent, const char* text )
    : m_Parent( parent ),
      m_Text( text ),
      m_State( 0 )
{
    assert(parent != this);
    if ( parent )
    {
        parent->addChild( this );
    }

    m_Row = rowSlow();
}


//----------------------------------------------------------------------------
/*! 
 *  Destructor.
 */

DTreeItem::~DTreeItem()
{
}


//----------------------------------------------------------------------------
/*! 
 *  Add node to tree item.
 */


void DTreeItem::addNode( DLeafNode* node )
{
    m_Nodes.push_back( node );
}


//----------------------------------------------------------------------------
/*! 
 *  Add tree item child.
 */

void DTreeItem::addChild( DTreeItem* child )
{
    assert( child->m_Parent == this );
    m_Children.push_back( child );
}


//----------------------------------------------------------------------------
/*! 
 *  Return number of tree item children.
 */

int DTreeItem::row() 
{
    return rowSlow();
}


//----------------------------------------------------------------------------
/*! 
 *  Return number of tree item children.
 */

int DTreeItem::rowSlow() 
{
    if (m_Parent == NULL )
    {
        return 0;
    }

    DChildrenIterator it = m_Parent->m_Children.begin();
    while ( it != m_Parent->m_Children.end() )
    {
        if ( *it == this )
        {
            return it - m_Parent->m_Children.begin();
        }
        ++it;
    }
    assert(0);
    return 0;
}


//----------------------------------------------------------------------------
/*! 
 *  Returned true if item is selected (checked)
 */

bool DTreeItem::checked() const
{
    return m_State & STATE_CHECKED;
}


//----------------------------------------------------------------------------
/*! 
 *  Select item.
 */

void DTreeItem::check()
{
    m_State |= STATE_CHECKED;    
}


//----------------------------------------------------------------------------
/*! 
 *  Deselect item.
 */

void DTreeItem::uncheck()
{
    m_State &= ~STATE_CHECKED;    
}


//----------------------------------------------------------------------------
/*! 
 *  Select or deselect item.
 */

void DTreeItem::setChecked( bool checked )
{
    if ( checked )
    {
        check();
    }
    else
    {
        uncheck();
    }
}


//----------------------------------------------------------------------------
/*! 
 *  Return true if item has children.
 */

bool DTreeItem::hasChildren() const
{
    return m_Children.size() != 0;
}


//----------------------------------------------------------------------------
/*! 
 *  Find leaf node matching key.
 */

DLeafNode* DTreeItem::findLeaf( const char* key )
{
    DLeafNodesIterator it = m_Nodes.begin();
    DLeafNodesIterator itEnd = m_Nodes.end();
    while ( it != itEnd )
    {
        if ( strcmp( (*it)->m_Key, key ) == 0 )
        {
            return *it;
        }
        it++;
    }
    return NULL;
}


//----------------------------------------------------------------------------
/*! 
 *  Find items root parent.
 */

DTreeRootItem* DTreeItem::findRootItem()
{
    DTreeItem* i = this;
    while ( i )
    {
        if ( dynamic_cast<DTreeRootItem*>(i) )
        {
            return dynamic_cast<DTreeRootItem*>(i);
        }
        i = i->m_Parent;
    }
    return NULL;
}


//----------------------------------------------------------------------------
/*! 
 *  Get vector of anchestors.
 */

DTreeItems DTreeItem::Anchestors( DTreeItem* item )
{
    DTreeItems items;

    item = item->m_Parent;
    while ( item )
    {
        items.push_front( item );
        item = item->m_Parent;
    }

    return items;
}


/****************************************************************************/
/*! \class DTreeRootItem dtreeitem.h
 *  \ingroup Insight
 *  \brief Leaf node in XML tree
 *
 *  All the children of the root node is allocated in a node / item pools 
 *  for fast speed. But the items of the node itself are allocated on the free
 *  store.
 */

//----------------------------------------------------------------------------
/*! 
 *  Constructor.
 */

DTreeRootItem::DTreeRootItem( DTreeItem* parent, const QString& text )
  : DTreeItem( parent, text ),
    m_CurrentTreeItemBlockEnd(NULL),
    m_CurrentTreeItemBlockPos(NULL),
    m_CurrentLeafNodeBlockEnd(NULL),
    m_CurrentLeafNodeBlockPos(NULL)
{

}


//----------------------------------------------------------------------------
/*! 
 *  Constructor.
 */

DTreeRootItem::DTreeRootItem( DTreeItem* parent, const char* text )
  : DTreeItem( parent, text ),
    m_CurrentTreeItemBlockEnd(NULL),
    m_CurrentTreeItemBlockPos(NULL),
    m_CurrentLeafNodeBlockEnd(NULL),
    m_CurrentLeafNodeBlockPos(NULL)
{

}


//----------------------------------------------------------------------------
/*! 
 *  Destructor.
 */

DTreeRootItem::~DTreeRootItem()
{
    deleteChildren();
    free( (char*)m_Text );

    DLeafNodesIterator it = m_Nodes.begin();
    DLeafNodesIterator itEnd = m_Nodes.end();
    while ( it != itEnd )
    {
        delete *it;
        it++;
    }
}


//----------------------------------------------------------------------------
/*! 
 *  Delete all children. This clears the memory pool controlled by the root
 *  item.
 */

void DTreeRootItem::deleteChildren()
{
    std::vector<void*>::iterator it = m_TreeItemBlocks.begin();
    while ( it != m_TreeItemBlocks.end() )
    {
        DTreeItem* p = (DTreeItem*)*it;
        DTreeItem* e = it == --m_TreeItemBlocks.end() ? m_CurrentTreeItemBlockPos : p + TREE_ITEM_BLOCK_SIZE;
        while ( p < e )
        {
            p->~DTreeItem();
            p++;
        }
        delete[] (char*)*it;
        it++;
    }

    std::vector<void*>::iterator itL = m_LeafNodeBlocks.begin();
    while ( itL != m_LeafNodeBlocks.end() )
    {
        DLeafNode* p = (DLeafNode*)*itL;
        DLeafNode* e = itL == --m_LeafNodeBlocks.end() ? m_CurrentLeafNodeBlockPos : p + LEAF_NODE_BLOCK_SIZE;
        while ( p < e )
        {
            p->~DLeafNode();
            p++;
        }
        delete[] (char*)*itL;
        itL++;
    }

    m_TreeItemBlocks.clear();
    m_LeafNodeBlocks.clear();
    m_CurrentTreeItemBlockPos = NULL;
    m_CurrentTreeItemBlockEnd = NULL;
    m_CurrentLeafNodeBlockPos = NULL;
    m_CurrentLeafNodeBlockEnd = NULL;
    m_Children.clear();
}


//----------------------------------------------------------------------------
/*! 
 *  Create item (on pool)
 */

DTreeItem* DTreeRootItem::createItem( DTreeItem* parent, const char* text )
{
    if ( m_CurrentTreeItemBlockPos >= m_CurrentTreeItemBlockEnd )
    {
        m_CurrentTreeItemBlockPos = (DTreeItem*)new char[sizeof(DTreeItem) * TREE_ITEM_BLOCK_SIZE];
        m_CurrentTreeItemBlockEnd = m_CurrentTreeItemBlockPos + TREE_ITEM_BLOCK_SIZE;
        m_TreeItemBlocks.push_back( m_CurrentTreeItemBlockPos );                    
    }

    DTreeItem* item = new(m_CurrentTreeItemBlockPos)DTreeItem(parent, text);
    m_CurrentTreeItemBlockPos++;
    return item;
}


//----------------------------------------------------------------------------
/*! 
 *  Create leaf (on pool)
 */

DLeafNode* DTreeRootItem::createLeaf( const QString& key, const QString& value )
{
    std::string k = key.toStdString();
    std::string v = value.toStdString();
    return createLeaf( DXmlParser::AddToHashMap(k.c_str()), strdup(v.c_str()) );
}


//----------------------------------------------------------------------------
/*! 
 *  Create lead (on pool)
 */

DLeafNode* DTreeRootItem::createLeaf( const char* key, const char* value )
{
    if ( m_CurrentLeafNodeBlockPos >= m_CurrentLeafNodeBlockEnd )
    {
        m_CurrentLeafNodeBlockPos = (DLeafNode*)new char[sizeof(DLeafNode) * TREE_ITEM_BLOCK_SIZE];
        m_CurrentLeafNodeBlockEnd = m_CurrentLeafNodeBlockPos + TREE_ITEM_BLOCK_SIZE;
        m_LeafNodeBlocks.push_back( m_CurrentLeafNodeBlockPos );                    
    }

    DLeafNode* item = new(m_CurrentLeafNodeBlockPos)DLeafNode(key, value);
    m_CurrentLeafNodeBlockPos++;
    return item;
}


//----------------------------------------------------------------------------
/*! 
 *  Add node.
 */

void DTreeRootItem::addNode( DLeafNode* node )
{
    m_Nodes.push_back( new DLeafNode( *node ) );
}


//----------------------------------------------------------------------------
/*! 
 *  Update node.
 */

void DTreeRootItem::updateNode( DLeafNode* node )
{
    DLeafNodesIterator it = m_Nodes.begin();
    DLeafNodesIterator itEnd = m_Nodes.end();
    while ( it != itEnd )
    {
        if ( (*it)->m_Key == node->m_Key )
        {
            DLeafNode* old = *it;
            *it = new DLeafNode( *node );
            delete old;
            return;
        }

        it++;
    }

    addNode( node );
}


//----------------------------------------------------------------------------
/*! 
 *  Remove node.
 */

void DTreeRootItem::removeNode( const QString& key )
{
    DLeafNodesIterator it = m_Nodes.begin();
    DLeafNodesIterator itEnd = m_Nodes.end();
    while ( it != itEnd )
    {
        if ( (*it)->m_Key == key )
        {
            delete *it;
            m_Nodes.erase( it );
            return;
        }
        it++;
    }
}
