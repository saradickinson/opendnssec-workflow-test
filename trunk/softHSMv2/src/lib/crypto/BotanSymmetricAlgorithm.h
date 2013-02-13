/* $Id$ */

/*
 * Copyright (c) 2010 .SE (The Internet Infrastructure Foundation)
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

/*****************************************************************************
 BotanSymmetricAlgorithm.h

 Botan symmetric algorithm implementation
 *****************************************************************************/

#ifndef _SOFTHSM_V2_BOTANSYMMETRICALGORITHM_H
#define _SOFTHSM_V2_BOTANSYMMETRICALGORITHM_H

#include <string>
#include "config.h"
#include "SymmetricKey.h"
#include "SymmetricAlgorithm.h"

#include <botan/pipe.h>

class BotanSymmetricAlgorithm : public SymmetricAlgorithm
{
public:
	// Constructor
	BotanSymmetricAlgorithm();

	// Destructor
	virtual ~BotanSymmetricAlgorithm();

	// Encryption functions
	virtual bool encryptInit(const SymmetricKey* key, const std::string mode = "cbc", const ByteString& IV = ByteString());
	virtual bool encryptUpdate(const ByteString& data, ByteString& encryptedData);
	virtual bool encryptFinal(ByteString& encryptedData);

	// Decryption functions
	virtual bool decryptInit(const SymmetricKey* key, const std::string mode = "cbc", const ByteString& IV = ByteString());
	virtual bool decryptUpdate(const ByteString& encryptedData, ByteString& data);
	virtual bool decryptFinal(ByteString& data);

	// Return the block size
	virtual size_t getBlockSize() const = 0;

protected:
	// Return the right cipher for the operation
	virtual std::string getCipher() const = 0;

private:
	// The current context
	Botan::Pipe* cryption;
};

#endif // !_SOFTHSM_V2_BOTANSYMMETRICALGORITHM_H

