#ifndef     DOSXTOOLS_H
#define     DOSXTOOLS_H

/*****************************************************************************
**
**  Definition of DOsXTools class
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
