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
**  Copyright (c) 2017 Piql AS. All rights reserved.
**
*****************************************************************************/

//  PROJECT INCLUDES
//
#include    "ui_dinsightmainwindow.h"
#include    "dinsightconfig.h"
#include    "dtreeitem.h"
#include    "dimport.h"
#include    "dxmlparser.h"

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


//============================================================================
// CLASS: DInsightMainWindow

class DInsightMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    DInsightMainWindow();
    virtual ~DInsightMainWindow();

public:
    static void    ReplaceString( QString& key, const DRegExps& regExps );

public slots:
    void importFileFinished( bool ok );
    void indexingFinished( bool ok );
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
    void    importFile( const QString& fileName );
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
                bool onlyChecked=true,
                bool replaceLabels=true );
    int     treeNodeCount( bool onlyChecked );
    int     treeNodeCountRecursive( Node* parent, bool onlyChecked );
    bool    isNode( const QString& key, DRegExps& regExps );
    bool    isDocumentNode( const QString& key );
    bool    isFolderNode( const QString& key );
    bool    isDeleteNode( const QString& key );
    bool    isImportNode( const QString& key );
    void    startSearch();
    void    cancelSearchThread();
    void    cancelSearch();
    void    cancelIndexerThread();
    void    cancelSearchDeamon();
    QString getInfoViewLabel( const char* key );
    QString getTreeItemMatchString( DTreeItem* item, const QString& searchText );
    QString getAttachmentMatchString( const QString& attacmentTextFileName );
    QString getHighlighMatchString( const QString& searchText, const QString& key, const QString& text, bool &match );
    void    addSearchResult( const QString& location, const QString& matchString, const QModelIndex& matchingIndex );
    DImport*findImport( DTreeItem* item );
    QString createCombinedSeachConfigFile();
    void    startSearchDeamon();
    void    setupUiForImport();
    void    importDocumentClicked( const QString& document, QModelIndex& index );
    void    deleteImportFolder( const QString& document, QModelIndex& index );
    void    getNodesToExcludeFromSearch( DXmlParser::StringHash& nodesToExclude );
    void    treeNodeSelectionCountChanged();
    
private:
    Ui::IngestToolMainWindow    m_Ui;
    DRegExps                    m_TreeViewLabelRegExp;
    DRegExps                    m_TreeViewNodeRegExp;
    DRegExps                    m_InfoViewLabelRegExp;
    DRegExps                    m_DocumentTypeRegExp;
    DRegExps                    m_FolderTypeRegExp;
    DRegExps                    m_DeleteTypeRegExp;
    DRegExps                    m_ImportTypeRegExp;
    QStringList                 m_NodeStatisticsXQuery;
    unsigned int                m_SearchResultMax;

    DImports                    m_Imports;
    DImport*                    m_CurrentImport;
    DTreeModel*                 m_Model;
    DSearchThread*              m_SearchThread;
    QStatusBar*                 m_StatusBar;
    QProgressBar*               m_ProgressBar;
    QLabel*                     m_ProgressBarInfo;
    QProcess*                   m_SearchDeamonProcess;
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
    explicit DTableSearchResultDelegate(QObject *parent = 0);

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // DINSIGHTMAINWINDOW_H
