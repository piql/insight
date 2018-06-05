#ifndef DINSIGHTSEARCHEXPRESSION_H
#define DINSIGHTSEARCHEXPRESSION_H

/*****************************************************************************
**  
**  Definition of the DInsightSearchExpression class
**
**  Creation date:  2017/09/08
**  Created by:     Ole Liabo
**
**
**  Copyright (c) 2017 Piql AS. All rights reserved.
**
*****************************************************************************/

//  PROJECT INCLUDES
//


//  QT INCLUDES
//
#include   <QString>
#include   <QRegularExpression>

//  FORWARD DECLARATIONS
//
class DTreeItem;

//============================================================================
// CLASS: DInsightSearchExpression

class DInsightSearchExpression
{
public:
    DInsightSearchExpression();

    void updateText( const QString& text );
    bool match( DTreeItem* node );

private:
    enum { PLAIN, REGEXP, XPATH };
    int                 m_Type;
    QString             m_Text;
    QRegularExpression  m_RegExp;
    QStringList         m_Words;
};


#endif // DINSIGHTSEARCHEXPRESSION_H
