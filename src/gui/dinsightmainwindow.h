#ifndef DINSIGHTMAINWINDOW_H
#define DINSIGHTMAINWINDOW_H

/*****************************************************************************
**  
**  Definition of the DInsightMainWindow class
**
**  Creation date:  2017/03/29
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
#include    "ui_dinsightmainwindow.h"
#include    "dinsightconfig.h"
#include    "dtreeitem.h"
#include    "dimport.h"
#include    "dxmlparser.h"
#include    "dattachmentparser.h"

//  QT INCLUDES
//
#include    <QRegularExpression>
#include    <QStyledItemDelegate>
#include    <QProcess>


//  FORWARD DECLARATIONS
//
class DTreeItem;
class QTreeWidgetItem;
class DInsightReport;
class DTreeModel;
class DXmlParser;
class DAttachmentParser;
class DAttachmentIndexer;
class QStatusBar;
class QProgressBar;
class QSortFilterProxyModel;
class DSearchThread;
class DTableSearchResultCell;
class QProcess;
class DInsightMainWindow;
class DImportFormats;

//============================================================================
// CLASS: DInsightMainWindow

class DInsightMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    DInsightMainWindow( DImportFormats* formats, const QString& attachmentParsing );
    virtual ~DInsightMainWindow();

public:
    static void    ReplaceString( QString& key, const DRegExps& regExps );
    static QString GetTreeItemLabel( DTreeItem *item );
    static void    MakeAbsolute( QString& filename, DTreeItem* item, DImports& imports );
    static DImport*FindImport( const DTreeItem* item, DImports& imports );

signals:
    void importComplete( bool ok );

public:
    void importFile(
        const QString& fileName,
        const QString& importFormatName,
        const QString& exportFile = QString(),
        DTreeItem* parent = nullptr );
    void exportReport( const QString& fileName = QString() );

public slots:
    void importFileFinished( bool ok );
    void indexingFinished( DImport::DIndexingState state );
    void indexingProgress( float progress );
    void indexingIndexerStated();
    void loadXmlProgress( unsigned long /*count*/, float progress );
    void unloadFinished( bool ok );


private slots:
    void aboutButtonClicked();
    void importButtonClicked();
    void exportButtonClicked();
    void selectButtonClicked();
    void activatedTreeItem(const QModelIndex &);
    void activatedTreeItem();
    void contextMenuTreeItem( const QPoint& pos );
    void viewDocumentClicked();
    void deleteFolderClicked();
    void importDocumentClicked();
    void checksumClicked();
    void journalClicked();
    void searchEditChanged( const QString & text );
    void dataChanged(const QModelIndex &, const QModelIndex &, const QVector<int> &);
    void loadMenuClicked( bool checked );
    void deleteMenuClicked( bool checked );
    void selectChildrenMenuClicked( bool checked );
    void deselectChildrenMenuClicked( bool checked );
    void selectAllMenuClicked( bool checked );
    void deselectAllMenuClicked( bool checked );
    void invertSelectionMenuClicked( bool checked );
    void searchFinished();
    void searchProgress( float progress );
    void searchMatch( const QModelIndex& matchingIndex );
    void attachmentMatch( int importIndex, int attachmentIndex );
    void searchResultDoubleClicked(QTableWidgetItem *item);
    void searchResultDoubleClicked(DTableSearchResultCell *item);
    void searchResultHeaderClicked(int logicalIndex);
    void searchAttachmentClicked(bool checked);
    void searchDeamonError(QProcess::ProcessError error);
    void prevSearchResultPageClicked();
    void nextSearchResultPageClicked();
    void searchOptionsToggled( bool on );
    void treeNodesSearchFilterChanged( bool checked );
    
private:
    typedef DTreeItem Node;

private:

    void    updateInfo( Node* parent );
    void    enumerateProjects( const QString& rootDir );
    void    cancelImport();
    void    cancelIndexer();
    bool    searchInfo( const QString& text );
    void    setCheckStateRecursive( DTreeItem* parent, Qt::CheckState checkState );
    void    invertCheckStateRecursive( DTreeItem* parent );
    void    createReport( 
                const QString& reportHeading, 
                Node* parent, DInsightReport& report, 
                int level, 
                int maxLevel, 
                QStringList& attachments,
                DJournals& journals,
                bool onlyChecked=true,
                bool replaceLabels=true );
    int     treeNodeCount( bool onlyChecked );
    int     treeNodeCountRecursive( Node* parent, bool onlyChecked );
    bool    isNode( const QString& key, const DRegExps& regExps );
    bool    isDocumentNode( const DImportFormat* format, const QString& key, const QString& value );
    bool    isFolderNode( const DImportFormat* format, const QString& key );
    bool    isDeleteNode( const DImportFormat* format, const QString& key );
    bool    isImportNode( const DImportFormat* format, const QString& key, const QString& value );
    bool    isChecksumNode( const DImportFormat* format, const QString& key );
    bool    isChecksumSourceNode( const DImportFormat* format, const QString& key );
    void    startSearch();
    void    cancelSearchThread();
    void    cancelSearch();
    void    cancelIndexerThread();
    void    cancelSearchDeamon();
    QString getInfoViewLabel( const DImportFormat* format, const char* key );
    QString getTreeItemMatchString( DTreeItem* item, const QString& searchText );
    QString getAttachmentMatchString( const QString& attacmentTextFileName );
    QString getHighlighMatchString( const QString& searchText, const QString& key, const QString& text, bool &match );
    void    addSearchResult( const QString& location, const QString& matchString, const QModelIndex& matchingIndex );
    DImport*findImport( const DTreeItem* item );
    QString createCombinedSearchConfigFile();
    void    startSearchDeamon();
    void    startIndexing();
    void    setupUiForImport();
    void    importDocumentClicked( const QString& document, QModelIndex& index );
    void    importDocument( const QString& document, DTreeItem* item );
    void    deleteImportFolder( const QString& document, QModelIndex& index );
    void    getNodesToExcludeFromSearch( DXmlParser::StringHash& nodesToExclude );
    void    treeNodeSelectionCountChanged();
    void    makeAbsolute( QString& filename, const QModelIndex& index );
    void    makeAbsolute( QString& filename, DTreeItem* item );

    
private:
    Ui::IngestToolMainWindow    m_Ui;
    QGridLayout*                m_InfoView;
    unsigned int                m_SearchResultMax;

    DImports                    m_Imports;
    DImport*                    m_CurrentImport;
    DPendingImports             m_PendingImports;
    DImports                    m_PendingIndexing;
    DTreeModel*                 m_Model;
    DSearchThread*              m_SearchThread;
    QStatusBar*                 m_StatusBar;
    QProgressBar*               m_ProgressBar;
    QLabel*                     m_ProgressBarInfo;
    QProcess*                   m_SearchDeamonProcess;
    DImportFormats*             m_ImportFormats;
    QString                     m_AttachmentParsing;
    QString                     m_ExportFile;
};


//============================================================================
// CLASS: DTableSearchResultCell

class DTableSearchResultCell : public QLabel
{
    Q_OBJECT

public:
    DTableSearchResultCell( const QString& text );
    
protected:
    void mouseDoubleClickEvent(QMouseEvent *event);

signals:
    void doubleClicked( DTableSearchResultCell* label );
};

class DTableSearchResultDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit DTableSearchResultDelegate(QObject *parent = nullptr);

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // DINSIGHTMAINWINDOW_H
