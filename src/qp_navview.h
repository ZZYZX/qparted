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

/* About QP_NavView class:
 *
 * This is a widget that display a tree of hard disk (using QP_DriveList) and
 * a small detail of the hard disk selected by the user.
 */

#ifndef QP_NAVVIEW_H
#define QP_NAVVIEW_H

#include <qwidget.h>
#include <qlabel.h>
#include <QMenu>

#include "qp_drivelist.h"
#include "qp_libparted.h"
#include "qp_settings.h"

class QP_NavView : public QWidget {
Q_OBJECT

public:
    QP_NavView(QWidget *parent=0, QP_Settings *settings=0);
    ~QP_NavView();
    void setPopup(QMenu *); /*---set the popup menu (right click)---*/
    void init();
    QActionGroup *agDevices();
    QP_Device *selDevice(); /*---return the selected device---*/

protected:
    QP_DriveList *drivelist;
    QLabel *details;

signals:
    void deviceSelected(QP_Device *);  /*---emitted when user select a hard disk---*/

protected slots:
    void displayInfo(QP_Device *);
};

#endif
