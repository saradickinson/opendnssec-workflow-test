/*+
 * KsmKey - Manipulation of Key Information
 *
 * Description:
 *      Holds the functions needed to manipulate the KEYDATA table.
 *
 *      N.B.  The table is the KEYDATA table - rather than the KEY table - as
 *      KEY is a reserved word in SQL.
 *
 *
 * Copyright:
 *      Copyright 2008 Nominet
 *      
 * Licence:
 *      Licensed under the Apache Licence, Version 2.0 (the "Licence");
 *      you may not use this file except in compliance with the Licence.
 *      You may obtain a copy of the Licence at
 *      
 *          http://www.apache.org/licenses/LICENSE-2.0
 *      
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the Licence is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the Licence for the specific language governing permissions and
 *      limitations under the Licence.
-*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "database.h"
#include "database_statement.h"
#include "db_fields.h"
#include "debug.h"
#include "kmedef.h"
#include "ksm.h"
#include "ksm_internal.h"
#include "message.h"
#include "string_util.h"

/*+
 * KsmKeyPairCreate - Create Entry in the KeyPairs table 
 *                    (i.e. key creation in the HSM)
 *
 * Description:
 *      Creates a key in the database.
 *
 * Arguments:
 *      policy_id
 *          policy that the key is created for
 *      HSMKeyID
 *          ID the key is refered to in the HSM
 *      smID
 *          security module ID
 *      size
 *          size of key
 *      alg
 *          algorithm used
 *      generate
 *          timestamp of generation
 *
 *      DB_ID* id (returned)
 *          ID of the created entry.  This will be undefined on error.
 *
 * Returns:
 *      int
 *          Status return.  0=> Success, non-zero => error.
-*/
int KsmKeyPairCreate(int policy_id, const char* HSMKeyID, int smID, int size, int alg, const char* generate, DB_ID* id)
{
    unsigned long rowid;			/* ID of last inserted row */
    int         status = 0;         /* Status return */
    char*       sql = NULL;         /* SQL Statement */

    sql = DisSpecifyInit("keypairs", "policy_id, HSMkey_id, securitymodule_id, size, algorithm, generate");
    DisAppendInt(&sql, policy_id);
    DisAppendString(&sql, HSMKeyID);
    DisAppendInt(&sql, smID);
    DisAppendInt(&sql, size);
    DisAppendInt(&sql, alg);
    DisAppendString(&sql, generate);
    DisEnd(&sql);

    /* Execute the statement */

    status = DbExecuteSqlNoResult(DbHandle(), sql);
    DisFree(sql);

    if (status == 0) {

        /* Succcess, get the ID of the inserted record */

		status = DbLastRowId(DbHandle(), &rowid);
		if (status == 0) {
			*id = (DB_ID) rowid;
		}
    }

    return status;
}

/*+
 * KsmDnssecKeyCreate - Create Entry in Dnsseckeys table 
 *                      (i.e. when a key is assigned to a policy/zone)
 *
 * Description:
 *      Allocates a key in the database.
 *
 * Arguments:
 *      KSM_KEY* data
 *          Data to insert into the database.  The ID argument is ignored.
 *
 *      DB_ID* id (returned)
 *          ID of the created entry.  This will be undefined on error.
 *
 * Returns:
 *      int
 *          Status return.  0=> Success, non-zero => error.
-*/

int KsmDnssecKeyCreate(int zone_id, int keypair_id, int keytype, DB_ID* id)
{
	unsigned long rowid;			/* ID of last inserted row */
    int         status = 0;         /* Status return */
    char*       sql = NULL;         /* SQL Statement */

    sql = DisSpecifyInit("dnsseckeys", "zone_id, keypair_id, keytype");
    DisAppendInt(&sql, zone_id);
    DisAppendInt(&sql, keypair_id);
    DisAppendInt(&sql, keytype);
    DisEnd(&sql);

    /* Execute the statement */

    status = DbExecuteSqlNoResult(DbHandle(), sql);
    DisFree(sql);

    if (status == 0) {

        /* Succcess, get the ID of the inserted record */

		status = DbLastRowId(DbHandle(), &rowid);
		if (status == 0) {
			*id = (DB_ID) rowid;
		}
    }

    return status;
}



