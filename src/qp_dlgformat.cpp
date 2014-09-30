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

#include <qcombobox.h>
#include <qlineedit.h>
#include <qradiobutton.h>

#include "qp_combospin.h"
#include "qp_dlgformat.h"

QP_dlgFormat::QP_dlgFormat(QWidget *parent):QDialog(parent),Ui::QP_UIFormat() {
	setupUi(this);
}

QP_dlgFormat::~QP_dlgFormat() {
}

void QP_dlgFormat::init_dialog() {
    /*---clear combo type and combo filesystem---*/
    cmbFilesystem->clear();

    /*---enable combo filesystem box---*/
    cmbFilesystem->setEnabled(true);
}

void QP_dlgFormat::addFileSystem(QString name) {
    cmbFilesystem->addItem(name);
}

int QP_dlgFormat::show_dialog() {
    return exec();
}

QString QP_dlgFormat::fileSystemName() {
    /*---return the selected filesystem---*/
    return cmbFilesystem->currentText();
}

QString QP_dlgFormat::Label() {
    /*---return the label choosed by the user---*/
    return txtLabel->text();
}
