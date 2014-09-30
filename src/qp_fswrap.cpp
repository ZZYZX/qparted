/*
    qtparted - a frontend to libparted for manipulating disk partitions
    Copyright (C) 2002-2003 Vanni Brutto
    Vanni Brutto <zanac (-at-) libero dot it>

    Copyright (C) 2005-2010 Bernhard Rosenkraenzer
    bero (-at-) arklinux dot org

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

#include <qregexp.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <stdint.h>
#include <sys/mount.h>
#include <qapplication.h>

#include "qp_fswrap.h"
#include "qp_actlist.h"
#include "qp_common.h"
#include "qp_debug.h"

#define NOTFOUND tr("command not found")
#define TMP_MOUNTPOINT "/tmp/mntqp"

#define my_min(a,b) ((a)<(b) ? (a):(b))

bool QP_FSWrap::fs_open(QString cmdline, bool localized)
{

	/*---this force stderr to stdout and a parsable language---*/
	QString dupcmdline = QString("%1 %2")
	                     .arg(cmdline)
	                     .arg(QString("2>&1"));

	if (!localized)
		dupcmdline = "LC_ALL=POSIX " + dupcmdline;

	/*---open a pipe from the command line---*/
	fp = popen(dupcmdline.toLatin1(), "r");

	if (fp)
		return true;
	else
		return false;
}

char *QP_FSWrap::fs_getline()
{
	bool rc = fgets(line, sizeof line, fp);

	if (rc)
		return line;
	else
		return NULL;
}

int QP_FSWrap::fs_close()
{
	return pclose(fp);
}

QP_FSWrap *QP_FSWrap::fswrap(QString name)
{
	QP_FSWrap *fswrap;

	if (name.compare("ntfs") == 0)
		fswrap = new QP_FSNtfs();
	else if (name.compare("jfs") == 0)
		fswrap = new QP_FSJfs();
	else if (name.compare("ext2") == 0)
		fswrap = new QP_FSExt2();
	else if (name.compare("ext3") == 0)
		fswrap = new QP_FSExt3();
	else if (name.compare("ext4") == 0)
		fswrap = new QP_FSExt4();
	else if (name.compare("ext3") == 0)
		fswrap = new QP_FSBtrFS();
	else if (name.compare("xfs") == 0)
		fswrap = new QP_FSXfs();
	else if (name.compare("swap") == 0)
		fswrap = new QP_FSswap();
	else if (name == "fat" || name == "vfat")
		fswrap = new QP_FSFat();
	else if (name == "fat16")
		fswrap = new QP_FSFat16();
	else if (name == "fat32")
		fswrap = new QP_FSFat32();
	else
		fswrap = 0;

	return fswrap;
}

QString QP_FSWrap::get_label(PedPartition * part, QString name)
{
	if (name.compare("ntfs") == 0)
		return QP_FSNtfs::_get_label(part);
	else if (name.compare("jfs") == 0)
		return QP_FSJfs::_get_label(part);
	else if (name.compare("ext2") == 0)
		return QP_FSExt2::_get_label(part);
	else if (name.compare("ext3") == 0)
		return QP_FSExt3::_get_label(part);
	else if (name.compare("ext4") == 0)
		return QP_FSExt4::_get_label(part);
	else if (name.compare("btrfs") == 0)
		return QP_FSBtrFS::_get_label(part);
	else if (name.compare("xfs") == 0)
		return QP_FSXfs::_get_label(part);
	else if (name == "fat" || name == "fat16" || name == "fat32" || name == "vfat")
		return QP_FSFat::_get_label(part);
	else if (name.compare("reiserfs") == 0)
		return QP_FSReiserFS::_get_label(part);
	else if (name.compare("swap") == 0)
		return QP_FSswap::_get_label(part);
	else
		return QString::null;
}

bool QP_FSWrap::read_sector(PedPartition * part, PedSector offset,
			    PedSector count, char *buffer)
{
	/*---open a new device, read a sector and close it---*/
	if (!ped_device_open(part->geom.dev))
		return false;

	if (!ped_geometry_read(&part->geom, buffer, offset, count)) {
		ped_device_close(part->geom.dev);
		return false;
	}

	if (!ped_device_close(part->geom.dev))
		return false;

	return true;
}

bool QP_FSWrap::qpMount(QString device)
{
	bool error = false;

	/*---just to be sure! :)---*/
	qpUMount(device);

	/*---mount the partition---*/
	QString cmdline = lstExternalTools->getPath("mount") + " " + device + " " + TMP_MOUNTPOINT;

	if (!fs_open(cmdline, true)) {
		_message = QString(NOTFOUND);
		return false;
	}

	char *cline;
	while ((cline = fs_getline())) {
		QString line = QString(cline);

		QRegExp rx = QRegExp("^mount: (.*)$");
		if (rx.indexIn(line) == 0) {
			QString capError = rx.cap(1);
			_message = capError;
			error = true;
		}
	}
	fs_close();

	return !error;
}

