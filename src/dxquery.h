#ifndef DREGEXP_H
#define DREGEXP_H

/*****************************************************************************
**  
**  Definition of the DRegExp class
**
**  Creation date:  2017/08/16
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
#include   <QRegularExpression>
#include   <QVector>

//  FORWARD DECLARATIONS
//

//============================================================================
// CLASS: DRegExps
typedef QRegularExpression DRegExp;
typedef QVector<DRegExp>   DRegExps;


#endif // DREGEXP_H
