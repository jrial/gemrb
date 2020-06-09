/* GemRB - Infinity Engine Emulator
 * Copyright (C) 2005-2006 The GemRB Project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef PALETTE_H
#define PALETTE_H

#include "RGBAColor.h"
#include "exports.h"
#include "ie_types.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>

namespace GemRB {

struct RGBModifier {
	Color rgb;
	int speed;
	int phase;
	enum Type {
		NONE,
		ADD,
		TINT,
		BRIGHTEN
	} type;
	bool locked;
};


class GEM_EXPORT Palette {
private:
	~Palette() {}
public:
	Palette(const Color &color, const Color &back);

	Palette(const Color* begin, const Color* end)
	: Palette() {
		std::copy(begin, end, col);
	}

	Palette() {
		alpha = false;
		refcount = 1;
		named = false;
		version = 0;
	}

	Color col[256]; //< RGB or RGBA 8 bit palette
	bool alpha; //< true if this is a RGBA palette
	bool named; //< true if the palette comes from a bmp and cached

	void acquire() {
		refcount++;
	}

	void release() {
		assert(refcount > 0);
		if (!--refcount)
			delete this;
	}

	unsigned short GetVersion() const { return version; }

	void CreateShadedAlphaChannel();
	void Brighten();
	void Darken();

	void SetupPaperdollColours(const ieDword* Colors, unsigned int type);
	void SetupRGBModification(const Palette* src, const RGBModifier* mods,
		unsigned int type);
	void SetupGlobalRGBModification(const Palette* src,
		const RGBModifier& mod);

	Palette* Copy() const;

	void CopyColorRange(const Color* srcBeg, const Color* srcEnd, uint8_t dst);
	bool operator==(const Palette&) const;
	bool operator!=(const Palette&) const;
private:
	unsigned int refcount;
	unsigned short version;
	// FIXME: version is not enough since `col` is public
	// must make it private to fully capture changes to it
};

}

#endif
