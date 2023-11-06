#ifndef DCONTEXT_H
#define DCONTEXT_H

/*****************************************************************************
**  
**  Definition of the DContext class
**
**  Creation date:  2023/10/29
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
#include    "dtreemodel.h"
#include    "dimport.h"
#include    "dimportformat.h"

//  QT INCLUDES
//
#include    <QString>

//  FORWARD DECLARATIONS
//

//============================================================================
// CLASS: DContext

class DContext : public QObject
{
public:
    DContext(DImportFormats& importFormats);

    void                loadImports(const QString& rootDir);
    DImport*            findImport(const DTreeItem* item);
    void                makeAbsolute(QString& filename, DTreeItem* item);

public:
    static void         MakeAbsolute(QString& filename, DTreeItem* item, DImports& imports);
    static DImport*     FindImport(const DTreeItem* item, DImports& imports);

public:
    DTreeModel          m_Model;
    DImports            m_Imports;
    DImportFormats&     m_ImportFormats;

};


#endif // DCONTEXT_H
