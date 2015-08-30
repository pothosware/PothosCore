########################################################################
# menu launcher and MIME type
########################################################################
install(
    FILES
        pothos-gui.desktop
        pothos-gui.xml
    DESTINATION share/Pothos/Desktop
    COMPONENT pothos_gui
)

########################################################################
# install/uninstall scripts
########################################################################
install(
    PROGRAMS
        install
        uninstall
    DESTINATION share/Pothos/Desktop
    COMPONENT pothos_gui
)
