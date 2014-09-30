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

#include <qpainter.h>

#include "qp_libparted.h"
#include "qp_listchart.h"

#define MIN_HDWIDTH		 190
#define MIN_HDHEIGHT		 70
#define MIN_PARTITION_WIDTH  16

class QP_ChartItem {
public:
	QP_PartWidget *partwidget;
	QP_PartInfo *partinfo;
	int width;
};

QP_ListChart::QP_ListChart(QWidget *parent, Qt::WFlags f)
	:QP_PartList(parent, f) {

	/*---prevent a segfualt in ::clear method!---*/
	_selPartInfo = NULL;

	container = new QWidget(this);

	setMinimumWidth(MIN_HDWIDTH);
	setMinimumHeight(MIN_HDHEIGHT);
}

QP_ListChart::~QP_ListChart() {
}

void QP_ListChart::setselPartInfo(QP_PartInfo *selpartinfo) {
	/*---if a partition was selected remove the selection to it---*/
	/*---please note that in ::clear() selPartInfo is "nulled!"---*/
	if (selPartInfo() && selPartInfo()->partwidget) {
		/*---redraw the (un)selected partition---*/
		QP_PartWidget *selwidget = selPartInfo()->partwidget;
		selwidget->setSelected(false);
		selwidget->update();
	}

	/*---call the ancestor to change the selection---*/
	QP_PartList::setselPartInfo(selpartinfo);

	/*---redraw the selected partition---*/
	QP_PartWidget *selwidget = selPartInfo()->partwidget;
	selwidget->setSelected(true);
	selwidget->update();
}

void QP_ListChart::clear() {
	/*--just "unselect" a partition. This prevent segfault in setselpartinfo---*/
	_selPartInfo = NULL;
	
	/*---destroy logical partition widgets attached---*/
	QListIterator <QP_ChartItem*> it(logilist);
	while(it.hasNext())
	{
		delete it.next()->partwidget;
	}

	/*---destroy primary/extended partition widgets attached---*/
	QListIterator <QP_ChartItem *> partit(partlist);
	while(partit.hasNext())
	{
		delete partit.next()->partwidget;
	}
	/*---clear the pointer list of partition  ---*/
	/*---this is usefull to avoid memory leak!---*/
	logilist.clear();
	partlist.clear();
}

void QP_ListChart::addPrimary(QP_PartInfo *partinfo) {
	/*---append to the partition list---*/
	QP_ChartItem *item = new QP_ChartItem();
	item->partinfo = partinfo;
	partlist.append(item); //aleste

	if (partinfo->type == QTParted::primary) {
		/*---create a new primary partition  ---*/
		partinfo->partwidget = new QP_Partition(partinfo, container);
	} else {
		/*---create a new extended partition ---*/
		partinfo->partwidget = new QP_Extended(partinfo, container);
		extpartinfo = partinfo;
	}

	//aleste
	item->partwidget = partinfo->partwidget;

	/*---show the widget---*/
	partinfo->partwidget->show();

	/*---connect sigPopup and sigSelectPart---*/
	setSignals(partinfo->partwidget);
}

void QP_ListChart::addLogical(QP_PartInfo *partinfo) {
	/*---append to the partition list---*/
	QP_ChartItem *item = new QP_ChartItem();
	item->partinfo = partinfo;
	logilist.append(item);

	QP_Extended *extended = (QP_Extended *)extpartinfo->partwidget;

	/*---add a logical partition the the extended---*/
	partinfo->partwidget = extended->addLogical(partinfo);
	item->partwidget = partinfo->partwidget;//aleste
	
	/*---show the widget---*/
	partinfo->partwidget->show();

	/*---connect sigPopup and sigSelectPart---*/
	setSignals(partinfo->partwidget);
}

