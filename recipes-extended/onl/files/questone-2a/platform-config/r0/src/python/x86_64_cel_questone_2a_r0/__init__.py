from onl.platform.base import *
from onl.platform.celestica import *

class OnlPlatform_x86_64_cel_questone_2a_r0(OnlPlatformCelestica,
                                            OnlPlatformPortConfig_48x25_8x100):
    PLATFORM='x86-64-cel-questone-2a-r0'
    MODEL="Questone-2A"
    SYS_OBJECT_ID=".2060.1"

    def baseconfig(self):
        onlp_interval_time = 30  # second
        file_path = "/var/opt/interval_time.txt"
        qsfp_quantity = 8
        sfp_quantity = 48
        sfp_i2c_start_bus = 2

        #Celestica Blacklist file
        blacklist_file_path="/etc/modprobe.d/celestica-blacklist.conf"
        #Blacklist the unuse module.
        if os.path.exists(blacklist_file_path):
            os.system("rm {0}".format(blacklist_file_path))
       
        os.system("touch {0}".format(blacklist_file_path))
        cel_paths = "/lib/modules/{0}/onl/celestica/".format(os.uname()[2])
        cel_dirs = os.listdir(cel_paths)
        for dir in cel_dirs:
            full_cel_path=cel_paths+dir
            if os.path.isdir(full_cel_path):
                modules=os.listdir(full_cel_path)
                for module in modules:
                    os.system("echo 'blacklist {0}' >> {1}".format(module[0:-3],blacklist_file_path))

        print("Initialize and Install the driver here")
        self.insmod("questone2a_switchboard.ko")
        self.insmod("questone2a_baseboard_cpld.ko")
        self.insmod("optoe.ko")
        self.insmod("mc24lc64t.ko")

        # os.system("insmod /lib/modules/`uname -r`/kernel/drivers/char/ipmi/ipmi_devintf.ko")
        # os.system("insmod /lib/modules/`uname -r`/kernel/drivers/char/ipmi/ipmi_si.ko")
        # os.system("insmod /lib/modules/`uname -r`/kernel/drivers/char/ipmi/ipmi_ssif.ko")

        # ###### new configuration for SDK support ########
        # os.system("insmod /lib/modules/`uname -r`/kernel/net/core/pktgen.ko")
        # os.system("insmod /lib/modules/`uname -r`/kernel/net/core/drop_monitor.ko")
        # os.system("insmod /lib/modules/`uname -r`/kernel/net/ipv4/tcp_probe.ko")

        # eeprom driver
        self.new_i2c_device('24lc64t', 0x56, 1)
        # initialize SFP devices name
        for actual_i2c_port in range(sfp_i2c_start_bus, sfp_i2c_start_bus+(qsfp_quantity+sfp_quantity)):
            port_number = actual_i2c_port - (sfp_i2c_start_bus-1)
            if(port_number <= sfp_quantity):
                #print("echo 'QSFP{1}' > /sys/devices/i2c-{0}/{0}-0050/port_name".format(actual_i2c_port,port_number))
                os.system("echo 'SFP{1}' > /sys/devices/i2c-{0}/{0}-0050/port_name".format(actual_i2c_port,port_number))
            else:
                #print("echo 'SFP{1}' > /sys/devices/i2c-{0}/{0}-0050/port_name".format(actual_i2c_port,port_number-qsfp_quantity))
                os.system("echo 'QSFP{1}' > /sys/devices/i2c-{0}/{0}-0050/port_name".format(actual_i2c_port,port_number-sfp_quantity))
            # self.new_i2c_device('sff8436', 0x50, port)
            # self.new_i2c_device('as5912_54x_sfp%d' % port, 0x51, port+25)
        
        # Script for create interval_time cache.
        if os.path.exists(file_path):
            pass
        else:
            with open(file_path, 'w') as f:  
                f.write("{0}\r\n".format(onlp_interval_time))
            f.close()

        #initialize onlp cache files
        print("Initialize ONLP Cache files")
        os.system("ipmitool fru > /tmp/onlp-fru-cache.tmp; sync; rm -f /tmp/onlp-fru-cache.txt; mv /tmp/onlp-fru-cache.tmp /tmp/onlp-fru-cache.txt")
        os.system("ipmitool sensor list > /tmp/onlp-sensor-list-cache.tmp; sync; rm -f /tmp/onlp-sensor-list-cache.txt; mv /tmp/onlp-sensor-list-cache.tmp /tmp/onlp-sensor-list-cache.txt")
        
        return True
