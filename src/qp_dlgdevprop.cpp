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

#include <qlabel.h>
#include <qstring.h>
#include "qp_dlgdevprop.h"

QP_dlgDevProperty::QP_dlgDevProperty(QWidget *parent):QDialog(parent),Ui::QP_UIDeviceProperty() {
	setupUi(this);
}

QP_dlgDevProperty::~QP_dlgDevProperty() {
}

void QP_dlgDevProperty::init_dialog() {
}

int QP_dlgDevProperty::show_dialog() {
	return exec();
}

void QP_dlgDevProperty::setShortName(QString shortname) {
	lblShortName->setText(shortname);
}

void QP_dlgDevProperty::setLongName(QString longname) {
	lblLongName->setText(longname);
}

void QP_dlgDevProperty::setDevfsEnabled(bool devfs) {
	if (devfs) {
		lblDevfsEnabled->setText(tr("Devfs is enabled."));
	} else {
		lblDevfsEnabled->setText(tr("Devfs is disabled."));
	}
}

void QP_dlgDevProperty::setState(QString state) {
	lblBusy->setText(state);
}

void QP_dlgDevProperty::setPartitionTable(QString partitiontable) {
	QString label = QString(tr("Partition table: %1"))
							.arg(partitiontable);
	lblPartitionTable->setText(label);
}
