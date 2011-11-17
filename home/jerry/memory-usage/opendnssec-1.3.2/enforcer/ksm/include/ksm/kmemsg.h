#ifndef KSM_KMEMSG_H
#define KSM_KMEMSG_H

/* This file is generated by a perl script - it should not be edited */

#define KME_MIN_VALUE (KME_BASE)
#define KME_MAX_VALUE (KME_BASE + 38)
#define KME_ARRAY_SIZE (39)

static const char* m_messages[] = {
	"INFO: %d keys in 'active' state will have their expected retire date modified",
	"INFO: %d keys current in 'publish', 'ready' and 'active' states",
	"ERROR: internal error, buffer overflow in %s",
	"ERROR: unable to delete %s because child objects are associated with it",
	"ERROR: failed to create '%s'",
	"ERROR: object with name '%s' already exists",
	"ERROR: program error - number of fields returned did not match number expected",
	"INFO: %d %ss available in 'generate' state",
	"INFO: %d %ss available in 'generate' state (need %d) - unable to promote until more keys generated",
	"INFO: moving %d key(s) from '%s' state to '%s' state",
	"INFO: %d keys required, therefore %d new keys need to be put in 'publish' state",
	"WARNING: %s rollover for zone '%s' not completed as there are no keys in the 'ready' state; ods-enforcerd will try again when it runs next",
	"ERROR: no such parameter with name %s",
	"ERROR: unable to find object '%s'",
	"WARNING: Command not implemented yet",
	"ERROR: %s is not a zone",
	"ERROR: it is not permitted to delete the permanent object %s",
	"INFO: %d %ss in the 'ready' state",
	"INFO: %d %ss remaining in 'active' state",
	"INFO: requesting issue of %s signing keys",
	"INFO: %d 'active' keys will be retiring in the immediate future",
	"ERROR: database operation failed - %s",
	"ERROR: unknown key type, code %d",
	"WARNING: unrecognised condition code %d: code ignored",
	"WARNING: key ID %d is in unrecognised state %d",
	"INFO: Promoting %s from publish to active as this is the first pass for the zone",
	"ERROR: Trying to make non-backed up %s active when RequireBackup flag is set",
	"WARNING: Making non-backed up %s active, PLEASE make sure that you know the potential problems of using keys which are not recoverable",
	"INFO: Old DS record for %s can now be removed (key moved from retired to dead state)",
	"INFO: Old DS record for %s and all zones on its policy can now be removed (key moved from retired to dead state)",
	"INFO: %s has been rolled for %s ",
	"INFO: %s has been rolled for %s (and any zones sharing keys with %s)",
	"DEBUG: Timeshift in operation; ENFORCER_TIMESHIFT set to %s",
	"INFO: Manual rollover due for %s of zone %s",
	"ERROR: database version number incompatible with software; require %d, found %d. Please run the migration scripts",
	"ERROR: Too many rows returned from dbadmin table; there should be only one.",
	"WARNING: New KSK has reached the ready state; please submit the DS for %s and use ods-ksmutil key ds-seen when the DS appears in the DNS.",
	"ERROR: Key %s in DB but not repository.",
	"INFO: New DS records needed for the zone %s; details will follow",

/* ... and null-terminate the array to be on the safe side */

	NULL
};
#endif /* KSM_KMEMSG_H */