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

/* About QP_dlgCreate class:
 *
 * This class is derived from QP_UICreate that is made by designer. If you want to change
 * the layout of this dialog just use QT designer!
 *
 * This dialog is used when user want to create a new partition...
 */

#ifndef QP_DLGCREATE_H
#define QP_DLGCREATE_H

#include <QDialog>
#include "qtparted.h"
#include "ui_qp_ui_create.h"
#include "qp_libparted.h"

class QP_dlgCreate : public QDialog, public Ui_QP_UICreate {
	Q_OBJECT

public:
	QP_dlgCreate(QWidget *parent=0);
	~QP_dlgCreate();
	void init_dialog();			/*---clear combo box and other stuff					 ---*/
	void setFileSystem(QP_FileSystem *);	/*---filesystem list									 ---*/
	void addFileSystem(QString name);	/*---add a filesystem to the combobox					---*/
	void setMaxSize(PedSector maxsize);	/*---partition cannot be > of that size				  ---*/
	void addTypePrimary();			/*---add a primary type in the combobox				  ---*/
	void addTypeExtended();			/*---add an extended type in the combobox				---*/
	void addTypeLogical();			/*---add a logical type in the combobox				  ---*/
	int show_dialog();			/*---just show the dialog. Call it after init_dialog	 ---*/
	PedSector Size();			/*---return the size choosed by the user				 ---*/
	QTParted::partType type();		/*---return the type (primary/extended/logical) choosed  ---*/
	QString fileSystemName();		/*---return the filesystem choosed					   ---*/
	QString Label();			/*---return the label choosed by the user				---*/

private:
	PedSector _maxsize;			/*---keep the maxsize of the free space				  ---*/
	QP_FileSystem *_filesystem;		/*---keep the filesystem list							---*/

protected:
	void addType(QString);			/*---add a label in the combobox extended/primary/logical---*/

protected slots:
	void slotFSChanged(int);		/*---the user changed the type in filesystem			 ---*/
	void slotTypeChanged(int);		/*---the user changed the type in combobox			   ---*/
	void slotRatioChanged(int);		/*---the user changed the ration in the spinbox		  ---*/
	void slotSizeChanged(PedSector);	/*---the user changed size in the spinbox				---*/
};

#endif
