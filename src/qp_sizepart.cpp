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

#include <qpainter.h>
#include <qcolor.h>
#include <qcursor.h>
#include <qstyle.h>
#include <qpixmap.h>
#include <QStyleOption>

#include "qp_sizepart.h"
#include "xpm/left_arrow.xpm"
#include "xpm/right_arrow.xpm"

#define ARROW_MARGIN 12

QP_SizeContainer::QP_SizeContainer(QWidget *parent, Qt::WFlags f)
    :QWidget(parent, f) {
    sizepartition = new QP_SizePartition(this);

    connect(sizepartition, SIGNAL(sigChangedStart()),
            this, SLOT(slotChangedStart()));
    connect(sizepartition, SIGNAL(sigChangedEnd()),
            this, SLOT(slotChangedEnd()));
    connect(sizepartition, SIGNAL(sigChangedPos()),
            this, SLOT(slotChangedPos()));
}

void QP_SizeContainer::setGeometry(int x, int y, int w, int h) {
    QWidget::setGeometry(x, y, w, h);

    /*---calculate the "virtual" start of the partition---*/
    PedSector partstart = _StartPartSector - _GrowStartPartSector;

    /*---calculate the "virtual" end of the disk partition---*/
    PedSector disksize = _GrowEndPartSector - _GrowStartPartSector;

    /*---calculate the size of the partition---*/
    PedSector partsize = _EndPartSector - _StartPartSector;

    /*---calculate the initial position of the partition---*/
    int x1 = (width()*partstart)/disksize;
    int x2 = (width()*partsize)/disksize;

    /*---calculate the minimal size in pixel---*/
    int MinSizeInPixel = (width()*_MinPartSector)/disksize;

    if (x2 <= ARROW_MARGIN*3) x2 = ARROW_MARGIN*3;
    if ((x1+x2) > width()) x1 = width() - x2;
        
    sizepartition->setMinSizeInPixel(MinSizeInPixel);
    sizepartition->setGeometry(x1, 0, x2, height());
}

void QP_SizeContainer::setMaxPartSector(PedSector MaxPartSector) {
    _MaxPartSector = MaxPartSector;
}

void QP_SizeContainer::setMinPartSector(PedSector MinPartSector) {
    _MinPartSector = MinPartSector;
}

void QP_SizeContainer::setStartPartSector(PedSector StartPartSector) {
    _StartPartSector = StartPartSector;
}

void QP_SizeContainer::setEndPartSector(PedSector EndPartSector) {
    _EndPartSector = EndPartSector;
}

void QP_SizeContainer::setGrowStartPartSector(PedSector GrowStartPartSector) {
    _GrowStartPartSector = GrowStartPartSector;
}

void QP_SizeContainer::setGrowEndPartSector(PedSector GrowEndPartSector) {
    _GrowEndPartSector = GrowEndPartSector;
}

void QP_SizeContainer::setMode(QTParted::actType moveresize) {
    sizepartition->setMode(moveresize);
}

void QP_SizeContainer::slotChangedStart() {
    /*---calculate the "virtual" end of the disk partition---*/
    PedSector disksize = _GrowEndPartSector - _GrowStartPartSector;
    
    _StartPartSector = (sizepartition->x()*disksize)/width() + _GrowStartPartSector;

    emit sigChangedStart(_StartPartSector);
}

void QP_SizeContainer::slotChangedEnd() {
    /*---calculate the "virtual" end of the disk partition---*/
    PedSector disksize = _GrowEndPartSector - _GrowStartPartSector;

    _EndPartSector = ((sizepartition->x()+sizepartition->width())*disksize)/width() + _GrowStartPartSector;

    emit sigChangedEnd(_EndPartSector);
}

void QP_SizeContainer::slotChangedPos() {
    /*---calculate the "virtual" end of the disk partition---*/
    PedSector disksize = _GrowEndPartSector - _GrowStartPartSector;
    
    _StartPartSector = (sizepartition->x()*disksize)/width() + _GrowStartPartSector;
    _EndPartSector = ((sizepartition->x()+sizepartition->width())*disksize)/width() + _GrowStartPartSector;

    emit sigChangedPos(_StartPartSector, _EndPartSector);
}






QP_SizePartition::QP_SizePartition(QWidget *parent, Qt::WFlags f)
    :QWidget(parent, f) {

    setMouseTracking(true);
    mouseposition = MP_none;
}

QP_SizePartition::~QP_SizePartition() {
}

void QP_SizePartition::setMinSizeInPixel(int MinSizeInPixel) {
    _MinSizeInPixel = MinSizeInPixel;
}

void QP_SizePartition::setMode(QTParted::actType moveresize) {
    _moveresize = moveresize;
}

void QP_SizePartition::mousePressEvent(QMouseEvent *e) {
    /*---save the position of the mouse in clickPos---*/
    clickPos = e->pos();

    /*if ((clickPos.x() < ARROW_MARGIN)
    &&  (_moveresize == QTParted::resize))
        mouseposition = MP_resize_left;
    else*/
    if (((width() - clickPos.x()) < ARROW_MARGIN)
    &&  (_moveresize == QTParted::resize))
        mouseposition = MP_resize_right;
    else
    if (_moveresize == QTParted::move)
        mouseposition = MP_move;
}

