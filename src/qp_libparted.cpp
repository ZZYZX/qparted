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

#include <sys/mount.h>
#include <qmessagebox.h>
#include <stdlib.h>
#include <qapplication.h>
#include "qp_libparted.h"
#include "qp_filesystem.h"
#include "qp_fswrap.h"
#include "qp_actlist.h"
#include "qp_common.h"
#include "qp_debug.h"

#define TMP_MOUNTPOINT "/tmp/mntqp"
#define MIN_FREESPACE		(1000 * 2)	/* 1000k */
#define PED_ASSERT(cond, action)		while (0) {}

#define SUCCESS tr("Operation successfully completed.")
#define NOTSUCCESS tr("An error has occurs during the operation.")
#define ERROR_PED_DISK_NEW tr("An error has happened during the ped_disk_new call.")
#define ERROR_PED_DISK_GET_PARTITION tr("An error has happened during the ped_disk_get_partition call.")
#define ERROR_PED_PARTITION_NEW tr("An error has happened during ped_partition_new call.")
#define ERROR_PED_DISK_ADD_PARTITION tr("An error has happened during ped_disk_add_partition call.")
#define ERROR_PED_PARTITION_SET_SYSTEM tr("An error has happened during ped_partition_set_system call.")
#define ERROR_PED_PARTITION_SET_FLAG tr("An error has happened during ped_partition_set_flag call.")
#define ERROR_PED_DISK_DELETE_PARTITION tr("An error has happened during ped_disk_delete_partition call.")

// define the GNU-Parted version required for QtParted
#define PARTED_REQUESTED_MAJOR 1
#define PARTED_REQUESTED_MINOR 6
#define PARTED_REQUESTED_MICRO 6

QString MB2String ( float mbyte )
{
	QString label;

	/*---use the size in MByte or in GByte---*/

	if ( mbyte > ( 1024 * 1024 ) )
	{
		label.sprintf ( "%3.2fTB", mbyte / ( 1024 * 1024 ) );
	}
	else if ( mbyte > 1024 )
	{
		label.sprintf ( "%3.2fGB", mbyte / 1024 );
	}
	else
	{
		label.sprintf ( "%3.2fMB", mbyte );
	}

	return label;
}

/*-begin of QP_PartInfo--------------------------------------------------------------------------*/
QP_PartInfo::QP_PartInfo()
{
	_mountPoint = QString::null;
}

QP_Device *QP_PartInfo::device()
{
	return _device;
}

void QP_PartInfo::setDevice ( QP_Device *device )
{
	showDebug ( "%s", "partinfo::setDevice\n" );
	_device = device;
}

float QP_PartInfo::mb_t_start()
{
	showDebug ( "%s", "partinfo::mb_t_start\n" );
	return t_start * 1.0 / MEGABYTE_SECTORS;
}

float QP_PartInfo::mb_t_end()
{
	showDebug ( "%s", "partinfo::mb_t_end\n" );
	return t_end * 1.0 / MEGABYTE_SECTORS;
}

float QP_PartInfo::mb_start()
{
	return start * 1.0 / MEGABYTE_SECTORS;
}

float QP_PartInfo::mb_end()
{
	return end * 1.0 / MEGABYTE_SECTORS;
}

float QP_PartInfo::mb_min_size()
{
	return min_size * 1.0 / MEGABYTE_SECTORS;
}

QString QP_PartInfo::partname()
{
	return QString ( "%1%2" ) .arg ( device()->shortname() ) .arg ( num );
}

QString QP_PartInfo::shortname()
{
	return QString ( "%1%2" ) .arg ( device()->shortname() ) .arg ( num );
}

QString QP_PartInfo::longname()
{
	QString devpath = QString ( device()->longname() );
	devpath.truncate ( devpath.length() - 4 ); //remove ending "disc" characters
	devpath.append ( "part" );
	QString devstr = QString ( "%1%2" )
					 .arg ( devpath )
					 .arg ( num );

	return devstr;
}

bool QP_PartInfo::isFree()
{
	return ( fsspec == _free );
}

bool QP_PartInfo::isUnknown()
{
	return ( fsspec == _unknown );
}

bool QP_PartInfo::isActive()
{
	return _active;
}

bool QP_PartInfo::isHidden()
{
	return _hidden;
}

bool QP_PartInfo::canBeActive()
{
	return _canBeActive;
}

bool QP_PartInfo::canBeHidden()
{
	/*---bad patch! it seems that libparted can only hide fat partitions :\---*/
	if ( _canBeHidden )
	{
		if ( ( fsspec->name().compare ( "fat32" ) == 0 )
				|| ( fsspec->name().compare ( "fat16" ) == 0 )
				|| ( fsspec->name().compare ( "ntfs" )  == 0 ) )
		{
			return _canBeHidden;
		}
	}

	return false;
}

bool QP_PartInfo::setActive ( bool active )
{
	return _libparted->partition_set_flag_active ( this, active );
}

bool QP_PartInfo::setHidden ( bool hidden )
{
	return _libparted->partition_set_flag_hidden ( this, hidden );
}

bool QP_PartInfo::resize ( PedSector new_start, PedSector new_end )
{
	showDebug ( "%s", "partinfo::resize\n" );

	if ( isVirtual() )
	{
		_libparted->_message = QString ( tr ( "This is a virtual partition. You cannot alter it: use undo instead." ) );
		return false;
	}

	/*---set the virtual flag---*/
	_virtual = true;

	/*---test if the partition is busy (ie mounted)---*/
//	if (partition_is_busy())
//		return false;

	/*---does it exist a wrapper for this filesystem?---*/
	if ( fsspec->fswrap() )
	{
		/*---does it exist a resize wrapper?				---
		 *---please note that this test should be a surplus!---*/
		if ( fsspec->fswrap()->wrap_resize )
		{
			showDebug ( "%s", "Resizing a filesystem using a wrapper\n" );
			bool rc = fsspec->fswrap()->resize ( _libparted, _libparted->_write, this, new_start, new_end );

			if ( !rc )
			{
				_libparted->_message = fsspec->fswrap()->message();
				_libparted->emitSigTimer ( 100, _libparted->message(), QString::null );
				return false;
			}
			else
			{
				_libparted->emitSigTimer ( 100, SUCCESS, QString::null );
				return true;
			}
		}
	}
	else
	{
		/*---the filesystem is supported by libparted, :)---*/
		showDebug ( "%s", "resize a filesystem using libparted\n" );
		bool rc = _libparted->resize ( this, new_start, new_end );

		if ( !rc )
		{
			showDebug ( "%s", "resize ko\n" );
			_libparted->emitSigTimer ( 100, _libparted->message(), QString::null );
		}
		else
		{
			showDebug ( "%s", "resize ok\n" );
			_libparted->emitSigTimer ( 100, SUCCESS, QString::null );
		}

		return rc;
	}

	_libparted->emitSigTimer ( 100, NOTSUCCESS, QString::null );

	return false;
}

bool QP_PartInfo::mkfs ( QP_FileSystemSpec *fsspec, QString label )
{
	showDebug ( "%s", "partinfo::mkfs\n" );

	if ( isVirtual() )
	{
		_libparted->_message = QString ( tr ( "Bug during mkfs! Cannot format virtual partitions!" ) );
		return false;
	}
	else
	{
		showDebug ( "%s", "qp_partinfo::mkfs\n" );
		return _libparted->mkfs ( this, fsspec, label );
	}
}

bool QP_PartInfo::move ( PedSector new_start, PedSector new_end )
{
	showDebug ( "%s", "partinfo::move\n" );

	if ( isVirtual() )
	{
		_libparted->_message = QString ( tr ( "This is a virtual partition. You cannot alter it: use undo instead." ) );
		return false;
	}

	/*---set the virtual flag---*/
	_virtual = true;

	showDebug ( "%s", "qp_partinfo::move\n" );

	bool rc = _libparted->move ( this, new_start, new_end );

	if ( !rc )
	{
		showDebug ( "%s", "qp_partinfo::move ko\n" );
		_libparted->emitSigTimer ( 100, _libparted->message(), QString::null );
	}
	else
	{
		showDebug ( "%s", "qp_partinfo::move ok\n" );
		_libparted->emitSigTimer ( 100, SUCCESS, QString::null );
	}

	return rc;
}

bool QP_PartInfo::partition_is_busy()
{
	showDebug ( "%s", "partinfo::partition_is_busy\n" );
	return _libparted->partition_is_busy ( this->num );
}

