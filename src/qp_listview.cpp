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
#include "qp_listview.h"
#include "qp_filesystem.h"
#include <QResizeEvent>

#define MIN_HDWIDTH 190


/*----------QP_ListViewItem----------------------------------------------------------*/
/*---																			 ---*/

QP_ListViewItem::QP_ListViewItem ( QTreeWidget *parent,
								   const QString &number,
								   const QString &diskName,
								   const QString &fstype,
								   const QString &status,
								   const QString &sizeStr,
								   const QString &usedStr,
								   const QString &startStr,
								   const QString &endStr,
								   const QString &label,
								   QP_PartInfo *pinfo )
		: QTreeWidgetItem ( parent )
{

	partinfo = pinfo;
	partinfo->listviewitem = this;

	setIcon ( 0, pinfo->fsspec->pixmap() );
	setText ( 0, number );
	setText ( 1, diskName );
	setText ( 2, fstype );
	setText ( 3, status );
	setText ( 4, sizeStr );
	setText ( 5, usedStr );
	setText ( 6, startStr );
	setText ( 7, endStr );
	setText ( 8, label );
}

QP_ListViewItem::QP_ListViewItem ( QTreeWidgetItem *parent,
								   const QString &number,
								   const QString &diskName,
								   const QString &fstype,
								   const QString &status,
								   const QString &sizeStr,
								   const QString &usedStr,
								   const QString &startStr,
								   const QString &endStr,
								   const QString &label,
								   QP_PartInfo *pinfo )
		: QTreeWidgetItem ( parent )
{

	partinfo = pinfo;
	partinfo->listviewitem = this;

	setIcon ( 0, pinfo->fsspec->pixmap() );
	setText ( 0, number );
	setText ( 1, diskName );
	setText ( 2, fstype );
	setText ( 3, status );
	setText ( 4, sizeStr );
	setText ( 5, usedStr );
	setText ( 6, startStr );
	setText ( 7, endStr );
	setText ( 8, label );
}

/*----------QP_RealListView----------------------------------------------------------*/
/*---																			 ---*/
QP_RealListView::QP_RealListView ( QWidget *parent )
		: QTreeWidget ( parent )
{
	setContextMenuPolicy(Qt::CustomContextMenu);

	itemextended = NULL;

	QStringList hlabel;
	hlabel << tr ( "Number" ) << tr ( "Partition" ) << tr ( "Type" ) << tr ( "Status" ) << tr ( "Size" ) << tr ( "Used Space" ) << tr ( "Start" ) << tr ( "End" ) << tr ( "Label" ); // number 8

	setHeaderLabels ( hlabel );

	/*---get if user want to change selectior or want to popup a menu---*/
	connect ( this, SIGNAL ( itemClicked ( QTreeWidgetItem *, int ) ),
			  SLOT ( selectionChanged ( QTreeWidgetItem * ) ) );
	connect ( this, SIGNAL ( customContextMenuRequested ( const QPoint & ) ),
			  SLOT ( rightButtonClicked ( const QPoint & ) ) );
}

QP_RealListView::~QP_RealListView()
{
}

void QP_RealListView::setDevice ( QP_Device *device )
{
	_device = device;
}

void QP_RealListView::addPrimary ( QP_PartInfo *partinfo, int number )
{
	/*---get the filesystem---*/
	QString fstype = partinfo->fsspec->name();

	/*---force "extended" label if the type is extended---*/

	if ( partinfo->type == QTParted::extended ) fstype = "extended";

	/*---set label for start, end and size---*/
	QString startStr = MB2String ( partinfo->mb_start() );

	QString endStr = MB2String ( partinfo->mb_end() );

	QString sizeStr = MB2String ( partinfo->mb_end() - partinfo->mb_start() );

	QString snumber;

	snumber.sprintf ( "%.2d", number );

	QString name = partinfo->partname();

	QString label = partinfo->label();

	QString status;

	if ( partinfo->isActive() ) status = QString ( tr ( "Active" ) );
	else status = QString::null;

	if ( partinfo->isHidden() )
	{
		if ( !status.isNull() ) status += "/";

		status += QString ( tr ( "Hidden" ) );
	}

	QString usedStr;

	if ( partinfo->min_size == -1 )
		usedStr = QString ( tr ( "N/A" ) );
	else
		usedStr = MB2String ( partinfo->mb_min_size() );

	/*---if doesn't exit a partition table make "fake" listitem---*/
	if ( !_device->partitionTable() )
	{
		fstype = QString::null;
		startStr = QString::null;
		endStr = QString::null;
		sizeStr = QString::null;
		usedStr = QString::null;
		snumber = QString ( "01" );
		status = QString ( tr ( "Empty" ) );
		name = QString ( tr ( "Partition table" ) );

	}

	/*---make a new line (every line is a primary or an extended partition)---*/

	QP_ListViewItem *item = new QP_ListViewItem ( this,
			snumber,
			name,
			fstype,
			status,
			sizeStr, usedStr, startStr, endStr,
			label,
			partinfo );

	/*---if you add an extended... just save the pointer---*/
	if ( partinfo->type == QTParted::extended ) itemextended = item;
}

