#/bin/sh

SCRIPT=/etc/init.d/load-hd-firmware.sh

if [ $# -ne 1 ]
then
        echo "Usage ./change_display.sh <lcd|hdmi>"
        exit 1;
fi

case $1 in
        lcd )
                echo "==== Changing default display to LCD ===="
                sed -i -e "s/#configure_lcd$/configure_lcd/g" \
                       $SCRIPT;;
        hdmi )
                echo "==== Changing default display to HDMI ===="
                sed -i -e "s/configure_lcd$/#configure_lcd/g" \
                       $SCRIPT;;
        * )
                echo "Error: Unsupported Display"
                echo "Usage ./change_display.sh <lcd|hdmi>"
                exit -1;;
esac

# Force a VFS sync immediately
sync

echo "==== Please reboot your board NOW! ===="
