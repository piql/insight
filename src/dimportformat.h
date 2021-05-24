#ifndef DIMPORTFORMAT_H
#define DIMPORTFORMAT_H

/*****************************************************************************
**  
**  Definition of the DImportFormat class
**
**  Creation date:  2019/03/19
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
#include "dregexp.h"
#include "dleafmatcher.h"
#include "djournalmatcher.h"

//  QT INCLUDES
//
#include <QCoreApplication>

//  FORWARD DECLARATIONS
//
class DInsightConfig;

//============================================================================
// CLASS: DImportFormat

class DImportFormat 
{
    Q_DECLARE_TR_FUNCTIONS(DImportFormat)
    
public:
    DImportFormat();
    
    const QString &             name() const;
    const DRegExps &            patterns() const;
    const QString&              parser() const;
    const DRegExps &            treeViewLabelRegExp() const;
    const DRegExps &            treeViewNodeRegExp() const;
    const DRegExps &            infoViewLabelRegExp() const;
    const DLeafMatchers &       documentTypeRegExp() const;
    const DJournalMatchers &    journalMatchers() const;
    const DRegExps &            folderTypeRegExp() const;
    const DRegExps &            deleteTypeRegExp() const;
    const DLeafMatchers &       importTypeRegExp() const;
    const DLeafMatchers &       autoImportRegExp() const;
    const DRegExps &            checksumTypeRegExp() const;
    const DRegExps &            checksumSourceTypeRegExp() const;
    QString                     extractTool( const QString& fileName, const QString& destination ) const;
    QString                     fileIdTool( const QString& fileName ) const;
    QString                     convertToPdfTool( const QString& filesFile, const QString& out, const QString& temp ) const;

    
public:
    static bool Load( DImportFormat& format, const QString& fileName );

private:
    DRegExps                    getRegExps( const QString& key );
    DLeafMatchers               getLeafMatchers( const QString& key );
    
    
private:
    QString                     m_Name;
    DRegExps                    m_Patterns;
    QString                     m_Parser;
    DRegExps                    m_TreeViewLabelRegExp;
    DRegExps                    m_TreeViewNodeRegExp;
    DRegExps                    m_InfoViewLabelRegExp;
    DLeafMatchers               m_DocumentTypeRegExp;
    DJournalMatchers            m_JournalMatchers;
    DRegExps                    m_FolderTypeRegExp;
    DRegExps                    m_DeleteTypeRegExp;
    DLeafMatchers               m_ImportTypeRegExp;
    DLeafMatchers               m_AutoImportRegExp;
    DRegExps                    m_ChecksumTypeRegExp;
    DRegExps                    m_ChecksumSourceTypeRegExp;
    DRegExps                    m_ExtractTool;
    QString                     m_FileIdTool;
    QString                     m_PdfCreatorTool;

    DImportFormat*              m_BaseFormat;
    DInsightConfig*             m_Config;
};


//============================================================================
// CLASS: DImportFormats

class DImportFormats : public QVector<DImportFormat>
{
public:

    static bool Load( DImportFormats& formats, const QString& dir );

    const DImportFormat* defaultFormat() const;
    const DImportFormat* findMatching( const QString& fileName ) const;
    const DImportFormat* find( const QString& formatName ) const;
};

#endif // DIMPORTFORMAT_H
