#ifndef DXMLPARSER_H
#define DXMLPARSER_H

/*****************************************************************************
**  
**  Definition of the DXmlParser class
**
**  Creation date:  2017/11/06
**  Created by:     Ole Liabo
**
**
**  Copyright (c) 2017 Piql AS. All rights reserved.
**
*****************************************************************************/

//  PROJECT INCLUDES
//
#include    "dtreeitem.h"

//  SYSTEM INCLUDES
//
#include    <unordered_set>

//  QT INCLUDES
//
#include    <QThread>
#include    <QString>

//  FORWARD DECLARATIONS
//
class DTreeModel;

//============================================================================
// CLASS: DXmlParser

class DXmlParser : public QThread
{
    Q_OBJECT
public:
    struct UnorderedSetEqualString
    {
        bool operator()(const char* s1, const char* s2) const
        {
            return strcmp(s1, s2) == 0;
        }
    };

    struct UnorderedSetDereference
    {
        size_t operator()(const char* p) const
        {
            size_t result = 0;
            const size_t prime = 31;
            while ( *p )
            {
                result = *p + (result * prime);
                p++;
            }
            return result;
        }
    };
    typedef std::unordered_set<const char*, UnorderedSetDereference, UnorderedSetEqualString> StringHash;
    typedef StringHash::const_iterator StringHashIterator;

public:
    DXmlParser( DTreeItems* treeItems, const QString& fileName, DTreeModel* model, DTreeRootItem* rootNode, const DImportFormat* importFormat );
    virtual ~DXmlParser();

    unsigned long         nodeCount();
    DTreeRootItem*        root();
    const DImportFormat*  importFormat() const;
    bool                  loadedOK() const;

signals:
    void       nodesReady( unsigned long count, float progress );    

public:
    static const char*  AddToNodeHashMap( const char* text );
    static const char*  AddToHashMap( const char* text );
    static StringHashIterator NodeHashMapBegin();
    static StringHashIterator NodeHashMapEnd();

private:
    virtual void run();

protected:
    void         reportProgress( unsigned long nodeCount, float progress );

protected:
    DTreeItems*           m_TreeItems;
    QString               m_Filename;
    DTreeModel*           m_Model;
    DTreeRootItem*        m_RootNode;
    const DImportFormat*  m_ImportFormat;
    bool                  m_LoadedOk;

    // Statistics
    unsigned long         m_NodeCount;

    friend class DXmlContext;
};


#endif // DXMLPARSER_H
