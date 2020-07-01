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
