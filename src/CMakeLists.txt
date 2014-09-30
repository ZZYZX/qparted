set(SRCS main.cpp
	qp_dlgcreate.cpp
	qp_dlgdevprop.cpp
	qp_dlgformat.cpp
	qp_dlgconfig.cpp
	qp_dlgresize.cpp
	qp_dlgprogress.cpp
	qp_drivelist.cpp
	qp_devlist.cpp
	qp_libparted.cpp
	qp_partlist.cpp
	qp_actlist.cpp
	qp_extended.cpp
	qp_listchart.cpp
	qp_partition.cpp
	qp_partwidget.cpp
	qp_diskview.cpp
	qp_window.cpp
	qp_combospin.cpp
	qp_exttools.cpp
	qp_listview.cpp
	qp_settings.cpp
	qp_common.cpp
	qp_filesystem.cpp
	qp_navview.cpp
	qp_fswrap.cpp
	qp_sizepart.cpp
	qp_spinbox.cpp
	qp_debug.cpp
	statistics.cpp)

set(PRIVATE_HEADERS qtparted.h
	qp_dlgcreate.h
	qp_dlgdevprop.h
	qp_dlgformat.h
	qp_dlgconfig.h
	qp_dlgresize.h
	qp_dlgprogress.h
	qp_drivelist.h
	qp_devlist.h
	qp_libparted.h
	qp_partlist.h
	qp_actlist.h
	qp_extended.h
	qp_listchart.h
	qp_partition.h
	qp_partwidget.h
	qp_diskview.h
	qp_window.h
	qp_combospin.h
	qp_exttools.h
	qp_listview.h
	qp_settings.h
	qp_common.h
	qp_filesystem.h
	qp_navview.h
	qp_fswrap.h
	qp_sizepart.h
	qp_spinbox.h
	statistics.h)

set(UIS	qp_ui_create.ui
	qp_ui_devprop.ui
	qp_ui_format.ui
	qp_ui_progress.ui
	qp_ui_resize.ui)

qt4_wrap_ui(UI_SRCS ${UIS})
qt4_wrap_cpp(MOC_SRCS ${PRIVATE_HEADERS} ${UI_SRCS})

add_executable(qtparted ${SRCS} ${MOC_SRCS})
target_link_libraries(qtparted ${QT_QTMAIN_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QT3SUPPORT_LIBRARY} parted)
install(TARGETS qtparted DESTINATION bin)
