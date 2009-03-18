"""This class defines Zones, with all information needed to sign them"""

import os
import time
import subprocess
from datetime import datetime
import traceback
import syslog

from ZoneConfig import ZoneConfig

import Util

# todo: move this path to general engine config too?
#tools_dir = "../signer_tools";

class Zone:
    """Zone representation, with all information needed to sign them"""
    def __init__(self, _zone_name, engine_config):
        self.zone_name = _zone_name
        self.engine_config = engine_config
        self.locked = False
        
        # information received from KASP through the xml file
        self.zone_config = None
        
        # last_update as specified in zonelist.xml, to see when
        # the config for this zone needs to be reread
        self.last_update = None
        # this isn't used atm
        self.last_read = None
    
    # we define two zone objects the same if the zone names are equal
    def __eq__(self, other):
        return self.zone_name == other.zone_name
    
    # todo: make this already the xml format that is also used to
    # read zone information?
    # (will we have/need more data than that?)
    def __str__(self):
        result = ["name: " + self.zone_name]
        result.append("last config file read: " + str(self.last_read))
        return "\n".join(result)

    def get_zone_input_filename(self):
        """Returns the file name of the source zone file"""
        return self.engine_config.zone_input_dir + os.sep \
            + self.zone_name
        
    def get_zone_output_filename(self):
        """Returns the file name of the final signed output file"""
        return self.engine_config.zone_output_dir + os.sep \
            + self.zone_name + ".signed"
        
    def get_zone_config_filename(self):
        """Returns the file name of the zone configuration xml file"""
        return self.engine_config.zone_config_dir \
            + os.sep + self.zone_name + ".xml"

    def get_zone_tmp_filename(self, ext=""):
        """Returns the file name of the temporary zone file"""
        return self.engine_config.zone_tmp_dir + os.sep + \
            self.zone_name + ext

    def get_tool_filename(self, tool_name):
        """Returns the complete path to the tool file tool_name"""
        return self.engine_config.tools_dir + os.sep + tool_name
        
    def read_config(self):
        """Read the zone xml configuration from the standard location"""
        self.zone_config = ZoneConfig(self.get_zone_config_filename())
        self.last_read = datetime.now()
    
    def get_input_serial(self):
        """Returns the serial number from the SOA record in the input
        zone file"""
        zone_file = self.get_zone_input_filename()
        cmd = [ self.get_tool_filename("get_serial"),
                "-f", zone_file ]
        get_serial_c = Util.run_tool(cmd)
        result = 0
        for line in get_serial_c.stdout:
            result = int(line)
        status = get_serial_c.wait()
        if (status == 0):
            return result
        else:
            syslog.syslog(syslog.LOG_WARNING,
                          "Warning: get_serial returned " + str(status))
            return 0

    def get_output_serial(self):
        """Returns the serial number from the SOA record in the signed
        output file"""
        zone_file = self.get_zone_output_filename()
        cmd = [ self.get_tool_filename("get_serial"),
                "-f", zone_file ]
        get_serial_c = Util.run_tool(cmd)
        result = 0
        for line in get_serial_c.stdout:
            result = int(line)
        status = get_serial_c.wait()
        if (status == 0):
            return result
        else:
            syslog.syslog(syslog.LOG_WARNING,
                          "Warning: get_serial returned " + str(status))
            return 0

    # this uses the locator value to find the right pkcs11 module
    # creates a DNSKEY string to add to the unsigned zone,
    # and calculates the correct tool_key_id
    # returns True if the key is found
    def find_key_details(self, key):
        """Fills in the details about the key by querying all configured
        HSM tokens for the key (by its locator value)."""
        syslog.syslog(syslog.LOG_DEBUG,
                      "Generating DNSKEY rr for " + str(key["id"]))
        # just try all modules to generate the dnskey?
        # first one to return anything is good?
        for token in self.engine_config.tokens:
            mpath = token["module_path"]
            mpin = token["pin"]
            tname = token["name"]
            syslog.syslog(syslog.LOG_DEBUG, "Try token " + tname)
            cmd = [ self.get_tool_filename("create_dnskey_pkcs11"),
                    "-n", tname,
                    "-m", mpath,
                    "-p", mpin,
                    "-o", self.zone_name,
                    "-a", str(key["algorithm"]),
                    "-f", str(key["flags"]),
                    "-t", str(key["ttl"]),
                    key["locator"]
                  ]
            create_p = Util.run_tool(cmd)
            for line in create_p.stdout:
                output = line
            status = create_p.wait()
            for line in create_p.stderr:
                syslog.syslog(syslog.LOG_ERR,
                            "create_dnskey stderr: " + line)
            syslog.syslog(syslog.LOG_DEBUG,
                          "create_dnskey status: " + str(status))
            syslog.syslog(syslog.LOG_DEBUG,
                          "equality: " + str(status == 0))
            if status == 0:
                key["token_name"] = tname
                key["pkcs11_module"] = mpath
                key["pkcs11_pin"] = mpin
                key["tool_key_id"] = key["locator"] + \
                                     "_" + str(key["algorithm"])
                key["dnskey"] = str(output)
                syslog.syslog(syslog.LOG_INFO,
                              "Found key " + key["locator"] +\
                              " in token " + tname)
                return True
        return False

    def sort(self):
        """Sort the zone according to the relevant signing details
        (either in 'normal' or 'NSEC3' space). The zone is read from
        the input file, and the result is stored in the temp dir,
        without an extension. If key data is not filled in with
        find_key_details, this is done now."""
        syslog.syslog(syslog.LOG_INFO,
                      "Sorting zone: " + self.zone_name)
        unsorted_zone_file = open(self.get_zone_input_filename(), "r")
        cmd = [ self.get_tool_filename("sorter") ]
        if self.zone_config.denial_nsec3:
            cmd.extend(["-o", self.zone_name,
                        "-n",
                        "-s", self.zone_config.denial_nsec3_salt,
                        "-t",
                        str(self.zone_config.denial_nsec3_iterations),
                        "-a",
                        str(self.zone_config.denial_nsec3_algorithm)])
        sort_process = Util.run_tool(cmd, subprocess.PIPE)
        
        # sort published keys and zone data
        try:
            for k in self.zone_config.publish_keys:
                if not k["dnskey"]:
                    try:
                        syslog.syslog(syslog.LOG_DEBUG,
                                      "No information yet for key " +\
                                      k["locator"])
                        if (self.find_key_details(k)):
                            sort_process.stdin.write(k["dnskey"]+ "\n")
                        else:
                            syslog.syslog(syslog.LOG_ERR,
                                          "Error: could not find key "+\
                                          k["locator"])
                    except Exception, exc:
                        syslog.syslog(syslog.LOG_ERR,
                                      "Error: Unable to find key " +\
                                      k["locator"])
                        syslog.syslog(syslog.LOG_ERR, str(exc))
                        sort_process.stdin.write(
                            "; Unable to find key " + k["locator"])
                else:
                    sort_process.stdin.write(k["dnskey"]+ "\n") 
            
            for line in unsorted_zone_file:
                sort_process.stdin.write(line)
            sort_process.stdin.close()
            
            unsorted_zone_file.close()
            sorted_zone_file = open(self.get_zone_tmp_filename(), "w")
            
            for line in sort_process.stderr:
                syslog.syslog(syslog.LOG_ERR,
                              "stderr from sorter: " + line)
            
            for line in sort_process.stdout:
                sorted_zone_file.write(line)
            sorted_zone_file.close()
        except Exception, exc:
            syslog.syslog(syslog.LOG_ERR, "Error sorting zone\n")
            syslog.syslog(syslog.LOG_WARNING, str(exc))
            syslog.syslog(syslog.LOG_WARNING,
                          "Command was: " + " ".join(cmd))
            for line in sort_process.stderr:
                syslog.syslog(syslog.LOG_WARNING,
                              "sorter stderr: " + line)
            raise exc
        syslog.syslog(syslog.LOG_INFO, "Done sorting")
        
    def sign(self):
        """Sign the zone. The input is read from the temporary zone file
        produced with sort(). It is stripped, nsec(3)ed, and finally
        signed. The result is written to the default output file."""
        self.lock("sign()")
        try:
            # todo: only sort if necessary (depends on 
            #       what has changed in the policy)
            self.sort()
            syslog.syslog(syslog.LOG_INFO, "Signing zone: " + self.zone_name)
            # hmz, todo: stripped records need to be re-added
            # and another todo: move strip and nsec to stored file too?
            # (so only signing needs to be redone at re-sign time)
            strip_p = Util.run_tool([self.get_tool_filename("stripper"),
                                "-o", self.zone_name,
                                "-f", self.get_zone_tmp_filename()]
                               )
            
            if self.zone_config.denial_nsec:
                nsec_p = Util.run_tool(
                                  [self.get_tool_filename("nseccer")],
                                  strip_p.stdout)
            elif self.zone_config.denial_nsec3:
                nsec_p = Util.run_tool(
                    [
                        self.get_tool_filename("nsec3er"),
                        "-o", self.zone_name,
                        "-s",
                        self.zone_config.denial_nsec3_salt,
                        "-t",
                        str(self.zone_config.denial_nsec3_iterations),
                        "-a",
                        str(self.zone_config.denial_nsec3_algorithm)
                    ],
                    strip_p.stdout)
            cmd = [self.get_tool_filename("signer_pkcs11") ]

            sign_p = Util.run_tool(cmd)
            sign_p.stdin.write("\n")
            sign_p.stdin.write(":origin " + self.zone_name + "\n")
            syslog.syslog(syslog.LOG_DEBUG,
                          "send to signer: " +\
                          ":origin " + self.zone_name)
            
            # optional SOA modification values
            if self.zone_config.soa_ttl:
                sign_p.stdin.write(":soa_ttl " +\
                               str(self.zone_config.soa_ttl) + "\n")
            if self.zone_config.soa_minimum:
                sign_p.stdin.write(":soa_minimum " +\
                               str(self.zone_config.soa_ttl) + "\n")
            if self.zone_config.soa_serial:
                # there are a few options;
                # by default, plain copy the original soa serial
                # (which must have been read...)
                # for now, only support 'unixtime'
                soa_serial = "123"
                if self.zone_config.soa_serial == "unixtime":
                    soa_serial = int(time.time())
                    syslog.syslog(syslog.LOG_DEBUG,
                                  "set serial to " + str(soa_serial))
                    sign_p.stdin.write(":soa_serial " +\
                                   str(soa_serial) + "\n")
                elif self.zone_config.soa_serial == "counter":
                    # try output serial first, if not found, use input
                    prev_serial = self.get_output_serial()
                    if not prev_serial:
                        prev_serial = self.get_input_serial()
                    if not prev_serial:
                        prev_serial = 0
                    soa_serial = prev_serial + 1
                    syslog.syslog(syslog.LOG_DEBUG,
                                  "set serial to " + str(soa_serial))
                    sign_p.stdin.write(":soa_serial " +\
                                   str(soa_serial) + "\n")
                elif self.zone_config.soa_serial == "datecounter":
                    # if current output serial >= <date>00,
                    # just increment by one
                    soa_serial = int(time.strftime("%Y%m%d")) * 100
                    output_serial = self.get_output_serial()
                    if output_serial >= soa_serial:
                        soa_serial = output_serial + 1
                    syslog.syslog(syslog.LOG_DEBUG,
                                  "set serial to " + str(soa_serial))
                    sign_p.stdin.write(":soa_serial " +\
                                   str(soa_serial) + "\n")
                else:
                    syslog.syslog(syslog.LOG_WARNING,
                                  "warning: unknown serial type " +\
                                  self.zone_config.soa_serial)
            for k in self.zone_config.signature_keys:
                syslog.syslog(syslog.LOG_DEBUG,
                              "use signature key: " + k["locator"])
                if not k["dnskey"]:
                    try:
                        syslog.syslog(syslog.LOG_DEBUG,
                                      "No information yet for key " +\
                                      k["locator"])
                        self.find_key_details(k)
                    except Exception:
                        syslog.syslog(syslog.LOG_ERR,
                                      "Error: Unable to find key " +\
                                      k["locator"])
                if k["token_name"]:
                    scmd = [":add_module",
                            k["token_name"],
                            k["pkcs11_module"],
                            k["pkcs11_pin"]
                           ]
                    syslog.syslog(syslog.LOG_DEBUG,
                                  "send to signer " + " ".join(scmd))
                    sign_p.stdin.write(" ".join(scmd) + "\n")
                    scmd = [":add_key",
                            k["token_name"],
                            k["tool_key_id"],
                            str(k["algorithm"]),
                            str(k["flags"])
                           ]
                    syslog.syslog(syslog.LOG_DEBUG,
                                  "send to signer " + " ".join(scmd))
                    sign_p.stdin.write(" ".join(scmd) + "\n")
                else:
                    syslog.syslog(syslog.LOG_WARNING,
                                  "warning: no token for key " +\
                                  k["locator"])
            for line in nsec_p.stdout:
                #syslog.syslog(syslog.LOG_DEBUG, "send to signer " + l)
                sign_p.stdin.write(line)
            sign_p.stdin.close()
            sign_p.wait()
            output = open(self.get_zone_output_filename(), "w")
            for line in sign_p.stdout:
                output.write(line)
            for line in sign_p.stderr:
                syslog.syslog(syslog.LOG_WARNING, "signer stderr: line")
            output.close()
        except Exception:
            traceback.print_exc()
        syslog.syslog(syslog.LOG_INFO, "Done signing " + self.zone_name)
        #Util.debug(1, "signer result: " + str(status));
        self.release()
        
    def lock(self, caller=None):
        """Lock the zone with a simple spinlock"""
        msg = "waiting for lock on zone " +\
              self.zone_name + " to be released"
        if caller:
            msg = str(caller) + ": " + msg
        while (self.locked):
            syslog.syslog(syslog.LOG_DEBUG, msg)
            time.sleep(1)
        self.locked = True
        msg = "Zone " + self.zone_name + " locked"
        if caller:
            msg = msg + " by " + str(caller)
        syslog.syslog(syslog.LOG_DEBUG, msg)
    
    def release(self):
        """Release the lock on this zone"""
        syslog.syslog(syslog.LOG_DEBUG,
                      "Releasing lock on zone " + self.zone_name)
        self.locked = False

    def calc_resign_from_output_file(self):
        """Checks the output file, and calculates the number of seconds
        until it should be signed again. This can be negative!
        If the file is not found, 0 is returned (and signing should be
        scheduled immediately"""
        output_file = self.get_zone_output_filename()
        try:
            statinfo = os.stat(output_file)
            return int(statinfo.st_mtime +\
                       self.zone_config.signatures_resign_time -\
                       time.time())
        except OSError:
            return 0
    

# quick test-as-we-go function
# use this for unit testing?
# otherwise remove it.
if __name__ == "__main__":
    # this will of course be retrieved from the general zone config dir
    #CONFFILE = "/home/jelte/repos/opendnssec/signer_engine/engine.conf"
    #TZONE = Zone("zone1.example", EngineConfiguration(CONFFILE))
    #TZONE.read_config()
    #s = TZONE.calc_resign_from_output_file()
    #z.sign()
    print "nothing atm"
