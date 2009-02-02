/*
 * $Id$
 *
 * Copyright (c) 2009 Nominet UK.
 * All rights reserved.
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
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <security/pkcs11_unix.h>

// typedef unsigned char BOOL;
static CK_BBOOL    true    = CK_TRUE;
static CK_BBOOL    false   = CK_FALSE;

/*

 The working of hsm-toolkit is straightforward.
 o  If no arguments are given, the toolkit uses the following defaults:
 o  It prompts for the PIN, reads slot:0 for keys.
 o
 o  When generating keys, it will first search the slot to see if the label
 o  already exists.
 o
*/

typedef struct {CK_RV rv;const char *rv_str;}
error_table;

error_table error_str[] =
{
    { CKR_OK, "CKR_OK"},
    { CKR_CANCEL, "CKR_CANCEL"},
    { CKR_HOST_MEMORY, "CKR_HOST_MEMORY"},
    { CKR_SLOT_ID_INVALID, "CKR_SLOT_ID_INVALID"},
    { CKR_GENERAL_ERROR, "CKR_GENERAL_ERROR"},
    { CKR_FUNCTION_FAILED, "CKR_FUNCTION_FAILED"},
    { CKR_ARGUMENTS_BAD, "CKR_ARGUMENTS_BAD"},
    { CKR_NO_EVENT, "CKR_NO_EVENT"},
    { CKR_NEED_TO_CREATE_THREADS, "CKR_NEED_TO_CREATE_THREADS"},
    { CKR_CANT_LOCK, "CKR_CANT_LOCK"},
    { CKR_ATTRIBUTE_READ_ONLY, "CKR_ATTRIBUTE_READ_ONLY"},
    { CKR_ATTRIBUTE_SENSITIVE, "CKR_ATTRIBUTE_SENSITIVE"},
    { CKR_ATTRIBUTE_TYPE_INVALID, "CKR_ATTRIBUTE_TYPE_INVALID"},
    { CKR_ATTRIBUTE_VALUE_INVALID, "CKR_ATTRIBUTE_VALUE_INVALID"},
    { CKR_DATA_INVALID, "CKR_DATA_INVALID"},
    { CKR_DATA_LEN_RANGE, "CKR_DATA_LEN_RANGE"},
    { CKR_DEVICE_ERROR, "CKR_DEVICE_ERROR"},
    { CKR_DEVICE_MEMORY, "CKR_DEVICE_MEMORY"},
    { CKR_DEVICE_REMOVED, "CKR_DEVICE_REMOVED"},
    { CKR_ENCRYPTED_DATA_INVALID, "CKR_ENCRYPTED_DATA_INVALID"},
    { CKR_ENCRYPTED_DATA_LEN_RANGE, "CKR_ENCRYPTED_DATA_LEN_RANGE"},
    { CKR_FUNCTION_CANCELED, "CKR_FUNCTION_CANCELED"},
    { CKR_FUNCTION_NOT_PARALLEL, "CKR_FUNCTION_NOT_PARALLEL"},
    { CKR_FUNCTION_NOT_SUPPORTED, "CKR_FUNCTION_NOT_SUPPORTED"},
    { CKR_KEY_HANDLE_INVALID, "CKR_KEY_HANDLE_INVALID"},
    { CKR_KEY_SIZE_RANGE, "CKR_KEY_SIZE_RANGE"},
    { CKR_KEY_TYPE_INCONSISTENT, "CKR_KEY_TYPE_INCONSISTENT"},
    { CKR_KEY_NOT_NEEDED, "CKR_KEY_NOT_NEEDED"},
    { CKR_KEY_CHANGED, "CKR_KEY_CHANGED"},
    { CKR_KEY_NEEDED, "CKR_KEY_NEEDED"},
    { CKR_KEY_INDIGESTIBLE, "CKR_KEY_INDIGESTIBLE"},
    { CKR_KEY_FUNCTION_NOT_PERMITTED, "CKR_KEY_FUNCTION_NOT_PERMITTED"},
    { CKR_KEY_NOT_WRAPPABLE, "CKR_KEY_NOT_WRAPPABLE"},
    { CKR_KEY_UNEXTRACTABLE, "CKR_KEY_UNEXTRACTABLE"},
    { CKR_MECHANISM_INVALID, "CKR_MECHANISM_INVALID"},
    { CKR_MECHANISM_PARAM_INVALID, "CKR_MECHANISM_PARAM_INVALID"},
    { CKR_OBJECT_HANDLE_INVALID, "CKR_OBJECT_HANDLE_INVALID"},
    { CKR_OPERATION_ACTIVE, "CKR_OPERATION_ACTIVE"},
    { CKR_OPERATION_NOT_INITIALIZED, "CKR_OPERATION_NOT_INITIALIZED"},
    { CKR_PIN_INCORRECT, "CKR_PIN_INCORRECT"},
    { CKR_PIN_INVALID, "CKR_PIN_INVALID"},
    { CKR_PIN_LEN_RANGE, "CKR_PIN_LEN_RANGE"},
    { CKR_PIN_EXPIRED, "CKR_PIN_EXPIRED"},
    { CKR_PIN_LOCKED, "CKR_PIN_LOCKED"},
    { CKR_SESSION_CLOSED, "CKR_SESSION_CLOSED"},
    { CKR_SESSION_COUNT, "CKR_SESSION_COUNT"},
    { CKR_SESSION_HANDLE_INVALID, "CKR_SESSION_HANDLE_INVALID"},
    { CKR_SESSION_PARALLEL_NOT_SUPPORTED, "CKR_SESSION_PARALLEL_NOT_SUPPORTED"},
    { CKR_SESSION_READ_ONLY, "CKR_SESSION_READ_ONLY"},
    { CKR_SESSION_EXISTS, "CKR_SESSION_EXISTS"},
    { CKR_SESSION_READ_ONLY_EXISTS, "CKR_SESSION_READ_ONLY_EXISTS"},
    { CKR_SESSION_READ_WRITE_SO_EXISTS, "CKR_SESSION_READ_WRITE_SO_EXISTS"},
    { CKR_SIGNATURE_INVALID, "CKR_SIGNATURE_INVALID"},
    { CKR_SIGNATURE_LEN_RANGE, "CKR_SIGNATURE_LEN_RANGE"},
    { CKR_TEMPLATE_INCOMPLETE, "CKR_TEMPLATE_INCOMPLETE"},
    { CKR_TEMPLATE_INCONSISTENT, "CKR_TEMPLATE_INCONSISTENT"},
    { CKR_TOKEN_NOT_PRESENT, "CKR_TOKEN_NOT_PRESENT"},
    { CKR_TOKEN_NOT_RECOGNIZED, "CKR_TOKEN_NOT_RECOGNIZED"},
    { CKR_TOKEN_WRITE_PROTECTED, "CKR_TOKEN_WRITE_PROTECTED"},
    { CKR_UNWRAPPING_KEY_HANDLE_INVALID, "CKR_UNWRAPPING_KEY_HANDLE_INVALID"},
    { CKR_UNWRAPPING_KEY_SIZE_RANGE, "CKR_UNWRAPPING_KEY_SIZE_RANGE"},
    { CKR_UNWRAPPING_KEY_TYPE_INCONSISTENT, "CKR_UNWRAPPING_KEY_TYPE_INCONSISTENT"},
    { CKR_USER_ALREADY_LOGGED_IN, "CKR_USER_ALREADY_LOGGED_IN"},
    { CKR_USER_NOT_LOGGED_IN, "CKR_USER_NOT_LOGGED_IN"},
    { CKR_USER_PIN_NOT_INITIALIZED, "CKR_USER_PIN_NOT_INITIALIZED"},
    { CKR_USER_TYPE_INVALID, "CKR_USER_TYPE_INVALID"},
    { CKR_USER_ANOTHER_ALREADY_LOGGED_IN, "CKR_USER_ANOTHER_ALREADY_LOGGED_IN"},
    { CKR_USER_TOO_MANY_TYPES, "CKR_USER_TOO_MANY_TYPES"},
    { CKR_WRAPPED_KEY_INVALID, "CKR_WRAPPED_KEY_INVALID"},
    { CKR_WRAPPED_KEY_LEN_RANGE, "CKR_WRAPPED_KEY_LEN_RANGE"},
    { CKR_WRAPPING_KEY_HANDLE_INVALID, "CKR_WRAPPING_KEY_HANDLE_INVALID"},
    { CKR_WRAPPING_KEY_SIZE_RANGE, "CKR_WRAPPING_KEY_SIZE_RANGE"},
    { CKR_WRAPPING_KEY_TYPE_INCONSISTENT, "CKR_WRAPPING_KEY_TYPE_INCONSISTENT"},
    { CKR_RANDOM_SEED_NOT_SUPPORTED, "CKR_RANDOM_SEED_NOT_SUPPORTED"},
    { CKR_RANDOM_NO_RNG, "CKR_RANDOM_NO_RNG"},
    { CKR_DOMAIN_PARAMS_INVALID, "CKR_DOMAIN_PARAMS_INVALID"},
    { CKR_BUFFER_TOO_SMALL, "CKR_BUFFER_TOO_SMALL"},
    { CKR_SAVED_STATE_INVALID, "CKR_SAVED_STATE_INVALID"},
    { CKR_INFORMATION_SENSITIVE, "CKR_INFORMATION_SENSITIVE"},
    { CKR_STATE_UNSAVEABLE, "CKR_STATE_UNSAVEABLE"},
    { CKR_CRYPTOKI_NOT_INITIALIZED, "CKR_CRYPTOKI_NOT_INITIALIZED"},
    { CKR_CRYPTOKI_ALREADY_INITIALIZED, "CKR_CRYPTOKI_ALREADY_INITIALIZED"},
    { CKR_MUTEX_BAD, "CKR_MUTEX_BAD"},
    { CKR_MUTEX_NOT_LOCKED, "CKR_MUTEX_NOT_LOCKED"},
    { CKR_NEW_PIN_MODE, "CKR_NEW_PIN_MODE"},
    { CKR_NEXT_OTP, "CKR_NEXT_OTP"},
    { CKR_FUNCTION_REJECTED, "CKR_FUNCTION_REJECTED"}
};