/*+
 * KsmKeyModify - Modify KEYDATA Entry
 *
 * Description:
 *      Modifies a key in the database.
 *
 * Arguments:
 *      KSM_KEY* data
 *          Data to update.  Only fields where the flagss bit is set are
 *          updated.
 *
 *      int low
 *          Lower bound of range to modify.
 *
 *      int high
 *          Higher bound of range to modify.
 *
 * Returns:
 *      int
 *          Status return.  0=> Success, non-zero => error.
-*/
/*  everything except size, HSMkey_id and policy_id in keypairs */
int KsmKeyModify(KSM_KEYDATA* data, int low, int high)
{
    int         set = 0;            /* For the SET clause */
    int         status = 0;         /* Status return */
    char*       sql = NULL;         /* SQL Statement */
    int         temp;               /* For ensuring low < high */
    int         where = 0;          /* for the WHERE clause */

    /* Ensure range ordering is correct */

    if (low > high) {
        temp = low;
        low = high;
        high = temp;
    }

    /* Create the update command */

    sql = DusInit("keypairs");

    if (data->flags & KEYDATA_M_STATE) {
        DusSetInt(&sql, "STATE", data->state, set++);
    }

    if (data->flags & KEYDATA_M_ALGORITHM) {
        DusSetInt(&sql, "ALGORITHM", data->algorithm, set++);
    }

 /*   if (data->flags & KEYDATA_M_SIGLIFETIME) {
        DusSetInt(&sql, "SIGLIFETIME", data->siglifetime, set++);
    }*/

    /* ... the times */

    if (data->flags & KEYDATA_M_ACTIVE) {
        DusSetString(&sql, "ACTIVE", data->active, set++);
    }

    if (data->flags & KEYDATA_M_DEAD) {
        DusSetString(&sql, "DEAD", data->dead, set++);
    }

    if (data->flags & KEYDATA_M_GENERATE) {
        DusSetString(&sql, "GENERATE", data->generate, set++);
    }

    if (data->flags & KEYDATA_M_PUBLISH) {
        DusSetString(&sql, "PUBLISH", data->publish, set++);
    }

    if (data->flags & KEYDATA_M_READY) {
        DusSetString(&sql, "READY", data->ready, set++);
    }

    if (data->flags & KEYDATA_M_RETIRE) {
        DusSetString(&sql, "RETIRE", data->retire, set++);
    }

    /* ... and location */

    if (data->flags & KEYDATA_M_SMID) {
        DusSetInt(&sql, "SECURITYMODULE_ID", data->securitymodule_id, set++);
    }


    /* For what keys? */

    if (low == high) {
        DusConditionInt(&sql, "ID", DQS_COMPARE_EQ, low, where++);
    }
    else {
        DusConditionInt(&sql, "ID", DQS_COMPARE_GE, low, where++);
        DusConditionInt(&sql, "ID", DQS_COMPARE_LE, high, where++);
    }

    /* All done, close the statement */

    DusEnd(&sql);

    /* Execute the statement */

    status = DbExecuteSqlNoResult(DbHandle(), sql);

    /* This is now in a different table  */
    /* KEYTYPE SHOULD NOT CHANGE!!!
    if (status == 0 && (data->flags & KEYDATA_M_KEYTYPE)) {
        sql = DusInit("dnsseckeys");
        DusSetInt(&sql, "KEYTYPE", data->keytype, set++);

        if (low == high) {
            DusConditionInt(&sql, "ID", DQS_COMPARE_EQ, low, where++);
        }
        else {
            DusConditionInt(&sql, "ID", DQS_COMPARE_GE, low, where++);
            DusConditionInt(&sql, "ID", DQS_COMPARE_LE, high, where++);
        }

        DusEnd(&sql);
        status = DbExecuteSqlNoResult(DbHandle(), sql);
    }
    */

    DusFree(sql);

    return status;
}




/*+
 * KsmKeyInitSql - Query for Key Information With Sql Query
 *
 * Description:
 *      Performs a query for keys in the keydata table that match the given
 *      conditions.
 *
 * Arguments:
 *      DB_RESULT* result
 *          Pointer to a result to be used for information retrieval.  Will
 *          be NULL on error.
 *
 *      const char* sql
 *          SQL statement to select keys.
 *
 *          (Actually, the statement could be anything, but it is assumed
 *          that it is an SQL statement starting "SELECT xxx FROM KEYDATA".)
 *
 * Returns:
 *      int
 *          Status return.  0 on success.
-*/

int KsmKeyInitSql(DB_RESULT* result, const char* sql)
{
    return DbExecuteSql(DbHandle(), sql, result);
}




/*+
 * KsmKeyInit - Query for Key Information
 *
 * Description:
 *      Performs a query for keys in the keydata table that match the given
 *      conditions.
 *
 * Arguments:
 *      DB_RESULT* result
 *          Pointer to a result to be used for information retrieval.  Will
 *          be NULL on error.
 *
 *      DQS_QUERY_CONDITION* condition
 *          Array of condition objects, each defining a condition.  The
 *          conditions are ANDed together.  The array should end with an object
 *          with a condition code of 0.
 *
 *          If NULL, all objects are selected.
 *
 * Returns:
 *      int
 *          Status return.  0 on success.
-*/

