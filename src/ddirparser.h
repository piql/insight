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
