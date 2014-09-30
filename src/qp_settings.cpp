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

#include "qp_settings.h"
#include "qp_common.h"
#include <QStringList>
#include <QFile>
#include <cstdio>
#include <cstdlib>

static QString findApp(QString const &name) {
	QStringList s=QString(getenv("PATH")).split(':');
	// Add locations where external applications are commonly found...
	s << "/sbin" << "/usr/sbin" << "/usr/local/sbin" << "/opt/local/sbin" << "/bin" << "/usr/bin" << "/usr/local/bin" << "/opt/local/bin";
	foreach(QString const p, s) {
		QString const guess=p + "/" + name;
		if(QFile::exists(guess))
			return guess;
	}
	return QString::null;
}

QP_Settings::QP_Settings():settings(QSettings::SystemScope, "QtParted", "QtParted") {
	_layout = settings.value("/qtparted/layout", 0).toInt();

	QString mkntfs_path = settings.value("/qtparted/mkntfs", findApp("mkntfs")).toString();
	QString ntfsresize_path = settings.value("/qtparted/ntfsresize", findApp("ntfsresize")).toString();
	QString mkfs_ext3_path = settings.value("/qtparted/mkfs.ext3", findApp("mkfs.ext3")).toString();
	QString mkfs_jfs_path = settings.value("/qtparted/mkfs.jfs", findApp("mkfs.jfs")).toString();
	QString mkfs_xfs_path = settings.value("/qtparted/mkfs.xfs", findApp("mkfs.xfs")).toString();
	QString mount_path = settings.value("/qtparted/mount", findApp("mount")).toString();
	QString umount_path = settings.value("/qtparted/umount", findApp("umount")).toString();
	QString xfs_growfs_path = settings.value("/qtparted/xfs_growfs", findApp("xfs_growfs")).toString();

	lstExternalTools->add("mkntfs", 
			  mkntfs_path,
			  QObject::tr("A program that create NTFS partitions."));
	lstExternalTools->add("ntfsresize",
			  ntfsresize_path,
			  QObject::tr("A program that resize NTFS partitions."));
	lstExternalTools->add("mkfs.ext3",
			  mkfs_ext3_path,
			  QObject::tr("A program that create EXT3 partitions."));
	lstExternalTools->add("mkfs.jfs",
			  mkfs_jfs_path,
			  QObject::tr("A program that create JFS partitions."));
	lstExternalTools->add("mkfs.xfs",
			  mkfs_xfs_path,
			  QObject::tr("A program that create XFS partitions."));
	lstExternalTools->add("mount",
			  mount_path,
			  QObject::tr("Utility for mount a filesystem."));
	lstExternalTools->add("umount",
			  umount_path,
			  QObject::tr("Utility for umount a filesystem."));
	lstExternalTools->add("xfs_growfs",
			  xfs_growfs_path,
			  QObject::tr("Utility for grow a xfs filesystem."));
}

QP_Settings::~QP_Settings() {
}

int QP_Settings::layout() {
	return _layout;
}

void QP_Settings::setLayout(int layout) {
	settings.setValue("/qtparted/layout", layout);
	_layout = layout;

	QString mkntfs_path = lstExternalTools->getPath("mkntfs");
	QString ntfsresize_path = lstExternalTools->getPath("ntfsresize");
	QString mkfs_ext3_path = lstExternalTools->getPath("mkfs.ext3");
	QString mkfs_jfs_path = lstExternalTools->getPath("mkfs.jfs");
	QString mkfs_xfs_path = lstExternalTools->getPath("mkfs.xfs");
	QString mount_path = lstExternalTools->getPath("mount");
	QString umount_path = lstExternalTools->getPath("umount");
	QString xfs_growfs_path = lstExternalTools->getPath("xfs_growfs");

	settings.setValue("/qtparted/mkntfs", mkntfs_path);
	settings.setValue("/qtparted/ntfsresize", ntfsresize_path);
	settings.setValue("/qtparted/mkfs.ext3", mkfs_ext3_path);
	settings.setValue("/qtparted/mkfs.jfs", mkfs_jfs_path);
	settings.setValue("/qtparted/mkfs.xfs", mkfs_xfs_path);
	settings.setValue("/qtparted/mount", mount_path);
	settings.setValue("/qtparted/umount", umount_path);
	settings.setValue("/qtparted/xfs_growfs", xfs_growfs_path);
}

time_t QP_Settings::getDevUpdate(QString device) {
	QString entry = QString("%1%2")
			.arg("/qtparted")
			.arg(device);
	
	QString time = settings.value(entry, "0").toString();

	long long longTime;
	sscanf(time.toLatin1(), "%lld", &longTime);

	return (time_t)longTime;
}

void QP_Settings::setDevUpdate(QString device, time_t time) {
	QString entry = QString("%1%2")
					.arg("/qtparted")
					.arg(device);
	
	char buf[255];
	sprintf(buf, "%lld", (long long)time);

	settings.setValue(entry, buf);
}
