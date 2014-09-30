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

#include <qframe.h>
#include <qlabel.h>
#include <qlineedit.h>

#include "qp_combospin.h"
#include "qp_dlgresize.h"

QP_dlgResize::QP_dlgResize(QWidget *parent):QDialog(parent),Ui::QP_UIResize() {
	setupUi(this);

	sizecontainer = NULL;
	
	/*---this is a fixed dialog!---*/
	setFixedSize(minimumSizeHint());
	
	sizecontainer = new QP_SizeContainer(container);
	connect(sizecontainer, SIGNAL(sigChangedStart(PedSector)),
			this, SLOT(slotChangedStart(PedSector)));
	connect(sizecontainer, SIGNAL(sigChangedEnd(PedSector)),
			this, SLOT(slotChangedEnd(PedSector)));
	connect(sizecontainer, SIGNAL(sigChangedPos(PedSector, PedSector)),
			this, SLOT(slotChangedPos(PedSector, PedSector)));

	connect(spinFreeBefore, SIGNAL(valueChanged(PedSector)),
			this, SLOT(slotFreeBeforeChanged(PedSector)));
	connect(spinFreeAfter, SIGNAL(valueChanged(PedSector)),
			this, SLOT(slotFreeAfterChanged(PedSector)));
	connect(spinNewSize, SIGNAL(valueChanged(PedSector)),
			this, SLOT(slotNewSizeChanged(PedSector)));
}

QP_dlgResize::~QP_dlgResize() {
}

void QP_dlgResize::init_dialog(QTParted::actType moveresize) {
	_moveresize = moveresize;

	sizecontainer->setMode(_moveresize);
	sizecontainer->setGrowStartPartSector(_GrowStartPartSector);

	if (_moveresize == QTParted::resize) {
		/*---if the filesystem has a "grow" limit... ---*/
		if (_MaxPartSector != 0) {
			PedSector _tmpGrowEndPartSector = _GrowStartPartSector + _MaxPartSector;
			/*---maybe the "tmpGrow" is > then the "real" hard disk grow. This make  ---
			 *---not sense to change GrowEnd... 'cause you cannot enlarge a partition---
			 *---in a space > then the real disk size!							   ---*/
			if (_tmpGrowEndPartSector < _GrowEndPartSector)
				_GrowEndPartSector = _tmpGrowEndPartSector;
		}
		sizecontainer->setGrowEndPartSector(_GrowEndPartSector);
	} else {
		sizecontainer->setGrowEndPartSector(_GrowEndPartSector);
	}
	sizecontainer->setStartPartSector(_StartPartSector);
	sizecontainer->setEndPartSector(_EndPartSector);
	sizecontainer->setMinPartSector(_MinPartSector);

	QString label;
	label = QString(tr("Minimum Size: %1 MB"))
			.arg((int)_MinPartSector/MEGABYTE_SECTORS);
	lblMinimumSize->setText(label);

	spinFreeBefore->setEnabled(true);
	spinFreeAfter->setEnabled(true);
	spinNewSize->setEnabled(true);

	setValNewSize();
	setValFreeBefore();
	setValFreeAfter();

	if (_moveresize == QTParted::resize) {
		spinFreeBefore->setEnabled(false);
		setWindowTitle(tr("Resize partition"));
	} else {
		spinNewSize->setEnabled(false);
		setWindowTitle(tr("Move partition"));
	}

	sizecontainer->setGeometry(2, 2, container->width()-4, container->height()-4);
}

int QP_dlgResize::show_dialog() {
	return exec();
}

void QP_dlgResize::resizeEvent(QResizeEvent *) {
}

void QP_dlgResize::setMaxPartSector(PedSector MaxPartSector) {
	_MaxPartSector = MaxPartSector;
}

void QP_dlgResize::setMinPartSector(PedSector MinPartSector) {
	_MinPartSector = MinPartSector;
}

void QP_dlgResize::setStartPartSector(PedSector StartPartSector) {
	_StartPartSector = StartPartSector;
}

void QP_dlgResize::setEndPartSector(PedSector EndPartSector) {
	_EndPartSector = EndPartSector;
}

void QP_dlgResize::setGrowStartPartSector(PedSector GrowStartPartSector) {
	_GrowStartPartSector = GrowStartPartSector;
}

void QP_dlgResize::setGrowEndPartSector(PedSector GrowEndPartSector) {
	_GrowEndPartSector = GrowEndPartSector;
}

