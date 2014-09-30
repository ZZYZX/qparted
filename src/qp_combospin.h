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

/* About QP_ComboSpin class:
 *
 * This is a widget that join a QP_SpinBox and a combobox... it is just an idea ;)
 */

#include <qlayout.h>
#include <qcombobox.h>
#include <qwidget.h>
#include "qp_spinbox.h"

#ifndef QP_COMBOSPIN_H
#define QP_COMBOSPIN_H

class QP_ComboSpin : public QWidget {
	Q_OBJECT
public:
	QP_ComboSpin(QWidget *parent = 0L);
	~QP_ComboSpin();
	void setRange(PedSector, PedSector);
	void setMaxValue(PedSector);
	void setMinValue(PedSector);
	void setValue(PedSector);
	PedSector value();
	PedSector maxValue();

signals:
	void valueChanged(PedSector);

protected:
	QHBoxLayout *layout;
	QComboBox *combobox;
	QP_SpinBox *spinbox;
};
#endif
