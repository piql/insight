#ifndef DLEAFMATCHER_H
#define DLEAFMATCHER_H

/*****************************************************************************
**  
**  Definition of the DLeafMatcher class
**
**  Creation date:  2019/10/16
**  Created by:     Ole Liabo
**
**
**  Copyright (c) 2019 Piql AS. All rights reserved.
**
*****************************************************************************/

//  PROJECT INCLUDES
//


//  QT INCLUDES
//
#include   <QRegularExpression>
#include   <QVector>

//  FORWARD DECLARATIONS
//

//============================================================================
// CLASS: DLeafMatcher
class DLeafMatcher
{
public:
    QRegularExpression      m_LeafMatch;
    QRegularExpression      m_ContentMatch;

public:
    static bool CreateFromString( DLeafMatcher& matcher, const QString& str );
};

typedef QVector<DLeafMatcher>   DLeafMatchers;


#endif // DLEAFMATCHER_H