bool QP_PartInfo::set_system ( QP_FileSystemSpec *fsspec )
{
	showDebug ( "%s", "partinfo::set_system\n" );
	return _libparted->set_system ( this, fsspec );
}

bool QP_PartInfo::fswrap()
{
	showDebug ( "%s", "partinfo::fswrap\n" );
	return fsspec->fswrap();
}

QString QP_PartInfo::message()
{
	showDebug ( "%s", "partinfo::message\n" );
	return fsspec->fswrap()->message();
}

bool QP_PartInfo::isVirtual()
{
	showDebug ( "%s", "partinfo::isVirtual\n" );
	return _virtual;
}

bool QP_PartInfo::partMount()
{
	// security: if already mounted
	if ( !_mountPoint.isNull() ) umount ( _mountPoint.toLatin1() );

	char type[255];

	if ( ( ( fsspec->name().compare ( "fat32" ) == 0 )
			|| ( fsspec->name().compare ( "fat16" ) == 0 ) ) )
	{
		strcpy ( type, "vfat" );
	}
	else
	{
		strcpy ( type, fsspec->name().toLatin1() );
	}

	_mountPoint = TMP_MOUNTPOINT;

	int nRes = mount ( partname().toLatin1(), _mountPoint.toLatin1(), type, MS_NOATIME | MS_RDONLY, NULL );

	if ( nRes == 0 )
		return true;
	else
		return false;
}

bool QP_PartInfo::partUMount()
{
	return ( bool ) umount ( _mountPoint.toLatin1() );
}

QString QP_PartInfo::mountPoint()
{
	return _mountPoint;
}

QString QP_PartInfo::label()
{
	return _label;
}

/*-end of QP_PartInfo----------------------------------------------------------------------------*/





/*-begin of QP_LibParted-------------------------------------------------------------------------*/


/*structur used for the update of the "time left" on progress bar*/

typedef struct
{
	time_t	last_update;
	time_t	predicted_time_left;
	QP_LibParted *libparted;
} TimerContext;

static PedTimer *timer;
static TimerContext timer_context;

//function called by libparted. 'Cause libparted is write in C you cannot use a method!
void _timer_handler ( PedTimer *timer, void *context )
{
	TimerContext *tcontext = ( TimerContext * ) context;
	int draw_this_time;

	if ( tcontext->last_update != timer->now && timer->now > timer->start )
	{
		tcontext->predicted_time_left = timer->predicted_end - timer->now;
		tcontext->last_update = timer->now;
		draw_this_time = 1;
	}
	else
	{
		draw_this_time = 0;
	}

	if ( draw_this_time )
	{
		if ( timer->state_name )
		{
			int percent = ( int ) ( 100.0 * timer->frac );
			QString state = QString ( timer->state_name );
			QString timeleft;
			timeleft.sprintf ( "%.2d:%.2d",
							   ( int ) ( tcontext->predicted_time_left / 60 ),
							   ( int ) ( tcontext->predicted_time_left % 60 ) );
			tcontext->libparted->emitSigTimer ( percent, state, timeleft );
		}
	}
}


QP_LibParted::QP_LibParted()
{
	showDebug ( "%s", "libparted::libparted\n" );

	/*---get all filesystem supported by libparted---*/
	filesystem = new QP_FileSystem();
	get_filesystem ( filesystem );

	dev = NULL;
	actlist = NULL;

	_FastScan = true;
	_write = false;

	/*tacc*/
	showDebug ( "%s", "creating timer for progressbar\n" );
	timer_context.libparted = this;
	timer = ped_timer_new ( _timer_handler, &timer_context );

	if ( !timer ) printf ( "no timer!\n" );

	timer_context.last_update = 0;
}

QP_LibParted::~QP_LibParted()
{
	showDebug ( "%s", "libparted::~libparted\n" );

	/*TODO FIXME static void
	_done (PedDevice* dev)   in destroy!
	*/
	showDebug ( "%s", "destroy timer\n" );
	ped_timer_destroy ( timer );

	showDebug ( "%s", "destroy partlist\n" );
	logilist.clear();
	partlist.clear();

	showDebug ( "%s", "destroy filesystem wrapper\n" );
	delete filesystem;

//	if (disk) ped_disk_destroy(disk);

	if ( dev ) ped_device_destroy ( dev );
}


void QP_LibParted::setDevice ( QP_Device *device )
{
	showDebug ( "%s", "libparted::setDevice\n" );

	/*---set the device---*/
	_qpdevice = device;

	if ( actlist ) delete actlist;

	/*---the device has not partition table: return!---*/
	if ( !device->partitionTable() )
	{
		showDebug ( "%s", "the device has not partition table!\n" );
		dev = NULL;
		actlist = NULL;

		return ;
	}

	char const * const szdevice = strdup(device->shortname().toLatin1().data());

	dev = ped_device_get ( szdevice );

	free(const_cast<char*>(szdevice));

	if ( dev == NULL )
	{
		printf ( "error!\n" );
		exit ( 1 );
	}

	/*---make a new action list (used for commit/undo)---*/

	actlist = new QP_ActionList ( this );

	/*---and connect the right signal to able the use of commit/undo ;)---*/
	connect(actlist, SIGNAL(sigDiskChanged()),
			this, SIGNAL(sigDiskChanged()));
	connect(actlist, SIGNAL(sigOperations(QString, QString, int, int)),
			this, SIGNAL(sigOperations(QString, QString, int, int)));

	/*---update the commit/undo buttons---*/
	emit sigDiskChanged();
}


/*--- update the partlist/logilist the the latest PedDisk of actlist---*/
void QP_LibParted::scan_partitions()
{
	showDebug ( "%s", "libparted::scan_partitions\n" );

	/*---delete old partition list---*/
	logilist.clear();
	partlist.clear();

	_message = QString::null;

	/*---if doesn't exist a partition table...---*/

	if ( !_qpdevice->partitionTable() )
	{
		showDebug ( "%s", "no partition table: make a fake partlist\n" );
		/*---make a "fake" partition table---*/
		_mb_hdsize = 1;

		QP_PartInfo *partinfo = new QP_PartInfo();
		partinfo->start = 0;
		partinfo->end = 1 * MEGABYTE_SECTORS;
		partinfo->num = 1;
		partinfo->setDevice ( _qpdevice );
		partinfo->min_size = -1;
		partinfo->_label = "table";
		partinfo->_free = filesystem->free();
		partinfo->_unknown = filesystem->unknown();
		partinfo->_libparted = this;
		partinfo->_active = false;
		partinfo->_canBeActive = false;
		partinfo->_canBeHidden = false;
		partinfo->_virtual = true;
		partinfo->fsspec = filesystem->unknown();
		partinfo->type = QTParted::primary;
		partinfo->t_start = -1;
		partinfo->t_end = -1;
		partlist.append ( partinfo );

		//emitSigTimer(100, message(), QString::null);
		return ;
	}

	_mb_hdsize = ( dev->length * dev->sector_size ) / MEGABYTE;

	_message = QString ( tr ( "Scanning all disk partitions." ) );

	showDebug ( "%s", "call: update partlist of actlist\n" );

	actlist->update_listpartitions();

	/*---loop for all partition of the disk---*/
	foreach ( QP_PartInfo* p, actlist->partlist )
	{
		showDebug ( "%s", "update partlist of libparted using last partlist of actlist\n" );
		partlist.append ( p );

		if ( p->type == QTParted::extended )
		{
			/*---loop for every logical partitions---*/

			foreach ( QP_PartInfo* logi, actlist->logilist)
			{
				logilist.append ( logi );
			}
		}
	}
	//emitSigTimer(100, message(), QString::null);
}


/*--- update the partlist/logilist the the original PedDisk of actlist---*/
void QP_LibParted::scan_orig_partitions()
{
	showDebug ( "%s", "libparted::scan_orig_partitions\n" );

	/*---delete old partition list---*/
	logilist.clear();
	partlist.clear();

	_message = QString::null;

	_mb_hdsize = ( dev->length * dev->sector_size ) / MEGABYTE;

	_message = QString ( tr ( "Scanning all disk partitions." ) );

	/*---loop for all partition of the disk---*/

	foreach ( QP_PartInfo* p, actlist->orig_partlist )
	{
		showDebug ( "%s", "update partlist of libparted using \"orig\" partlist of actlist\n" );
		partlist.append ( p );

		if ( p->type == QTParted::extended )
		{
			/*---loop for every logical partitions---*/

			foreach ( QP_PartInfo* logi, actlist->orig_logilist )
			{
				logilist.append ( logi );
			}
		}
	}

	//emitSigTimer ( 100, message(), QString::null );
}

