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

/* About QP_dlgProgress class:
 *
 * This class is derived from QP_UIProgress that is made by designer. If you want to change
 * the layout of this dialog just use QT designer!
 *
 * This dialog is used when user request a "long time" operation.
 */

#ifndef QP_DLGPROGRESS_H
#define QP_DLGPROGRESS_H

#include <QDialog>
#include "ui_qp_ui_progress.h"
//Added by qt3to4:
#include <QCloseEvent>

class QP_dlgProgress : public QDialog, public Ui::QP_UIProgress
{
	Q_OBJECT

public:
	QP_dlgProgress(QWidget *parent=0);
	~QP_dlgProgress();
	void init_dialog();
	int show_dialog();

protected:
	void closeEvent (QCloseEvent *);

public slots:
	void slotTimer(int, QString, QString);
	void slotOperations(QString, QString, int, int);
};

#endif
