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

/* About QP_LibParted class:
 *
 * This is a the wrapper to the libparted. Just use this class instead of call
 * native method!
 */

#ifndef QT_LIBPARTED_H
#define QT_LIBPARTED_H

#include <QList>
#include <QWidget>
#include <parted/parted.h>
#include "qtparted.h"
#include "qp_devlist.h"

#ifndef PED_SECTOR_SIZE
#define PED_SECTOR_SIZE PED_SECTOR_SIZE_DEFAULT
#endif

class QP_PartWidget;
class QP_ListViewItem;
class QP_LibParted;
class QP_ActionList;
class QP_FileSystemSpec;
class QP_FileSystem;
class QP_FSWrap;

QString MB2String(float);

class QP_PartInfo : public QObject {
	friend class QP_LibParted;
	friend class QP_ActionList;
	Q_OBJECT
public:
	QP_PartInfo();
	QP_FileSystemSpec *fsspec;
	QTParted::partType type;
	int num;
	PedSector t_start;
	PedSector t_end;
	PedSector start;
	PedSector end;
	PedSector min_size;
	QP_PartWidget *partwidget;
	QP_ListViewItem *listviewitem;
	int  width;
	QP_Device *device();
	void setDevice(QP_Device *);
	float mb_t_start();
	float mb_t_end();
	float mb_start();
	float mb_end();
	float mb_min_size();
	QString partname();
	QString shortname();
	QString longname();
	bool isFree();								 /*---return true if the filesystem is free space  ---*/
	bool isUnknown();								/*---return true if the filesystem is unknown space---*/
	bool isActive();								/*---return true if the partition is active		---*/
	bool isHidden();								/*---return true if the partition is hidden		---*/
	bool canBeActive();							/*---return true if the partition can be actived  ---*/
	bool canBeHidden();							/*---return true if the partition can be hidden	---*/
	bool setActive(bool);						/*---change the active status							---*/
	bool setHidden(bool);						/*---change the hidden status							---*/
	bool resize(PedSector, PedSector);		/*---resize the partition (start, end sectors	 ---*/
	bool mkfs(QP_FileSystemSpec *, QString);/*---format the partition (filesystem, label)	 ---*/
	bool move(PedSector, PedSector);		 /*---move the partition (start, end sectors		---*/
	bool partition_is_busy();					/*---test if the partition is busy (ie mounted)	---*/
	bool set_system(QP_FileSystemSpec *);	/*---change the systemid of the partition			---*/
	bool fswrap();
	QString message();
	bool isVirtual();							 /*---return if it is a virtual partinfo			 ---*/
	bool partMount();							 /*---mount the partition								 ---*/
	bool partUMount();							/*---umount the partition								---*/
	QString mountPoint();						/*---return the mountpoint								---*/
	QString label();								/*---return the label of the partition				---*/

private:
	QString _label;
	QP_Device *_device;							/*---pointer to the device								---*/
	QP_FileSystemSpec *_free;					/*---internal pointer to the "free" flag			---*/
	QP_FileSystemSpec *_unknown;				/*---internal pointer to the "unknown" flag		 ---*/
	bool _active;									/*---flag active											---*/
	bool _hidden;									/*---flag hidden											---*/
	bool _canBeActive;							/*---flag can be active									---*/
	bool _canBeHidden;							/*---flag can be hidden									---*/
	QP_LibParted *_libparted;
	PedGeometry _geometry;						/*---geometry of the partition						 ---*/
	bool _virtual;								 /*---if this flag is on this partinfo=virtual!	---*/
	QString _mountPoint;						 /*---mountpoint of the partition						---*/
};

class qtp_DriveInfo {
public:
	QString device;
	QString model;
	QString mb_capacity;
	QString length_sectors;
};

class QP_LibParted : public QObject
{
	friend class QP_PartInfo;
	friend class QP_ActionList;
	friend class QP_FSNtfs;
	friend class QP_FSJfs;
	friend class QP_FSXfs;
	Q_OBJECT
public:
	QP_LibParted();
	~QP_LibParted();
	void setDevice(QP_Device *);
	void scan_partitions(); //update partlist/logilist with the actlist PedDisk
	void scan_orig_partitions(); //update partlist/logilist... with the "original" actlist PedDisk
	static qtp_DriveInfo device_info(QString);
	static bool checkForParted();									/*---return if it is installed the right version of libparted---*/
	void get_filesystem(QP_FileSystem *);
	QList<QP_PartInfo*> partlist;
	QList<QP_PartInfo*> logilist;
	bool partition_set_flag_active(QP_PartInfo *, bool);	 /*---set active flag---*/
	bool partition_set_flag_hidden(QP_PartInfo *, bool);	 /*---set hidden flag---*/
	bool partition_set_flag_active(int, bool);					/*---set active flag---*/
	bool partition_set_flag_hidden(int, bool);					/*---set hidden flag---*/
	QP_PartInfo *numToPartInfo(int);								/*---return partinfo associated to partition num---*/
	QP_PartInfo *partActive();										/*---return the active partition---*/
	int grow_over_small_freespace (PedGeometry* geom, PedDisk *disk);
	PedSector get_right_bound (PedSector sector, PedDisk *disk);
	PedSector get_left_bound (PedSector sector, PedDisk *disk);
	bool set_system(QP_PartInfo *, QP_FileSystemSpec *);
	bool mkfs(int, QP_FileSystemSpec *, QString);
	int mkfs(QP_PartInfo *, QP_FileSystemSpec *, QString);
	int mkpart(QTParted::partType type, PedSector start, PedSector end, QP_FSWrap *, QString);
	int mkpartfs(QTParted::partType, QP_FileSystemSpec *, PedSector, PedSector, QString);
	bool rm(int);
	bool partition_is_busy(int); /*---test if the partition is busy (ie mounted)---*/
	float mb_hdsize();
	bool has_extended;
	QP_FileSystem *filesystem; /*---a class with all feature of filesystems	---*/
	void setFastScan(bool);	/*---make the scan of filesystems fast!		 ---*/
	bool move(int, PedSector, PedSector);
	bool move(QP_PartInfo *, PedSector, PedSector);
	bool resize(int, PedSector, PedSector);
	bool resize(QP_PartInfo *, PedSector, PedSector);
	PedGeometry get_geometry(QP_PartInfo *);
	bool set_geometry(QP_PartInfo *, PedSector, PedSector);
	QString message();
	void emitSigTimer(int, QString, QString);
	void setWrite(bool);
	bool write();
	bool canUndo();
	void undo();
	void commit();

private:
	bool _test_move(QP_PartInfo *, PedSector, PedSector);
	PedPartitionType type2parttype(QTParted::partType);
	bool _partition_warn_busy(PedPartition *);
	int disk_commit(PedDisk *);
	PedDevice *dev;
	QP_Device *_qpdevice;
	bool _FastScan;
	float _mb_hdsize;
	QString _message;
	bool _write;
	QP_ActionList *actlist;

signals:
	/*---emitted when there is need to update a progress bar---*/
	void sigTimer(int, QString, QString);
	void sigOperations(QString, QString, int, int);
	void sigDiskChanged();
};

#endif
