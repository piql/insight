/*****************************************************************************
**
**  Implementation of the DDataTypes class
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
#include	"ddatatypes.h"

//  QT INCLUDES
//
#include	<QtCore/QJsonArray>


QJsonArray DDataTypes::toJson(const DImports& imports)
{
	QJsonArray array;
	for(size_t i = 0; i < imports.size(); i++)
	{
		array.append(imports[i]->uuid().toString());
	}
	return array;
}

QJsonArray DDataTypes::toJson(const DLeafNodes& nodes)
{
	QJsonArray array;
	foreach (auto leaf, nodes)
	{
		array.append(QJsonValue(QJsonObject{ {leaf->m_Key, leaf->m_Value} }));
	}
	return array;
}

QJsonObject DDataTypes::toJson(const DTreeRootItem* item)
{
	return QJsonObject {
		{ "text", item->m_Text },
		{ "is_root", item->isToplevelRoot() },
		{ "children", toJson(&item->m_Children) },
		{ "nodes", toJson(item->m_Nodes) }
	};
}

QJsonObject DDataTypes::toJson(const DImport* import)
{
	return QJsonObject { 
		{ "format", import->formatName() },
		{ "state", import->state() },
		{ "filename", import->fileName() },
		{ "root", toJson(import->root()) },
	};
}

QJsonArray DDataTypes::toJson(const DTreeItem::DChildren* children)
{
	QJsonArray array;
	for(auto it = children->begin(); it != children->end(); it++)
	{
		array.append(toJson(*it));
	}
	return array;
}

QJsonObject DDataTypes::toJson(const DTreeItem* treeItem)
{
	return QJsonObject{
		{ "text", treeItem->m_Text },
		{ "children", toJson(&treeItem->m_Children) },
		{ "nodes", toJson(treeItem->m_Nodes) }
	};
}
