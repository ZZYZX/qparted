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


#include "qp_combospin.h"

QP_ComboSpin::QP_ComboSpin(QWidget *parent)
	:QWidget(parent)
{
	layout = new QHBoxLayout(this); //, "layout");

	spinbox = new QP_SpinBox(this);

	//spinbox->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, spinbox->sizePolicy().hasHeightForWidth()));

	layout->addWidget(spinbox);
	connect(spinbox, SIGNAL(valueChanged(PedSector)),
		this, SIGNAL(valueChanged(PedSector)));

	combobox = new QComboBox(this);
	//combobox->setSizePolicy(QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, 0, 0, combobox->sizePolicy().hasHeightForWidth()));
	connect(combobox, SIGNAL(activated(int)),
		spinbox, SLOT(setFormat(int)));
	layout->addWidget(combobox);

	combobox->clear();
	combobox->addItem("MB");
	combobox->addItem("GB");
}

QP_ComboSpin::~QP_ComboSpin() {
}

void QP_ComboSpin::setRange(PedSector min, PedSector max) {
	spinbox->setRange(min, max);
}

void QP_ComboSpin::setValue(PedSector val) {
	spinbox->setValue(val);
}

void QP_ComboSpin::setMaxValue(PedSector max) {
	spinbox->setMaxValue(max);
}

void QP_ComboSpin::setMinValue(PedSector min) {
	spinbox->setMinValue(min);
}

PedSector QP_ComboSpin::value() {
	return spinbox->pedValue();
}

PedSector QP_ComboSpin::maxValue() {
	return spinbox->pedMaxValue();
}
