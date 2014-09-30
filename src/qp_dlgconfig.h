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

/* About QP_dlgConfig class:
 *
 * This class is derived from QP_UIConfig that is made by designer. If you want to change
 * the layout of this dialog just use QT designer!
 *
 * This dialog is by the user to change some QTParte setting
 */

#ifndef QP_DLGCONFIG_H
#define QP_DLGCONFIG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QHBoxLayout>
#include "qtparted.h"

class QP_dlgConfig : public QDialog {
	Q_OBJECT
public:
	QP_dlgConfig(QWidget *parent=0);
	~QP_dlgConfig();
	int layout();		  /*---get the layout		---*/
	void setLayout(int);   /*---set the layout		---*/

protected slots:
	void ok();

protected:
	QVBoxLayout	_layout;
	QLabel *	_lblLayout;
	QComboBox *	_cmbLayout;
	QLabel *	_lblExtTools;
	QTableWidget *	_extTools;
	QHBoxLayout *	_buttonLayout;
	QPushButton *	_btnOk;
	QPushButton *	_btnCancel;
};
#endif
