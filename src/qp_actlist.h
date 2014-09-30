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

/* About QP_ActList class
 *
 * This class that keep a list of action that must be committed/undo
 */


#ifndef QP_ACTLIST_H
#define QP_ACTLIST_H

#include <QList>
#include <QString>
#include <QObject>
#include "qp_libparted.h"
#include "qp_fswrap.h"

/* move,   -> num, start, end
 * resize, -> num, start, end
 * rm,	 -> num
 * create  -> start, end, tipo_estesa_logica, fsspec, label
 * */
class QP_ActListItem {
public:
	/*---type (move+resize), num, start, end, geometry, parttype---*/
	QP_ActListItem(QTParted::actType, int, PedSector, PedSector, PedGeometry, PedPartitionType);

	/*---type, num (rm)---*/
	QP_ActListItem(QTParted::actType, int);

	/*---type (format), num, fsspec, label, geometry, parttype---*/
	QP_ActListItem(QTParted::actType, int, QP_FileSystemSpec *, QString, PedGeometry, PedPartitionType);

	/*---type, num, active---*/
	QP_ActListItem(QTParted::actType, int, bool);

	/*---type, logical/extended, start, end, typoFS, label, geometry, parttype---*/
	QP_ActListItem(QTParted::actType,
				   QTParted::partType,
				   PedSector, PedSector,
				   QP_FileSystemSpec *,
				   QString,
				   PedGeometry,
				   PedPartitionType);

//private:
	QTParted::actType _action;
	PedSector _start;
	PedSector _end;
	int _num;
	QTParted::partType _type;
	QP_FileSystemSpec *_fsspec;
	QString _label;
	PedGeometry _geom;
	PedPartitionType _part_type;
	bool _status; //used for boot and hidden flags
};


class QP_ActionList : public QObject {
	friend class QP_LibParted;
	friend class QP_PartInfo;
	Q_OBJECT
public:
	QP_ActionList(QP_LibParted *);
	~QP_ActionList();
	void update_listpartitions();
	void scan_partitions(); //scan for every partition
	bool get_partfilesystem_info(PedPartition *, QP_PartInfo *);
	bool get_partfilesystem_label(PedPartition *part, QP_PartInfo *partinfo);
	void ins_resize(int, PedSector, PedSector, PedGeometry, PedPartitionType);
	void ins_move(int, PedSector, PedSector, PedGeometry, PedPartitionType);
	void ins_rm(int);
	void ins_mkfs(QP_FileSystemSpec *, int, QString, PedGeometry, PedPartitionType);
	void ins_mkpart(QTParted::partType, PedSector, PedSector, QP_FileSystemSpec *, QString, PedGeometry, PedPartitionType);
	void ins_active(int, bool);
	void ins_hidden(int, bool);
	void get_partinfo(QP_PartInfo *, PedPartition *);
	bool canUndo();  //Does the user can undo/commit?
	void undo();	 //undo last operation
	void commit();   //commit all operations
	PedDisk *disk(); //return the actual state of the disk
	QP_PartInfo *partActive(); //return the partinfo that is bootable
	QList<QP_PartInfo*> partlist;
	QList<QP_PartInfo*> logilist;

private:
	void partition_get_flags(QP_PartInfo *, PedPartition *); //will get the active flag
	void ins_newdisk();
	QList<QP_ActListItem*> actlist;
	QList<PedDisk*> listdisk;
	PedDisk *_disk;
	QP_LibParted *_libparted;
	QP_PartInfo *_partActive; //a pointer to the partinfo that is current active (ie bootable)
	QList<QP_PartInfo*> orig_partlist;
	QList<QP_PartInfo*> orig_logilist;

signals:
	/*---emitted when the state of the disk was changed---*/
	void sigDiskChanged();
	void sigOperations(QString, QString, int, int);
};
#endif
