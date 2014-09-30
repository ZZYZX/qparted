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

#include "qp_partlist.h"

QP_PartList::QP_PartList(QWidget *parent, Qt::WFlags f)
	:QWidget(parent, f) {
}

QP_PartList::~QP_PartList() {
}

QP_PartInfo *QP_PartList::selPartInfo() {
	return _selPartInfo;
}

void QP_PartList::setselPartInfo(QP_PartInfo *partinfo) {
	_selPartInfo = partinfo;
}

void QP_PartList::clear() {
}

QP_Device *QP_PartList::device() {
	return _device;
}

void QP_PartList::setDevice(QP_Device *device) {
	_device = device;
}

void QP_PartList::addPrimary(QP_PartInfo *) {
}

void QP_PartList::addLogical(QP_PartInfo *) {
}

void QP_PartList::draw() {
}

float QP_PartList::mb_hdsize() {
	return _mb_hdsize;
}

void QP_PartList::set_mb_hdsize(float mb_hdsize) {
	_mb_hdsize = mb_hdsize;
}
