/***************************************************************************
**
**  Implementation of the DContext class
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
***************************************************************************/

//  PROJECT INCLUDES
//
#include    "dcontext.h"
#include    "dinsightreport.h"

//  QT INCLUDES
//
#include    <QDir>
#include    <QDirIterator>
#include    <QMessageBox>
#include    <QtGlobal>
#include    <QProcess>

//  SYSTEM INCLUDES
//
#include    <cassert>


/****************************************************************************/
/*! \class DContext DContext.h
 *  \ingroup Insight
 *  \brief Holds all loaded data
 */
 
 //----------------------------------------------------------------------------
 /*!
  *  Constructor.
  */

DContext::DContext(DImportFormats& importFormats)
    : m_Model(&importFormats),
      m_ImportFormats(importFormats)
{
}

//----------------------------------------------------------------------------
/*! 
 *  Load imports in folder.
 */

void DContext::loadImports(const QString& rootDir)
{
    QStringList nameFilters;
    nameFilters << DInsightReport::getXmlReportName();
    QDirIterator dirIt(rootDir, nameFilters, QDir::NoFilter, QDirIterator::Subdirectories);

    while (dirIt.hasNext())
    {
        QString fileName = dirIt.next();
        DImport* import = DImport::CreateFromReport(fileName, &m_Model, &m_ImportFormats);
        if (import)
        {
            //connectImportLoad(import);
            m_Imports.push_back(import);
        }
        else
        {
            DInsightConfig::Log() << "Import failed: " << fileName << Qt::endl;
        }
    }
}

DImport* DContext::findImport(const DTreeItem* item)
{
    return FindImport(item, m_Imports);
}
    
DImport* DContext::FindImport(const DTreeItem * item, DImports& imports)
{
    DImportsIterator it = imports.begin();
    DImportsIterator itEnd = imports.end();

    while (it != itEnd)
    {
        if ((*it)->root() == item)
        {
            return *it;
        }
        it++;
    }
    return nullptr;
}


void DContext::makeAbsolute(QString& filename, DTreeItem* item)
{
    MakeAbsolute(filename, item, m_Imports);
}

void DContext::MakeAbsolute(QString& filename, DTreeItem* item, DImports& imports)
{
    if (!QFile::exists(filename))
    {
        const DTreeRootItem* root = item->findRootItem();
        const DImportFormat* format = root->format();
        DImport* import = FindImport(root, imports);

        if (format->parser() == "dir")
        {
            if (format->extractTool("a", "b").length() != 0)
            {
                // The content is relative to the "extract" folder under the import folder
                QDir dir(import->reportsDir());
                dir.cd(tr("extract"));
                QString path = item->findRootPath();
                dir.cd(path);
                filename = dir.filePath(filename);
            }
            else
            {
                // The content is relative to the import filename root
            }
        }
        else
        {
            QDir dir(import->fileNameRoot());
            filename = dir.filePath(filename);
        }
    }
}