int KsmKeyInit(DB_RESULT* result, DQS_QUERY_CONDITION* condition)
{
    int     i;                  /* Condition index */
    char*   sql = NULL;         /* SQL query */
    int     status = 0;         /* Status return */

    /* Construct the query */

    sql = DqsSpecifyInit("KEYDATA_VIEW", DB_KEYDATA_FIELDS);
    if (condition) {
        for (i = 0; condition[i].compare != DQS_END_OF_LIST; ++i) {
            switch (condition[i].code) {
            case DB_KEYDATA_ALGORITHM:
                DqsConditionInt(&sql, "ALGORITHM", condition[i].compare,
                    condition[i].data.number, i);
                break;

            case DB_KEYDATA_ID:
                DqsConditionInt(&sql, "ID", condition[i].compare,
                    condition[i].data.number, i);
                break;

            case DB_KEYDATA_KEYTYPE:
                DqsConditionInt(&sql, "KEYTYPE", condition[i].compare,
                    condition[i].data.number, i);
                break;

            case DB_KEYDATA_STATE:
                DqsConditionInt(&sql, "STATE", condition[i].compare,
                    condition[i].data.number, i);
                break;

            case DB_KEYDATA_ZONE_ID:
                DqsConditionInt(&sql, "ZONE_ID", condition[i].compare,
                    condition[i].data.number, i);
                break;

            default:

                /* Warn about unrecognised condition code */

                MsgLog(KME_UNRCONCOD, condition[i].code);
            }
        }
    }
    DqsEnd(&sql);

    /* Execute query and free up the query string */

    status = KsmKeyInitSql(result, sql);
    DqsFree(sql);

    return status;
}



/*+
 * KsmKeyInitId - Query for Key Information by ID
 *
 * Description:
 *      Performs a query for a key in the zone table that matches the
 *      given ID.
 *
 * Arguments:
 *      DB_RESULT* result
 *          Pointer to a result to be used for information retrieval.  Will
 *          be NULL on error.
 *
 *      DB_ID id
 *          ID of the object.
 *
 * Returns:
 *      int
 *          Status return.  0 on success.
-*/

int KsmKeyInitId(DB_RESULT* result, DB_ID id)
{
    DQS_QUERY_CONDITION condition[2];   /* Condition for query */

    /* Initialize */

    condition[0].code = DB_KEYDATA_ID;
    condition[0].compare = DQS_COMPARE_EQ;
    condition[0].data.number = (int) id;

    condition[1].compare = DQS_END_OF_LIST;

    return KsmKeyInit(result, condition);
}



/*+
 * KsmKey - Return Key Information
 *
 * Description:
 *      Returns information about the next key in the result set.
 *
 * Arguments:
 *      DB_RESULT result
 *          Handle from KsmKeyInit
 *
 *      KSM_KEYDATA* data
 *          Data is returned in here.
 *
 * Returns:
 *      int
 *          Status return:
 *              0           success
 *              -1          end of record set reached
 *              non-zero    some error occurred and a message has been output.
 *
 *          If the status is non-zero, the returned data is meaningless.
-*/

int KsmKey(DB_RESULT result, KSM_KEYDATA* data)
{
    DB_ROW      row;            /* Row data */
    int         status = 0;     /* Return status */

	/* Initialize */

	memset(data, 0, sizeof(KSM_KEYDATA));

    /* Get the next row from the data and copy data across */

	status = DbFetchRow(result, &row);

	if (status == 0) {
        status = DbUnsignedLong(row, DB_KEYDATA_ID, &(data->keypair_id));
	}

	if (status == 0) {
        status = DbInt(row, DB_KEYDATA_STATE, &(data->state));
	}

	if (status == 0) {
        status = DbStringBuffer(row, DB_KEYDATA_GENERATE,
            data->generate, sizeof(data->generate));
	}

	if (status == 0) {
        status = DbStringBuffer(row, DB_KEYDATA_PUBLISH,
            data->publish, sizeof(data->publish));
	}

	if (status == 0) {
        status = DbStringBuffer(row, DB_KEYDATA_READY,
            data->ready, sizeof(data->ready));
	}

	if (status == 0) {
        status = DbStringBuffer(row, DB_KEYDATA_ACTIVE,
            data->active, sizeof(data->active));
	}

	if (status == 0) {
        status = DbStringBuffer(row, DB_KEYDATA_RETIRE,
            data->retire, sizeof(data->retire));
	}

	if (status == 0) {
        status = DbStringBuffer(row, DB_KEYDATA_DEAD,
            data->dead, sizeof(data->dead));
	}

	if (status == 0) {
        status = DbInt(row, DB_KEYDATA_KEYTYPE, &(data->keytype));
	}

	if (status == 0) {
        status = DbInt(row, DB_KEYDATA_ALGORITHM, &(data->algorithm));
	}

/*	if (status == 0) {
        status = DbInt(row, DB_KEYDATA_SIGLIFETIME, &(data->siglifetime));
	}
*/
	if (status == 0) {
        status = DbStringBuffer(row, DB_KEYDATA_LOCATION,
            data->location, sizeof(data->location));
    }

	DbFreeRow(row);

    return status;
}


