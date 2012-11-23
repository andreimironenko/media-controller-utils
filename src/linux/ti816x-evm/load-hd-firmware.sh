#!/bin/sh
#
# manage HDVICP2 and HDVPSS Engine firmware

PATH=$PATH:/usr/share/ti/ti-media-controller-utils
HDVICP2_ID=1
HDVPSS_ID=2

case "$1" in
    start)
        echo "Loading HDVICP2 Firmware"
	prcm_config_app s
        modprobe syslink
        until [[ -e /dev/syslinkipc_ProcMgr && -e /dev/syslinkipc_ClientNotifyMgr ]]
        do                                                
            sleep 0.5
        done
        firmware_loader $HDVICP2_ID /usr/share/ti/ti-media-controller-utils/dm816x_hdvicp.xem3 start
        echo "Loading HDVPSS Firmware"
        firmware_loader $HDVPSS_ID /usr/share/ti/ti-media-controller-utils/dm816x_hdvpss.xem3 start
        modprobe vpss sbufaddr=0xBFB00000 mode=hdmi:1080p-60,dvo2:1080p-60,hdcomp:1080p-60 i2c_mode=1
        modprobe ti81xxfb vram=0:24M,1:16M,2:6M
        fbset -depth 32 -rgba 8/16,8/8,8/0,0/0
        modprobe ti81xxhdmi
      ;;
    stop)
        echo "Unloading HDVICP2 Firmware"
        firmware_loader $HDVICP2_ID /usr/share/ti/ti-media-controller-utils/dm816x_hdvicp.xem3 stop
        echo "Unloading HDVPSS Firmware"
        rmmod ti81xxfb
        rmmod ti81xxhdmi
        rmmod vpss
        firmware_loader $HDVPSS_ID /usr/share/ti/ti-media-controller-utils/dm816x_hdvpss.xem3 stop
        rmmod syslink
      ;;
    *)
        echo "Usage: /etc/init.d/load-hd-firmware.sh {start|stop}"
        exit 1
        ;;
esac

exit 0