qtp_DriveInfo QP_LibParted::device_info ( QString strdev )
{
	showDebug ( "%s", "libparted::device_info\n" );
	qtp_DriveInfo driveinfo;

	PedDevice *dev = ped_device_get ( ( const char * ) strdev.toLatin1() );

	long double length = ( dev->length * ( dev->sector_size / 1024.0 ) ) / 1024.0;

	driveinfo.device = strdev;
	driveinfo.model = dev->model;

	char str[30];
	sprintf ( str, "%Lg", length );
	driveinfo.mb_capacity = str;

	sprintf ( str, "%lld", dev->length );
	driveinfo.length_sectors = str;

	return driveinfo;
}


bool QP_LibParted::checkForParted()
{
	showDebug ( "%s", "libparted::checkForParted\n" );
	int major, minor, micro;
	const char *version;

	if ( ! ( version = ped_get_version () ) ) //|| ( sscanf ( version, "%d.%d.%d", &major, &minor, &micro ) != 3 ) )
	{
	printf ( "Cannot get parted version\n" );
	QString label = QString ( QObject::tr ( "Cannot get parted version." ) );
	QMessageBox::information ( 0, "QtParted", label );
	showDebug ( "%s", "Cannot get parted version\n" );

		return false;
	}

	if(sscanf(version, "%d.%d.%d", &major, &minor, &micro) != 3) {
	micro = 0;
	if(sscanf(version, "%d.%d", &major, &minor) != 2) {
		minor = 0;
		if(sscanf(version, "%d", &major) != 1) {
			QMessageBox::information(0, "QtParted", tr("Can't identify parted version; claims to be %1").arg(version));
			return false;
		}
	}
	}

	if ( ( major > PARTED_REQUESTED_MAJOR ) ||
			( ( major == PARTED_REQUESTED_MAJOR ) && ( minor > PARTED_REQUESTED_MINOR ) ) ||
			( ( major == PARTED_REQUESTED_MAJOR ) && ( minor == PARTED_REQUESTED_MINOR )
			  && ( micro >= PARTED_REQUESTED_MICRO ) ) )
	{
		showDebug ( "Parted version is okay: %d.%d.%d\n", PARTED_REQUESTED_MAJOR,
					PARTED_REQUESTED_MINOR,
					PARTED_REQUESTED_MICRO );
		return true;
	}
	else
	{
		printf ( "Parted homepage: http://www.gnu.org/software/parted/\n" );

		QString label = QString ( QObject::tr ( "Parted is too old:\n"
												"- installed parted version: %1.%2.%3\n"
												"- required parted version: %4.%5.%6 (or better)\n\n"
												"Parted homepage: http://www.gnu.org/software/parted/" ) )
						.arg ( major ) .arg ( minor ) .arg ( micro )
						.arg ( PARTED_REQUESTED_MAJOR ) .arg ( PARTED_REQUESTED_MINOR ) .arg ( PARTED_REQUESTED_MICRO );
		showDebug ( "%s", label.toStdString().c_str() );
		QMessageBox::information ( NULL, "QtParted", label );
		return false;
	}
}

void QP_LibParted::get_filesystem ( QP_FileSystem *filesystem )
{
	showDebug ( "%s", "libparted::get_filesystem\n" );

#ifdef USE_PARTED2_FS_SUPPORT // Filesystem support was removed from parted 3.x
	/*---scan all filesystem supported by parted---*/
	PedFileSystemType* walk = NULL;

	for ( walk = ped_file_system_type_get_next ( NULL ); walk;
			walk = ped_file_system_type_get_next ( walk ) )
	{

		/*---if the filesystem is supported by parted add it to the list---*/
		filesystem->addFileSystem ( walk->name,
									walk->ops->create,
									walk->ops->resize,
									walk->ops->copy,   //Move and copy is the same thing for parted!
									walk->ops->copy );

	}
#else
	QStringList filesystems = QStringList() << "ext2" << "ext3" << "ext4" << "btrfs" << "jfs" << "xfs" << "reiserfs" << "swap" << "fat" << "fat16" << "fat32";
	foreach(QString fs, filesystems) {
		QP_FSWrap *fsw = QP_FSWrap::fswrap(fs);
	if(fsw) {
			filesystem->addFileSystem(fs,
									  fsw->wrap_create,
					  fsw->wrap_resize,
					  fsw->wrap_move,
					  fsw->wrap_copy);
		}
	}
#endif

	/*---loop into fswraplist, and add it to sigTimer ('cause wrapper handle by itself)---*/
	QP_FSWrap *p;

	for ( int idx = 0; idx < filesystem->fswraplist.size(); idx++ )
	{
		//for (p = (QP_FSWrap *)filesystem->fswraplist.first(); p; p = (QP_FSWrap *)filesystem->fswraplist.next()) {
		p = filesystem->fswraplist.at ( idx );
		connect ( p, SIGNAL ( sigTimer ( int, QString, QString ) ),
				  this, SIGNAL ( sigTimer ( int, QString, QString ) ) );
	}
}

bool QP_LibParted::partition_set_flag_active ( QP_PartInfo *partinfo, bool active )
{
	showDebug ( "%s", "libparted::partition_set_flag_active\n" );

	PedPartition *part;
	part = ped_disk_get_partition ( actlist->disk(), partinfo->num );

	if ( !part )
	{
		showDebug ( "%s", "libparted::partition_set_flag_active, get_partition ko\n" );
		_message = QString ( ERROR_PED_DISK_GET_PARTITION );
		goto error;
	}

	if ( !ped_partition_is_flag_available ( part, PED_PARTITION_BOOT ) )
	{
		showDebug ( "%s", "libparted::partition_set_flag_active, is_fag_available ko\n" );
		_message = QString ( tr ( "Cannot change the active status on this partition" ) );
		goto error;
		return false;
	}

	if ( !ped_partition_set_flag ( part, PED_PARTITION_BOOT, active ) )
	{
		showDebug ( "%s", "libparted::partition_set_flag_active, set_flag ko\n" );
		_message = QString ( ERROR_PED_PARTITION_SET_FLAG );
		goto error;
		return false;
	}

	if ( _write )
	{
		if ( disk_commit ( actlist->disk() ) == 0 )
		{
			showDebug ( "%s", "libparted::partition_set_flag_active, commit ko\n" );
			return false;
		}
	}
	else
	{
		showDebug ( "%s", "operation added to undo/commit list\n" );
		actlist->ins_active ( partinfo->num, active );
	}

	return true;

error:
	return false;
}

bool QP_LibParted::partition_set_flag_hidden ( QP_PartInfo *partinfo, bool hidden )
{
	showDebug ( "%s", "libparted::partition_set_flag_hidden\n" );

	PedPartition *part;
	part = ped_disk_get_partition ( actlist->disk(), partinfo->num );

	if ( !part )
	{
		showDebug ( "%s", "libparted::partition_set_flag_hidden, get_partition ko\n" );
		_message = QString ( ERROR_PED_DISK_GET_PARTITION );
		goto error;
	}

	if ( !ped_partition_is_flag_available ( part, PED_PARTITION_HIDDEN ) )
	{
		showDebug ( "%s", "libparted::partition_set_flag_hidden, is_fag_available ko\n" );
		_message = QString ( tr ( "Cannot change the hidden status on this partition" ) );
		goto error;
		return false;
	}

	if ( !ped_partition_set_flag ( part, PED_PARTITION_HIDDEN, hidden ) )
	{
		showDebug ( "%s", "libparted::partition_set_flag_hidden, set_flag ko\n" );
		_message = QString ( ERROR_PED_PARTITION_SET_FLAG );
		goto error;
		return false;
	}

	if ( _write )
	{
		if ( disk_commit ( actlist->disk() ) == 0 )
		{
			showDebug ( "%s", "libparted::partition_set_flag_hidden, commit ko\n" );
			return false;
		}
	}
	else
	{
		showDebug ( "%s", "operation added to undo/commit list\n" );
		actlist->ins_hidden ( partinfo->num, hidden );
	}

	return true;

error:
	return false;
}