/*+
 * KsmKeyEnd - End Key Information
 *
 * Description:
 *      Called at the end of a ksm_key cycle, frees up the stored
 *      result set.
 *
 *      N.B. This does not clear stored error information, so allowing it
 *      to be called after a failure return from KsmKey to free up database
 *      context whilst preserving the reason for the error.
 *
 * Arguments:
 *      DB_RESULT result
 *          Handle from KsmKeyInit
-*/

void KsmKeyEnd(DB_RESULT result)
{
    DbFreeResult(result);
}



/*+
 * KsmKeyData - Return Data for Key
 *
 * Description:
 *      Returns data for the named Key.
 *
 * Arguments:
 *      DB_ID id
 *          Name/ID of the Key.
 *
 *      KSM_GROUP* data
 *          Data for the Key.
 *
 * Returns:
 *      int
 *          Status return.  One of:
 *
 *              0       Success
 *              -1      Key not found
 *              Other   Error
-*/

int KsmKeyData(DB_ID id, KSM_KEYDATA* data)
{
    DB_RESULT   result;     /* Handle to the data */
    int         status;     /* Status return code */

    status = KsmKeyInitId(&result, id);
    if (status == 0) {

        /* Retrieve the key data */

        status = KsmKey(result, data);
        (void) KsmKeyEnd(result);
    }
    /*
     * else {
     *      On error, a message will have been output
     * }
     */

    return status;
}

/*+
 * KsmKeyPredict - predict how many keys are needed
 *
 * Description:
 *      Given a policy and a keytype work out how many keys will be required
 *      during the timeinterval specified (in seconds).
 *
 *      We assume no emergency rollover and that a key has just been published
 *
 *      Dt	= interval
 *      Sp	= safety margin
 *      Lk	= lifetime of the key (either KSK or ZSK)
 *      Ek  = no of emergency keys
 *
 *      no of keys = ( (Dt + Sp)/Lk ) + Ek
 *      
 *      (rounded up)
 *
 * Arguments:
 *      int policy_id
 *          The policy in question
 *      KSM_TYPE key_type
 *          KSK or ZSK
 *      int shared_keys
 *          0 if keys not shared between zones
 *      int interval
 *          timespan (in seconds)
 *      int *count
 *          (OUT) the number of keys (-1 on error)
 *
 * Returns:
 *      int
 *          Status return.  One of:
 *
 *              0       Success
 *              Other   Error
-*/

int ksmKeyPredict(int policy_id, int keytype, int shared_keys, int interval, int *count)
{
    int			    status = 0;		/* Status return */
	KSM_PARCOLL     coll; /* Parameters collection */

    DB_RESULT result;
	int zone_count = 0;

    /* how many zones on this policy */
    status = KsmZoneCountInit(&result, policy_id);
    if (status == 0) {
        status = KsmZoneCount(result, &zone_count);
    }
    DbFreeResult(result);

    if (status == 0) {
        /* make sure that we have at least one zone */
        if (zone_count == 0) {
            *count = 0;
            return status;
        }
    } else {
        *count = -1;
        return status;
    }

    /* Check that we have a valid key type */
    if ((keytype != KSM_TYPE_KSK) && (keytype != KSM_TYPE_ZSK)) {
		status = MsgLog(KME_UNKEYTYPE, keytype);
		return status;
	}

    /* Get list of parameters */
    status = KsmParameterCollection(&coll, policy_id);
    if (status != 0) {
        *count = -1;
        return status;
    }

    /* We should have the policy now */
    if (keytype == KSM_TYPE_KSK)
    {
        *count = ((interval + coll.pub_safety)/coll.ksklife) + coll.nemkskeys + 1;
    }
    else if (keytype == KSM_TYPE_ZSK)
    {
        *count = ((interval + coll.pub_safety)/coll.zsklife) + coll.nemzskeys + 1;
    }

    if (shared_keys == KSM_KEYS_NOT_SHARED) {
        *count *= zone_count;
    }

    return status;
}