void QP_SizePartition::mouseReleaseEvent (QMouseEvent *) {
    mouseposition = MP_none;
}

void QP_SizePartition::mouseMoveEvent(QMouseEvent *e) {
    QPoint point;
    int x1 = x();             /*---left margin   ---*/
    int x2 = width();         /*---right margin  ---*/
    int y1 = y();             /*---top margin    ---*/
    int y2 = height();        /*---bottom margin ---*/
    int x1_old = x();         /*---old x1 position---*/
    int x2_old = width();     /*---old x2 position---*/

    if ((e->pos().x()         < ARROW_MARGIN)
    ||  (width()-e->pos().x() < ARROW_MARGIN))
        setCursor(QCursor(Qt::SizeHorCursor));
    else
    if ((e->pos().x() >= ARROW_MARGIN)
    &&  (e->pos().x() <= width()-5))
        setCursor(QCursor(Qt::SizeAllCursor));
    else
        setCursor(QCursor(Qt::ArrowCursor));

    /*---subtract of old mouse position from the new position---*/
    /*---...so in point we store the "offset" of mouse move! ---*/
    if (mouseposition != MP_none) point = e->pos()-clickPos;

    if (mouseposition == MP_resize_left) {
        /*---set the left margin to offset+old_left_positio---*/
        x1 = point.x()+x();

        /*---reduce the width to the offset---*/
        x2 = width()-point.x();

        /*---the filesystem has some data, so you must not strict < MinSize---*/
        if (x2 < _MinSizeInPixel) {
            x2 = _MinSizeInPixel;
            x1 = x() + width() - _MinSizeInPixel;
        }

        /*---you cannot resize with margin < of zero!---*/
        if (x1 < 0) {
            x1 = 0;
            x2 = x()+width();
        }

        /*---this is the minimum size of the widget---*/
        if (x2 <= ARROW_MARGIN*3) {
            x2 = ARROW_MARGIN*3;
            x1 = x()+width()-x2;
        }
            
        /*---move the widget!---*/
        setGeometry(x1, y1, x2, y2);
    }
    
    if (mouseposition == MP_resize_right) {
        /*---the left margin will not be changed!---*/
        x1 = x();

        /*---set the width to offset+old width---*/
        x2 = width()+point.x();

        /*---the filesystem has some data, so you must not strict < MinSize---*/
        if (x2 < _MinSizeInPixel) {
            x1 = x();
            x2 = _MinSizeInPixel;
        }

        if (x2 <= ARROW_MARGIN*3)
            x2 = ARROW_MARGIN*3;
        else
        if (x1+x2 >= parentWidget()->width())
            x2 = parentWidget()->width()-x1;
        else
            /*---save the position of the mouse in clickPos---*/
            clickPos = e->pos();

        /*---move the widget!---*/
        setGeometry(x1, y1, x2, y2);
    }

    if (mouseposition == MP_move) {
        /*---set the left margin to offset+old_left_positio---*/
        x1 = point.x()+x();

        /*---the width doesn't change!---*/
        x2 = width();

        /*---if you try to move partition in a position < of the left
         *---margin just move the partition to the zero position!---*/
        if (x1 < 0) {
            x1 = 0;
            x2 = width();
            clickPos = e->pos();
        }

        /*---if you try to move partition in a position > of the right
         *---margin just move the partition to rightest margin!   ---*/
        if ((x1+x2) > parentWidget()->width()) {
            x1 = parentWidget()->width()-width();
            x2 = width();
            clickPos = e->pos();
        }

        /*---move the widget!---*/
        setGeometry(x1, y1, x2, y2);
    }

    if ((x1_old != x1)
    &&  (x1_old+x2_old != x1+x2))
        emit sigChangedPos();
    else {
        if (x1_old != x1) emit sigChangedStart();
        if (x1_old+x2_old != x1+x2) emit sigChangedEnd();
    }
}

void QP_SizePartition::paintEvent(QPaintEvent *) {
    QPainter paint(this);

    paint.fillRect(rect(), Qt::green);

    /*---fill with white color---*/
    paint.fillRect(ARROW_MARGIN, 6, width()-ARROW_MARGIN*2, height()-12, Qt::white);

    /*---draw two small arrow on the border---*/
    paint.drawPixmap(QPoint(2, (height()-8)/2), QPixmap(left_arrow_xpm));
    paint.drawPixmap(QPoint(width()-ARROW_MARGIN+2, (height()-8)/2), QPixmap(right_arrow_xpm));
    

    /*---draw a small border around the widget---*/
    QStyleOption opt;
    opt.initFrom(this);
    opt.state = QStyle::State_Active|QStyle::State_Enabled|QStyle::State_HasFocus;
    style()->drawPrimitive(QStyle::PE_FrameFocusRect, &opt, &paint, this);
}
