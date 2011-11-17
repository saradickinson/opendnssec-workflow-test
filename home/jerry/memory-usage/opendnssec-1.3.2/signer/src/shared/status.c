/*
 * $Id$
 *
 * Copyright (c) 2009-2011 NLNet Labs. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/**
 *
 * Status.
 */

#include "config.h"
#include "mm.h"
#include "shared/status.h"

#include <stdlib.h>

ods_lookup_table ods_status_str[] = {
    { ODS_STATUS_OK, "All OK" },
    { ODS_STATUS_ASSERT_ERR, "Assertion error"},
    { ODS_STATUS_CFG_ERR, "Configuration error"},
    { ODS_STATUS_CHDIR_ERR, "Change directory failed"},
    { ODS_STATUS_CHROOT_ERR, "Change root failed"},
    { ODS_STATUS_CMDHANDLER_ERR, "Command handler error"},
    { ODS_STATUS_CONFLICT_ERR, "Conflict detected"},
    { ODS_STATUS_ERR, "General error"},
    { ODS_STATUS_FOPEN_ERR, "Unable to open file"},
    { ODS_STATUS_FORK_ERR, "fork() failed"},
    { ODS_STATUS_FREAD_ERR, "Unable to read file"},
    { ODS_STATUS_FWRITE_ERR, "Unable to write file"},
    { ODS_STATUS_HSM_ERR, "HSM error"},
    { ODS_STATUS_INSECURE, "Insecure"},
    { ODS_STATUS_MALLOC_ERR, "Memory allocation error"},
    { ODS_STATUS_PARSE_ERR, "Parse error"},
    { ODS_STATUS_PRIVDROP_ERR, "Unable to drop privileges"},
    { ODS_STATUS_RNG_ERR, "RelaxNG error"},
    { ODS_STATUS_SETSID_ERR, "setsid() failed"},
    { ODS_STATUS_UNCHANGED, "Status unchanged"},
    { ODS_STATUS_WRITE_PIDFILE_ERR, "Unable to write process id to pidfile"},
    { ODS_STATUS_XML_ERR, "XML error"},
    { 0, NULL }
};

ods_lookup_table*
ods_lookup_by_id(ods_lookup_table *table, int id)
{
    while (table->name != NULL) {
        if (table->id == id) {
            return table;
        }
        table++;
    }
    return NULL;
}


/**
 * Look up a descriptive text by each status.
 *
 */
const char *
ods_status2str(ods_status status)
{
    ods_lookup_table *lt;
    lt = ods_lookup_by_id(ods_status_str, status);
    if (lt) {
        return lt->name;
    }
    return NULL;
}
