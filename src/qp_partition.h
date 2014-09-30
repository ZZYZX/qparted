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

/* About QP_Partition class:
 *
 * This is a widget derived from QP_PartWidget that display a primary or a logical
 * partition. You can use it as a primary partition attaching it to the QP_ListChart
 * or you can use it as a logical partition attaching it to the QP_Extended
 */

#ifndef QP_PARTITION_H
#define QP_PARTITION_H

#include <qwidget.h>
#include "qp_partwidget.h"

class QP_Partition : public QP_PartWidget {
Q_OBJECT

public:
	QP_Partition(QP_PartInfo *, QWidget *parent=0, Qt::WFlags f = 0);
	~QP_Partition();
};

#endif
