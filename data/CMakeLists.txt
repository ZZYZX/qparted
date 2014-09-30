install(FILES qtparted.xpm DESTINATION share/pixmaps)
file(GLOB PICS "*.png")
install(FILES ${PICS} DESTINATION share/qtparted/pics)
install(FILES qtparted.desktop DESTINATION share/applications)
