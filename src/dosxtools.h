#ifndef     DOSXTOOLS_H
#define     DOSXTOOLS_H

/*****************************************************************************
**
**  Definition of DOsXTools class
**
**
**  Copyright (c) 2019 Piql AS. All rights reserved.
**
**
*****************************************************************************/

//  SYSTEM INCLUDES
//
#include    <string>

//============================================================================
//  CLASS:  DOsXTools

class  DOsXTools
{
public:

    static std::string  GetBundleResource( const std::string& filename );

};


#endif  /* DOSXTOOLS_H */
