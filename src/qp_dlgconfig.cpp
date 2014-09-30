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
#include <QHeaderView>
#include "qp_dlgconfig.h"
#include "qp_common.h"

QP_dlgConfig::QP_dlgConfig(QWidget *p):QDialog(p),_layout(this) {
	setWindowTitle(tr("Configuration"));

	_lblLayout = new QLabel(tr("QTParted &layout"), this);
	_layout.addWidget(_lblLayout);

	_cmbLayout = new QComboBox(this);
	_lblLayout->setBuddy(_cmbLayout);
	_cmbLayout->clear();
	_cmbLayout->insertItems(0, QStringList()
		<< tr("Chart and ListBox")
		<< tr("Only Chart")
		<< tr("Only ListBox")
	);
	_layout.addWidget(_cmbLayout);

	_lblExtTools = new QLabel(tr("&External tools"), this);
	_layout.addWidget(_lblExtTools);

	_extTools = new QTableWidget(lstExternalTools->count(), 3, this);
	_extTools->verticalHeader()->hide();
	_extTools->setHorizontalHeaderLabels(QStringList() << tr("Tool") << tr("Full path") << tr("Description"));
	_layout.addWidget(_extTools);
	_lblExtTools->setBuddy(_extTools);

	/*---clear combo box used for external tools---*/
	int row=0;
	for(QP_ListExternalTools::ConstIterator it = lstExternalTools->begin(); it != lstExternalTools->end(); ++it) {
		QTableWidgetItem *w = new QTableWidgetItem(it.value()->name());
		w->setFlags(0); // Get rid of Qt::ItemIsEditable
		_extTools->setItem(row, 0, w);
		_extTools->setItem(row, 1, new QTableWidgetItem(it.value()->path()));
		w = new QTableWidgetItem(it.value()->description());
		w->setFlags(Qt::NoItemFlags); // Get rid of Qt::ItemIsEditable
		_extTools->setItem(row, 2, w);
		row++;
	}

	_buttonLayout = new QHBoxLayout();
	_btnOk = new QPushButton(tr("&OK"), this);
	_buttonLayout->addWidget(_btnOk);
	connect(_btnOk, SIGNAL(clicked()), this, SLOT(ok()));

	_btnCancel = new QPushButton(tr("&Cancel"), this);
	_buttonLayout->addWidget(_btnCancel);
	connect(_btnCancel, SIGNAL(clicked()), this, SLOT(reject()));

	_layout.addLayout(_buttonLayout);
}

QP_dlgConfig::~QP_dlgConfig() {
}

void QP_dlgConfig::ok() {
	for(int i=0; i<_extTools->rowCount(); i++)
		lstExternalTools->setPath(_extTools->item(i, 0)->text(), _extTools->item(i, 1)->text());
	accept();
}

int QP_dlgConfig::layout() {
	return _cmbLayout->currentIndex();
}

void QP_dlgConfig::setLayout(int layout) {
	_cmbLayout->setCurrentIndex(layout);
}