bool QP_LibParted::partition_set_flag_active ( int num, bool active )
{
	showDebug ( "%s", "libparted::set_flag_active\n" );

	/*---scan to find the partinfo to resize---*/
	QP_PartInfo *partinfo = numToPartInfo ( num );
	showDebug ( "%s", "libparted::set_flag_active, numtopartinfo\n" );
	return partition_set_flag_active ( partinfo, active );
}

bool QP_LibParted::partition_set_flag_hidden ( int num, bool hidden )
{
	showDebug ( "%s", "libparted::set_flag_hidden\n" );

	/*---scan to find the partinfo to resize---*/
	QP_PartInfo *partinfo = numToPartInfo ( num );
	showDebug ( "%s", "libparted::set_flag_hidden, numtopartinfo\n" );
	return partition_set_flag_hidden ( partinfo, hidden );
}

QP_PartInfo * QP_LibParted::numToPartInfo ( int num )
{
	showDebug ( "%s", "libparted::numToPartInfo\n" );

	/*---scan to find the partinfo to resize---*/

	/*---loop for all partition of the disk---*/
	QP_PartInfo *partinfo = NULL;
	QP_PartInfo *p;

	for ( int idx = 0; idx < partlist.size(); idx++ )
	{
		//for (p = (QP_PartInfo*)partlist.first(); p; p = (QP_PartInfo*)partlist.next()) {
		p = partlist.at ( idx );
		showDebug ( "%s", "libparted::numToPartInfo, scan for every partition\n" );
		/*---does this partition match?---*/

		if ( p->num == num )
			partinfo = p;

		if ( p->type != QTParted::extended )
		{
		}
		else
		{
			QP_PartInfo *logi;
			/*---loop for every logical partitions---*/

			for ( int index = 0; index < logilist.size(); index++ )
			{
				//for (logi = (QP_PartInfo*)logilist.first(); logi; logi = (QP_PartInfo*)logilist.next()) {
				/*---does this partition match?---*/
				logi = logilist.at ( index );

				if ( logi->num == num )
					partinfo = logi;
			}
		}
	}

	return partinfo;
}

QP_PartInfo * QP_LibParted::partActive()
{
	showDebug ( "%s", "libparted::partActive\n" );

	return actlist->partActive();
}

PedSector QP_LibParted::get_left_bound ( PedSector sector, PedDisk *disk )
{
	showDebug ( "%s", "libparted::get_left_bound\n" );

	PedPartition *under_sector;

	under_sector = ped_disk_get_partition_by_sector ( disk, sector );
	PED_ASSERT ( under_sector != NULL, return sector );

	if ( under_sector->type & PED_PARTITION_FREESPACE )
		return under_sector->geom.start;
	else
		return sector;
}

PedSector QP_LibParted::get_right_bound ( PedSector sector, PedDisk *disk )
{
	showDebug ( "%s", "libparted::get_right_bound\n" );

	PedPartition *under_sector;

	under_sector = ped_disk_get_partition_by_sector ( disk, sector );
	PED_ASSERT ( under_sector != NULL, return sector );

	if ( under_sector->type & PED_PARTITION_FREESPACE )
		return under_sector->geom.end;
	else
		return sector;
}

/* if the gap between the new partition and the partition-to-be-aligned is
 * less than MIN_FREESPACE, then gobble up the gap!
 */
int QP_LibParted::grow_over_small_freespace ( PedGeometry* geom, PedDisk *disk )
{
	showDebug ( "%s", "libparted::grow_oever_small_freespace\n" );
	PedSector	start;
	PedSector	end;

	/* hack: give full control for small partitions */

	if ( geom->length < MIN_FREESPACE * 5 )
		return 1;

	start = get_left_bound ( geom->start, disk );

	PED_ASSERT ( start < geom->end, return 0 );

	if ( geom->start - start < MIN_FREESPACE )
		ped_geometry_set_start ( geom, start );

	end = get_right_bound ( geom->end, disk );

	PED_ASSERT ( end > geom->start, return 0 );

	if ( end - geom->end < MIN_FREESPACE )
		ped_geometry_set_end ( geom, end );

	return 1;
}

bool QP_LibParted::set_system ( QP_PartInfo *partinfo, QP_FileSystemSpec *fsspec )
{
	showDebug ( "%s", "libparted::set_system\n" );
	PedPartition *part;
	const PedFileSystemType *fs_type = NULL;

	if ( fsspec == NULL )
	{
		_message = QString ( tr ( "A bug was found in QTParted during set_system, please report it!" ) );
		return false;
	}

	part = ped_disk_get_partition ( actlist->disk(), partinfo->num );

	if ( !part )
	{
		_message = QString ( ERROR_PED_DISK_GET_PARTITION );
		goto error;
	}

	fs_type = ped_file_system_type_get ( fsspec->name().toLatin1() );

	if ( !fs_type )
	{
		_message = QString ( tr ( "A bug was found in QTParted during ped_file_system_type_get, please report it!" ) );
		goto error;
	}

	if ( !ped_partition_set_system ( part, fs_type ) )
	{
		_message = QString ( ERROR_PED_PARTITION_SET_SYSTEM );
		goto error;
	}

	if ( _write ) if ( !ped_disk_commit ( actlist->disk() ) ) goto error;

	return true;

error:
	return false;
}

bool QP_LibParted::mkfs ( int num, QP_FileSystemSpec *fsspec, QString label )
{
	showDebug ( "%s", "libparted::mkfs(num)\n" );
	/*---scan to find the partinfo to resize---*/
	QP_PartInfo *partinfo = numToPartInfo ( num );

	if ( partinfo )
	{
		showDebug ( "%s", "libparted::mkfs, partinfo found\n" );
		return partinfo->mkfs ( fsspec, label );
	}
	else
	{
		showDebug ( "%s", "libparted::mkfs, partinfo not found (ko)\n" );
		_message = QString ( tr ( "A bug was found in QTParted during \"mkfs\" scan, please report it!" ) );
		return false;
	}
}

int QP_LibParted::mkfs ( QP_PartInfo *partinfo, QP_FileSystemSpec *fsspec, QString label )
{
	showDebug ( "%s", "libparted::mkfs(partinfo)\n" );
	PedPartition *part;
	PedFileSystem *fs = NULL;
	PedFileSystemType *fs_type = NULL;
	_message = QString::null;

	part = ped_disk_get_partition ( actlist->disk(), partinfo->num );

	if ( !part )
	{
		showDebug ( "%s", "libparted::mkfs, get_partition ko\n" );
		_message = QString ( ERROR_PED_DISK_GET_PARTITION );
		goto error;
	}

	/*---get the file_system_id---*/
	fs_type = ped_file_system_type_get ( fsspec->name().toLatin1() );

	/*---if there are not wrapper for write---*/
	if ( !fsspec->fswrap()
			&& _write )
	{
		showDebug ( "%s", "libparted::mkfs, (NOT wrapper and want to commit)\n" );
		/*---and change the filesystem id ;)---*/

		if ( !ped_partition_set_system ( part, fs_type ) )
		{
			showDebug ( "%s", "libparted::mkfs, set_system ko\n" );
			_message = QString ( ERROR_PED_PARTITION_SET_SYSTEM );
			goto error;
		}

#ifdef USE_PARTED2_FS_SUPPORT // Filesystem support was removed from parted 3.x
		fs = ped_file_system_create ( &part->geom, fs_type, timer );

		if ( !fs )
		{
			showDebug ( "%s", "libparted::mkfs, file_system_create ko\n" );
			goto error;
		}

		ped_file_system_close ( fs );
#else
	// We rely on the wrapper being there --> next if block
#endif

		if ( disk_commit ( actlist->disk() ) == 0 )
		{
			showDebug ( "%s", "libparted::mkfs, commit ko\n" );
			goto error;
		}
	}

	/*---if it exist a wrapper just make the filesystem---*/
	if ( fsspec->fswrap()
			&& _write )
	{
		showDebug ( "%s", "libparted::mkfs, (wrapper and want to commit)\n" );
		/*---Destroys all file system signatures---*/

#if 0
		if ( !ped_file_system_clobber ( &part->geom ) )
		{
			showDebug ( "%s", "libparted::mkfs, ped_file_system_clobber ko\n" );
			goto error;
		}
#endif

		bool rc = fsspec->fswrap()->mkpartfs ( partinfo->partname(), label );

		if ( !rc )
		{
			showDebug ( "%s", "libparted::mkfs, file_system_create ko\n" );
			_message = fsspec->fswrap()->message();
		}

		return rc;

		/*---and change the filesystem id ;)---*/

		if ( !ped_partition_set_system ( part, fs_type ) )
		{
			showDebug ( "%s", "libparted::mkfs, set_system ko\n" );
			_message = QString ( ERROR_PED_PARTITION_SET_SYSTEM );
			goto error;
		}

		if ( disk_commit ( actlist->disk() ) == 0 )
		{
			showDebug ( "%s", "libparted::mkfs, commit ko\n" );
			goto error;
		}
	}

	if ( !_write )
	{
		showDebug ( "%s", "operation added to undo/commit list\n" );
		actlist->ins_mkfs ( fsspec, partinfo->num, label, part->geom, part->type );
	}

	return true;

error:
	return false;
}