const char*
get_rv_str(CK_RV rv)
{
    int i=0;
    while(error_str[i].rv_str != NULL_PTR)
    {
        if (error_str[i].rv == rv) return error_str[i].rv_str;
        i++;
    }
    return NULL;
}

/*
 * Handles return values from PKCS11 functions
 *
 * if return value is not CKR_OK (0x00000000), the function will exit.
 * for convenience, a message can be displayed alongside the error message.
 */

void
check_rv (const char *message,CK_RV rv)
{
    if (rv != CKR_OK)
    {
        fprintf (stderr, "Error %s in %s\n", get_rv_str(rv), message);
        exit (1);
    }
}

CK_ULONG
LabelExists(CK_SESSION_HANDLE ses, CK_UTF8CHAR* label)
{
    // build template
    CK_ATTRIBUTE search[] =
    {
        {
            CKA_LABEL, label, strlen ((char *) label)
        }
    };
    CK_ULONG count = 0;
    CK_OBJECT_HANDLE key;

    CK_RV rv = C_FindObjectsInit (ses, search, 1);
    check_rv("C_FindObjectsInit", rv );

    rv = C_FindObjects(ses, &key, 1, &count);
    check_rv("C_FindObjects", rv );

    rv = C_FindObjectsFinal(ses);
    check_rv("C_FindObjectsFinal", rv );
    return count;
}

