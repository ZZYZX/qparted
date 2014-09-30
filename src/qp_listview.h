/*
	qtparted - a frontend to libparted for manipulating disk partitions
	Copyright (C) 2002-2003 Vanni Brutto <zanac (-at-) libero dot it>
	Copyright (C) 2010-2011 Bernhard Rosenkraenzer <bero (-at-) arklinux dot ch>

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

/* About QP_ListView class:
 *
 * This is a widget derived from QP_PartList that display partitions as a list
 * Using methods "addPrimary" and "addLogical" you can easily display your hard drive;)
 */

#ifndef QP_LISTVIEW_H
#define QP_LISTVIEW_H

#include <QTreeWidget>
#include "qp_devlist.h"
#include "qp_partlist.h"

/*----------QP_ListViewItem----------------------------------------------------------*/
/*--- every item represents a partition. Use it adding a line in QP_ListView :)   ---*/
/*																				---*/
/* original author:															   ---*/
/* Richard Moore, rich@kde.org													---*/
/*																				---*/
/* adaption to qtparted by Vanni Brutto										   ---*/
/*																				---*/
class QP_ListViewItem : public QTreeWidgetItem {
public:
	QP_ListViewItem(QTreeWidget *parent,
			const QString &number,
			const QString &diskName,
			const QString &fstype,
			const QString &status,
			const QString &sizeStr,
			const QString &usedStr,
			const QString &startStr,
			const QString &endStr,
			const QString &label,
			QP_PartInfo *pinfo);
	QP_ListViewItem(QTreeWidgetItem *parent,
			const QString &number,
			const QString &diskName,
			const QString &fstype,
			const QString &status,
			const QString &sizeStr,
			const QString &usedStr,
			const QString &startStr,
			const QString &endStr,
			const QString &label,
			QP_PartInfo *pinfo);
	~QP_ListViewItem() {}
	QP_PartInfo *partinfo;
};
/*-----------------------------------------------------------------------------------*/





/*----------QP_RealListView----------------------------------------------------------*/
/* A widget that displays a list of all partitions on a disk.					 ---*/
/* This is the "Real" widget. It is inherited by QListView... but you must not	---*/
/* use this widget directly! You must use insteat QP_ListView (see below).		---*/
/*																				---*/
/* original author:															   ---*/
/* Richard Moore, rich@kde.org													---*/
/*																				---*/
/* adaption to qtparted by Vanni Brutto										   ---*/
/*																				---*/
class QP_RealListView : public QTreeWidget {
	Q_OBJECT
public:
	QP_RealListView(QWidget *parent=0);
	~QP_RealListView();

	void setDevice(QP_Device *);		/*---set the device								   ---*/
	void addPrimary(QP_PartInfo *, int);	/*---add a Primary or Extended partition			  ---*/
	void addLogical(QP_PartInfo *, int);	/*---add a Logical partition						  ---*/

private:
	QP_Device *_device;

protected:
	QP_ListViewItem *itemextended;		/*---this contain a pointer to the extended partition ---*/

signals:
	void sigSelectPart(QP_PartInfo *);		/*---emitted when you change the selection			---*/
	void sigPopup();				/*---emitted when you want to popup a menu			---*/


protected slots:
	void selectionChanged(QTreeWidgetItem *);	/*---connected to get when user change a selected line---*/
	void rightButtonClicked(const QPoint &);	/*---the user want to popup a menu ---*/
};
/*-----------------------------------------------------------------------------------*/





/*----------QP_ListView--------------------------------------------------------------*/
/* This is a widget derived from QP_PartList that display partitions as lines.	---*/
/* Using methods "addPrimary" and "addLogical" you can add new lines easily ;)	---*/
/* QP_ListView is only a "wrapper" to the QP_RealListView.			---*/

class QP_ListView : public QP_PartList {
	Q_OBJECT
public:
	QP_ListView(QWidget *parent=0, Qt::WFlags f = 0);
	~QP_ListView();
	void setselPartInfo(QP_PartInfo *);
	void setDevice(QP_Device *);		/*---ovverride the setDevice method		 ---*/
	void clear();				/*---clear the chart						---*/
	void addPrimary(QP_PartInfo *);		/*---add a Primary or Extended partition	---*/
	void addLogical(QP_PartInfo *);		/*---add a Logical partition				---*/

private:
	int number;				/*---just a prog. number for every partition---*/

protected:
	QP_RealListView *listview;		/*---the real list view					 ---*/
	void resizeEvent(QResizeEvent*);	/*---get the size from QP_RealListView	  ---*/
};
/*-----------------------------------------------------------------------------------*/

#endif