void QP_RealListView::addLogical ( QP_PartInfo *partinfo, int number )
{
	// Not supported by DOS-style partition tables
	//  const char *nameStr = ped_partition_get_name( part );
	//  QString name( nameStr ? nameStr : "" );

	/*---set label for start, end and size---*/
	QString startStr = MB2String ( partinfo->mb_start() );
	QString endStr = MB2String ( partinfo->mb_end() );
	QString sizeStr = MB2String ( partinfo->mb_end() - partinfo->mb_start() );
	QString snumber;
	snumber.sprintf ( "%.2d", number );
	QString label = partinfo->label();

	QString status;

	if ( partinfo->isActive() ) status = QString ( tr ( "Active" ) );
	else status = QString::null;

	if ( partinfo->isHidden() )
	{
		if ( !status.isNull() ) status += "/";

		status += QString ( tr ( "Hidden" ) );
	}

	QString usedStr;

	if ( partinfo->min_size == -1 )
		usedStr = QString ( tr ( "N/A" ) );
	else
		usedStr = MB2String ( partinfo->mb_min_size() );

	//printf("position: %2d %s\n", partinfo->position, partinfo->partname().toLatin1());

	/*---make a new line (every line is a logical partition)---*/

	new QP_ListViewItem ( itemextended,
						  snumber,
						  partinfo->partname(),
						  partinfo->fsspec->name(),
						  status,
						  sizeStr, usedStr, startStr, endStr,
						  label,
						  partinfo );

	/*---open the extended tree---*/
	itemextended->setExpanded(true);
}


void QP_RealListView::selectionChanged ( QTreeWidgetItem *i )
{
	/*---emit the sigSelectPart signal---*/
	QP_ListViewItem *di = static_cast<QP_ListViewItem *> ( i );
	emit sigSelectPart ( di->partinfo );
}


void QP_RealListView::rightButtonClicked ( const QPoint &point )
{
	/*---emit the sigPopup signal---*/
	emit sigPopup();
}

/*-----------------------------------------------------------------------------------*/





/*----------QP_ListView--------------------------------------------------------------*/
/*---																			 ---*/
QP_ListView::QP_ListView ( QWidget *parent, Qt::WFlags f )
		: QP_PartList ( parent, f )
{

	/*---init progressive number---*/
	number = 0;

	/*---listview is the "real" listview!---*/
	listview = new QP_RealListView ( this );

	/*---QP_ListView must expand both horizontal or verticaly!---*/
	setMinimumWidth ( MIN_HDWIDTH );
	setSizePolicy ( QSizePolicy ( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding ) );

	/*---connect sigPopup and sigSelectPart with the signals of the RealListView---*/
	connect ( listview, SIGNAL ( sigSelectPart ( QP_PartInfo * ) ),
			  this, SIGNAL ( sigSelectPart ( QP_PartInfo * ) ) );
	connect ( listview, SIGNAL ( sigPopup() ),
			  this, SIGNAL ( sigPopup() ) );
}

QP_ListView::~QP_ListView()
{
	delete listview;
}

void QP_ListView::setselPartInfo ( QP_PartInfo *selpartinfo )
{
	/*---call the ancestor to change the selection---*/
	QP_PartList::setselPartInfo(selpartinfo);

	/*---just a wrap to the QP_RealListView---*/
	listview->setCurrentItem(selpartinfo->listviewitem);
}

void QP_ListView::setDevice ( QP_Device *device )
{
	QP_PartList::setDevice(device);
	listview->setDevice ( device );
}

void QP_ListView::clear()
{
	/*---init progressive number---*/
	number = 0;

	/*---call the ancestor to init the selected partinfo---*/
	QP_PartList::setselPartInfo(NULL);

	/*---just a wrap to the QP_RealListView---*/
	listview->clear();
}

void QP_ListView::addPrimary ( QP_PartInfo *partinfo )
{
	/*---just a wrap to the QP_RealListView---*/
	listview->addPrimary ( partinfo, ++number );
}

void QP_ListView::addLogical ( QP_PartInfo *partinfo )
{
	/*---just a wrap to the QP_RealListView---*/
	listview->addLogical ( partinfo, ++number );
}

void QP_ListView::resizeEvent ( QResizeEvent *event )
{
	/*---resize QP_RealListView to fit the ListView wrapper---*/
	listview->resize ( event->size() );
}
/*-----------------------------------------------------------------------------------*/