void
ActionRemoveObject(CK_SESSION_HANDLE ses, CK_UTF8CHAR* label)
{
    if (label==NULL_PTR)
    {
        fprintf (stderr, "No label specified.\n");
        exit (1);
    }

    if (!LabelExists(ses,label))
    {
        fprintf (stderr, "Object with label '%s' does not exist.\n",(char*)label);
        exit (1);
    }

    CK_ATTRIBUTE search[] =
    {
        {
            CKA_LABEL, label, strlen ((char *) label)
        }
    };
    CK_OBJECT_CLASS class = 0;
    CK_ATTRIBUTE attributes[] =
    {
        {CKA_CLASS, &class, sizeof(class)},
    };

    CK_ULONG count = 0;
    CK_OBJECT_HANDLE object;

    CK_RV rv = C_FindObjectsInit (ses, search, 1);
    check_rv("C_FindObjectsInit", rv );

    while (1)
    {
        rv = C_FindObjects(ses, &object, 1, &count);
        check_rv("C_FindObjects", rv);
        if (count == 0) break;
        rv = C_GetAttributeValue(ses, object, attributes, 1);
        check_rv("C_GetAttributeValue",rv);

        rv = C_DestroyObject(ses, object);
        check_rv("C_DestroyObject",rv);
        printf("Destroyed %s key object, labeled %s\n",(class == CKO_PRIVATE_KEY)?"Private":"Public ",label);
    }

    rv = C_FindObjectsFinal(ses);
    check_rv("C_FindObjectsFinal", rv );
}

