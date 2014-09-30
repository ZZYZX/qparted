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

/* About QP_ListChart class:
 *
 * This is a widget derived from QP_PartList that display a "chart" of partitions
 * Using methods "addPrimary" and "addLogical" you can draw the chart easily ;)
 */

#ifndef QP_LISTCHART_H
#define QP_LISTCHART_H

#include <QWidget>
#include <QList>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include "qp_partlist.h"
#include "qp_partition.h"
#include "qp_extended.h"

class QP_ChartItem;

/*---Implementazione della lista delle partizioni su ListChart---*/
class QP_ListChart : public QP_PartList {
Q_OBJECT

public:
    QP_ListChart(QWidget *parent=0, Qt::WFlags f = 0);
    ~QP_ListChart();
    void setselPartInfo(QP_PartInfo *);   /*---change the selected partition                ---*/
    void clear();                         /*---clear the chart (remove attached partition)  ---*/
    void addPrimary(QP_PartInfo *);       /*---add a Primary or Extended partition          ---*/
    void addLogical(QP_PartInfo *);       /*---add a Logical partition                      ---*/
    void draw();                          /*---resize and redraw partitions inside listchart---*/
    void draw_extended();                 /*---resize and redraw partitions in QP_Extended  ---*/

protected:
    QWidget *container;                   /*---Widget in which you attach partitions        ---*/
    QP_PartInfo *extpartinfo;             /*---QP_Extended contain logical partitions       ---*/
    QList<QP_ChartItem*> partlist;      /*---list of the primary/extended partitions      ---*/
    QList<QP_ChartItem*> logilist;      /*---list of the logical partitions               ---*/
    void paintEvent(QPaintEvent *);       /*---reimplemented to draw a small border ;)      ---*/
    void resizeEvent(QResizeEvent *);     /*---reimplemented to resize partitions inside    ---*/
    void mouseReleaseEvent(QMouseEvent *);/*---reimplemented to get mouse popup             ---*/
    void setSignals(QP_PartWidget *);     /*---connect sigPopup and sigSelectPart signals   ---*/
};

#endif
