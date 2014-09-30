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

#include <qcolor.h>

#ifndef QT_PARTED_H
#define QT_PARTED_H

#define MEGABYTE (1024 * 1024)
#define MEGABYTE_SECTORS (MEGABYTE / 512)

enum Resize {
	None = 0,
	Shrink = 1,
	Enlarge = 2,
	Both = Shrink|Enlarge,
};

class Q_DECL_EXPORT QTParted {
public:
    enum partType {
        primary,
        extended,
        logical
    };

    enum actType {
        move,
        resize,
        rm,
        create,
        active,
        hidden,
        format
    };
};

#endif