bool QP_FSWrap::qpUMount(QString device)
{
	int ret = umount(device.toLatin1());
	if (ret) {
		_message = QString(strerror(errno));
		return false;
	}
	return true;
}



/*---NTFS WRAPPER-----------------------------------------------------------------*/
QP_FSNtfs::QP_FSNtfs():QP_FSWrap()
{
	/*---check if the wrapper is installed---*/
	QString cmdline = "which " + lstExternalTools->getPath("mkntfs");
	fs_open(cmdline);
	char *cline;
	while ((cline = fs_getline()))
		wrap_create = true;
	fs_close();

	/*---check if the wrapper is installed---*/
	cmdline = "which " + lstExternalTools->getPath("ntfsresize");
	fs_open(cmdline);


	while ((cline = fs_getline())) {
		wrap_resize = Both;
		wrap_min_size = true;
	}
	fs_close();
}

bool QP_FSNtfs::resize(QP_LibParted * _libparted, bool write,
		       QP_PartInfo * partinfo, PedSector new_start,
		       PedSector new_end)
{
	/*---pointer to libparted---*/
	QP_LibParted *libparted = _libparted;

	showDebug("%s", "Resizing a filesystem using a wrapper\n");

	/*---the user want to shrink or enlarge the partition?---*/
	/*---the user want to shrink!---*/
	if (new_end < partinfo->end) {
	/*---update the filesystem---*/
		showDebug("%s", "shrinking filesystem...\n");
		if (!ntfsresize
		    (write, partinfo->partname(),
		     new_end - new_start - 4 * MEGABYTE_SECTORS)) {
			showDebug("%s", "shrinking filesystem ko\n");
			return false;
		}

		/*---and now update geometry of the partition---*/
		showDebug("%s", "update geometry...\n");
		if (!libparted->set_geometry(partinfo, new_start, new_end)) {
			showDebug("%s", "update geometry ko\n");
			_message = libparted->message();
			return false;
		} else {
			/*---if you are NOT committing then add in the undo/commit list---*/
			if (!write) {
				showDebug("%s",
					  "operation added to undo/commit list\n");
				PedPartitionType parttype =
				    libparted->type2parttype(partinfo->
							     type);
				PedGeometry geom =
				    libparted->get_geometry(partinfo);
				libparted->actlist->ins_resize(partinfo->
							       num,
							       new_start,
							       new_end,
							       geom,
							       parttype);
			}
			return true;
		}
		/*---the user wants to enlarge!---*/
	} else {
		/*---cannot enlarge if we cannot change disk geometry!---*/
		if (partinfo->device()->isBusy()) {
			showDebug("%s",
				  "the device is busy, so you cannot enlarge it\n");
			_message = tr(
			    "Cannot enlarge a partition if the disk device is busy");
			return false;
		}

		/*---first se the geometry of the partition---*/
		showDebug("%s", "update geometry...\n");
		if (!libparted->set_geometry(partinfo, new_start, new_end)) {
			showDebug("%s", "update geometry ko\n");
			_message = libparted->message();
			return false;
		} else {
			/*---if you are NOT committing then add in the undo/commit list---*/
			if (!write) {
				showDebug("%s",
					  "operation added to undo/commit list\n");
				PedPartitionType parttype =
				    libparted->type2parttype(partinfo->
							     type);
				PedGeometry geom =
				    libparted->get_geometry(partinfo);
				libparted->actlist->ins_resize(partinfo->
							       num,
							       new_start,
							       new_end,
							       geom,
							       parttype);
			}
		}

		if (write) {
			/*---and now update the filesystem!---*/
			showDebug("%s", "enlarge filesystem...\n");
			if (!ntfsresize
			    (write, partinfo->partname(),
			     new_end - new_start - 4 * MEGABYTE_SECTORS)) {
				showDebug("%s", "enlarge filesystem ko\n");
				return false;
			} else {
				return true;
			}
		} else {
			return true;
		}
	}
}

