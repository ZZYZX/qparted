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

#include <stdio.h>

#include "qp_filesystem.h"
#include "qtparted.h"
#include "qp_fswrap.h"

#include "xpm/part_dos.xpm"
#include "xpm/part_windows.xpm"
#include "xpm/part_linux.xpm"
#include "xpm/part_free.xpm"


/*-----------------------------------------------------------------------------------*/
/*---the qpfslist is used for select a color for a filesystem					 ---*/
/*---very important: leave free and unknown to the last lines!					 ---*/
/*---																			 ---*/

class QP_FSType {
public:
	QString fstype;	  // type of filesystem
	QColor color;		// color used when draw chart
	void *pixmap;		// little icon
	PedSector minFsSize; // minimal file system size
	PedSector maxFsSize; // maximal file system size
};

/* About file systems sizes:
   - Values are given in Sector
   - 0 mean "unknown/unlimited"
*/

static const QP_FSType qpfslist[] = {
	{"fat16", Qt::green, &part_dos_xpm, 10*MEGABYTE_SECTORS, 2048*MEGABYTE_SECTORS}, // ok
	{"fat32", Qt::darkGreen, &part_windows_xpm, 512*MEGABYTE_SECTORS, 0},
	{"ntfs", Qt::red, &part_windows_xpm, 2*MEGABYTE_SECTORS, 0}, // TODO: check 2MB are enough
	{"linux-swap",  Qt::blue, &part_linux_xpm, 2*MEGABYTE_SECTORS, 0},
	{"linux-swap(v0)",  Qt::blue, &part_linux_xpm, 2*MEGABYTE_SECTORS, 0},
	{"linux-swap(v1)",  Qt::blue, &part_linux_xpm, 2*MEGABYTE_SECTORS, 0},
	{"ext2", Qt::magenta, &part_linux_xpm, 2*MEGABYTE_SECTORS, 0},
	{"ext3", Qt::darkMagenta, &part_linux_xpm, 2*MEGABYTE_SECTORS, 0},
	{"ext4", Qt::darkMagenta, &part_linux_xpm, 2*MEGABYTE_SECTORS, 0},
	{"btrfs", Qt::darkMagenta, &part_linux_xpm, 2*MEGABYTE_SECTORS, 0},
	{"reiserfs", QColor(0, 100, 255), &part_linux_xpm, 34*MEGABYTE_SECTORS, 0}, // max is "17,6 TeraBytes"
	{"jfs", Qt::darkYellow, &part_linux_xpm, 16*MEGABYTE_SECTORS, 0}, // ok
	{"xfs", QColor(0, 255, 100), &part_linux_xpm, 5*MEGABYTE_SECTORS, 0}, // ok
	{"free", Qt::gray, &part_free_xpm, 0, 0},
	{"unknown", Qt::white, &part_free_xpm, 0, 0}
};
#define MAXFS (sizeof(qpfslist)/sizeof(QP_FSType))
/*-----------------------------------------------------------------------------------*/





/*----------QP_FileSystemSpec--------------------------------------------------------*/
/*---																			 ---*/
QP_FileSystemSpec::QP_FileSystemSpec(QString name, bool create,
		bool resize, bool move, bool copy, bool min_size, QP_FSWrap *fswrap) {
	/*---setting the private property---*/
	_name = name;
	_create = create;
	_resize = resize;
	_move = move;
	_copy = copy;
	_min_size = min_size;
	_fswrap = fswrap;
	_minFsSize = 0;
	_maxFsSize = 0;

	/*---default color is unknow---*/
	_color = qpfslist[MAXFS-1].color;
	_pixmap = QPixmap((const char **)qpfslist[MAXFS-1].pixmap);

	/*---look for a specific color for that filesystem---*/
	for (int i=0; i<MAXFS; i++)
		if (name.compare(qpfslist[i].fstype) == 0) {
			_color = qpfslist[i].color;
			_pixmap = QPixmap((const char **)qpfslist[i].pixmap);
			_minFsSize = qpfslist[i].minFsSize;
			_maxFsSize = qpfslist[i].maxFsSize;
		}
}

QP_FileSystemSpec::~QP_FileSystemSpec() {
}

QString QP_FileSystemSpec::name() {
	return _name;
}

QColor QP_FileSystemSpec::color() {
	return _color;
}

QPixmap QP_FileSystemSpec::pixmap() {
	return _pixmap;
}

bool QP_FileSystemSpec::create() {
	return _create;
}

bool QP_FileSystemSpec::resize() {
	return _resize;
}

bool QP_FileSystemSpec::move() {
	return _move;
}

bool QP_FileSystemSpec::copy() {
	return _copy;
}

bool QP_FileSystemSpec::min_size() {
	return _min_size;
}

PedSector QP_FileSystemSpec::minFsSize() {
	return _minFsSize;
}


PedSector QP_FileSystemSpec::maxFsSize() {
	return _maxFsSize;
}

QP_FSWrap *QP_FileSystemSpec::fswrap() {
	return _fswrap;
}

/*-----------------------------------------------------------------------------------*/



	
/*----------QP_FileSystemSpec--------------------------------------------------------*/
/*---										 ---*/
QP_FileSystem::QP_FileSystem() {
	/*---make a "free" filesystem---*/
	_free = new QP_FileSystemSpec("free", false, false, false, false, false, NULL);
	filesystemlist.append(_free);
	
	/*---make an "unknown" filesystem---*/
	_unknown = new QP_FileSystemSpec("unknown", false, false, false, false, false, NULL);
	filesystemlist.append(_unknown);
}

QP_FileSystem::~QP_FileSystem() {
	fswraplist.clear();
	filesystemlist.clear();
}


void QP_FileSystem::addFileSystem(QString name, bool create,
		bool resize, bool move, bool copy) {

	QP_FSWrap *fswrap = QP_FSWrap::fswrap(name);

	QP_FileSystemSpec *filesystemspec;
	if (!fswrap) {
		filesystemspec = new QP_FileSystemSpec(name, 
				create,
				resize,
				move,
				copy,
				true,
				NULL);
	} else {
		filesystemspec = new QP_FileSystemSpec(name, 
				fswrap->wrap_create,
				fswrap->wrap_resize,
				fswrap->wrap_move,
				fswrap->wrap_copy,
				fswrap->wrap_min_size,
				fswrap);
		fswraplist.append(fswrap);
	}
	filesystemlist.append(filesystemspec);
}

QP_FileSystemSpec *QP_FileSystem::nameToFSSpec(QString name) {
	foreach(QP_FileSystemSpec* p, filesystemlist)
	{
		if (p->name().compare(name) == 0)
			return p;
	}
	// Parted has some aliases for swap:
	// linux-swap
	// linux-swap(v0)
	// linux-swap(v1)
	// ...
	if(name.contains("swap") && name!="swap")
		return nameToFSSpec("swap");
	// Some versions of parted also have aliases for fat
	if(name.contains("fat") && name!="fat")
		return nameToFSSpec("fat");
	return unknown();
}

QP_FileSystemSpec *QP_FileSystem::free() {
	return _free;
}

QP_FileSystemSpec *QP_FileSystem::unknown() {
	return _unknown;
}
/*-----------------------------------------------------------------------------------*/
