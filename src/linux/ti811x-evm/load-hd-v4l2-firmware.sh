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
        echo "Loading HDVPSS (V4L2) Firmware "
        firmware_loader $HDVPSS_ID /usr/share/ti/ti-media-controller-utils/dm814x_hdvpss_v4l2.xem3 start
        modprobe vpss sbufaddr=0xBFB00000 mode=hdmi:1080p-60 i2c_mode=0
        modprobe ti81xxfb vram=0:40M,1:1M,2:1M
        configure_lcd
        modprobe ti81xxvo
        modprobe tvp7002
        modprobe ti81xxvin
        fbset -depth 32 -rgba 8/16,8/8,8/0,0/0
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
        rmmod ti81xxvin
        rmmod ti81xxvo
        rmmod tvp7002
        rmmod ti81xxhdmi
        rmmod vpss
        firmware_loader $HDVPSS_ID /usr/share/ti/ti-media-controller-utils/dm814x_hdvpss_v4l2.xem3 stop
        rmmod syslink
      ;;
    *)
        echo "Usage: /etc/init.d/load-hd-v4l2-firmware.sh {start|stop}"
        exit 1
        ;;
esac

exit 0