void QP_ListChart::draw() {	
	/*---in this method partitions are resized to fit the container   ---*/
	/*---only primaries and extended partitions are resized!		  ---*/
	/*---Logicals partitions are resized inside draw_extended method! ---*/

	/*---return if the partition list is empty... for example at startup ;)---*/
	if (partlist.count() == 0) return;

	/*---totwidth contain the minimal width of the QP_ListChart---*/
	int totwidth = 0;

	/*---the first "for" loop is needed to calculate the totwidth---*/

	foreach(QP_ChartItem* p, partlist)
	{
		totwidth += p->width = MIN_PARTITION_WIDTH;
		if (totwidth > container->width())
			qFatal("Error calculating size of QP_ListChar! %d\n", container->width());
	}

	/*---resize the QP_ListChart---*/
	if (totwidth > MIN_HDWIDTH)
		setMinimumWidth(totwidth);

	/*---this "for" loop is needed to calculate how much partitions can grow---*/
	/*---it loop until partition fit the container						  ---*/
	QP_ChartItem* p = NULL;
	QListIterator <QP_ChartItem*> partit(partlist);
	
	while (totwidth < container->width()) {
		if (p == NULL)
	{
		if(partit.hasNext())
		{
		p = partit.next();
		}
	}
		/*---calculate the size of the partition---*/
		float mbsize = p->partinfo->mb_end() - p->partinfo->mb_start();

		/*---calculate the width of the partition---*/
		int newwidth = (int)((mbsize*container->width())/mb_hdsize() + 1);

		/*---if the actual width is less then the width calculate increment it---*/
		if (p->width < newwidth) {
			p->width++;
			totwidth++;
		}

		if(partit.hasNext())
		{
			p = partit.next();
		}
		else
		{
			p = NULL;
			partit.toFront();
		}
	}

	/*---the last "for" loop is needed to resize the partitions with the calculated width---*/
	int lastleft = 0;
	totwidth = 0;
	foreach(QP_ChartItem* p, partlist)
	{	
		QP_PartWidget *partwidget = p->partwidget;
		partwidget->setGeometry(lastleft, 0, p->width, container->height());
		lastleft += p->width;

		if ((p->partinfo->type == QTParted::extended)
		&&  (logilist.count()  != 0)) {
			/*---loop inside for size for logicals partitions ;)---*/
			draw_extended();

			/*---draw_extended changed the extended size!---*/
			if (p->width != partwidget->width())
				lastleft += (partwidget->width() - p->width);
		}
		totwidth += partwidget->width();
		if ((partwidget->x()+partwidget->width()) > container->width()) {
			container->setMinimumWidth(totwidth);
			setMinimumWidth(totwidth+12);
		}
	}
}

void QP_ListChart::draw_extended() {
	/*taccon: cambiare extended->container!*/
	/*---in this method logical partitions are resized to fit the container---*/

	/*---get extended partition---*/
	QP_Extended *extended = (QP_Extended *)extpartinfo->partwidget;

	/*---totwidth contain the minimal width of the Extended partition---*/
	int totwidth = 0;

	/*---the first "for" loop is needed to calculate the totwidth---*/
	foreach( QP_ChartItem* p, logilist )
	{
		totwidth += p->width = MIN_PARTITION_WIDTH;
		//if (totwidth > extended->container->width())
		//	qFatal("Error calculating size of QP_ListChar! %d\n", container->width());
	}

	/*---if the width of the widget is not enough for extended resize it!---*/
	if (totwidth != extended->container->width())
		extended->setMinimumWidth(extended->width()+(totwidth - extended->container->width()));

	/*---this "for" loop is needed to calculate how much partitions can grow---*/
	/*---it loop until partition fit the container						  ---*/
	QP_ChartItem* p = NULL;
	QListIterator <QP_ChartItem*> logiit(logilist);

	while (totwidth < extended->container->width()) {
		if (!p)
		{
			if(logiit.hasNext())
				p = logiit.next();
		}

		/*---calculate the size of the extended partition---*/
		float extsize = extpartinfo->end - extpartinfo->start;

		/*---calculate the size of the partition---*/
		float mbsize = p->partinfo->end - p->partinfo->start;

		/*---calculate the width of the partition---*/
		int newwidth = (int)((mbsize*extended->container->width())/extsize + 1);

		/*---if the actual width is less then the width calculate increment it---*/
		if (p->width < newwidth) {
			p->width++;
			totwidth++;
		}

		p = logiit.next();
	}

	/*---the last "for" loop is needed to resize the partitions with the calculated width---*/
	int lastleft = 0;
	foreach( QP_ChartItem* p, logilist )
	{
		QP_PartWidget *partwidget = p->partwidget;
		partwidget->setGeometry(lastleft, 0, p->width, extended->container->height());
		lastleft += p->width;
	}
}

void QP_ListChart::paintEvent(QPaintEvent *) {
	QPainter paint(this);
	QColor color = Qt::white;
	paint.fillRect(2, 2, width()-4, MIN_HDHEIGHT-4, color);
}

void QP_ListChart::resizeEvent(QResizeEvent *e) {
	int w = e->size().width();
	//int h = e->size().height();
	container->setGeometry(6, 6, w-12, MIN_HDHEIGHT-12);
	draw();
}

void QP_ListChart::mouseReleaseEvent(QMouseEvent *e) {
	/*---call the "sigDevicePopup", used to display the popupmenu context---*/
	if (device() && e->button() == Qt::RightButton) 
		emit sigDevicePopup();

}

void QP_ListChart::setSignals(QP_PartWidget *partwidget) {
	connect(partwidget, SIGNAL(sigPopup()),
		this, SIGNAL(sigPopup()));
	connect(partwidget, SIGNAL(sigSelectPart(QP_PartInfo *)),
		this, SIGNAL(sigSelectPart(QP_PartInfo *)));
}
