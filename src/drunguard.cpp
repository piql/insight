/***************************************************************************
**
**  Implementation of the DRunGuard class
**
**  Creation date:  2018/01/15
**  Created by:     Ole Liabo
**
**
**  Copyright (c) 2018 Piql AS. All rights reserved.
**
***************************************************************************/

//  PROJECT INCLUDES
//
#include "drunguard.h"

//  QT INCLUDES
//
#include <QCryptographicHash>

//----------------------------------------------------------------------------
/*! 
 *  Generate hash from unique key.
 */

static QString generateKeyHash( const QString& key, const QString& salt )
{
    QByteArray data;

    data.append( key.toUtf8() );
    data.append( salt.toUtf8() );
    data = QCryptographicHash::hash( data, QCryptographicHash::Sha1 ).toHex();

    return data;
}


/****************************************************************************/
/*! \class DRunGuard drunguard.h
 *  \ingroup Insight
 *  \brief Ensure one application instance.
 */

//----------------------------------------------------------------------------
/*! 
 *  Constructor.
 */

DRunGuard::DRunGuard( const QString& key )
    : key( key )
    , memLockKey( generateKeyHash( key, "_memLockKey" ) )
    , sharedmemKey( generateKeyHash( key, "_sharedmemKey" ) )
    , sharedMem( sharedmemKey )
    , memLock( memLockKey, 1 )
{
    memLock.acquire();
    {
        QSharedMemory fix( sharedmemKey );    // Fix for *nix: http://habrahabr.ru/post/173281/
        fix.attach();
    }
    memLock.release();

}


//----------------------------------------------------------------------------
/*! 
 *  Destructor.
 */

DRunGuard::~DRunGuard()
{
    release();
}


//----------------------------------------------------------------------------
/*! 
 *  Return true if another application instance is running.
 */

bool DRunGuard::isAnotherRunning()
{
    if ( sharedMem.isAttached() )
        return false;

    memLock.acquire();
    const bool isRunning = sharedMem.attach();
    if ( isRunning )
        sharedMem.detach();
    memLock.release();

    return isRunning;
}


//----------------------------------------------------------------------------
/*! 
 *  Return true if another application is not running.
 */

bool DRunGuard::tryToRun()
{
    if ( isAnotherRunning() )   // Extra check
        return false;

    memLock.acquire();
    const bool result = sharedMem.create( sizeof( quint64 ) );
    memLock.release();
    if ( !result )
    {
        release();
        return false;
    }

    return true;
}


//----------------------------------------------------------------------------
/*! 
 *  Release shared memory.
 */

void DRunGuard::release()
{
    memLock.acquire();
    if ( sharedMem.isAttached() )
    {
        sharedMem.detach();
    }
    memLock.release();
}
