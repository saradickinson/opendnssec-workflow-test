#!/usr/bin/env bash
#
#TEST: Test to see that the DSSUB command with --cka_id is dealt with as expected
#TEST: We use TIMESHIFT to get to the point where the KSK moves to the ready state

ENFORCER_WAIT=90	# Seconds we wait for enforcer to run
ENFORCER_COUNT=2	# How many log lines we expect to see

cp dssub.pl "$INSTALL_ROOT/var/opendnssec/tmp/" &&
chmod 744 "$INSTALL_ROOT/var/opendnssec/tmp/dssub.pl" &&

if [ -n "$HAVE_MYSQL" ]; then
        ods_setup_conf conf.xml conf-mysql.xml
fi &&

ods_reset_env &&

##################  SETUP ###########################
# Start enforcer (Zone already exists and we let it generate keys itself)
export ENFORCER_TIMESHIFT='01-01-2010 12:00' &&
log_this_timeout ods-control-enforcer-start $ENFORCER_WAIT ods-enforcerd -1 &&
syslog_waitfor $ENFORCER_WAIT 'ods-enforcerd: .*all done' &&

# Make sure TIMESHIFT worked:
syslog_grep "ods-enforcerd: .*Timeshift mode detected, running once only!" &&
syslog_grep "ods-enforcerd: .*DEBUG: Timeshift in operation; ENFORCER_TIMESHIFT set to 01-01-2010 12:00" &&

# Check that we are trying to use the correct command:
syslog_grep " ods-enforcerd: .*Using command: $INSTALL_ROOT/var/opendnssec/tmp/dssub.pl to submit DS records" &&

# Check that we have 2 keys
log_this ods-ksmutil-key-list1 ods-ksmutil key list &&
log_grep ods-ksmutil-key-list1 stdout 'ods                             KSK           publish' &&
log_grep ods-ksmutil-key-list1 stdout 'ods                             ZSK           active' &&

# Grab the CKA_ID of the KSK
log_this ods-ksmutil-cka_id ods-ksmutil key list --all --verbose &&
KSK_CKA_ID=`log_grep -o ods-ksmutil-cka_id stdout "ods                             KSK           publish" | awk '{print $9}'` &&

## Jump forward a couple of hours so the KSK will be ready
##################  STEP 1: Time = 2hrs ###########################
export ENFORCER_TIMESHIFT='01-01-2010 14:00' &&

# Run the enforcer
log_this_timeout ods-control-enforcer-start $ENFORCER_WAIT ods-enforcerd -1 &&
syslog_waitfor_count $ENFORCER_WAIT $ENFORCER_COUNT 'ods-enforcerd: .*all done' &&
syslog_grep "ods-enforcerd: .*DEBUG: Timeshift in operation; ENFORCER_TIMESHIFT set to 01-01-2010 14:00" &&

# We should be ready for a ds-seen on ods
syslog_grep "ods-enforcerd: .*Once the new DS records are seen in DNS please issue the ds-seen command for zone ods with the following cka_ids, $KSK_CKA_ID" &&

# Check that no dssub.out file exists
echo "Testing dssub command ran" &&
test -f "$INSTALL_ROOT/var/opendnssec/tmp/dssub.out" &&

echo "Testing contents of dssub.out" &&
grep "ods. 600 IN DNSKEY 257 3 7 AwEAA.*" "$INSTALL_ROOT/var/opendnssec/tmp/dssub.out" &&
grep "; {cka_id = $KSK_CKA_ID}" "$INSTALL_ROOT/var/opendnssec/tmp/dssub.out" &&

# Clean up
echo "Cleaning up files" &&
rm "$INSTALL_ROOT/var/opendnssec/tmp/dssub.pl" &&
rm "$INSTALL_ROOT/var/opendnssec/tmp/dssub.out" &&


return 0

# Something went wrong, make sure clean up tmp if nothing else
rm "$INSTALL_ROOT/var/opendnssec/tmp/dssub.pl" &&
mv "$INSTALL_ROOT/var/opendnssec/tmp/dssub.out" "." &&

echo
echo "************ERROR******************"
echo
ods_kill
return 1

