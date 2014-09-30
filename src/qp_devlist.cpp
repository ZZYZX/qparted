/*
	qtparted - a frontend to libparted for manipulating disk partitions
	Copyright (C) 2002-2003 Vanni Brutto <zanac (-at-) libero dot it>
	Copyright (C) 2005-2010 Bernhard Rosenkraenzer <bero@arklinux.org>
        Copyright (C) 2007-2008 David Tio <deux@arklinux.org>

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

/* About QP_DevList class:
 *
 * This class keep a list with all devices (ie all hard disk) detected by libparted
 */

#include <parted/parted.h>
#include <sys/param.h>  // MAXPATHLEN
#include <sys/stat.h>   // S_ISLNK
#include <unistd.h>	 // readlink
#include <dirent.h>
#include "qp_devlist.h"
#include "qp_common.h"


#define UPTIME_FILE "/proc/uptime"


time_t uptime() {
	char buf[255];
	double up = 0, idle = 0;
	FILE *uptime_fd;

	uptime_fd = fopen(UPTIME_FILE, "r");
	
	fread(buf, 255, 1, uptime_fd);
	if (sscanf(buf, "%lf %lf", &up, &idle) < 2) {
		fprintf(stderr, "bad data in " UPTIME_FILE "\n");
		return 0;
	}
	fclose(uptime_fd);

	return (time_t)up;
}

QP_Device::QP_Device(QP_Settings *set) {
	settings = set;
}

QP_Device::~QP_Device() {
}

bool QP_Device::newPartTable() {
	PedDisk *disk;
	const PedDiskType *type;
	PedDiskType *def_type;
	PedDevice *dev = ped_device_get(shortname().toLatin1());

	if (!dev)
		goto error;
	
	def_type = ped_disk_probe(dev);

	if (ped_device_is_busy(dev))
		goto error;

	type = ped_disk_type_get("msdos");
	if (!type)
		goto error;

	disk = ped_disk_new_fresh(dev, type);
	if (!disk)
		goto error;

	if (!ped_disk_commit (disk))
		goto error_destroy_disk;
	ped_disk_destroy (disk);
	
	_partitionTable = true;

	return true;

error_destroy_disk:
	ped_disk_destroy (disk);
error:
	return false;
}

QString QP_Device::longname() {
	return _longname;
}

void QP_Device::setLongname(QString longname) {
	_longname = longname;

	/*---get the shortname---*/
	char newstr[MAXPATHLEN];
	convertDevfsNameToShortName(longname.toLatin1(), newstr, sizeof(newstr));
	_shortname = QString(newstr);
}

QString QP_Device::shortname() {
	return _shortname;
}

void QP_Device::setShortname(QString shortname) {
	_shortname = shortname;
	_longname = QString::null;
}

bool QP_Device::isBusy() {
	return _isBusy;
}

void QP_Device::setIsBusy(bool isbusy) {
	_isBusy = isbusy;
}

void * QP_Device::data() {
	return _data;
}

void QP_Device::setData(void * data) {
	_data = data;
}

bool QP_Device::partitionTable() {
	return _partitionTable;
}

void QP_Device::setPartitionTable(bool partitiontable) {
	_partitionTable = partitiontable;
}

bool QP_Device::canUpdateGeometry() {
	/*geometry cannot be changed if last update was > boottime!!*/

	time_t lastUpdate = settings->getDevUpdate(shortname());
	time_t boottime = time((time_t)0) - uptime();

	if ((lastUpdate > boottime) && isBusy())
		return false;
	else
		return true;
}

void QP_Device::commit() {
	/*---if the device is busy must be updated to "readonly"---*/
	if (isBusy()) {
		time_t now = time((time_t)0);
		settings->setDevUpdate(shortname(), now);
	}
}

/*---this function convert a longname device to a shortname device
 *   the code was bring from partimage software made by Fran?ois Dupoux---*/
int QP_Device::convertDevfsNameToShortName(const char *szDevfs, char *szShort, int nMaxShort) {
	DIR *fdDir;
	struct dirent *dire;
	struct stat st;
	int nRes;
	char szTemp[MAXPATHLEN];
	char szTemp1[MAXPATHLEN];
	char szTemp2[MAXPATHLEN];
	int i, dwSlash;

	// in case of error, use this file:
	snprintf(szShort, nMaxShort, "%s", szDevfs);

	for (i=0, dwSlash=0; szDevfs[i]; i++)
	   if (szDevfs[i] == '/')
		   dwSlash++;

	//printf ("Slash(%s)=%d\n", szDevfs, (int)dwSlash);

	// exit if not a devfs name
	if (dwSlash <= 2)
		return 0;

	fdDir = opendir("/dev/");
	if (!fdDir) {
		return -1;
	}

	while ((dire = readdir(fdDir)) != NULL) {
		// stats
		snprintf(szTemp1, sizeof(szTemp1), "/dev/%s", dire->d_name);
		nRes = lstat(szTemp1, &st);
	  
		// if this is a sym link
		if ((nRes == 0) && S_ISLNK(st.st_mode)) {
			memset(szTemp, 0, sizeof(szTemp));
			nRes = readlink(szTemp1, szTemp, sizeof(szTemp));
	  
			memset(szTemp2, 0, sizeof(szTemp2));
			snprintf(szTemp2, sizeof(szTemp2), "/dev/%s", szTemp);
	  
			if (strcmp(szDevfs, szTemp2) == 0) {
				memset(szShort, 0, nMaxShort);
				snprintf(szShort, nMaxShort, "%s", szTemp1);
				closedir(fdDir);
				return 0;
			}
		}
	}

	closedir(fdDir);

	return 0;
}





QP_DevList::QP_DevList(QP_Settings *settings) {
	_settings = settings;
}

QP_DevList::~QP_DevList() {
	devlist.clear();
}

void QP_DevList::getDevices() {
	/*---get all the device (ie /dev/sda, /dev/hda /etc /etc)---*/
	PedDevice *dev = NULL;
	ped_device_probe_all();
	
	while ((dev = ped_device_get_next(dev))) {
		// Workaround for parted detecting CD-ROMs as harddisks
		// FIXME remove if/when parted gets fixed
		QString p(dev->path);
		if(!p.startsWith("/dev/sd") && (p.contains("/dev/scd") || p.contains("/dev/sr") || access(("/proc/ide/" + p.section('/', 2, 2) + "/cache").toLatin1(), R_OK)))
			continue;
		
		QP_Device *device = new QP_Device(_settings);

		/*---with devfs libparted return longname, otherwise the shortname---*/
		if (isDevfsEnabled())
			device->setLongname(dev->path);
		else
			device->setShortname(dev->path);

		PedDiskType *disktype = ped_disk_probe(dev);
		if (disktype)
			device->setPartitionTable(true);
		else
			device->setPartitionTable(false);

		/*---get if the device is busy---*/
		device->setIsBusy(ped_device_is_busy(dev));

		devlist.append(device);
	}
}
