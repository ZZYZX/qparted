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

/* About QP_dlgDevevicePropoperty class:
 *
 * This class is derived from QP_UIFormat that is made by designer. If you want to change
 * the layout of this dialog just use QT designer!
 *
 * This dialog is used when user want to format an existing partition...
 */

#ifndef QP_DLGDEVPROP_H
#define QP_DLGDEVPROP_H

#include <QDialog>
#include "ui_qp_ui_devprop.h"

class QP_dlgDevProperty : public QDialog,public Ui::QP_UIDeviceProperty {
Q_OBJECT

public:
    QP_dlgDevProperty(QWidget *parent=0);
    ~QP_dlgDevProperty();
    void init_dialog();                     /*---clear combo box and other stuff                     ---*/
    int show_dialog();                      /*---just show the dialog. Call it after init_dialog     ---*/
    void setShortName(QString);             /*---set the label of shortname                          ---*/
    void setLongName(QString);              /*---set the label of longname                           ---*/
    void setDevfsEnabled(bool);             /*---set the label of devfs                              ---*/
    void setState(QString);                 /*---set the label of busy                               ---*/
    void setPartitionTable(QString);        /*---set the partition table type                        ---*/
};

#endif