bool QP_FSNtfs::ntfsresize(bool write, QString dev, PedSector newsize)
{
	/*---init of the error message---*/
	_message = QString::null;

	/*---calculate size of the partition in bytes---*/
	PedSector size = (PedSector) ((newsize - 1) * 512);

	/*---read-only test---*/
	QString cmdline = lstExternalTools->getPath("ntfsresize") + " " +
	                  "-n -ff -s " + QString::number(size) + " " + dev;

	if (!fs_open(cmdline)) {
		_message = QString(NOTFOUND);
		return false;
	}

	bool error = false;
	char *cline;
	while ((cline = fs_getline())) {
		QString line = QString(cline);

		QRegExp rx;

		if (!error) {
			rx = QRegExp("^ERROR.*: (.*)");
			if (rx.indexIn(line) == 0) {
				QString captured = rx.cap(1);
				_message = QString(captured);
				error = true;
			}
		}

		if (!error) {
			rx = QRegExp("^The volume end is fragmented.*");
			if (rx.indexIn(line) == 0) {
				_message =
				    QString
				    ("The partition is fragmented.");
				error = true;
			}
		}

		/*---progress bar!---*/
		QString linesub = line;
#ifdef QT30COMPATIBILITY
		linesub.replace(QRegExp("\r"), " ");
#else
		linesub.replace(QChar('\r'), " ");
#endif
		//example: 34,72 percent completed
		rx = QRegExp("^.* (\\d*),(\\d*) percent completed.*$");
		if (rx.indexIn(linesub) == 0) {
			QString capIntPercent = rx.cap(1);
			printf("letto: %s\n", capIntPercent.toLatin1().data());
			//QString capFloatPercent = rx.cap(2);

			bool rc;
			int iPerc = capIntPercent.toInt(&rc) - 1;
			if (iPerc < 0)
				iPerc = 0;	//the percente completed run many times, but i don't want it reach 100%

			emit sigTimer(iPerc,
				      QString(tr("Resizing in progress.")),
				      QString::null);
		}
		//BETA: change could with might with ntfsresize 1.9
		//rx = QRegExp("^Now You could resize at \\d* bytes or (\\d*) .*");
		rx = QRegExp("^.*You ..... resize at \\d* bytes or (\\d*) .*");
		if (rx.indexIn(line) == 0) {
			QString captured = rx.cap(1);
			_message = QString("The partition is fragmented. Try to defragment it, or resize to %1MB")
			           .arg(captured);
			error = true;
		}
	}
	fs_close();

	if (error)
		return false;

	/*---if the user want to run a readonly test just return true---*/
	if (!write)
		return true;


	/*---ok, the readonly test seems ok... now we resize it!---*/
	cmdline = lstExternalTools->getPath("ntfsresize") + " -ff -s " + QString::number(size) + " " + dev;
	if (!fs_open(cmdline)) {
		_message = QString(NOTFOUND);
		return false;
	}

	bool success = false;
	while ((cline = fs_getline())) {
		QString line = QString(cline);
		QRegExp rx;

		/*---progress bar!---*/
		QString linesub = line;
#ifdef QT30COMPATIBILITY
		linesub.replace(QRegExp("\r"), " ");
#else
		linesub.replace(QChar('\r'), " ");
#endif
		//example: 34,72 percent completed
		rx = QRegExp("^.* (\\d*),(\\d*) percent completed.*$");
		if (rx.indexIn(linesub) == 0) {
			QString capIntPercent = rx.cap(1);
			//QString capFloatPercent = rx.cap(2);

			bool rc;
			int iPerc = capIntPercent.toInt(&rc) - 1;
			if (iPerc < 0)
				iPerc = 0;	//the percente completed run many times, but i don't want it reach 100%

			emit sigTimer(iPerc,
				      QString(tr("Resizing in progress.")),
				      QString::null);
		}
		//rx = QRegExp("^Successfully resized NTFS on device");
		rx = QRegExp("^.*[Ss]uccessfully.*");
		if (rx.indexIn(line) == 0)
			success = true;
		rx = QRegExp("^Nothing to do: NTFS volume size is already OK.");
		if (rx.indexIn(line) == 0)
			success = true;

		rx = QRegExp("^Syncing device.*");
		if (rx.indexIn(line) == 0) {
			emit sigTimer(99, QString(tr("Syncing device.")),
				      QString::null);
		}

		rx = QRegExp("^ERROR.*: (.*)");
		if (rx.indexIn(line) == 0) {
			QString captured = rx.cap(1);
			_message = QString(captured);
		}
	}
	fs_close();

	if (success)
		return true;
	else {
		if (_message.isNull())
			_message = QString("An error occured! :(");
		return false;
	}
}

bool QP_FSNtfs::mkpartfs(QString dev, QString label)
{
	QString cmdline;

	/*---init of the error message---*/
	_message = QString::null;

	/*---prepare the command line---*/
	if (label.isEmpty())
		cmdline = " -f -s 512 " + dev;
	else
		cmdline = " -f -s 512 -L " + label + " " + dev;
	cmdline = lstExternalTools->getPath("mkntfs") + cmdline;

	if (!fs_open(cmdline)) {
		_message = QString(NOTFOUND);
		return false;
	}


	bool success = false;
	char *cline;
	while ((cline = fs_getline())) {
		QString line = QString(cline);

		QRegExp rx;
		rx = QRegExp("^mkntfs completed successfully. Have a nice day.");
		if (rx.indexIn(line) == 0)
			success = true;

		rx = QRegExp("^ERROR.*: (.*)");
		if (rx.indexIn(line) == 0) {
			QString captured = rx.cap(1);
			_message = QString(captured);
			success = false;
		}
	}
	fs_close();

	return success;
}

