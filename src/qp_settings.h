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

/* About QP_Settings class:
 *
 * This is a class use to load/store user settings of QTParted.
 */

#include <time.h>
#include <qobject.h>
#include <QSettings>
#ifndef QP_SETTINGS_H
#define QP_SETTINGS_H

class QP_Settings : public QObject {
	Q_OBJECT
public:
	QP_Settings();
	~QP_Settings();
	int layout();
	void setLayout(int);
	time_t getDevUpdate(QString);	   //get the last time that a device was updated (ie commited)
	void setDevUpdate(QString, time_t); //the device was commit, so save the time!
private:
	QSettings settings;
	int _layout;
};
#endif
