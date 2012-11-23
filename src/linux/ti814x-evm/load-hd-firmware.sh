#!/bin/sh
#
# manage HDVICP2 and HDVPSS Engine firmware

PATH=$PATH:/usr/share/ti/ti-media-controller-utils
HDVICP2_ID=1
HDVPSS_ID=2

configure_lcd()
{
    echo "Configuring fb0 to LCD"
    echo 1:dvo2 > /sys/devices/platform/vpss/graphics0/nodes
    echo 0 > /sys/devices/platform/vpss/display1/enabled
    echo 33500,800/164/89/10,480/10/23/10,1 > /sys/devices/platform/vpss/display1/timings
    echo triplediscrete,rgb888 > /sys/devices/platform/vpss/display1/output
    echo 1 > /sys/devices/platform/vpss/display1/enabled
    fbset -xres 800 -yres 480 -vxres 800 -vyres 480
}


case "$1" in
    start)
        echo "Loading HDVICP2 Firmware"
        prcm_config_app s
        modprobe syslink
        until [[ -e /dev/syslinkipc_ProcMgr && -e /dev/syslinkipc_ClientNotifyMgr ]]
        do                                                
            sleep 0.5
        done
        firmware_loader $HDVICP2_ID /usr/share/ti/ti-media-controller-utils/dm814x_hdvicp.xem3 start
        echo "Loading HDVPSS Firmware"
        firmware_loader $HDVPSS_ID /usr/share/ti/ti-media-controller-utils/dm814x_hdvpss.xem3 start
        modprobe vpss sbufaddr=0xBFB00000 mode=hdmi:1080p-60 i2c_mode=1
        modprobe ti81xxfb vram=0:24M,1:16M,2:6M
        configure_lcd
        modprobe ti81xxhdmi
        modprobe tlc59108
      ;;
    stop)
        echo "Unloading HDVICP2 Firmware"
        firmware_loader $HDVICP2_ID /usr/share/ti/ti-media-controller-utils/dm814x_hdvicp.xem3 stop
        echo "Unloading HDVPSS Firmware"
        rmmod tlc59108
        rmmod ti81xxhdmi
        rmmod ti81xxfb
        rmmod vpss
        firmware_loader $HDVPSS_ID /usr/share/ti/ti-media-controller-utils/dm814x_hdvpss.xem3 stop
        rm /tmp/firmware.$HDVPSS_ID
        rmmod syslink
      ;;
    *)
        echo "Usage: /etc/init.d/load-hd-firmware.sh {start|stop}"
        exit 1
        ;;
esac

exit 0
