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

#include <qapplication.h>
#include <qprogressbar.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include "qp_dlgprogress.h"

QP_dlgProgress::QP_dlgProgress(QWidget *parent):QDialog(parent),Ui::QP_UIProgress() {
	setupUi(this);

	QPalette pal=lblMessage->palette();
	pal.setColor(QPalette::WindowText, Qt::red); // Error messages in red
	lblMessage->setPalette(pal);
}

QP_dlgProgress::~QP_dlgProgress() {
}

void QP_dlgProgress::init_dialog() {
	btnOk->setEnabled(false);
	progressBar->setValue(0);
	lblState->setText(tr("Initializing"));
	lblMessage->setText(QString::null);
	lblTimeLeft->setText(QString::null);
}

int QP_dlgProgress::show_dialog() {
	return exec();
}

void QP_dlgProgress::closeEvent (QCloseEvent *ce) {
	if (btnOk->isEnabled())
		ce->accept();
	else
		ce->ignore();
}

void QP_dlgProgress::slotTimer(int percent, QString state, QString timeleft) {
	QString tleft;
	tleft = QString(tr("Time Left: %1"))
					.arg(timeleft);

	progressBar->setValue(percent);
	lblState->setText(state);
	lblTimeLeft->setText(tleft);
	qApp->processEvents();
}

void QP_dlgProgress::slotOperations(QString operation, QString message, int count, int total) {
	QString label = QString(tr("Operation: %1 of %2.\nCurrent operation: %3"))
				   .arg(count)
				   .arg(total)
				   .arg(operation);

	lblStep->setText(label);

	lblMessage->setText(message);

	if (count == total) {
		if (lblMessage->text().isNull()) {
			lblMessage->setText(tr("Operations completed sucessfully."));
		}
		btnOk->setEnabled(true);
	}
}