int QP_LibParted::mkpart ( QTParted::partType type, PedSector start, PedSector end, QP_FSWrap *fswrap, QString label )
{
	showDebug ( "%s", "libparted::mkpart\n" );
	PedPartition *part;
	PedFileSystemType *fs_type = NULL;
	PedConstraint *constraint;
	QString devstr;
	PedGeometry part_geom;
	int num;

	_message = QString::null;

	/*---it is not possible that part_type is different than primary and logical... 'cause
	 *---the only case is when a partition is extended, but just above i tested if it was!---*/
	PedPartitionType part_type;

	if ( type == QTParted::primary )
		part_type = ( PedPartitionType ) 0;
	else if ( type == QTParted::logical )
		part_type = PED_PARTITION_LOGICAL;
	else if ( type == QTParted::extended )  	//add
	{
		part_type = PED_PARTITION_EXTENDED;   //add
	}
	else
	{
		_message = QString ( "A bug was found in QTParted during mkpartfs, please report it!" );
		return false;
	}

	constraint = ped_constraint_any ( dev );

	if ( !constraint )
	{
		showDebug ( "%s", "libparted::mkpart, ped_constraint_any ko\n" );
		goto error;
	}

	/*---if it exist a wrapper change the file_system_id---*/
	if ( fswrap ) fs_type = ped_file_system_type_get ( fswrap->fsname().toLatin1() );

	part = ped_partition_new ( actlist->disk(), part_type, fs_type, start, end );

	if ( !part )
	{
		showDebug ( "%s", "libparted::mkpart, ped_partition_new ko\n" );
		_message = QString ( ERROR_PED_PARTITION_NEW );
		goto error_destroy_constraint;
	}

	if ( !grow_over_small_freespace ( &part->geom, actlist->disk() ) )
	{
		showDebug ( "%s", "libparted::mkpart, grow_over ko\n" );
		goto error_destroy_part;
	}

	if ( !ped_disk_add_partition ( actlist->disk(), part, constraint ) )
	{
		showDebug ( "%s", "libparted::mkpart, add_partition ko\n" );
		_message = QString ( ERROR_PED_DISK_ADD_PARTITION );
		goto error_destroy_part;
	}

	/*---get the new device name made---*/
	num = part->num;

	part_geom = part->geom;

	/*---get the device name made.
	 *   with devfs it is /dev/ide/tricktrack/etc/etc/partNumber
	 *   if there is not devfs it is /dev/hd*number
	 */
	if ( flagDevfsEnabled )
	{
		QString devpath = QString ( dev->path );
		devpath.truncate ( strlen ( dev->path ) - 4 ); //remove ending "disc" characters
		devpath.append ( "part" );
		devstr = QString ( "%1%2" )
				 .arg ( devpath )
				 .arg ( part->num );
	}
	else
	{
		devstr = QString ( "%1%2" )
				 .arg ( dev->path )
				 .arg ( part->num );
	}

	ped_constraint_destroy ( constraint );

	/*TODO*/
	/*	if (!_solution_check_distant (start, end,
						  part->geom.start, part->geom.end,
			_("You requested to create a partition at %.3f-%.3fMb. The "
			  "closest Parted can manage is %.3f-%.3fMb.")))
			goto error_remove_part;*/

	if ( !ped_partition_set_system ( part, fs_type ) )
	{
		showDebug ( "%s", "libparted::mkpart, set_system ko\n" );
		_message = QString ( ERROR_PED_PARTITION_SET_SYSTEM );
		goto error;
	}

	if ( _write )
	{
		/*---Destroys all file system signatures---*/
#ifdef USE_PARTED2_FS_SUPPORT // Filesystem support was removed from parted 3.x
		if ( !ped_file_system_clobber ( &part->geom ) )
		{
			showDebug ( "%s", "libparted::mkpart, ped_file_system_clobber ko\n" );
			goto error;
		}
#endif

		if ( disk_commit ( actlist->disk() ) == 0 )
		{
			showDebug ( "%s", "libparted::mkpart, commit ko\n" );
			goto error;
		}
	}

	/*---if it exist a wrapper just make the filesystem---*/
	if ( fswrap
			&& _write )
	{
		showDebug ( "%s", "libparted::mkpart, (wrap + commit)\n" );
		bool rc = fswrap->mkpartfs ( devstr, label );

		if ( !rc )
		{
			showDebug ( "%s", "libparted::mkpart, mkpartfs ko\n" );
			_message = fswrap->message();
		}

		return rc;
	}

	if ( !_write )
	{
		showDebug ( "%s", "libparted::mkpart, do in virtual actlist\n" );
		QP_FileSystemSpec *fsspec = NULL;

		if ( fswrap ) fsspec = filesystem->nameToFSSpec ( fswrap->fsname() );
		else fsspec = filesystem->nameToFSSpec ( "extended" );

		showDebug ( "%s", "operation added to undo/commit list\n" );

		actlist->ins_mkpart ( type, start, end, fsspec, label, part_geom, part_type );
	}

	return true;

	/*error_remove_part:*/
	ped_disk_remove_partition ( actlist->disk(), part );

error_destroy_part:
	ped_partition_destroy ( part );

error_destroy_constraint:
	ped_constraint_destroy ( constraint );

error:
	return false;
}