void
ActionListObjects(CK_SESSION_HANDLE ses, CK_UTF8CHAR* label)
{
    CK_RV rv;
    if (label != NULL_PTR)
    {
        // list one
        CK_ATTRIBUTE search[] =
        {
            {
                CKA_LABEL, label, strlen ((char *) label)
            }
        };

        rv = C_FindObjectsInit (ses, search, 1);
    }
    else
    {
        // list all
        rv = C_FindObjectsInit (ses, NULL_PTR, 0);
    }
    check_rv("C_FindObjectsInit", rv );

    CK_OBJECT_HANDLE object;
    CK_OBJECT_CLASS class = 0;
    CK_ULONG found = 0;
    rv = C_FindObjects(ses, &object, 1, &found);
    check_rv("C_FindObjects", rv);
    while (found)
    {
        CK_ATTRIBUTE attributes[] =
        {
            {CKA_CLASS, &class, sizeof(class)},
            {CKA_LABEL, NULL_PTR, 0},
            {CKA_MODULUS, NULL_PTR, 0}
        };
        rv = C_GetAttributeValue(ses, object, attributes, 3);
        check_rv("C_GetAttributeValue",rv);
        int i=1;
        for (;i<3;i++)
        {
            attributes[i].pValue = calloc(attributes[i].ulValueLen+1,1);
        }
        rv = C_GetAttributeValue(ses, object, attributes, 3);
        check_rv("C_GetAttributeValue",rv);

        printf("%d-bit %s key object, labeled %s\n",
            (int) attributes[2].ulValueLen *8,
            (class == CKO_PRIVATE_KEY)?"Private":"Public ",
            (char*)attributes[1].pValue);
        free(attributes[1].pValue);
        free(attributes[2].pValue);
        rv = C_FindObjects(ses, &object, 1, &found);
        check_rv("C_FindObjects", rv);
    }
    rv = C_FindObjectsFinal(ses);
    check_rv("C_FindObjectsFinal", rv );
}

