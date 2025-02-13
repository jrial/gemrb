/* GemRB - Infinity Engine Emulator
 * Copyright (C) 2003 The GemRB Project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 */

#ifndef TLKIMPORTER_H
#define TLKIMPORTER_H


#include "StringMgr.h"
#include "Variables.h"
#include "TlkOverride.h"

namespace GemRB {

class TLKImporter : public StringMgr {
private:
	DataStream* str = nullptr;

	//Data
	ieWord Language = 0;
	ieDword StrRefCount = 0;
	ieDword Offset = 0;
	CTlkOverride *OverrideTLK = nullptr;
	Variables gtmap;
	int charname = 0;

public:
	TLKImporter(void);
	~TLKImporter(void) override;
	/** open string refs coming from saved game */
	void OpenAux() override;
	/** purge string defs coming from saved game */
	void CloseAux() final;
	bool Open(DataStream* stream) override;
	/** construct a new custom string */
	ieStrRef UpdateString(ieStrRef strref, const char *newvalue) override;
	/** resolve a string reference */
	String* GetString(ieStrRef strref, ieDword flags = 0) override;
	char* GetCString(ieStrRef strref, ieDword flags = 0) override;
	StringBlock GetStringBlock(ieStrRef strref, unsigned int flags = 0) override;
	bool HasAltTLK() const override;
private:
	/** resolves day and monthname tokens */
	void GetMonthName(int dayandmonth);
	/** replaces tags in dest, don't exceed Length */
	bool ResolveTags(char* dest, const char* source, size_t Length);
	/** returns the needed length in Length, 
		if there was no token, returns false */
	bool GetNewStringLength(const char* string, size_t& Length);
	/**returns the decoded length of the built-in token
		 if dest is not NULL it also returns the decoded value */
	int BuiltinToken(const char* Token, char* dest);
	int ClassStrRef(int slot) const;
	int RaceStrRef(int slot) const;
	int GenderStrRef(int slot, int malestrref, int femalestrref) const;
	char *Gabber() const;
	char *CharName(int slot) const;
};

}

#endif
