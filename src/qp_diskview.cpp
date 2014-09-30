/*
   qtparted - a frontend to libparted for manipulating disk partitions
   Copyright (C) 2002-2003 Vanni Brutto <zanac (-at-) libero dot it>
   Copyright (C) 2007-2010 Bernhard Rosenkraenzer <bero (-at-) arklinux dot org>
   Copyright (C) 2007-2008 David Tio <deux (-at-) arklinux dot org>

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

#include "qp_diskview.h"

QP_DiskView::QP_DiskView ( QWidget *parent, Qt::WindowFlags f )
		: QWidget ( parent, f ), _layout ( this )
{
	/*---the device is initialized---*/
	/*---you cannot call setDevice here or you'll get a segfault!---*/
	_qpdevice = NULL;

	/*---create the listchart, attach it and connect the signals---*/
	
	listchart = new QP_ListChart ( this );
	connect ( listchart, SIGNAL ( sigSelectPart ( QP_PartInfo * ) ),
			  this, SLOT ( slotListChartSelectPart ( QP_PartInfo * ) ) );
	connect ( listchart, SIGNAL ( sigPopup ( ) ),
			  this, SIGNAL ( sigPopup ( ) ) );
	connect ( listchart, SIGNAL ( sigDevicePopup ( ) ),
			  this, SIGNAL ( sigDevicePopup ( ) ) );
	_layout.addWidget ( listchart );
	
	/*---create the listview, attach it and connect the signals---*/
	listview = new QP_ListView ( this );
	connect ( listview, SIGNAL(sigSelectPart(QP_PartInfo *)),
		  this, SLOT(slotListViewSelectPart(QP_PartInfo *)));
	connect ( listview, SIGNAL(sigPopup()),
		  this, SIGNAL(sigPopup()));
	_layout.addWidget ( listview );

	/*---create the wrapper to parted---*/
	libparted = new QP_LibParted();

	/*---enable/disable fastscan---*/
	libparted->setFastScan ( false );

	/*---connect the signal "update progressbar"---*/
	connect(libparted, SIGNAL(sigTimer(int, QString, QString)),
			this, SIGNAL(sigTimer(int, QString, QString)));

	/*---connect the signal "update progressbar" during commit!!---*/
	connect(libparted, SIGNAL(sigOperations(QString, QString, int, int)),
			this, SIGNAL(sigOperations(QString, QString, int, int)));

	/*---connect the signal to get if the disk changed (used for undo/commit)---*/
	connect(libparted, SIGNAL(sigDiskChanged()),
		this, SIGNAL(sigDiskChanged()));
}

QP_DiskView::~QP_DiskView()
{
}

QP_PartInfo *QP_DiskView::selPartInfo() {
	return listview->selPartInfo();
}

void QP_DiskView::setDevice ( QP_Device *dev )
{
	/*---the device was empty? exit!---*/
	if ( dev == NULL ) return;

	/*---if a device was selected clear QP_PartList widgets and delete libparted---*/
	if ( _qpdevice != NULL )
	{
		/*---clear of QP_PartList---*/
		clear();
	}

	/*---change the device in the chart and list---*/
	listview->setDevice ( dev );
	listchart->setDevice ( dev );

	/*---selecte the _device string---*/
	_qpdevice = dev;

	/*---update the libparte device---*/
	libparted->setDevice ( dev );

	/*---fully redraw of the partitions (using QP_PartList widgets)---*/
	update_partlist();
}

void QP_DiskView::update_partlist()
{
	/*---scan all partitions---*/
	libparted->scan_partitions();

	/*---a simple wrapper for filesystems---*/
	filesystem = libparted->filesystem;

	/*---get the size of the hd and set it to QP_PartList---*/
	set_mb_hdsize ( libparted->mb_hdsize() );

	/*---refresh QP_PartList widgets!---*/
	refresh_widgets();
}

void QP_DiskView::refresh()
{
	/*---clear of QP_PartList---*/
	clear();

	update_partlist();
}

void QP_DiskView::setLayout ( int layout )
{
	if ( layout == 0 )
	{
		listview->show();
		listchart->show();
	}
	else if ( layout == 1 )
	{
		listview->hide();
		listchart->show();
	}
	else if ( layout == 2 )
	{
		listview->show();
		listchart->hide();
	}
}

bool QP_DiskView::canUndo()
{
	return libparted->canUndo();
}

void QP_DiskView::undo()
{
	/*---undo last operation---*/
	libparted->undo();

	/*---refresh the listview+listchart---*/
	refresh();
}

void QP_DiskView::commit()
{
	/*---commit all operations---*/
	libparted->commit();

	/*---refresh the listview+listchart---*/
	refresh();
}

void QP_DiskView::refresh_widgets()
{
	/*---loop for adding primary/extended partitions---*/
	foreach ( QP_PartInfo* p, libparted->partlist )
	{
		addPrimary ( p );

		if ( p->type == QTParted::extended )
		{
			foreach ( QP_PartInfo* logi, libparted->logilist )
			{
				/*---loop for adding logical partitions---*/
				addLogical ( logi );
			}
		}
	}

	/*---redraw QP_PartList widgets---*/
	draw();
}

void QP_DiskView::clear()
{
	listview->clear();
	listchart->clear();
}

void QP_DiskView::addPrimary ( QP_PartInfo *partinfo )
{
	listview->addPrimary ( partinfo );
	listchart->addPrimary ( partinfo );
}

void QP_DiskView::addLogical ( QP_PartInfo *partinfo )
{
	listview->addLogical ( partinfo );
	listchart->addLogical ( partinfo );
}

void QP_DiskView::set_mb_hdsize ( float mb_hdsize )
{
	listview->set_mb_hdsize ( mb_hdsize );
	listchart->set_mb_hdsize ( mb_hdsize );
}

void QP_DiskView::draw()
{
	listview->draw();
	listchart->draw();
}

void QP_DiskView::slotListChartSelectPart ( QP_PartInfo *partinfo )
{
	/*---syncronize selection of listview and listchart---*/
	listchart->blockSignals ( true );
	listview->blockSignals ( true );
	listchart->setselPartInfo ( partinfo );
	listview->setselPartInfo ( partinfo );
	listchart->blockSignals ( false );
	listview->blockSignals ( false );

	/*---emit the selected signals---*/
	emit sigSelectPart ( partinfo );
}

void QP_DiskView::slotListViewSelectPart ( QP_PartInfo *partinfo )
{
	/*---syncronize selection of listview and listchart---*/
	listchart->blockSignals ( true );
	listview->blockSignals ( true );
	listview->setselPartInfo ( partinfo );
	listchart->setselPartInfo ( partinfo );
	listview->blockSignals ( false );
	listchart->blockSignals ( false );

	/*---emit the selected signals---*/
	emit sigSelectPart ( partinfo );
}

