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

/* About QP_DiskView class:
 *
 * This is a widget that wrap libparted and QP_PartList.
 * It draw a Chart and a List of the partitions! Very Cool! :)
 */

#ifndef QP_DISKVIEW_H
#define QP_DISKVIEW_H

#include <QWidget>
#include <QVBoxLayout>
#include "qp_libparted.h"
#include "qp_partlist.h"
#include "qp_listview.h"
#include "qp_listchart.h"
#include "qp_devlist.h"

class QP_DiskView : public QWidget {
Q_OBJECT

public:
	QP_DiskView(QWidget *parent=0, Qt::WFlags f = 0);
	~QP_DiskView();
	QP_PartInfo *selPartInfo();					/*---return the selected partition			 ---*/
	void setDevice(QP_Device *);				   /*---set the device (example: /dev/hda)		---*/
	void update_partlist();						/*---redraw the QP_PartList widgets attached   ---*/
	void refresh();								/*---destroy libparted and call refresh_widgets---*/
	void setLayout(int);						   /*---set the layout of the widget			  ---*/
	bool canUndo();								/*---the state of the disk is changed?		 ---*/
	void undo();								   /*---undo last operation					   ---*/
	void commit();								 /*---commit all operations					 ---*/
	QP_LibParted *libparted;					   /*---libparted is the wrapper to parted		---*/
	QP_ListChart *listchart;					   /*---chart implementation of QP_PartList	   ---*/
	QP_ListView *listview;						 /*---list implementation of QP_PartList		---*/
	QP_FileSystem *filesystem;					 /*---a class with all feature of filesystems   ---*/

private:
	QP_Device *_qpdevice;						  /*---this is the device (example: /dev/hda)	---*/
	void refresh_widgets();						/*---recalculate and redraw QP_PartList		---*/
	void clear();								  /*---clear partitions						  ---*/
	void addPrimary(QP_PartInfo *);				/*---add a Primary or Extended partition	   ---*/
	void addLogical(QP_PartInfo *);				/*---add a Logical partition				   ---*/
	void draw();								   /*---repaint the widget of partitions		  ---*/
	void set_mb_hdsize(float);					 /*---set the size of the hardisk			   ---*/

signals:
	void sigSelectPart(QP_PartInfo *);			 /*---emitted when you change the selection	 ---*/
	void sigPopup();						 /*---emitted when you want to popup a menu	 ---*/
	void sigDevicePopup();					   /*---emitted when you want to pop deivce menu  ---*/
	void sigTimer(int, QString, QString);		  /*---emitted to update a progress bar		  ---*/
	void sigOperations(QString, QString, int, int);/*---emitted when doing commit				 ---*/
	void sigDiskChanged();						 /*---emitted to state of the disk changed	  ---*/

protected slots:
	void slotListChartSelectPart(QP_PartInfo *);   /*---connected to receive signal from ListChart---*/
	void slotListViewSelectPart(QP_PartInfo *);	/*---connected to receive signal from ListView ---*/

protected:
	QVBoxLayout	_layout;
};

#endif
