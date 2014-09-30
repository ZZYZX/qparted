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
#include <qlabel.h>
#include "qp_combospin.h"
#include "qp_dlgcreate.h"
#include "qp_filesystem.h"

/*---label used in the combo box---*/
#define STR_PRIMARY  "Primary Partition"
#define STR_EXTENDED "Extended Partition"
#define STR_LOGICAL  "Logical Partition"


QP_dlgCreate::QP_dlgCreate(QWidget *w):QDialog(w),Ui_QP_UICreate() {
	setupUi(this);

	/*---connect the combo type slot---*/
	connect(cmbType, SIGNAL(activated(int)),
			this, SLOT(slotTypeChanged(int)));

	/*---connect the combo filesystem type slot---*/
	connect(cmbFilesystem, SIGNAL(activated(int)),
			this, SLOT(slotFSChanged(int)));

	/*---connect the spinratio slot (received when you change the size)---*/
	connect(spinRatio, SIGNAL(valueChanged(int)),
			this, SLOT(slotRatioChanged(int)));

	/*---connect the spinsize slot (received when you change the size)---*/
	connect(spinSize, SIGNAL(valueChanged(PedSector)),
			this, SLOT(slotSizeChanged(PedSector)));
}

QP_dlgCreate::~QP_dlgCreate() {
}

void QP_dlgCreate::init_dialog() {
	/*---clear combo type and combo filesystem and the label box---*/
	cmbType->clear();
	cmbFilesystem->clear();
	txtLabel->setText(QString::null);

	/*---enable combo filesystem box and the label box---*/
	cmbFilesystem->setEnabled(true);
	txtLabel->setEnabled(true);
	
	/*---enable "begin" in radio "begin - end"---*/
	radioBegin->setChecked(true);
}

void QP_dlgCreate::setFileSystem(QP_FileSystem *filesystem) {
	_filesystem = filesystem;
}

void QP_dlgCreate::addFileSystem(QString name) {
	cmbFilesystem->addItem(name);
}

void QP_dlgCreate::setMaxSize(PedSector maxsize) {
	/*---keep the maxsize into a private var---*/
	_maxsize = maxsize;
	
	spinSize->setMaxValue(maxsize);
	spinSize->setValue(maxsize);
	spinRatio->setValue(100);
}

void QP_dlgCreate::addType(QString label) {
	cmbType->addItem(label);
}

void QP_dlgCreate::addTypePrimary() {
	addType(STR_PRIMARY);
}

void QP_dlgCreate::addTypeExtended() {
	addType(STR_EXTENDED);
}

void QP_dlgCreate::addTypeLogical() {
	addType(STR_LOGICAL);
}

int QP_dlgCreate::show_dialog() {
	//if the user can create only primary or logical partition combobox is not usefull
	if (cmbType->count() == 1)
		cmbType->setEnabled(false);
	else
		cmbType->setEnabled(true);

	/*---init of the filesystem---*/
	slotFSChanged(0);

	return exec();
}

PedSector QP_dlgCreate::Size() {
	return spinSize->value();
}

QTParted::partType QP_dlgCreate::type() {
		 if (cmbType->currentText().compare(STR_PRIMARY)  == 0)
		 return QTParted::primary;
	else if (cmbType->currentText().compare(STR_EXTENDED) == 0)
		 return QTParted::extended;
	else return QTParted::logical;
}

QString QP_dlgCreate::fileSystemName() {
	/*---return the selected filesystem---*/
	return cmbFilesystem->currentText();
}

void QP_dlgCreate::slotFSChanged(int) {
	QP_FileSystemSpec *fsspec = _filesystem->nameToFSSpec(fileSystemName());
	PedSector newmaxsize = _maxsize;
	
	lblMessage->setText(QString::null);

	if (type() != QTParted::extended) {
		if (fsspec->maxFsSize() != 0) {
			QString mbstring = MB2String((float)(fsspec->maxFsSize() / MEGABYTE_SECTORS));
			QString label = QString(tr("This filesystem cannot be greater of %1"))
									.arg(mbstring);

			
			lblMessage->setText(label);
			newmaxsize = fsspec->maxFsSize();

			/*---there is not sense to create a partition bigger then the space!---*/
			if (newmaxsize > _maxsize) newmaxsize = _maxsize;
		}

		/*---is changed the maxvalue?---*/
		if (spinSize->value() >= newmaxsize) {
			spinSize->setValue(newmaxsize);
		}
		if (spinSize->maxValue() != newmaxsize) {
			spinSize->setMaxValue(newmaxsize);
		}

		spinSize->setMinValue(fsspec->minFsSize());
	}
}

void QP_dlgCreate::slotTypeChanged(int index) {
	//if you choose extended just gray filesystem.... cause extended doesn't need it! :)
	if (index == 1) {
		cmbFilesystem->setEnabled(false);
		txtLabel->setEnabled(false);
	} else {
		cmbFilesystem->setEnabled(true);
		txtLabel->setEnabled(true);
	}
}

QString QP_dlgCreate::Label() {
	/*---return the label choosed by the user---*/
	return txtLabel->text();
}

void QP_dlgCreate::slotRatioChanged(int value) {
	/*---if user change ration just update the sizebox---*/
	int newsize = (spinSize->maxValue()*value)/100;
	spinSize->blockSignals(true);
	spinSize->setValue(newsize);
	spinSize->blockSignals(false);
}

void QP_dlgCreate::slotSizeChanged(PedSector value) {
	/*---if user change size just update the spinbox---*/
	int newratio = (value*100)/spinSize->maxValue();
	spinRatio->blockSignals(true);
	spinRatio->setValue(newratio);
	spinRatio->blockSignals(false);
}
