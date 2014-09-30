/*
	qtparted - a frontend to libparted for manipulating disk partitions
	Copyright (C) 2002-2003 Vanni Brutto <zanac (-at-) libero dot it>
	Copyright (C) 2007-2008 David Tio <deux@arklinux.org>

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

#include <QPainter>
#include <QColor>
#include <QCursor>
#include <QStyle>
#include <QStyleOption>
#include <QPixmap>

#include "qp_partwidget.h"
#include "qp_filesystem.h"

QP_PartWidget::QP_PartWidget(QP_PartInfo *pinfo, QWidget *parent, Qt::WFlags f)
	:QWidget(parent, f) {
	_Selected = false;
	partinfo = pinfo;   //Pointer to the QP_PartInfo class wich contain info about the partition
}

QP_PartWidget::~QP_PartWidget() {
}

/*--- the method setSelected is call when the user select a line from listview or a partition ---*
 *--- from the listchart. Partition that is selected is draw with a small border around	   ---*/
void QP_PartWidget::setSelected(bool selected) {
	_Selected = selected;
}

bool QP_PartWidget::Selected() {
	return _Selected;
}

void QP_PartWidget::mousePressEvent(QMouseEvent *e) {
	emit sigSelectPart(partinfo);

	if (e->button() == Qt::RightButton) emit sigPopup();
	
	setFocus(); //If the widget that has focus it is draw with a border around
}

void QP_PartWidget::paintEvent(QPaintEvent *) {
	/*---the paint area---*/
	QPainter paint(this);

	if (partinfo->type == QTParted::extended) {
		QColor color = Qt::cyan;
		paint.fillRect(2, 2, width()-4, height()-4, color);
	} else {
		int used_size;

		/*---draw a rect with filesystem color---*/
		paint.fillRect(5, 5, width()-10, height()-10, partinfo->fsspec->color());

		/*---fill with white color---*/
		paint.fillRect(8, 8, width()-16, height()-16, Qt::white);

		/*---if you know how much space is used fill calculate the size for fill---*
		 *---with yellow color							  ---*/
		if (partinfo->min_size != -1) {
			float size = partinfo->mb_end()-partinfo->mb_start();
			used_size = (int)(((width()-16)*partinfo->mb_min_size()) / size);
		} else {
			used_size = width()-16;
		}

		/*---if it is an empty partition or you cannot know how much filesystem is used...
		 * just fill with the right color!---*/
		if ((partinfo->isFree())
		||  (partinfo->min_size == -1)) {
			/*---just fill with the grey color (in fsspec!)...
			 * but of course if this is a "true" partition (min_size == -1)
			 * then fill with the right color---*/
			paint.fillRect(8, 8, used_size, height()-16, partinfo->fsspec->color());
		} else {
			/*---just fill with yellow in the sized used!---*/
			paint.fillRect(8, 8, used_size, height()-16, Qt::yellow);
		}
	}

	/*--- draw a small border around if the partition has focus ---*/
	if (Selected()) {
		QStyleOption opt;
		opt.initFrom(this);
		opt.state=QStyle::State_Active|QStyle::State_Enabled|QStyle::State_HasFocus;
		style()->drawPrimitive(QStyle::PE_FrameFocusRect, &opt, &paint, this);
	}

	/*---get the font metrics of the paint area---*/
	QFontMetrics fm = paint.fontMetrics();

	/*---create the showed label---*/
	QString label;
	QString partname = partinfo->partname().mid(5); //remove the "/dev/" from the name

	/*---if it was not possible to get how the filesystem is filled...---*/
	if (partinfo->min_size == -1) {
		label = QString("%1") .arg(partname);
	} else {
		/*---try to set the label with (device) GB---*/
		QString partsize = MB2String(partinfo->mb_min_size());
		label = QString("%1 (%2)")
						.arg(partname)
						.arg(partsize);

	
		/*---if the size of the label is too big just use the size!---*/
		QSize sizeText = fm.size(Qt::TextSingleLine, label);
		if (sizeText.width() > width()-10) {
			/*---use the size in MByte or in GByte---*/
			if (partinfo->mb_min_size() > 1024) {
				partsize.sprintf("%3.2fGB", partinfo->mb_min_size() / 1024);
				label = QString("(%1)")
								.arg(partsize);
			} else {
				partsize.sprintf("%3.2fMB", partinfo->mb_min_size());
				label = QString("(%1)")
								.arg(partsize);
			}
		}
	}

	/*---get the size of the label---*/
	QSize sizeText = fm.size(Qt::TextSingleLine, label);

	/*---if this is a primary or a logical partition draw label and pixmap---*/
	if (partinfo->type != QTParted::extended) {
		if (sizeText.width() <= width()-10) {
			paint.setPen(QPen(Qt::black, 1)); 

			QPoint pt;
			pt.setX((width()-sizeText.width())/2);
			//pt.setY((height()-sizeText.height())/2 + 10);
			pt.setY((height()-sizeText.height()));
			paint.drawText(pt, label);
		}

		if (partinfo->fsspec->pixmap().width() < (width()-12)) {
			paint.drawPixmap(3, 3, partinfo->fsspec->pixmap());
		}
	}
}
