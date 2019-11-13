#ifndef DTARPARSER_H
#define DTARPARSER_H

/*****************************************************************************
**  
**  Definition of the DTarParser class
**
**  Creation date:  2019/02/25
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
// CLASS: DTarParser

class DTarParser : public DXmlParser
{
public:
    DTarParser( DTreeItems* treeItems, const QString& fileName, DTreeModel* model, DTreeRootItem* rootNode, const DImportFormat* importFormat );
    virtual ~DTarParser();

private:
    virtual void run();
    void incItems( DTreeItem* item, bool finalItem );

private:
    size_t m_MaxItems;
    size_t m_Items;
    size_t m_ItemsLastReport;
};


#endif // DTARPARSER_H
