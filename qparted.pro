#    qparted - a frontend to libparted for manipulating disk partitions
#    Copyright (C) 2002-2003 Vanni Brutto; 2015- ZZYZX
#
#    Vanni Brutto <zanac (-at-) libero dot it>
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#
# QParted qmake profile
#


TEMPLATE     = app


DEFINES     += DATADIR='"\\"/usr/share/\\""'
DEFINES     += VERSION='"\\"0.6.1\\""'


INCLUDEPATH += . ui src ts

# Configuration.  Remove the word 'thread' to build against non-threaded Qt
CONFIG      += qt thread debug

#LIBS
unix:LIBS   += -ldl -lparted

# Executable name
TARGET       = qparted

# Header files
HEADERS      = src/qparted.h           \
               src/qp_common.h         \
               src/qp_settings.h       \
               src/qp_exttools.h       \
               src/qp_libparted.h      \
               src/qp_filesystem.h     \
               src/qp_fswrap.h         \
               src/qp_window.h         \
               src/qp_dlgcreate.h      \
               src/qp_dlgresize.h      \
               src/qp_dlgprogress.h    \
               src/qp_dlgformat.h      \
               src/qp_dlgconfig.h      \
               src/qp_partlist.h       \
               src/qp_listview.h       \
               src/qp_listchart.h      \
               src/qp_partition.h      \
               src/qp_partwidget.h     \
               src/qp_extended.h       \
               src/qp_drivelist.h      \
               src/qp_navview.h        \
               src/qp_diskview.h       \
               src/qp_sizepart.h       \
               src/qp_actlist.h        \
               src/qp_combospin.h      \
               src/qp_devlist.h        \
               src/qp_spinbox.h        \
               src/qp_dlgdevprop.h     \
               src/qp_debug.h          \
               src/statistics.h


# Source files
SOURCES      = src/main.cpp            \
               src/qp_common.cpp       \
               src/qp_settings.cpp     \
               src/qp_exttools.cpp     \
               src/qp_libparted.cpp    \
               src/qp_filesystem.cpp   \
               src/qp_fswrap.cpp       \
               src/qp_window.cpp       \
               src/qp_dlgcreate.cpp    \
               src/qp_dlgresize.cpp    \
               src/qp_dlgprogress.cpp  \
               src/qp_dlgformat.cpp    \
               src/qp_dlgconfig.cpp    \
               src/qp_partlist.cpp     \
               src/qp_listview.cpp     \
               src/qp_listchart.cpp    \
               src/qp_partition.cpp    \
               src/qp_partwidget.cpp   \
               src/qp_extended.cpp     \
               src/qp_drivelist.cpp    \
               src/qp_navview.cpp      \
               src/qp_diskview.cpp     \
               src/qp_sizepart.cpp     \
               src/qp_actlist.cpp      \
               src/qp_combospin.cpp    \
               src/qp_spinbox.cpp      \
               src/qp_devlist.cpp      \
               src/qp_dlgdevprop.cpp   \
               src/qp_debug.cpp        \
               src/statistics.cpp


# Qt Designer interfaces
FORMS        = ui/qp_ui_create.ui       \
               ui/qp_ui_format.ui       \
               ui/qp_ui_resize.ui       \
               ui/qp_ui_progress.ui     \
               ui/qp_ui_devprop.ui


# Translations
TRANSLATIONS = ts/qparted_ca.ts        \
               ts/qparted_cs.ts        \
               ts/qparted_de.ts        \
               ts/qparted_es.ts        \
               ts/qparted_fi.ts        \
               ts/qparted_fr.ts        \
               ts/qparted_it.ts        \
               ts/qparted_pl.ts        \
               ts/qparted_ru.ts        \
               ts/qparted_sv.ts        \
               ts/qparted_ua.ts

# Tidy
QMAKE_CLEAN += $(TARGET) $(QMAKE_TARGET)

# Install
QPARTED.path = /usr/bin
QPARTED.files = qparted
INSTALLS += QPARTED
