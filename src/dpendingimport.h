#ifndef DPENDINGIMPORT_H
#define DPENDINGIMPORT_H

/*****************************************************************************
**  
**  Definition of the DPendingImport class
**
**  Creation date:  2021/05/13
**  Created by:     Ole Liabo
**
**
**  Copyright (c) 2021 Piql AS.
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

//  QT INCLUDES
//
#include    <QString>
#include    <QList>

//  FORWARD DECLARATIONS
//
class DTreeItem;

//============================================================================
// CLASS: DPendingImport

class DPendingImport
{
public:
    DTreeItem *     m_Root;
    QString         m_Document;
};

//============================================================================
// CLASS: DPendingImports

typedef QList<DPendingImport> DPendingImports;

#endif // DPENDINGIMPORT_H
