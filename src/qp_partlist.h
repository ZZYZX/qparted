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

/* About QP_PartList class:
 *
 * This widget is used as a prototype: there are not any implementation here!
 * Actually there are two implementation: QP_ListChart and QP_ListView: see
 * the header of these two classes to have more info!
 */

#ifndef QP_PARTLIST_H
#define QP_PARTLIST_H

#include <qwidget.h>
#include "qp_devlist.h"
#include "qp_libparted.h"

class QP_PartList : public QWidget {
	Q_OBJECT
public:
	QP_PartList(QWidget *parent=0, Qt::WFlags f = 0);
	~QP_PartList();
	QP_PartInfo *selPartInfo();		/*---return selected partition              ---*/
	void setselPartInfo(QP_PartInfo *);	/*---change the selected partition          ---*/
	void clear();				/*---clear the chart                        ---*/
	QP_Device *device();			/*---return the device                      ---*/
	void setDevice(QP_Device *);		/*---set the Device                         ---*/
	void addPrimary(QP_PartInfo *);		/*---add a Primary or Extended partition    ---*/
	void addLogical(QP_PartInfo *);		/*---add a Logical partition                ---*/
	void draw();				/*---repaint the widget                     ---*/
	float mb_hdsize();			/*---return the size of the hardisk         ---*/
	void set_mb_hdsize(float);		/*---set the size of the hardisk            ---*/

private:
	float _mb_hdsize;			/*---size of the harddisk in megabyte       ---*/
	QP_Device *_device;			/*---the device                             ---*/

protected:
	QP_PartInfo *_selPartInfo;		/*---partition selected                     ---*/

signals:
	void sigSelectPart(QP_PartInfo *);	/*---emitted when you change the selection  ---*/
	void sigDevicePopup();			/*---emitted when you want to popup (device)---*/
	void sigPopup();			/*---emitted when you want to pop (partition)--*/
};

#endif
