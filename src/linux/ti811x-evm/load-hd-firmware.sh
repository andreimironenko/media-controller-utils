#!/bin/sh
#
# manage HDVICP2 and HDVPSS Engine firmware

PATH=$PATH:/usr/share/ti/ti-media-controller-utils
HDVICP2_ID=1
HDVPSS_ID=2
      
case "$1" in
    start)
        /usr/share/ti/j5eco-tvp5158/decoder_init

        echo "Loading HDVPSS Firmware"
        modprobe syslink
        until [[ -e /dev/syslinkipc_ProcMgr && -e /dev/syslinkipc_ClientNotifyMgr ]]
        do                                                
            sleep 0.5
        done
        firmware_loader $HDVPSS_ID /usr/share/ti/ti-media-controller-utils/ti811x_hdvpss.xem3 start
        modprobe vpss sbufaddr=0x9fd00000 mode=hdmi:720p-60 i2c_mode=1 debug=1
        modprobe ti81xxfb vram=0:16M,1:16M,2:6M  debug=1
        modprobe ti81xxvo video1_numbuffers=3 video2_numbuffers=3 debug=1
        modprobe tvp7002 debug=1
        modprobe ti81xxvin debug=1
        modprobe sii9022a
        modprobe tlc59108
      ;;

    stop)
        echo "Unloading HDVPSS Firmware"
        rmmod tlc59108
        rmmod sii9022a
        rmmod ti81xxvin
        rmmod tvp7002
        rmmod ti81xxvo
        rmmod ti81xxfb
        rmmod vpss
        firmware_loader $HDVPSS_ID /usr/share/ti/ti-media-controller-utils/ti811x_hdvpss.xem3 stop
        rm /tmp/firmware.$HDVPSS_ID
        rmmod syslink
      ;;
    *)
        echo "Usage: /etc/init.d/load-hd-firmware.sh {start|stop}"
        exit 1
        ;;
esac

exit 0