PedSector QP_FSNtfs::min_size(QString dev)
{
	PedSector size = -1;

	/*---init of the error message---*/
	_message = QString::null;

	/*---prepare the command line---*/
	QString cmdline = lstExternalTools->getPath("ntfsresize") + " -f -i " + dev;

	if (!fs_open(cmdline)) {
		_message = QString(NOTFOUND);
		return false;
	}


	bool success = false;
	char *cline;
	while ((cline = fs_getline())) {
		QString line = QString(cline);

		QRegExp rx;
		rx = QRegExp("^.*You ..... resize at (\\d*) bytes or (\\d*) .*");
		if (rx.indexIn(line) == 0) {
			QString captured = rx.cap(1);
			sscanf(captured.toLatin1(), "%lld", &size);
			size /= 512;
			size += 8 * MEGABYTE_SECTORS;

			success = true;
		}
	}
	fs_close();

	return size;
}

QString QP_FSNtfs::fsname()
{
	return QString("ntfs");
}

/*QString QP_FSNtfs::_get_label(PedPartition *) {
	return QString::null;
}*/

/*---SWAP WRAPPER----------------------------------------------------------------*/
QP_FSswap::QP_FSswap():QP_FSWrap()
{
	/*---check if the wrapper is installed---*/
	QString cmdline = "which " + lstExternalTools->getPath("mkswap");
	fs_open(cmdline);

	char *cline;
	while ((cline = fs_getline()))
		wrap_create = true;
	fs_close();
}

bool QP_FSswap::mkpartfs(QString dev, QString label) {
	QString cmdline;

	/*---init of the error message---*/
	_message = QString::null;

	/*---prepare the command line---*/
	if (!label.isEmpty())
		cmdline = " -L " + label;
	cmdline += " -v1 ";
	cmdline = lstExternalTools->getPath("mkswap") + cmdline + " " + dev;

	if (!fs_open(cmdline)) {
		_message = QString(NOTFOUND);
		return false;
	}


	bool writenode = false;
	bool success = false;
	char *cline;
	while ((cline = fs_getline())) {
		QString line = QString(cline);
	}
	success = (fs_close() == 0);

	if (!success)
		_message = tr("There was a problem with mkswap.");

	return success;
}

/*---JFS WRAPPER-----------------------------------------------------------------*/
QP_FSJfs::QP_FSJfs():QP_FSWrap(Enlarge)
{
	/*---check if the wrapper is installed---*/
	QString cmdline = "which " + lstExternalTools->getPath("mkfs.jfs");
	fs_open(cmdline);

	char *cline;
	while ((cline = fs_getline()))
		wrap_create = true;
	fs_close();
}

bool QP_FSJfs::resize(QP_LibParted * _libparted, bool write,
		      QP_PartInfo * partinfo, PedSector new_start,
		      PedSector new_end)
{
	/*---pointer to libparted---*/
	QP_LibParted *libparted = _libparted;

	showDebug("%s", "Resizing a filesystem using a wrapper\n");

	/*---the user wants to shrink or enlarge the partition?---*/
	/*---the user wants to shrink!---*/
	if (new_end < partinfo->end) {
		/*---update the filesystem---*/
		showDebug("%s",
			  "shrinking filesystem not supported with jfs...\n");
		return false;
	/*---the user want to enlarge!---*/
	} else {
		/*---cannot enlarge if we cannot change disk geometry!---*/
		if (partinfo->device()->isBusy()) {
			showDebug("%s",
				  "the device is busy, so you cannot enlarge it\n");
			_message = tr("Cannot enlarge a partition if the disk device is busy");
			return false;
		}

		/*---first se the geometry of the partition---*/
		showDebug("%s", "update geometry...\n");
		if (!libparted->set_geometry(partinfo, new_start, new_end)) {
			showDebug("%s", "update geometry ko\n");
			_message = libparted->message();
			return false;
		} else {
			/*---if you are NOT committing then add in the undo/commit list---*/
			if (!write) {
				showDebug("%s",
					  "operation added to undo/commit list\n");
				PedPartitionType parttype =
				    libparted->type2parttype(partinfo->
							     type);
				PedGeometry geom =
				    libparted->get_geometry(partinfo);
				libparted->actlist->ins_resize(partinfo->
							       num,
							       new_start,
							       new_end,
							       geom,
							       parttype);
			}
		}

		if (write) {
			/*---and now update the filesystem!---*/
			showDebug("%s", "enlarge filesystem...\n");
			if (!jfsresize
			    (write, partinfo, new_end - new_start)) {
				showDebug("%s", "enlarge filesystem ko\n");
				return false;
			} else {
				return true;
			}
		} else {
			return true;
		}
	}
}