int QP_LibParted::mkpartfs ( QTParted::partType type,
							 QP_FileSystemSpec *fsspec,
							 PedSector start,
							 PedSector end,
							 QString label )
{
	showDebug ( "%s", "libparted::mkpartfs\n" );
	_message = QString::null;

	/*---want to make an extended partition? Than call mkpart!---*/

	if ( type == QTParted::extended )
	{
		bool rc = mkpart ( type, start, end, NULL, QString::null );

		if ( !rc ) emitSigTimer ( 100, message(), QString::null );
		else	 emitSigTimer ( 100, SUCCESS, QString::null );

		return rc;
	}

	/*---it is not possible that part_type is different than primary and logical... 'cause
	 *---the only case is when a partition is extended, but just above i tested if it was!---*/
	PedPartitionType part_type;

	if ( type == QTParted::primary )
		part_type = ( PedPartitionType ) 0;
	else if ( type == QTParted::logical )
		part_type = PED_PARTITION_LOGICAL;
	else
	{
		_message = QString ( "A bug was found in QTParted during mkpartfs, please report it!" );
		return false;
	}

	/*---if it exist a wrapper maybe libparted doesn't support this filesystem...---*/
	if ( fsspec->fswrap() )
	{
		showDebug ( "%s", "libparted::mkpartfs, has a wrapper\n" );
		bool rc = false;
		/*---the wrapper support the mkpartfs?---*/

		if ( fsspec->fswrap()->wrap_create )
		{
			rc = mkpart ( type, start, end, fsspec->fswrap(), label );

			if ( !rc )
			{
				showDebug ( "%s", "libparted::mkpartfs, mkpart ko\n" );
				emitSigTimer ( 100, message(), QString::null );
			}
			else	 emitSigTimer ( 100, SUCCESS, QString::null );

			return rc;
		}

		showDebug ( "%s", "libparted::mkpartfs, the wrapper doesn't support wrap_create\n" );
	}

	PedPartition *part;

	const PedFileSystemType *fs_type;
	PedConstraint *constraint;
	PedFileSystem *fs;

	fs_type = ped_file_system_type_get ( fsspec->name().toLatin1() );

	if ( !fs_type )
	{
		showDebug ( "%s", "libparted::mkpartfs, ped_file_system_type_get ko\n" );
		goto error;
	}

#ifdef USE_PARTED2_FS_SUPPORT // Filesystem support was removed from parted 3.x
	constraint = ped_file_system_get_create_constraint ( fs_type, dev );
#else
	constraint = ped_constraint_any ( dev );
#endif

	part = ped_partition_new ( actlist->disk(), part_type, fs_type, ( int ) start, ( int ) end );

	if ( !part )
	{
		showDebug ( "%s", "libparted::mkpartfs, ped_partition_new ko\n" );
		_message = QString ( ERROR_PED_PARTITION_NEW );
		goto error_destroy_constraint;
	}

	if ( !grow_over_small_freespace ( &part->geom, actlist->disk() ) )
	{
		showDebug ( "%s", "libparted::mkpartfs, grow_over_small ko\n" );
		goto error_destroy_part;
	}

	if ( !ped_disk_add_partition ( actlist->disk(), part, constraint ) )
	{
		showDebug ( "%s", "libparted::mkpartfs, add_partition ko\n" );
		_message = QString ( ERROR_PED_DISK_ADD_PARTITION );
		goto error_destroy_part;
	}

	/* TODO */
	/*	if (!_solution_check_distant (start, end,
						  part->geom.start, part->geom.end,
			_("You requested to create a partition at %.3f-%.3fMb. The "
			  "closest Parted can manage is %.3f-%.3fMb.")))
			goto error_remove_part;*/

	if ( _write )
	{
#ifdef USE_PARTED2_FS_SUPPORT // Filesystem support was removed from parted 3.x
		showDebug ( "%s", "libparted::mkpartfs, want to commit\n" );
		fs = ped_file_system_create ( &part->geom, fs_type, timer );

		if ( !fs )
		{
			showDebug ( "%s", "libparted::mkpartfs, ped_file_system_create ko\n" );
			goto error_remove_part;
		}

		ped_file_system_close ( fs );
#else
	showDebug ( "%s", "Tried to create filesystem without wrapper. This shouldn't happen.\n" );
#endif
	}
	else
	{
		showDebug ( "%s", "operation added to undo/commit list\n" );
		actlist->ins_mkpart ( type, start, end, fsspec, label, part->geom, part_type );
	}

	ped_constraint_destroy ( constraint );

	if ( !ped_partition_set_system ( part, fs_type ) )
	{
		showDebug ( "%s", "libparted::mkpartfs, set_system ko\n" );
		goto error;
	}

	if ( _write )
	{
		showDebug ( "%s", "libparted::mkpartfs, want to commit\n" );

		if ( disk_commit ( actlist->disk() ) == 0 )
		{
			showDebug ( "%s", "libparted::mkpartfs, commit ko\n" );
			goto error;
		}
	}

	emitSigTimer ( 100, SUCCESS, QString::null );

	return true;

error_remove_part:
	ped_disk_remove_partition ( actlist->disk(), part );

error_destroy_part:
	ped_partition_destroy ( part );

error_destroy_constraint:
	ped_constraint_destroy ( constraint );

error:
	emitSigTimer ( 100, message(), QString::null );
	return false;
}

bool QP_LibParted::rm ( int num )
{
	showDebug ( "%s", "libparted::rm\n" );

	/*---scan to find the partinfo to resize---*/
	QP_PartInfo *partinfo = numToPartInfo ( num );

	if ( partinfo->isVirtual() )
	{
		_message = QString ( tr ( "This is a virtual partition. You cannot alter it: use undo instead." ) );
		return false;
	}

	PedPartition *part;

	part = ped_disk_get_partition ( actlist->disk(), num );

	if ( !part )
	{
		showDebug ( "%s", "libparted::rm, get_partition\n" );
		_message = QString ( ERROR_PED_DISK_GET_PARTITION );
		goto error;
	}

	if ( ped_partition_is_busy ( part ) )
	{
		showDebug ( "%s", "libparted::rm, partition is mounted\n" );
		_message = QString ( tr ( "The partition is mounted." ) );
		goto error;
	}

	if ( !ped_disk_delete_partition ( actlist->disk(), part ) )
	{
		showDebug ( "%s", "libparted::rm, delete_partition ko\n" );
		_message = QString ( ERROR_PED_DISK_DELETE_PARTITION );
		goto error;
	}

	if ( _write )
	{
		showDebug ( "%s", "libparted::rm, want to commit\n" );

		if ( disk_commit ( actlist->disk() ) == 0 )
		{
			showDebug ( "%s", "libparted::rm, commit ko\n" );
			goto error;
		}
	}
	else
	{
		/*---insert remove in the undo_action list---*/
		showDebug ( "%s", "operation added to undo/commit list\n" );
		actlist->ins_rm ( num );
	}

	return true;

error:
	return false;
}

bool QP_LibParted::partition_is_busy ( int num )
{
	showDebug ( "%s", "libparted::partition_is_busy\n" );
	PedPartition *part;
	_message = QString::null;

	/*---get the partition structure---*/
	part = ped_disk_get_partition ( actlist->disk(), num );

	if ( !part )
	{
		showDebug ( "%s", "libparted::partition_is_busy, get_partition ko\n" );
		_message = QString ( ERROR_PED_DISK_GET_PARTITION );
		goto error;
	}

	/*---test if the partition is busy (ie mounted)---*/
	if ( ped_partition_is_busy ( part ) )
	{
		_message = QString ( tr ( "The partition is mounted." ) );
		goto error;
	}

	return false;

error:
	return true;
}

bool QP_LibParted::move ( int num, PedSector start, PedSector end )
{
	showDebug ( "%s", "libparted::move(num)\n" );

	/*---scan to find the partinfo to resize---*/
	QP_PartInfo *partinfo = numToPartInfo ( num );

	if ( partinfo )
	{
		return partinfo->move ( start, end );
	}
	else
	{
		showDebug ( "%s", "libparted::move(num), numtopartinfo ko\n" );
		_message = QString ( tr ( "A bug was found in QTParted during \"move\" scan, please report it!" ) );
		return false;
	}
}

bool QP_LibParted::move ( QP_PartInfo *partinfo, PedSector start, PedSector end )
{
	showDebug ( "%s", "libparted::move(partinfo)\n" );

	PedPartition *part;
	PedGeometry old_geom;
	PedGeometry new_geom;
#ifdef USE_PARTED2_FS_SUPPORT // Filesystem support was removed from parted 3.x
	PedFileSystem *fs;
	PedFileSystem *fs_copy;
#endif
	PedConstraint *constraint;

	/*---libparted doesn't implement move of extended partition---*/

	if ( partinfo->type == QTParted::extended )
	{
		_message = QString ( tr ( "Can't move extended partitions." ) );
		goto error;
	}

	/*---do the move in "readonly mode". The move dirty PedDisk, so i choosed---
	 *---to do it in a PedDisk clone... this is the reason of this _test_move---*/
	if ( !_test_move ( partinfo, start, end ) )
	{
		showDebug ( "%s", "libparted::move, _test_move ko\n" );
		goto error;
	}

	/*---get the partition info---*/
	part = ped_disk_get_partition ( actlist->disk(), partinfo->num );

	if ( !part )
	{
		showDebug ( "%s", "libparted::move, get_partition ko\n" );
		_message = QString ( ERROR_PED_DISK_GET_PARTITION );
		goto error;
	}

	/*---if a partition is used...---*/
	if ( !_partition_warn_busy ( part ) )
	{
		showDebug ( "%s", "libparted::move, warn_busy ko\n" );
		goto error;
	}

	old_geom = part->geom;

#ifdef USE_PARTED2_FS_SUPPORT // Filesystem support was removed from parted 3.x
	/* open fs, and get copy constraint */
	fs = ped_file_system_open ( &old_geom );

	if ( !fs )
	{
		showDebug ( "%s", "libparted::move, file_system open ko\n" );
		goto error;
	}


	constraint = ped_file_system_get_copy_constraint ( fs, dev );
#else
	constraint = ped_constraint_any ( dev );
#endif

	/* set / test on "disk" */

	if ( !ped_geometry_init ( &new_geom, dev, start, end - start + 1 ) )
	{
		showDebug ( "%s", "libparted::move, geometry_init ko\n" );
		goto error_destroy_constraint;
	}

	/*TODO
	if (!_grow_over_small_freespace (&new_geom, disk))
		goto error_destroy_constraint;*/

	if ( !ped_disk_set_partition_geom ( actlist->disk(), part, constraint, new_geom.start, new_geom.end ) )
	{
		showDebug ( "%s", "libparted::move, set_partition_geom ko\n" );
		goto error_destroy_constraint;
	}

	ped_constraint_destroy ( constraint );

	if ( ped_geometry_test_overlap ( &old_geom, &part->geom ) )
	{
		showDebug ( "%s", "libparted::move, test_overlap ko\n" );
		_message = QString ( tr ( "Can't move a partition onto itself. Try using resize, perhaps?" ) );
		goto error_close_fs;
	}

	/*TODO
	if (!_solution_check_distant (start, end, part->geom.start, part->geom.end,
		_("You requested to move the partition to %.3f-%.3fMb. The "
		  "closest Parted can manage is %.3f-%.3fMb.")))
		goto error_close_fs;
	*/


	/* do the move */
	if ( _write )
	{
#ifdef USE_PARTED2_FS_SUPPORT // Filesystem support was removed from parted 3.x
		showDebug ( "%s", "libparted::move, want to commit\n" );
		fs_copy = ped_file_system_copy ( fs, &part->geom, timer );

		if ( !fs_copy )
		{
			showDebug ( "%s", "libparted::move, file_system_copy ko\n" );
			goto error_close_fs;
		}

		ped_file_system_close ( fs_copy );
#else
	showDebug ( "%s", "Move wrapper not used, this shouldn't happen\n" );
#endif
	}

#ifdef USE_PARTED2_FS_SUPPORT // Filesystem support was removed from parted 3.x
	ped_file_system_close ( fs );
#endif

	if ( _write )
	{
		showDebug ( "%s", "libparted::move, want to commit\n" );

		if ( disk_commit ( actlist->disk() ) == 0 )
		{
			showDebug ( "%s", "libparted::move, commit ko\n" );
			goto error;
		}
	}
	else
	{
		showDebug ( "%s", "libparted::move, do in virtual actlist\n" );
		/*---it is not possible that part_type is different than primary and logical... 'cause
		 *---the only case is when a partition is extended, but just above i tested if it was!---*/
		PedPartitionType part_type = type2parttype ( partinfo->type );

		showDebug ( "%s", "operation added to undo/commit list\n" );
		actlist->ins_move ( partinfo->num, start, end, part->geom, part_type );
	}

	return true;

error_destroy_constraint:
	ped_constraint_destroy ( constraint );

error_close_fs:
#ifdef USE_PARTED2_FS_SUPPORT // Filesystem support was removed from parted 3.x
	ped_file_system_close ( fs );
#endif

error:
	return false;
}

