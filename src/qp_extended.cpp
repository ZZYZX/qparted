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

#include "qp_extended.h"


QP_Extended::QP_Extended(QP_PartInfo *pinfo, QWidget *parent, Qt::WFlags f)
    :QP_PartWidget(pinfo, parent, f) {

    container = new QWidget(this);
}

QP_Extended::~QP_Extended() {
}

QP_Partition *QP_Extended::addLogical(QP_PartInfo *partinfo) {
    QP_Partition *logical = new QP_Partition(partinfo, container);
    return logical;
}

void QP_Extended::resizeEvent(QResizeEvent *e) {
    int w = e->size().width();
    int h = e->size().height();
    container->setGeometry(6, 6, w-12, h-12);
}