bool QP_FSJfs::jfsresize(bool write, QP_PartInfo * partinfo, PedSector)
{
	QString cmdline;

	bool error = false;

	if (!write)
		return true;

	/*---init of the error message---*/
	_message = QString::null;

	if (!qpMount(partinfo->partname()))
		return false;


	/*---do the resize!---*/
	cmdline = lstExternalTools->getPath("mount") + " -o remount,resize= " + TMP_MOUNTPOINT;

	if (!fs_open(cmdline)) {
		_message = QString(NOTFOUND);
		return false;
	}

	char *cline;
	while ((cline = fs_getline())) {
		QString line = QString(cline);

		QRegExp rx;
		rx = QRegExp("^mount: (.*)$");
		if (rx.indexIn(line) == 0) {
			QString capError = rx.cap(1);
			_message = capError;
			error = true;
		}
	}
	fs_close();

	if (error)
		return false;

	if (!qpUMount(partinfo->partname()))
		return false;

	return true;
	/*fino a qui */
}

bool QP_FSJfs::mkpartfs(QString dev, QString label)
{
	QString cmdline;

	/*---init of the error message---*/
	_message = QString::null;

	/*---prepare the command line---*/
	if (label.isEmpty())
		cmdline = " -q " + dev;
	else
		cmdline = " -q -L " + label + " " + dev;
	cmdline = lstExternalTools->getPath("mkfs.jfs") + cmdline;

	if (!fs_open(cmdline)) {
		_message = QString(NOTFOUND);
		return false;
	}


	bool success = false;
	char *cline;
	while ((cline = fs_getline())) {
		QString line = QString(cline);

		QRegExp rx;
		rx = QRegExp("^Format completed successfully.");
		if (rx.indexIn(line) == 0)
			success = true;
	}
	fs_close();

	return success;
}

QString QP_FSJfs::fsname()
{
	return QString("jfs");
}

QString QP_FSJfs::_get_label(PedPartition * part)
{
	char bootsect[2048];
	char label[12];

	if (!QP_FSWrap::read_sector(part, 64, 4, bootsect))
		return QString::null;

	memset(label, 0, sizeof(label));
	strncpy(label, bootsect + 101, 11);

	return QString(label);
}

/*---EXT2 WRAPPER----------------------------------------------------------------*/
QP_FSExt2::QP_FSExt2():QP_FSWrap(),_fsType("ext2"),_extraArgs(QString::null)
{
	/*---check if the wrapper is installed---*/
	QString cmdline = "which " + lstExternalTools->getPath("mkfs." + _fsType);
	fs_open(cmdline);

	char *cline;
	while ((cline = fs_getline()))
		wrap_create = true;
	fs_close();
}

QString QP_FSExt2::_get_label(PedPartition * part)
{
	char bootsect[2048];	// sector number 2 (offset 1024)
	char label[16];

	if (!QP_FSWrap::read_sector(part, 2, 4, bootsect))
		return QString::null;

	memset(label, 0, sizeof(label));
	strncpy(label, bootsect + 120, 16);

	//printf("returned buffer ext2/3: [%s]\n", label);

	return QString(label);
}

bool QP_FSExt2::mkpartfs(QString dev, QString label)
{
	QString cmdline;

	/*---init of the error message---*/
	_message = QString::null;

	/*---prepare the command line---*/
	if (!label.isEmpty())
		cmdline = " -L " + label;
	cmdline = lstExternalTools->getPath("mkfs." + _fsType) + " -t " + _fsType + " -m 1 " + cmdline + " " + _extraArgs + " " + dev;

	if (!fs_open(cmdline)) {
		_message = QString(NOTFOUND);
		return false;
	}


	bool writenode = false;
	bool success = false;
	char *cline;
	while ((cline = fs_getline())) {
		QString line = QString(cline);

		QRegExp rx;
		rx = QRegExp("^Writing inode tables");
		if (rx.indexIn(line) == 0) {
			writenode = true;
		}

		rx = QRegExp("^Creating journal");
		if (rx.indexIn(line) == 0) {
			writenode = false;
			emit sigTimer(90,
				      QString(tr
					      ("Writing superblocks and filesystem.")),
				      QString::null);
		}

		if (writenode) {
			QString linesub = line;

#ifdef QT30COMPATIBILITY
			linesub.replace(QRegExp("\b"), " ");
#else
			linesub.replace(QChar('\b'), " ");
#endif
			rx = QRegExp("^.* (\\d*)/(\\d*) .*$");
			if (rx.indexIn(linesub) == 0) {
				QString capActual = rx.cap(1);
				QString capTotal = rx.cap(2);

				bool rc;
				int iActual = capActual.toInt(&rc);
				int iTotal = capTotal.toInt(&rc);

				int iPerc = iActual * 80 / iTotal;	//The percentual is calculated in 80% ;)
				emit sigTimer(iPerc,
					      QString(tr
						      ("Writing inode tables.")),
					      QString::null);
			}
		}

		rx = QRegExp("^Writing superblocks and filesystem accounting information: done");
		if (rx.indexIn(line) == 0)
			success = true;
	}
	fs_close();

	if (!success)
		_message = tr("There was a problem with mkfs.%1.").arg(_fsType);

	return success;
}

