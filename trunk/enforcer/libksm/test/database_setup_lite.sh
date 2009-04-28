#!/bin/sh
#
# database.sh - Set Up/Tear Down Test Database
#
# Description:
#		This script is run from within the test program to set up a test
#		database before the the database test module is run and to tear it
#		down afterwards.
#
#		The following environment variables should be set before running the
#		test program to control access to the database:
#
#			DB_NAME			name of the database
#
# Invocation:
#		The script can also be run manually to set up and tear down the
#		database:
#
#			sh database.sh setup
#			sh database.sh teardown
#-

NAME=
if [ -n $DB_NAME ]; then
	NAME=$DB_NAME
fi

case $1 in
	setup)
		sqlite3 $NAME < '../utils/database_create.sqlite3';
		sqlite3 $NAME << EOF
DROP TABLE IF EXISTS TEST_BASIC;
CREATE TABLE TEST_BASIC (
   ID integer primary key AUTOINCREMENT,
   IVALUE INT,
   SVALUE VARCHAR(64),
   TVALUE varchar(64)
);
INSERT INTO TEST_BASIC VALUES(NULL, 100, NULL,  '20080101');
INSERT INTO TEST_BASIC VALUES(NULL, 200, 'ABC', '20080102');
INSERT INTO TEST_BASIC VALUES(NULL, 300, 'DEF', '20080103');

-- Create 2 policies:
INSERT INTO policies VALUES (NULL,'opendnssec','special policy for enforcer config', NULL, "2002-01-01 01:00:00");
INSERT INTO policies VALUES (NULL,'default','A default policy that will amaze you and your friends', NULL, "2002-01-01 01:00:00");

-- Parameter_policies for default
INSERT INTO parameters_policies VALUES (1,1,2,7200);
INSERT INTO parameters_policies VALUES (2,2,2,259200);
INSERT INTO parameters_policies VALUES (3,3,2,43200);
INSERT INTO parameters_policies VALUES (4,4,2,300);
INSERT INTO parameters_policies VALUES (5,5,2,3600);
INSERT INTO parameters_policies VALUES (6,6,2,604800);
INSERT INTO parameters_policies VALUES (7,7,2,1209600);
INSERT INTO parameters_policies VALUES (8,8,2,3600);
INSERT INTO parameters_policies VALUES (9,9,2,3);
INSERT INTO parameters_policies VALUES (10,10,2,1);
INSERT INTO parameters_policies VALUES (11,11,2,8640000);
INSERT INTO parameters_policies VALUES (12,12,2,1);
INSERT INTO parameters_policies VALUES (13,13,2,10);
INSERT INTO parameters_policies VALUES (14,14,2,160);
INSERT INTO parameters_policies VALUES (15,15,2,3600);
INSERT INTO parameters_policies VALUES (16,16,2,3600);
INSERT INTO parameters_policies VALUES (17,17,2,3600);
INSERT INTO parameters_policies VALUES (18,18,2,5);
INSERT INTO parameters_policies VALUES (19,19,2,2048);
INSERT INTO parameters_policies VALUES (20,20,2,86400);
INSERT INTO parameters_policies VALUES (21,21,2,2);
INSERT INTO parameters_policies VALUES (22,22,2,0);
INSERT INTO parameters_policies VALUES (23,23,2,1);
INSERT INTO parameters_policies VALUES (24,24,2,5);
INSERT INTO parameters_policies VALUES (25,25,2,1024);
INSERT INTO parameters_policies VALUES (26,26,2,86400);
INSERT INTO parameters_policies VALUES (27,27,2,2);
INSERT INTO parameters_policies VALUES (28,28,2,0);
INSERT INTO parameters_policies VALUES (29,29,2,9999);
INSERT INTO parameters_policies VALUES (30,30,2,3600);
INSERT INTO parameters_policies VALUES (31,31,2,3600);
INSERT INTO parameters_policies VALUES (32,32,2,1);
INSERT INTO parameters_policies VALUES (33,33,2,9999);
INSERT INTO parameters_policies VALUES (34,34,2,3600);
INSERT INTO parameters_policies VALUES (35,36,2,3600);
INSERT INTO parameters_policies VALUES (36,35,2,3600);
INSERT INTO parameters_policies VALUES (37,37,1,3600);
INSERT INTO parameters_policies VALUES (38,38,1,7257600);
INSERT INTO parameters_policies VALUES (39,39,1,259200);
INSERT INTO parameters_policies VALUES (40,40,2,0);
INSERT INTO parameters_policies VALUES (41,41,2,0);

-- A couple of Zones:
INSERT INTO zones VALUES (1,'opendnssec.org',2);
INSERT INTO zones VALUES (2,'opendnssec.se',2);

