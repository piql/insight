/*****************************************************************************
**
**  Definition of DOsXTools class
**
**
**  Copyright (c) 2019 Piql AS. All rights reserved.
**
**
*****************************************************************************/

//  PROJECT INCLUDES
//
#include    "dosxtools.h"

//  SYSTEM INCLUDES
//

#if defined ( __APPLE_CC__ )
#include    <corefoundation/CFBundle.h>
#include    <corefoundation/CFString.h>
#endif


/****************************************************************************/
/*! \class  DOsXTools  dosxtools.h
 *  \brief  This class provides basic tools.
 */


//============================================================================
//  P U B L I C   I N T E R F A C E
//============================================================================

//----------------------------------------------------------------------------
/*! 
 *  \param  filename     Resource name
 *  \return path to resource, empty if not found
 *
 *  On OS-X applications are distributed in a package called bundles. This
 *  function returns the path to a file in the resource folder in the OS-X
 *  bundle.
 */

std::string  DOsXTools::GetBundleResource( const std::string& filename )
{
#if defined ( __APPLE_CC__ )
    CFStringRef cfStringRef;
    cfStringRef = CFStringCreateWithBytes( CFAllocatorGetDefault(), (UInt8*)filename.c_str(), filename.length(), CFStringGetSystemEncoding(), false  );
    
    CFURLRef appUrlRef = CFBundleCopyResourceURL( CFBundleGetMainBundle(), cfStringRef, nullptr, nullptr );

    std::string path = filename;
    if ( appUrlRef )
    {
      CFStringRef filePathRef = CFURLCopyPath( appUrlRef );
      const char *s =  CFStringGetCStringPtr( filePathRef, kCFStringEncodingUTF8 );
      if ( s ) path = s;
      // Release references
      CFRelease( filePathRef );
      CFRelease( appUrlRef );
    }
    CFRelease( cfStringRef );
    
    return path;
#else
    // Do nothing
    return filename;
#endif        
}