QString QP_FSExt2::fsname()
{
	return _fsType;
}

/*---EXT3 WRAPPER----------------------------------------------------------------*/
QP_FSExt3::QP_FSExt3():QP_FSExt2()
{
	_fsType = "ext3";
	_extraArgs = " -j ";
}


/*---EXT4 WRAPPER----------------------------------------------------------------*/
QP_FSExt4::QP_FSExt4():QP_FSExt3()
{
	_fsType = "ext4";
	_extraArgs += " -j -O dir_index,extent,flex_bg,resize_inode,sparse_super,uninit_bg ";
}

/*---BTRFS WRAPPER----------------------------------------------------------------*/
QP_FSBtrFS::QP_FSBtrFS():QP_FSWrap()
{
	/*---check if the wrapper is installed---*/
	QString cmdline = "which " + lstExternalTools->getPath("mkfs.btrfs");
	fs_open(cmdline);

	char *cline;
	while ((cline = fs_getline()))
		wrap_create = true;
	fs_close();

}

bool QP_FSBtrFS::mkpartfs(QString dev, QString label)
{
	QString cmdline;

	/*---init of the error message---*/
	_message = QString::null;

	/*---prepare the command line---*/
	if (!label.isEmpty())
		cmdline = " -L " + label;
	cmdline = lstExternalTools->getPath("mkfs.btrfs") + cmdline + " " + dev;

	if (!fs_open(cmdline)) {
		_message = QString(NOTFOUND);
		return false;
	}

	bool writenode = false;
	bool success = false;
	char *cline;
	while ((cline = fs_getline())) {
		QString line = QString(cline);

		QRegExp rx;
		rx = QRegExp("^Writing inode tables");
		if (rx.indexIn(line) == 0) {
			writenode = true;
		}

		rx = QRegExp("^Creating journal");
		if (rx.indexIn(line) == 0) {
			writenode = false;
			emit sigTimer(90,
				      QString(tr
					      ("Writing superblocks and filesystem.")),
				      QString::null);
		}

		if (writenode) {
			QString linesub = line;

#ifdef QT30COMPATIBILITY
			linesub.replace(QRegExp("\b"), " ");
#else
			linesub.replace(QChar('\b'), " ");
#endif
			rx = QRegExp("^.* (\\d*)/(\\d*) .*$");
			if (rx.indexIn(linesub) == 0) {
				QString capActual = rx.cap(1);
				QString capTotal = rx.cap(2);

				bool rc;
				int iActual = capActual.toInt(&rc);
				int iTotal = capTotal.toInt(&rc);

				int iPerc = iActual * 80 / iTotal;	//The percentual is calculated in 80% ;)
				emit sigTimer(iPerc,
					      QString(tr
						      ("Writing inode tables.")),
					      QString::null);
			}
		}

		rx = QRegExp("^Writing superblocks and filesystem accounting information: done");
		if (rx.indexIn(line) == 0)
			success = true;
	}
	fs_close();

	if (!success)
		_message = QString(tr("There was a problem with mkfs.btrfs."));

	return success;
}

QString QP_FSBtrFS::fsname()
{
	return QString("btrfs");
}

QString QP_FSBtrFS::_get_label(PedPartition * p)
{
	return QP_FSExt2::_get_label(p);
	//return QString::null;
}

/*---XFS WRAPPER-----------------------------------------------------------------*/
QP_FSXfs::QP_FSXfs():QP_FSWrap()
{
	/*---check if the wrapper is installed---*/
	QString cmdline = "which " + lstExternalTools->getPath("mkfs.xfs");
	fs_open(cmdline);

	char *cline;
	while ((cline = fs_getline()))
		wrap_create = true;
	fs_close();


	/*---check if the wrapper is installed---*/
	cmdline = "which " + lstExternalTools->getPath("xfs_growfs");
	fs_open(cmdline);

	while ((cline = fs_getline()))
		wrap_resize = Enlarge;
	fs_close();
}

bool QP_FSXfs::mkpartfs(QString dev, QString label)
{
	QString cmdline;

	/*---init of the error message---*/
	_message = QString::null;

	/*---prepare the command line---*/
	if (label.isEmpty())
		cmdline = " -f " + dev;
	else
		cmdline = " -f -L " + label + " " + dev;
	cmdline = lstExternalTools->getPath("mkfs.xfs") + cmdline;

	if (!fs_open(cmdline)) {
		_message = QString(NOTFOUND);
		return false;
	}

	bool success = false;
	char *cline;
	while ((cline = fs_getline())) {
		QString line = QString(cline);

		QRegExp rx;
		rx = QRegExp("^realtime =.*");
		if (rx.indexIn(line) == 0)
			success = true;
	}
	fs_close();

	if (!success)
		_message = QString(tr("There was a problem with mkfs.xfs."));
	return success;
}