bool QP_LibParted::_test_move ( QP_PartInfo *partinfo, PedSector start, PedSector end )
{
	showDebug ( "%s", "libparted::_test_move\n" );
	PedPartition *part;
	PedGeometry old_geom;
	PedGeometry new_geom;
#ifdef USE_PARTED2_FS_SUPPORT // Filesystem support was removed from parted 3.x
	PedFileSystem *fs;
#endif
	//PedFileSystem *fs_copy;
	PedConstraint *constraint;
	PedDisk *disk;

	/*---make a backup of the disk---*/
	disk = ped_disk_duplicate ( actlist->disk() );

	if ( !disk )
	{
		showDebug ( "%s", "libparted::_test_move, disk_duplicate ko\n" );
		goto error;
	}

	/*---get the partition info---*/
	part = ped_disk_get_partition ( disk, partinfo->num );

	if ( !part )
	{
		showDebug ( "%s", "libparted::_test_move, get_partition ko\n" );
		_message = QString ( ERROR_PED_DISK_GET_PARTITION );
		goto error;
	}

	/*---if a partition is used...---*/
	if ( !_partition_warn_busy ( part ) )
	{
		showDebug ( "%s", "libparted::_test_move, device is mounted ko\n" );
		goto error;
	}

	old_geom = part->geom;

#ifdef USE_PARTED2_FS_SUPPORT // Filesystem support was removed from parted 3.x
	/* open fs, and get copy constraint */
	fs = ped_file_system_open ( &old_geom );

	if ( !fs )
	{
		showDebug ( "%s", "libparted::_test_move, file_system_open ko\n" );
		goto error;
	}


	constraint = ped_file_system_get_copy_constraint ( fs, dev );
#else
	constraint = ped_constraint_any ( dev );
#endif

	/* set / test on "disk" */

	if ( !ped_geometry_init ( &new_geom, dev, start, end - start + 1 ) )
	{
		showDebug ( "%s", "libparted::_test_move, geometry_init ko\n" );
		goto error_destroy_constraint;
	}

	/*TODO
	if (!_grow_over_small_freespace (&new_geom, disk))
		goto error_destroy_constraint;*/

	if ( !ped_disk_set_partition_geom ( disk, part, constraint, new_geom.start, new_geom.end ) )
	{
		showDebug ( "%s", "libparted::_test_move, set_partition_geom ko\n" );
		goto error_destroy_constraint;
	}

	ped_constraint_destroy ( constraint );

	if ( ped_geometry_test_overlap ( &old_geom, &part->geom ) )
	{
		showDebug ( "%s", "libparted::_test_move, _test_overlap ko\n" );
		_message = QString ( tr ( "Can't move a partition onto itself. Try using resize, perhaps?" ) );
		goto error_close_fs;
	}

	ped_disk_destroy ( disk );

	return true;

error_destroy_constraint:
	ped_constraint_destroy ( constraint );

error_close_fs:
#ifdef USE_PARTED2_FS_SUPPORT // Filesystem support was removed from parted 3.x
	ped_file_system_close ( fs );
#endif

error:
	ped_disk_destroy ( disk );
	return false;
}

bool QP_LibParted::resize ( int num, PedSector start, PedSector end )
{
	showDebug ( "%s", "libparted::resize(num)\n" );
	/*---scan to find the partinfo to resize---*/
	QP_PartInfo *partinfo = numToPartInfo ( num );

	if ( partinfo )
	{
		return partinfo->resize ( start, end );
	}
	else
	{
		showDebug ( "%s", "libparted::resize ko\n" );
		_message = QString ( tr ( "A bug was found in QTParted during \"resize\" scan, please report it!" ) );
		return false;
	}
}

bool QP_LibParted::resize ( QP_PartInfo *partinfo, PedSector start, PedSector end )
{
	showDebug ( "%s", "libparted::resize(partinfo)\n" );

	_message = QString::null;

	PedPartition *part;
	PedFileSystem *fs;
	PedConstraint *constraint;
	PedGeometry new_geom;

	/*---get the partition info---*/
	part = ped_disk_get_partition ( actlist->disk(), partinfo->num );

	if ( !part )
	{
		showDebug ( "%s", "libparted::resize, get_partition ko\n" );
		_message = QString ( ERROR_PED_DISK_GET_PARTITION );
		goto error;
	}

	/*---if a partition is used...---*/
	if ( !_partition_warn_busy ( part ) )
	{
		showDebug ( "%s", "libparted::resize, warn_busy ko\n" );
		goto error;
	}

	if ( !ped_geometry_init ( &new_geom, dev, start, end - start + 1 ) )
	{
		showDebug ( "%s", "libparted::resize, geometry_init ko\n" );
		_message = QString ( tr ( "An error happen during ped_geometry_init call." ) );
		goto error;
	}

	/*
	 * TODO
	if (!_grow_over_small_freespace (&new_geom, disk))
		goto error; */

	if ( part->type == PED_PARTITION_EXTENDED )
	{
		showDebug ( "%s", "libparted::resize, type extended\n" );
		constraint = ped_constraint_any ( dev );

		if ( !ped_disk_set_partition_geom ( actlist->disk(), part, constraint, new_geom.start, new_geom.end ) )
		{
			showDebug ( "%s", "libparted::resize, set_partition_geom ko\n" );
			_message = QString ( tr ( "An error happen during ped_disk_set_partition_geom call." ) );
			goto error_destroy_constraint;
		}

		//TODO
		/*if (!_solution_check_distant (start, end,
						  part->geom.start, part->geom.end,
			_("You requested to resize the partition to "
			  "%.3f-%.3fMb. The closest Parted can manage is "
			  "%.3f-%.3fMb.")))
			goto error_destroy_constraint;*/
		ped_partition_set_system ( part, NULL );
	}
	else
	{
#ifdef USE_PARTED2_FS_SUPPORT // Filesystem support was removed from parted 3.x
		showDebug ( "%s", "libparted::resize, not extended\n" );
		/*---n.b. the filesystem will be resized only during commit!---*/
		fs = ped_file_system_open ( &part->geom );

		if ( !fs )
		{
			showDebug ( "%s", "libparted::resize, file_system_open ko\n" );
			_message = QString ( tr ( "An error happen during ped_file_system_open call." ) );
			goto error;
		}

		constraint = ped_file_system_get_resize_constraint ( fs );

		if ( !ped_disk_set_partition_geom ( actlist->disk(), part, constraint,
											new_geom.start, new_geom.end ) )
		{
			showDebug ( "%s", "libparted::resize, set_partition_geom ko\n" );
			_message = QString ( tr ( "An error happen during ped_disk_set_partition_geom call." ) );
			goto error_close_fs;
		}

		//TODO
		/*if (!_solution_check_distant (start, end,
						  part->geom.start, part->geom.end,
			_("You requested to resize the partition to "
			  "%.3f-%.3fMb. The closest Parted can manage is "
			  "%.3f-%.3fMb.")))
			goto error_close_fs;*/

		if ( _write )
		{
			showDebug ( "%s", "libparted::resize, want to commit\n" );

			if ( !ped_file_system_resize ( fs, &part->geom, timer ) )
			{
				showDebug ( "%s", "libparted::resize, file_system_resize ko\n" );
				_message = QString ( tr ( "An error happen during ped_file_system_resize call." ) );
				goto error_close_fs;
			}
		}

		/* may have changed... eg fat16 -> fat32 */
		ped_partition_set_system ( part, fs->type );

		/*---close the filesystem---*/
		ped_file_system_close ( fs );
#else
	showDebug ( "%s", "Tried to resize without wrapper, this shouldn't happen\n" );
#endif
	}

	if ( _write )
	{
		showDebug ( "%s", "libparted::resize, want to commit\n" );

		if ( disk_commit ( actlist->disk() ) == 0 )
		{
			showDebug ( "%s", "libparted::resize, commit ko\n" );
			goto error;
		}
	}
	else
	{
		/*---it is not possible that part_type is different than primary and logical... 'cause
		 *---the only case is when a partition is extended, but just above i tested if it was!---*/
		PedPartitionType part_type = type2parttype ( partinfo->type );

		showDebug ( "%s", "operation added to undo/commit list\n" );
		actlist->ins_resize ( partinfo->num, start, end, part->geom, part_type );
	}

	ped_constraint_destroy ( constraint );

	return true;

error_close_fs:
	ped_file_system_close ( fs );

error_destroy_constraint:
	ped_constraint_destroy ( constraint );

error:
	return false;
}