void
ActionGenerateObject(CK_SESSION_HANDLE ses, CK_UTF8CHAR* label, CK_ULONG keysize)
{
	if (keysize <512)
	{
        fprintf (stderr, "Keysize (%u) too small.\n",(int)keysize);
        exit (1);
    }
    if (label==NULL_PTR)
    {
        fprintf (stderr, "No label specified.\n");
        exit (1);
    }

    if (LabelExists(ses,label))
    {
        fprintf (stderr, "Key with label '%s' already exists.\n",(char*)label);
        exit (1);
    }

    CK_MECHANISM mech = { CKM_RSA_PKCS_KEY_PAIR_GEN, NULL_PTR, 0 };

    /* A template to generate an RSA public key objects*/
    CK_BYTE pubex[] = { 1, 0, 1 };
    CK_KEY_TYPE keyType = CKK_RSA;
    CK_ATTRIBUTE publickey_template[] =
    {
        {CKA_LABEL, label, strlen ((char *) label)},
        {CKA_KEY_TYPE, &keyType, sizeof(keyType)},
        {CKA_VERIFY, &true, sizeof (true)},
        {CKA_ENCRYPT, &true, sizeof (false)},
        {CKA_WRAP, &true, sizeof (false)},
        {CKA_TOKEN, &true, sizeof (true)},
        {CKA_MODULUS_BITS, &keysize, sizeof (keysize)},
        {CKA_PUBLIC_EXPONENT, &pubex, sizeof (pubex)}
    };

    /* A template to generate an RSA private key objects*/
    CK_ATTRIBUTE privatekey_template[] =
    {
        {CKA_LABEL, label, strlen ((char *) label)},
        {CKA_KEY_TYPE, &keyType, sizeof(keyType)},
        {CKA_SIGN, &true, sizeof (true)},
        {CKA_DECRYPT, &true, sizeof (true)},
        {CKA_TOKEN, &true, sizeof (true)},
        {CKA_PRIVATE, &true, sizeof (true)},
        {CKA_SENSITIVE, &false, sizeof (false)},
        {CKA_UNWRAP, &true, sizeof (true)},
        {CKA_EXTRACTABLE, &true, sizeof (true)}
    };
    CK_OBJECT_HANDLE ignore;
    CK_RV rv = C_GenerateKeyPair(ses, &mech, publickey_template, 8,
        privatekey_template, 9, &ignore,&ignore);
    check_rv("C_GenerateKeyPair", rv);
    printf("Created RSA key pair object, labeled %s\n",label);
}

void
ActionChangePin(CK_SESSION_HANDLE ses, CK_UTF8CHAR* oldpin)
{
    CK_RV rv;
    CK_UTF8CHAR* newpin;
    newpin = (CK_UTF8CHAR *) getpass ("Enter New Pin: ");
    rv = C_SetPIN(ses, oldpin, sizeof(oldpin), newpin,
        sizeof(newpin));
    check_rv("C_GenerateKeyPair", rv);
}

int
main (int argc, char *argv[])
{
    CK_UTF8CHAR *pin    = NULL_PTR;               // NO DEFAULT VALUE
    CK_UTF8CHAR *label  = NULL_PTR;               // NO DEFAULT VALUE
    CK_SLOT_ID  slot    = 0;                      // default value
    CK_ULONG    keysize = 1024;                   // default value
    CK_SESSION_HANDLE ses;
    int         Action  = 0;
    int opt;
    while ((opt = getopt (argc, argv, "CGRLb:p:s:h")) != -1)
    {
        switch (opt)
        {
            case 'G': Action = 1; break;
			case 'R': Action = 2; break;
			case 'C': Action = 3; break;
            case 'b': keysize = atoi (optarg); break;
            case 'p': pin = (CK_UTF8CHAR*)optarg; break;
            case 's': slot = atoi (optarg); break;
            case 'h': fprintf (stderr,
	            "usage: hsm-toolkit [-s slot] [-p pin] [-G [-b keysize] label] [-R label]\n");
	        	exit (2);
			
        }
    }

    label = (CK_UTF8CHAR *) argv[optind];
    check_rv("C_Initialize",C_Initialize (NULL_PTR));
    check_rv("C_OpenSession",C_OpenSession (slot, CKF_RW_SESSION + CKF_SERIAL_SESSION, NULL_PTR, NULL_PTR, &ses));

    if (!pin) pin = (CK_UTF8CHAR *) getpass ("Enter Pin: ");

    check_rv("C_Login", C_Login(ses, CKU_USER, pin, strlen ((char*)pin)));
    memset(pin, 0, strlen((char *)pin));
    switch (Action)
    {
        case 1: ActionGenerateObject(ses,label,keysize); break;
        case 2: ActionRemoveObject(ses,label); break;
		case 3: ActionChangePin(ses,pin); break;
        default:
            ActionListObjects(ses,label);
    }
    check_rv("C_Logout", C_Logout(ses));
    check_rv("C_CloseSession", C_CloseSession(ses));
    check_rv("C_Finalize", C_Finalize (NULL_PTR));
    exit (0);
}
