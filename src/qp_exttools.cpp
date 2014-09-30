/*
	qtparted - a frontend to libparted for manipulating disk partitions
	Copyright (C) 2002-2003 Vanni Brutto

	Vanni Brutto <zanac (-at-) libero dot it>

	Copyright (C) 2011-2012 LinDev Bernhard Rosenkraenzer http://lindev.ch/

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

#include <stdio.h>

#include "qp_exttools.h"

QP_ListExternalTools::QP_ListExternalTools():QHash<QString,QP_ExternalTool*>() {
}

QP_ListExternalTools::~QP_ListExternalTools() {
	for(Iterator it=begin(); it!=end(); ++it)
		delete it.value();
}

void QP_ListExternalTools::add(QString name, QString path, QString description) {
	QP_ExternalTool *exttool = new QP_ExternalTool(name, path, description);
	insert(name, exttool);
}

QString QP_ListExternalTools::getPath(QString name) {
	if(!contains(name))
		return QString::null;
	return value(name)->path();
}

void QP_ListExternalTools::setPath(QString name, QString path) {
	if(!contains(name))
		return;
	value(name)->setPath(path);
}

QString QP_ListExternalTools::getDescription(QString name) {
	if(!contains(name))
		return QString::null;
	return value(name)->description();
}
