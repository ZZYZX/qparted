/***************************************************************************
	statistics.cpp  -  description
	-------------------
	begin				: Wed Sep  4 19:21:54 UTC 2002
	copyright			: (C) 2002 by Francois Dupoux
	email				: fdupoux@partimage.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or	   *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/vfs.h>
#include <mntent.h>
#include <sys/mount.h>
#include <sys/param.h>  // MAXPATHLEN

#include <qmessagebox.h>
#include "statistics.h"
#include "qp_filesystem.h"
#include "qp_common.h"

#define TMP_MOUNTPOINT "/tmp/mntqp"
#define KBYTE_SECTORS 2

//------------------------------------------------
PedSector space_stats(QP_PartInfo *partinfo) {
	unsigned long a;
	a = getFsUsedKiloBytes(partinfo);

	/*printf ("device(%s)=%lu KB used\n", partinfo->partname().toLatin1(), a);*/
	if (a == 0) return -1;
	else return a * KBYTE_SECTORS;
}

//------------------------------------------------
int my_mount(QP_PartInfo *partinfo, const char *szMountPoint) {
	// security: if already mounted
	umount(szMountPoint);

	char type[255];

	if (((partinfo->fsspec->name().compare("fat32") == 0)
	  || (partinfo->fsspec->name().compare("fat16") == 0))) {
		strcpy(type, "vfat");
	} else {
		strcpy(type, partinfo->fsspec->name().toLatin1());
	}
	return mount(partinfo->partname().toLatin1(),
	             szMountPoint, type, MS_NOATIME | MS_RDONLY, NULL);
}

//---------------------------------------
QString mountPoint(QP_PartInfo *partinfo) {
	FILE *f;
	struct mntent *mnt;

	if ((f = setmntent (MOUNTED, "r")) == 0)
		return QString::null;

	// loop: compare device with each mounted device
	while (((mnt = getmntent (f)) != 0)) {
		if ((partinfo->shortname().compare(mnt->mnt_fsname) == 0) || (isDevfsEnabled() && (partinfo->longname().compare(mnt->mnt_fsname) == 0))) {
			endmntent(f);
			return mnt->mnt_dir;
		}
	}
	endmntent(f);
	return QString::null;
}

//------------------------------------------------
unsigned long getFsUsedKiloBytes(QP_PartInfo *partinfo) {
	struct statfs sfs;
	unsigned long long a, b, c;
	bool bToBeUnmounted;
	unsigned long lResult = 0; // to be returned
	QString mnt;

	// init
	bToBeUnmounted = false;

	mnt = mountPoint(partinfo);
	
	/*---if not mounted -> mount it---*/
	if (mnt.isEmpty()) {
		mkdir (TMP_MOUNTPOINT, 755);
		int nRes = my_mount(partinfo, TMP_MOUNTPOINT);
		if(nRes != 0) // Can't mount --> can't get statistics
			return 0L;
		bToBeUnmounted = (nRes == 0);
		mnt = TMP_MOUNTPOINT;
	}

	if (statfs(mnt.toLatin1(), &sfs) != -1) {
		a = (sfs.f_blocks - sfs.f_bavail); // used blocks count
		b = a * sfs.f_bsize; // used bytes count
		c = b / 1024LL; // user KiloBytes count
		lResult = c;
	}

	if (bToBeUnmounted) {
		int nRes = umount(mnt.toLatin1());
		if (nRes != 0) {
			QString label = QObject::tr("Cannot umount partition device: %1."
			                "Please do it by hand first to commit the changes!")
			                .arg(partinfo->partname());
			QMessageBox::information(NULL, "QtParted", label);
		}
	}

	return lResult; // success
}