bool QP_FSXfs::resize(QP_LibParted * _libparted, bool write,
		      QP_PartInfo * partinfo, PedSector new_start,
		      PedSector new_end)
{
	/*---pointer to libparted---*/
	QP_LibParted *libparted = _libparted;

	showDebug("%s", "Resizing a filesystem using a wrapper\n");

	/*---the user wants to shrink or enlarge the partition?---*/
	/*---the user wants to shrink!---*/
	if (new_end < partinfo->end) {
		/*---update the filesystem---*/
		showDebug("%s",
			  "shrinking filesystem not supported with xfs...\n");
		return false;
	/*---the user want to enlarge!---*/
	} else {
		/*---cannot enlarge if we cannot change disk geometry!---*/
		if (partinfo->device()->isBusy()) {
			showDebug("%s",
				  "the device is busy, so you cannot enlarge it\n");
			_message =
			    tr
			    ("Cannot enlarge a partition if the disk device is busy");
			return false;
		}

		/*---first se the geometry of the partition---*/
		showDebug("%s", "update geometry...\n");
		if (!libparted->set_geometry(partinfo, new_start, new_end)) {
			showDebug("%s", "update geometry ko\n");
			_message = libparted->message();
			return false;
		} else {
			/*---if you are NOT committing then add in the undo/commit list---*/
			if (!write) {
				showDebug("%s",
					  "operation added to undo/commit list\n");
				PedPartitionType parttype =
				    libparted->type2parttype(partinfo->
							     type);
				PedGeometry geom =
				    libparted->get_geometry(partinfo);
				libparted->actlist->ins_resize(partinfo->
							       num,
							       new_start,
							       new_end,
							       geom,
							       parttype);
			}
		}

		if (write) {
			/*---and now update the filesystem!---*/
			showDebug("%s", "enlarge filesystem...\n");
			if (!xfsresize
			    (write, partinfo, new_end - new_start)) {
				showDebug("%s", "enlarge filesystem ko\n");
				return false;
			} else {
				return true;
			}
		} else {
			return true;
		}
	}
}

bool QP_FSXfs::xfsresize(bool write, QP_PartInfo * partinfo, PedSector)
{
	QString cmdline;

	bool error = false;

	if (!write)
		return true;

	/*---init of the error message---*/
	_message = QString::null;

	if (!qpMount(partinfo->partname()))
		return false;

	/*---do the resize!---*/
	cmdline = lstExternalTools->getPath("xfs_growfs") + " " + TMP_MOUNTPOINT;

	if (!fs_open(cmdline)) {
		_message = QString(NOTFOUND);
		return false;
	}

	error = true;
	char *cline;
	while ((cline = fs_getline())) {
		QString line = QString(cline);

		QRegExp rx;
		rx = QRegExp("^realtime =.*");
		if (rx.indexIn(line) == 0) {
			error = false;
		}
	}
	fs_close();

	if (error) {
		_message = QString(tr("Error during xfs_grow."));
		return false;
	}

	if (!qpUMount(partinfo->partname()))
		return false;

	return true;
}

QString QP_FSXfs::fsname()
{
	return QString("xfs");
}

QString QP_FSXfs::_get_label(PedPartition * part)
{
	char bootsect[2048];
	char label[13];

	if (!QP_FSWrap::read_sector(part, 0, 4, bootsect))
		return QString::null;

	memset(label, 0, sizeof(label));
	strncpy(label, bootsect + 108, 12);

	//printf("returned buffer ntfs: [%s]\n", label);

	return QString(label);
}

/*---FAT WRAPPER---------------------------------------------------------------*/
QP_FSFat::QP_FSFat(QString bitflag):QP_FSWrap(),_bitflag(bitflag) {
	/*---check if the wrapper is installed---*/
	QString cmdline = "which " + lstExternalTools->getPath("mkdosfs");
	fs_open(cmdline);

	char *cline;
	while ((cline = fs_getline()))
		wrap_create = true;
	fs_close();
}

bool QP_FSFat::mkpartfs(QString dev, QString label)
{
	QString cmdline;
	_message = QString::null;
	if (!label.isEmpty())
		cmdline += " -n " + label;
	cmdline += _bitflag;
	cmdline = lstExternalTools->getPath("mkdosfs") + cmdline + " " + dev;
	if (!fs_open(cmdline)) {
		_message = QString(NOTFOUND);
		return false;
	}
	char *cline;
	while ((cline = fs_getline()))
		QString line=QString(cline);
	return (fs_close() == 0);
}

