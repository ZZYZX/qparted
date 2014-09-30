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

/* About QP_PartWidget class:
 *
 * This is a generic widget used to draw logical, extended and primary partitions.
 * The class is inherited by QP_Partition (for draw primary and logical) and by
 * QP_Extended (for draw extended partition)
 *
 * You must not use this widget, you must use QP_Partition or QP_Extended insteed!
 */

#ifndef QP_PARTWIDGET_H
#define QP_PARTWIDGET_H

#include <QPaintEvent>
#include <QMouseEvent>
#include <QWidget>
#include "qtparted.h"
#include "qp_libparted.h"


class QP_PartWidget : public QWidget {
	Q_OBJECT
public:
	QP_PartWidget(QP_PartInfo *, QWidget *parent=0, Qt::WFlags f = 0);
	~QP_PartWidget();
	void setSelected(bool);			/*---this give the focus to the partition---*/
	bool Selected();			/*---return if the partition was selected---*/

protected:
	QP_PartInfo *partinfo;			/*---pointer to the class with info about ---*
						 *---the partition			  ---*/
	void mousePressEvent(QMouseEvent *);
	void paintEvent(QPaintEvent *);				 
	bool _Selected;

signals:
	void sigSelectPart(QP_PartInfo *);	/*---emitted when you change the selection  ---*/
	void sigPopup();			/*---emitted when you want to popup a menu  ---*/
};

#endif
