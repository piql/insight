#ifndef DDIRPARSER_H
#define DDIRPARSER_H

/*****************************************************************************
**  
**  Definition of the DDirParser class
**
**  Creation date:  2019/07/01
**  Created by:     Ole Liabo
**
**
**  Copyright (c) 2019 Piql AS. All rights reserved.
**
*****************************************************************************/

//  PROJECT INCLUDES
//
#include    "dxmlparser.h"


//============================================================================
// CLASS: DDirParser

class DDirParser : public DXmlParser
{
public:
    DDirParser( DTreeItems* treeItems, const QString& dir, DTreeModel* model, DTreeRootItem* rootNode, const DImportFormat* importFormat );
    virtual ~DDirParser();

private:
    virtual void run();
    void incItems( DTreeItem* item, bool finalItem );

private:
    size_t m_MaxItems;
    size_t m_Items;
    size_t m_ItemsLastReport;
};


#endif // DDIRPARSER_H