QString QP_FSFat::_get_label(PedPartition * part)
{
#ifdef PED_SECTOR_SIZE // PED_SECTOR_SIZE is gone in parted 1.7.x
	char *buffer=new char[PED_SECTOR_SIZE];
#else
	char *buffer=new char[part->disk->dev->sector_size];
#endif
	char label[12];

	if (!QP_FSWrap::read_sector(part, 0, 1, buffer)) {
		delete[] buffer;
		return QString::null;
	}

	memset(label, 0, sizeof(label));
	memcpy(label, buffer + 0x47, 11);
	//printf("returned fat buffer: %s\n", label);
	delete[] buffer;
	return QString(label);
}

/*---REISERFS WRAPPER------------------------------------------------------------*/
QString QP_FSReiserFS::_get_label(PedPartition *)
{
	//return QString::null;
	return tr("No label");
}


QString QP_FSNtfs::_get_label(PedPartition * part)
{
	uint8_t bootsect[16384];
	char label[32];
	memset(label, 0, sizeof(label));
	uint8_t cFileRecord[8192 + 1];	// 1024 should be enough (dwFileRecordSize)

	uint64_t qwOffset;
	uint16_t nOffsetSequenceAttribute;
	uint8_t *cData;
	uint32_t dwAttribType;
	uint32_t dwAttribLen;
	bool bAttribResident;
	uint8_t *cDataResident;
	uint16_t nAttrSize;
	uint32_t dwFileRecordSize;
	uint64_t i;

	if (!QP_FSWrap::read_sector(part, 0, 32, (char *) bootsect))
		return QString::null;

	// 1. check partition has an ntfs file system
	if (memcmp(bootsect + 3, "NTFS", 4) != 0) {
		//printf ("NTFS-001: not an NTFS partition\n");
		return QString::null;
	}

	uint16_t nBytesPerSector = NTFS_GETU16(bootsect + 0xB);
	uint8_t cSectorsPerCluster = NTFS_GETU8(bootsect + 0xD);
	uint64_t qwTotalSectorsCount = NTFS_GETU64(bootsect + 0x28);
	uint64_t qwLCNOfMftDataAttrib = NTFS_GETU64(bootsect + 0x30);
	int8_t cClustersPerMftRecord = NTFS_GETS8(bootsect + 0x40);

	// check informations validity
	if (nBytesPerSector % 512 != 0) {
		//printf ("NTFS-002: invalid nBytesPerSector value\n");
		return QString::null;
	}
	// Calculate clusters and misc informations
	uint32_t dwClusterSize = (uint32_t) nBytesPerSector * cSectorsPerCluster;


	if (cClustersPerMftRecord > 0)
		dwFileRecordSize = cClustersPerMftRecord * dwClusterSize;
	else
		dwFileRecordSize = 1 << (-cClustersPerMftRecord);

	// 1. read $Volume record
	qwOffset =
	    ((uint64_t) qwLCNOfMftDataAttrib * dwClusterSize) +
	    ((uint64_t) (3 * dwFileRecordSize));

	uint32_t dwOffsetToRead = (uint32_t) (qwOffset / ((uint64_t) 512));
	uint32_t dwSectorCountToRead = (uint32_t) dwFileRecordSize / 512;

	if (!QP_FSWrap::
	    read_sector(part, dwOffsetToRead, dwSectorCountToRead,
			(char *) cFileRecord)) {
		//printf("readVolumeLabel(): failed in readData()\n");
		return QString::null;
	}
	// -------- decode the MFT File record
	nOffsetSequenceAttribute = NTFS_GETU16(cFileRecord + 0x14);
	cData = cFileRecord + nOffsetSequenceAttribute;

	do {
		// szData points to the beginning of an attribute
		dwAttribType = NTFS_GETU32(cData);
		dwAttribLen = NTFS_GETU32(cData + 4);
		bAttribResident = (NTFS_GETU8(cData + 8) == 0);

		if (dwAttribType == 0x60)	// "volume_name"
		{
			if (bAttribResident == false) {
				//printf("readVolumeLabel(): failed: $volume_name attribute is not resident\n");
				return QString::null;
			}

			nAttrSize = NTFS_GETU16(cData + 0x10);
			cDataResident = cData + NTFS_GETU16(cData + 0x14);

			for (i = 0; i < (uint32_t) (nAttrSize / 2); i++)
				label[i] = cDataResident[2 * i];
		}
		/*if(dwAttribType == 0x70) // "volume_information"
		   {      
		   int nNtfsVersion = 0;
		   if (bAttribResident == true)
		   {      
		   nAttrSize = NTFS_GETU16(cData+0x10);
		   cDataResident = cData+NTFS_GETU16(cData+0x14);
		   nNtfsVersion = (((uint8_t) cDataResident[8]) << 8) | ((uint8_t) cDataResident[9]);
		   printf("readVolumeLabel(): version is %d.%d\n", cDataResident[8], cDataResident[9]);
		   }
		   } */

		cData += dwAttribLen;
	} while (dwAttribType != (uint32_t) - 1);	// attribute list ends with type -1

	return QString(label);
}