void QP_dlgResize::setValFreeBefore() {
	PedSector size = _EndPartSector - _StartPartSector;
	PedSector maxfree = _GrowEndPartSector - _GrowStartPartSector - size;
	PedSector sizebefore = _StartPartSector - _GrowStartPartSector;
	if (maxfree < 0) maxfree = 0;

	spinFreeBefore->blockSignals(true);
	spinFreeBefore->setMaxValue(maxfree);
	spinFreeBefore->setValue(sizebefore);
	spinFreeBefore->blockSignals(false);
}

void QP_dlgResize::setValFreeAfter() {
	PedSector maxfree;
	
	if (_moveresize == QTParted::resize) {
		maxfree = _GrowEndPartSector - (_StartPartSector + _MinPartSector);
	} else {
		PedSector size = _EndPartSector - _StartPartSector;
		maxfree = _GrowEndPartSector - _GrowStartPartSector - size;
	}
	PedSector sizeafter = _GrowEndPartSector - _EndPartSector;
	if (maxfree < 0) maxfree = 0;

	spinFreeAfter->blockSignals(true);
	spinFreeAfter->setMaxValue(maxfree);
	spinFreeAfter->setValue(sizeafter);
	spinFreeAfter->blockSignals(false);
}

void QP_dlgResize::setValNewSize() {
	PedSector valnewsize = _EndPartSector - _StartPartSector;
	PedSector maxval = _GrowEndPartSector - _StartPartSector;

	spinNewSize->blockSignals(true);
	spinNewSize->setRange(_MinPartSector, maxval);
	spinNewSize->setValue(valnewsize);
	spinNewSize->blockSignals(false);
}

void QP_dlgResize::slotChangedStart(PedSector StartSector) {
	_StartPartSector = StartSector;

	setValNewSize();
	setValFreeBefore();
}

void QP_dlgResize::slotChangedEnd(PedSector EndSector) {
	_EndPartSector = EndSector;
	
	setValNewSize();
	setValFreeAfter();
}

void QP_dlgResize::slotChangedPos(PedSector StartSector, PedSector EndSector) {
	PedSector size = _EndPartSector - _StartPartSector;
	_StartPartSector = StartSector;
	_EndPartSector = StartSector + size;

	setValFreeAfter();
	setValFreeBefore();
}

void QP_dlgResize::slotFreeBeforeChanged(PedSector FreeBefore) {
	if (_moveresize == QTParted::resize) {
		sizecontainer->blockSignals(true);
		_StartPartSector = FreeBefore + _GrowStartPartSector;
		sizecontainer->setStartPartSector(_StartPartSector);
		sizecontainer->setGeometry(2, 2, container->width()-4, container->height()-4);
		sizecontainer->blockSignals(false);
	} else {
		PedSector size = _EndPartSector - _StartPartSector;
		_StartPartSector = _GrowStartPartSector + FreeBefore;
		_EndPartSector = _StartPartSector + size;
		
		PedSector freeafter = _GrowEndPartSector - _EndPartSector;

		spinFreeAfter->setValue(freeafter);

		sizecontainer->blockSignals(true);
		sizecontainer->setStartPartSector(_StartPartSector);
		sizecontainer->setEndPartSector(_EndPartSector);
		sizecontainer->setGeometry(2, 2, container->width()-4, container->height()-4);
		sizecontainer->blockSignals(false);
	}
}

void QP_dlgResize::slotFreeAfterChanged(PedSector FreeAfter) {
	if (_moveresize == QTParted::resize) {
		sizecontainer->blockSignals(true);
		_EndPartSector = _GrowEndPartSector - FreeAfter;
		sizecontainer->setEndPartSector(_EndPartSector);
		sizecontainer->setGeometry(2, 2, container->width()-4, container->height()-4);
		sizecontainer->blockSignals(false);

		setValNewSize();
	} else {
		PedSector size = _EndPartSector - _StartPartSector;
		_EndPartSector = _GrowEndPartSector - FreeAfter;
		_StartPartSector = _EndPartSector - size;
		
		PedSector freebefore = _StartPartSector - _GrowStartPartSector;

		printf("non mi dire\n");
		spinFreeBefore->setValue(freebefore);

		sizecontainer->blockSignals(true);
		sizecontainer->setStartPartSector(_StartPartSector);
		sizecontainer->setEndPartSector(_EndPartSector);
		sizecontainer->setGeometry(2, 2, container->width()-4, container->height()-4);
		sizecontainer->blockSignals(false);
	}
}

void QP_dlgResize::slotNewSizeChanged(PedSector NewSize) {
	sizecontainer->blockSignals(true);
	
	_EndPartSector = _StartPartSector + NewSize;
	sizecontainer->setEndPartSector(_EndPartSector);
	sizecontainer->setGeometry(2, 2, container->width()-4, container->height()-4);
	
	sizecontainer->blockSignals(false);

	setValFreeAfter();
}
