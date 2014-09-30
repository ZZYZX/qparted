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

/* About QP_dlgResize class:
 *
 * This class is derived from QP_UIResize that is made by designer. If you want to change
 * the layout of this dialog just use QT designer!
 *
 * This dialog is used when user want to resize a new partition...
 */

#ifndef QP_DLGRESIZE_H
#define QP_DLGRESIZE_H

#include <QDialog>
#include "qtparted.h"
#include "qp_sizepart.h"
#include "ui_qp_ui_resize.h"
#include "qp_libparted.h"

class QP_dlgResize : public QDialog, public Ui::QP_UIResize {
	Q_OBJECT

public:
	QP_dlgResize(QWidget *parent=0);
	~QP_dlgResize();
    void init_dialog(QTParted::actType moveresize);      /*---clear combo box and other stuff                     ---*/
    int show_dialog();                                   /*---just show the dialog. Call it after init_dialog     ---*/
    virtual void resizeEvent(QResizeEvent *);
    void setMaxPartSector(PedSector);
    void setMinPartSector(PedSector);
    void setStartPartSector(PedSector);
    void setEndPartSector(PedSector);
    void setGrowStartPartSector(PedSector);
    void setGrowEndPartSector(PedSector);
    void setValFreeBefore();
    void setValFreeAfter();
    void setValNewSize();
    
//private:
    QP_SizePartition *sizepartition;
    QP_SizeContainer *sizecontainer;
    PedSector _MaxPartSector;
    PedSector _MinPartSector;
    PedSector _StartPartSector;
    PedSector _EndPartSector;
    PedSector _GrowStartPartSector;
    PedSector _GrowEndPartSector;
    QTParted::actType _moveresize;

protected slots:
    void slotChangedStart(PedSector);
    void slotChangedEnd(PedSector);
    void slotChangedPos(PedSector, PedSector);
    void slotFreeBeforeChanged(PedSector);
    void slotFreeAfterChanged(PedSector);
    void slotNewSizeChanged(PedSector);
};

#endif
