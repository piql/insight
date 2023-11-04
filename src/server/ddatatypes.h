#ifndef DDATATYPES_H
#define DDATATYPES_H

/*****************************************************************************
**
**  Definition of the DDataTypes class
**
**  Creation date:  2023/10/28
**  Created by:     Ole Liabo
**
**
**  Copyright (c) 2023 Piql AS.
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
#include	"dimport.h"
#include	"dtreeitem.h"

//  QT INCLUDES
//
#include	<QtCore/QJsonObject>


//  FORWARD DECLARATIONS
//
class DTreeRootItem;

//============================================================================
// CLASS: DDataTypes

class DDataTypes
{
public:
	static QJsonArray  toJson(const DImports& imports);
	static QJsonArray  toJson(const DLeafNodes& nodes);
	static QJsonObject toJson(const DTreeRootItem* item);
	static QJsonObject toJson(const DImport* imports);
	static QJsonArray  toJson(const DTreeItem::DChildren* children);
	static QJsonObject toJson(const DTreeItem* treeItem);
};


#endif