PedGeometry QP_LibParted::get_geometry ( QP_PartInfo *partinfo )
{
	showDebug ( "%s", "libparted::get_geometry\n" );

	/*---get the partition info---*/
	PedPartition *part = ped_disk_get_partition ( actlist->disk(), partinfo->num );

	if ( !part ) showDebug ( "%s", "libparted::get_geometry ko\n" );

	//FIXME: test if part != NULL

	return part->geom;
}

PedPartitionType QP_LibParted::type2parttype ( QTParted::partType type )
{
	showDebug ( "%s", "libparted::type2parttype\n" );
	PedPartitionType part_type;

	if ( type == QTParted::primary )
		part_type = ( PedPartitionType ) 0;
	else if ( type == QTParted::logical )
		part_type = PED_PARTITION_LOGICAL;
	else// if (type == QTParted::extended)
		part_type = PED_PARTITION_EXTENDED;

	return part_type;
}

bool QP_LibParted::set_geometry ( QP_PartInfo *partinfo, PedSector start, PedSector end )
{
	showDebug ( "%s", "libparted::set_geometry\n" );
	_message = QString::null;

	PedPartition *part;
	PedConstraint *constraint;
	PedGeometry new_geom;

	/*---get the partition info---*/
	part = ped_disk_get_partition ( actlist->disk(), partinfo->num );

	if ( !part )
	{
		showDebug ( "%s", "libparted::set_geometry, pet_partition ko\n" );
		_message = QString ( ERROR_PED_DISK_GET_PARTITION );
		goto error;
	}

	if ( !_partition_warn_busy ( part ) )
	{
		showDebug ( "%s", "libparted::set_geometry, warn_busy ko\n" );
		goto error;
	}

	if ( !ped_geometry_init ( &new_geom, dev, start, end - start + 1 ) )
	{
		showDebug ( "%s", "libparted::set_geometry, geometry_init ko\n" );
		_message = QString ( tr ( "An error happen during ped_geometry_init call." ) );
		goto error;
	}

	/*
	 * TODO
	if (!_grow_over_small_freespace (&new_geom, disk))
		goto error; */

	/*---don't align the start/end position---*/
	constraint = ped_constraint_exact ( &new_geom );

	if ( !ped_disk_set_partition_geom ( actlist->disk(), part, constraint, new_geom.start, new_geom.end ) )
	{
		showDebug ( "%s", "libparted::set_geometry, set_partition_geom ko\n" );
		_message = QString ( tr ( "An error happen during ped_disk_set_partition_geom call." ) );
		goto error_destroy_constraint;
	}

	//TODO
	/*if (!_solution_check_distant (start, end,
					  part->geom.start, part->geom.end,
		_("You requested to resize the partition to "
		  "%.3f-%.3fMb. The closest Parted can manage is "
		  "%.3f-%.3fMb.")))
		goto error_destroy_constraint;*/

	if ( _write )
	{
		showDebug ( "%s", "libparted::set_geometry, want to commit\n" );

		if ( disk_commit ( actlist->disk() ) == 0 )
		{
			showDebug ( "%s", "libparted::set_geometry, commit ko\n" );
			goto error;
		}
	}

	ped_constraint_destroy ( constraint );

	return true;

error_destroy_constraint:
	ped_constraint_destroy ( constraint );

error:
	return false;
}

QString QP_LibParted::message()
{
	showDebug ( "%s", "libparted::message\n" );
	return _message;
}

void QP_LibParted::emitSigTimer ( int percent, QString state, QString timer )
{
	showDebug ( "%s", "libparted::emitSigTimer\n" );
	emit sigTimer ( percent, state, timer );
}

void QP_LibParted::setWrite ( bool write )
{
	showDebug ( "%s", "libparted::setWrite\n" );
	_write = write;
}

bool QP_LibParted::write()
{
	showDebug ( "%s", "libparted::write\n" );
	return _write;
}

bool QP_LibParted::canUndo()
{
	showDebug ( "%s", "libparted::canUndo\n" );
	return actlist->canUndo();
}

void QP_LibParted::undo()
{
	showDebug ( "%s", "libparted::undo\n" );
	actlist->undo();
}

void QP_LibParted::commit()
{
	showDebug ( "%s", "libparted::commit\n" );
	actlist->commit();
	_qpdevice->commit();
}

float QP_LibParted::mb_hdsize()
{
	showDebug ( "%s", "libparted::mb_hdsize\n" );
	return _mb_hdsize;
}

void QP_LibParted::setFastScan ( bool FastScan )
{
	showDebug ( "%s", "libparted::setFastScan\n" );
	_FastScan = FastScan;
}

/*---code bring from parted.c---*/
/*---return if a partition is used!---*/
bool QP_LibParted::_partition_warn_busy ( PedPartition* part )
{
	showDebug ( "%s", "libparted::_partition_warn_busy\n" );
	char *path = ped_partition_get_path ( part );

	if ( ped_partition_is_busy ( part ) )
	{
		showDebug ( "%s", "libparted::_partition_warn_busy, the partition is busy\n" );
		QString label = QString ( tr ( "Partition %s is being used." ) );

		if ( ped_exception_throw ( PED_EXCEPTION_ERROR,
								   ( PedExceptionOption ) PED_EXCEPTION_IGNORE_CANCEL,
								   label.toLatin1(),
								   path ) != PED_EXCEPTION_IGNORE ) goto error_free_path;
	}

	free ( path );

	return true;

error_free_path:
	free ( path );

//error:
	return false;
}

int QP_LibParted::disk_commit ( PedDisk *disk )
{
	showDebug ( "%s", "libparted::disk_commit\n" );
	int rc = ped_disk_commit ( disk );

	if ( rc == 0 )
	{
		showDebug ( "%s", "libparted::disk_commit error (maybe busy?)\n" );
		_message = QString ( tr ( "Error committing device.\nThis can happen when a device is mounted in the disk.\n\
								  Try to unmount all partitions on this disk.\nPlease read the FAQ for this kind of error!" ) );
	}

	return rc;
}
/*-end of QP_LibParted---------------------------------------------------------------------------*/
