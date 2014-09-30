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

/* About QP_FileSystem class:
 *
 * This is a widget that implement a list with all filesystems
 * supported by parted
 *
 * The class QP_FileSystemSpec contain all feature (spec) that a filesystem own
 */

#ifndef QP_FILESYSTEM_H
#define QP_FILESYSTEM_H

#include <QColor>
#include <QPixmap>
#include <QList>
#include "qp_libparted.h"

class QP_FSWrap;

/*----------QP_FileSystemSpec--------------------------------------------------------*/
/*---this class contain property of a specific filesystem                         ---*/
/*---                                                                             ---*/
/*---a filesystem can be created, resized and copied...                           ---*/
/*---...but under QTParted a filesystem also has a color associated!              ---*/
class QP_FileSystemSpec {
public:
	QP_FileSystemSpec(QString name, bool create,
			bool resize, bool move, bool copy, bool min_size, QP_FSWrap *);
	~QP_FileSystemSpec();
	QString name();		/*---return the name (ext2, ext3 /etc /etc) ---*/
	QColor color();		/*---return the color (used for chartlist) ---*/
	QPixmap pixmap();	/*---return the pixmap (used for chartlist) ---*/
	bool create();		/*---parted can create partition with this filesystem? ---*/
	bool resize();		/*---maybe parted can also resize this filesystem ;) ---*/
	bool move();		/*---maybe parted can also move this filesystem ;) ---*/
	bool copy();		/*---...and can it copy it? :) ---*/
	bool min_size();	/*---...and get how is used! ---*/
	PedSector minFsSize(); /*---return the minimal filesystem size ---*/
	PedSector maxFsSize(); /*---return the maximal filesystem size ---*/
	QP_FSWrap *fswrap();   /*---maybe it has a wrapper ---*/

private:
	QString _name;
	QColor _color;
	QPixmap _pixmap;
	bool _create;
	bool _resize;
	bool _move;
	bool _copy;
	bool _min_size;
	PedSector _minFsSize;
	PedSector _maxFsSize;
	QP_FSWrap *_fswrap;
};
/*-----------------------------------------------------------------------------------*/





/*----------QP_FileSystem------------------------------------------------------------*/
/*---this class contain a simply list of all filesystem supported, so you can     ---*/
/*---easily know info about a filesystem                                          ---*/
/*---                                                                             ---*/
/*---after create an object just call n-times addFileSystem to populate the list! ---*/
class QP_FileSystem {
public:
	QP_FileSystem();
	~QP_FileSystem();
	void addFileSystem(QString name, bool create,
			bool resize, bool move, bool copy);
	QP_FileSystemSpec *nameToFSSpec(QString name); /*---name2 QP_FileSystemSpec ---*/
	QP_FileSystemSpec *free(); /*---virtual property used for "free" partition ---*/
	QP_FileSystemSpec *unknown(); /*---as above but for "unknown" partition ---*/

	QList<QP_FileSystemSpec*> filesystemlist;
	QList<QP_FSWrap*> fswraplist;

private:
	QP_FileSystemSpec *_free;
	QP_FileSystemSpec *_unknown;
};
/*-----------------------------------------------------------------------------------*/

#endif