-- some security modules
insert into securitymodules (id, name, capacity) values (NULL, "sca6000-1", 1000);
insert into securitymodules (id, name, capacity) values (NULL, "softHSM-01-1", 1000);

-- Create a dead key which we can purge out of the database
INSERT INTO keypairs VALUES(NULL, "0x1", 5, 1024, 1, 6, "2001-01-01 01:00:00", NULL, NULL, NULL, NULL, "2002-01-01 01:00:00", 2, NULL, "");
-- With 2 zones using it:
INSERT INTO dnsseckeys VALUES (NULL, 1, 1, 257, 1, 1, NULL, NULL);
INSERT INTO dnsseckeys VALUES (NULL, 1, 2, 257, 1, 1, NULL, NULL);

-- parameters for KsmParameter tests
INSERT INTO categories VALUES (NULL,		"Test");
INSERT INTO parameters VALUES (NULL, "Blah", "Used in unit test", (select id from categories where name = "Test")	);

INSERT INTO parameters VALUES (NULL, "Blah2", "Used in unit test", (select id from categories where name = "Test")	);
INSERT INTO parameters_policies VALUES (NULL, (select id from parameters where name = "Blah2"), 2, 1	);

-- Create a key which we can request from the database
INSERT INTO keypairs VALUES(NULL, "0x1", 5, 1024, 1, 1, "2001-01-01 01:00:00", NULL, NULL, NULL, NULL, "2002-01-01 01:00:00", 2, NULL, "");
INSERT INTO dnsseckeys VALUES (NULL, 2, 1, 257, 1, 1, NULL, NULL);

-- Create a set of keys which we can delete from the database
INSERT INTO keypairs VALUES(NULL, "0x1", 5, 1024, 1, 1, "2001-01-01 01:00:00", NULL, NULL, NULL, NULL, "2002-01-01 01:00:00", 2, NULL, "");
INSERT INTO keypairs VALUES(NULL, "0x1", 5, 1024, 1, 1, "2001-01-01 01:00:00", NULL, NULL, NULL, NULL, "2002-01-01 01:00:00", 2, NULL, "");
INSERT INTO keypairs VALUES(NULL, "0x1", 5, 1024, 1, 1, "2001-01-01 01:00:00", NULL, NULL, NULL, NULL, "2002-01-01 01:00:00", 2, NULL, "");
INSERT INTO keypairs VALUES(NULL, "0x1", 5, 1024, 1, 1, "2001-01-01 01:00:00", NULL, NULL, NULL, NULL, "2002-01-01 01:00:00", 2, NULL, "");
INSERT INTO keypairs VALUES(NULL, "0x1", 5, 1024, 1, 1, "2001-01-01 01:00:00", NULL, NULL, NULL, NULL, "2002-01-01 01:00:00", 2, NULL, "");
INSERT INTO keypairs VALUES(NULL, "0x1", 5, 1024, 1, 1, "2001-01-01 01:00:00", NULL, NULL, NULL, NULL, "2002-01-01 01:00:00", 2, NULL, "");
INSERT INTO keypairs VALUES(NULL, "0x1", 5, 1024, 1, 1, "2001-01-01 01:00:00", NULL, NULL, NULL, NULL, "2002-01-01 01:00:00", 2, NULL, "");
INSERT INTO keypairs VALUES(NULL, "0x1", 5, 1024, 1, 1, "2001-01-01 01:00:00", NULL, NULL, NULL, NULL, "2002-01-01 01:00:00", 2, NULL, "");
INSERT INTO keypairs VALUES(NULL, "0x1", 5, 1024, 1, 1, "2001-01-01 01:00:00", NULL, NULL, NULL, NULL, "2002-01-01 01:00:00", 2, NULL, "");
INSERT INTO keypairs VALUES(NULL, "0x1", 5, 1024, 1, 1, "2001-01-01 01:00:00", NULL, NULL, NULL, NULL, "2002-01-01 01:00:00", 2, NULL, "");
INSERT INTO keypairs VALUES(NULL, "0x1", 5, 1024, 1, 1, "2001-01-01 01:00:00", NULL, NULL, NULL, NULL, "2002-01-01 01:00:00", 2, NULL, "");
INSERT INTO keypairs VALUES(NULL, "0x1", 5, 1024, 1, 1, "2001-01-01 01:00:00", NULL, NULL, NULL, NULL, "2002-01-01 01:00:00", 2, NULL, "");
INSERT INTO keypairs VALUES(NULL, "0x1", 5, 1024, 1, 1, "2001-01-01 01:00:00", NULL, NULL, NULL, NULL, "2002-01-01 01:00:00", 2, NULL, "");
EOF
		;;

	teardown)
		sqlite3 $NAME << EOF
DROP TABLE IF EXISTS TEST_BASIC;
EOF
		;;

	*)
		echo "Usage: $0 [setup | teardown]"
		;;
esac
