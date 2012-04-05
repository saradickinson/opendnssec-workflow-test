#!/usr/bin/env bash
#
# Change WorkerThreads to 32 and check if 32 threads are used

if [ -n "$HAVE_MYSQL" ]; then
	ods_setup_conf conf.xml conf-mysql.xml
fi &&

ods_reset_env &&

log_this ods-control-start ods-control start &&
syslog_waitfor 60 'ods-enforcerd: .*Sleeping for' &&
syslog_waitfor 60 'ods-signerd: .*\[engine\] signer started' &&

log_this ods-control-stop ods-control stop &&
syslog_waitfor 60 'ods-enforcerd: .*all done' &&
syslog_waitfor 60 'ods-signerd: .*\[engine\] signer shutdown' &&

syslog_grep 'ods-signerd: .*\[worker\[32\]\] report for duty' &&
! syslog_grep 'ods-signerd: .*\[worker\[33\]\] report for duty' &&
return 0

ods_kill
return 1
