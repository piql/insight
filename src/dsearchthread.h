#ifndef DSEARCHTHREAD_H
#define DSEARCHTHREAD_H

/*****************************************************************************
**  
**  Definition of the DSearchThread class
**
**  Creation date:  2017/12/14
**  Created by:     Ole Liabo
**
**
**  Copyright (c) 2017 Piql AS. All rights reserved.
**
*****************************************************************************/

//  PROJECT INCLUDES
//
#include    "dimport.h"
#include    "dxmlparser.h"

//  SYSTEM INCLUDES
//
#include    <vector>

//  QT INCLUDES
//
#include    <QThread>
#include    <QString>

//  FORWARD DECLARATIONS
//
class DTreeModel;
class DTreeItem;

//============================================================================
// CLASS: DSearchThread

class DSearchThread : public QThread
{
    Q_OBJECT

public:
    enum SearchMode { SEARCH_MODE_TREE = 0x1, SEARCH_MODE_ATTACHMENTS = 0x2, SEARCH_MODE_CASE_INSENSITIVE = 0x4 };
    enum SearchResult { SEARCH_UNDEFINED, SEARCH_ABORTED, SEARCH_MAX_FOUND, SEARCH_NOT_FOUND, SEARCH_FOUND };

public:

    DSearchThread( 
        DTreeModel* model,
        DImports* imports, 
        const QString& searchString, 
        unsigned int maxMatchCount,
        unsigned int searchModes,
        DXmlParser::StringHash& nodesToExclude );

    unsigned int matchCount();
    unsigned int pageCount();
    unsigned int currentPage();
    bool         hasMorePages();
    void         gotoNextPage();
    void         gotoPrevPage();
    void         gotoFirstPage();
    
signals:
    void        searchProgress( float progress );
    void        searchMatch( const QModelIndex& matchingIndex );
    void        attachmentMatch( int importIndex, int attachmentIndex );

private:
    void        run();
    SearchResult searchTree( DTreeItem* node, const char* text, const QModelIndex &parent, bool& startFound );

private:
    DTreeModel*             m_Model;
    DImports*               m_Imports;
    QString                 m_SearchString;
    unsigned int            m_SearchStringLength;
    unsigned int            m_NodeCount;
    unsigned int            m_MatchCount;
    unsigned int            m_MaxMatchCount;
    unsigned int            m_SearchModes;
    std::vector<DTreeItem*> m_Pages;
    unsigned int            m_CurrentPage;
    SearchResult            m_SearchResult;
    DXmlParser::StringHash  m_NodesToExclude;
};


#endif // DSEARCHTHREAD_H
