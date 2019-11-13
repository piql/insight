#ifndef DRUNGUARD_H
#define DRUNGUARD_H

/*****************************************************************************
**  
**  Definition of the DRunGuard class
**
**  Creation date:  2018/01/15
**  Created by:     Ole Liabo
**  Note:           Based on https://stackoverflow.com/questions/5006547/qt-best-practice-for-a-single-instance-app-protection
**
**
**  Copyright (c) 2018 Piql AS. All rights reserved.
**
*****************************************************************************/

//  QT INCLUDES
//
#include <QObject>
#include <QSharedMemory>
#include <QSystemSemaphore>


//============================================================================
// CLASS: DRunGuard

class DRunGuard
{

public:
    DRunGuard( const QString& key );
    ~DRunGuard();

    bool isAnotherRunning();
    bool tryToRun();
    void release();

private:
    const QString key;
    const QString memLockKey;
    const QString sharedmemKey;

    QSharedMemory sharedMem;
    QSystemSemaphore memLock;

    Q_DISABLE_COPY( DRunGuard )
};


#endif // RUNGUARD_H
