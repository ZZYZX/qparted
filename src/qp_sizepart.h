/*
    qtparted - a frontend to libparted for manipulating disk partitions
    Copyright (C) 2002-2003 Vanni Brutto

    Vanni Brutto <zanac (-at-) libero dot it>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* About qp_sizepart.cpp
 *
 * A widget used for resize/move partitions
 *
 */

#include <qwidget.h>
#include "qtparted.h"
#include "qp_libparted.h"
//Added by qt3to4:
#include <QPaintEvent>
#include <QMouseEvent>

typedef enum {
    MP_none,
    MP_move,
    MP_resize_left,
    MP_resize_right
} MousePosition;

class QP_SizePartition;

class QP_SizeContainer : public QWidget {
Q_OBJECT

public:
    QP_SizeContainer(QWidget *parent=0, Qt::WFlags f = 0);
    void setGeometry(int x, int y, int w, int h);
    void setMaxPartSector(PedSector);
    void setMinPartSector(PedSector);
    void setStartPartSector(PedSector);
    void setEndPartSector(PedSector);
    void setGrowStartPartSector(PedSector);
    void setGrowEndPartSector(PedSector);
    void setMode(QTParted::actType);

protected slots:
    void slotChangedStart();
    void slotChangedEnd();
    void slotChangedPos();

private:
    QP_SizePartition *sizepartition;
    PedSector _MaxPartSector;
    PedSector _MinPartSector;
    PedSector _StartPartSector;
    PedSector _EndPartSector;
    PedSector _GrowStartPartSector;
    PedSector _GrowEndPartSector;

signals:
    void sigChangedStart(PedSector);
    void sigChangedEnd(PedSector);
    void sigChangedPos(PedSector, PedSector);
};

class QP_SizePartition : public QWidget {
Q_OBJECT

public:
    QP_SizePartition(QWidget *parent=0, Qt::WFlags f = 0);
    ~QP_SizePartition();
    void setMinSizeInPixel(int);
    void setMode(QTParted::actType);

protected:
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent (QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void paintEvent(QPaintEvent *);

private:
    QTParted::actType _moveresize;
    QPoint clickPos;
    MousePosition mouseposition;
    int _MinSizeInPixel;

signals:
    void sigChangedStart();
    void sigChangedEnd();
    void sigChangedPos();
};
