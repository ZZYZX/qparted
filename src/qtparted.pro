#    qtparted - a frontend to libparted for manipulating disk partitions
#    Copyright (C) 2002-2003 Vanni Brutto
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
# QTParted qmake profile
#

# Configuration.  Remove the word 'thread' to build against non-threaded Qt
CONFIG    += qt thread debug

#LIBS
unix:LIBS += -ldl -lparted

# Executable name
TARGET    = qtparted


# Header files
HEADERS   =  qtparted.h          \
             qp_options.h        \
             qp_common.h         \
             qp_settings.h       \
             qp_exttools.h       \
             qp_libparted.h      \
             qp_filesystem.h     \
             qp_fswrap.h         \
             qp_window.h         \
             qp_dlgcreate.h      \
             qp_dlgresize.h      \
             qp_dlgprogress.h    \
             qp_dlgformat.h      \
             qp_dlgconfig.h      \
             qp_dlgabout.h       \
             qp_partlist.h       \
             qp_listview.h       \
             qp_listchart.h      \
             qp_partition.h      \
             qp_partwidget.h     \
             qp_extended.h       \
             qp_drivelist.h      \
             qp_navview.h        \
             qp_diskview.h       \
             qp_sizepart.h       \
             qp_actlist.h        \
             qp_combospin.h      \
             qp_devlist.h        \
             qp_spinbox.h        \
             qp_dlgdevprop.h     \
             qp_debug.h          \
             statistics.h




# Source files
SOURCES   =  main.cpp            \
             qp_common.cpp       \
             qp_settings.cpp     \
             qp_exttools.cpp     \
             qp_libparted.cpp    \
             qp_filesystem.cpp   \
             qp_fswrap.cpp       \
             qp_window.cpp       \
             qp_dlgcreate.cpp    \
             qp_dlgresize.cpp    \
             qp_dlgprogress.cpp  \
             qp_dlgformat.cpp    \
             qp_dlgconfig.cpp    \
             qp_dlgabout.cpp     \
             qp_partlist.cpp     \
             qp_listview.cpp     \
             qp_listchart.cpp    \
             qp_partition.cpp    \
             qp_partwidget.cpp   \
             qp_extended.cpp     \
             qp_drivelist.cpp    \
             qp_navview.cpp      \
             qp_diskview.cpp     \
             qp_sizepart.cpp     \
             qp_actlist.cpp      \
             qp_combospin.cpp    \
             qp_spinbox.cpp      \
             qp_devlist.cpp      \
             qp_dlgdevprop.cpp   \
             qp_debug.cpp        \
             statistics.cpp


# Qt Designer interfaces
INTERFACES = qp_ui_create.ui     \
             qp_ui_format.ui     \
             qp_ui_resize.ui     \
             qp_ui_progress.ui   \
             qp_ui_devprop.ui    \
             qp_ui_config.ui

# Translations
TRANSLATIONS = ../ts/qtparted_ca.ts \
               ../ts/qtparted_cs.ts \
               ../ts/qtparted_de.ts \
               ../ts/qtparted_es.ts \
               ../ts/qtparted_fi.ts \
               ../ts/qtparted_fr.ts \
               ../ts/qtparted_it.ts \
               ../ts/qtparted_pl.ts \
               ../ts/qtparted_ru.ts \
               ../ts/qtparted_sv.ts \
               ../ts/qtparted_ua.ts

