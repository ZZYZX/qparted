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

/* About QP_dlgFormat class:
 *
 * This class is derived from QP_UIFormat that is made by designer. If you want to change
 * the layout of this dialog just use QT designer!
 *
 * This dialog is used when user want to format an existing partition...
 */

#ifndef QP_DLGFORMAT_H
#define QP_DLGFORMAT_H

#include "qtparted.h"
#include "ui_qp_ui_format.h"
#include "qp_libparted.h"

class QP_dlgFormat : public QDialog, public Ui::QP_UIFormat {
Q_OBJECT

public:
    QP_dlgFormat(QWidget *parent=0);
    ~QP_dlgFormat();
    void init_dialog();                     /*---clear combo box and other stuff                     ---*/
    void addFileSystem(QString name);       /*---add a filesystem to the combobox                    ---*/
    int show_dialog();                      /*---just show the dialog. Call it after init_dialog     ---*/
    QString fileSystemName();               /*---return the filesystem choosed                       ---*/
    QString Label();                        /*---return the label choosed by the user                ---*/
};

#endif
