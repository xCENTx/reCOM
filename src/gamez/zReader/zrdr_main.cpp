#include <cstring>

#include <freebsd/strcasecmp.h>

#include "zrdr.h"

#include "gamez/zSystem/zsys.h"
#include "gamez/zUtil/zutil.h"
#include "gamez/zUtil/util_systemio.h"
// #include "gamez/zWeapon/zwep_weapon.h"

bool zrdr_init = false;
bool warnonce = false;

char* cur_zrdr_path = 0;
s32 cur_zrdr_flags = 0;

_zrdr::_zrdr()
{
	type = ZRDR_NULL;
	isclone = false;
	packed = false;
	unused = 0;
	length = 0;

	array = NULL;
}

bool _zrdr::IsArray() const
{
	return type == ZRDR_ARRAY;
}

void _zrdr::Clone(const _zrdr* other, const CSTable* table)
{
	if (!zrdr_init)
	{
		warnonce = true;
		zrdr_init = true;
	}

	if (other == NULL)
	{
		return;
	}

	
	type = other->type;
	if (type == ZRDR_ARRAY)
	{

	}
}

int _zrdr::GetInt() const
{
	if (type == ZRDR_INTEGER)
	{
		return integer;
	}
	else if (type == ZRDR_REAL)
	{
		return static_cast<s32>(real);
	}

	return 0;
}

char* _zrdr::Get(s32 offset) const
{
	s32 length = 0;

	if (type == ZRDR_ARRAY)
	{
		length = array->integer - 1;
	}

	if (offset < length)
	{
		return string + (offset + 1) * 8;
	}

	return NULL;
}

_zrdr* zrdr_findtag(_zrdr* reader, const char* name)
{
	if (reader != NULL && reader->type == ZRDR_ARRAY)
	{
		s32 iterations = 0;
		for (s32 max_depth = 8; iterations < reader->array->integer; iterations++)
		{
			auto rdr = reader->array;

			if (rdr->type == ZRDR_ARRAY)
			{
				rdr = zrdr_findtag_startidx(rdr, name, 1);

				if (rdr != NULL)
				{
					return rdr;
				}
			}
			else if (rdr->type == ZRDR_STRING && strcmp(rdr->string, name) == 0)
			{
				return rdr + 1;
			}

			max_depth += 8;
		}
	}

	return NULL;
}

_zrdr* zrdr_findtag_startidx(_zrdr* reader, const char* name, s32 iterations)
{
	if (reader != NULL && reader->type == ZRDR_ARRAY)
	{
		for (s32 max_depth = iterations << 3; iterations < reader->array->integer; iterations++)
		{
			auto rdr = reader->array;

			if (rdr->type == ZRDR_ARRAY)
			{
				rdr = zrdr_findtag_startidx(rdr, name, 1);

				if (rdr != NULL)
				{
					return rdr;
				}
			}
			else if (rdr->type == ZRDR_STRING && strcmp(rdr->string, name) == 0)
			{
				return rdr + 1;
			}

			max_depth += 8;
		}
	}

	return NULL;
}

char* zrdr_findstring(_zrdr* reader, const char* name)
{
	auto tag = zrdr_findtag_startidx(reader, name, 1);

	if (tag == NULL)
	{
		return NULL;
	}
	else if (tag->type == ZRDR_TYPE::ZRDR_STRING)
	{
		return tag->string;
	}
	else if (tag->type == ZRDR_TYPE::ZRDR_ARRAY && tag->array->type == ZRDR_STRING)
	{
		return tag->array->string;
	}
	else
	{
		return NULL;
	}

	return NULL;
}

bool zrdr_findreal(_zrdr* reader, const char* tag, f32* output, s32 iterations)
{
	return false;
}

bool zrdr_finduint(_zrdr* reader, const char* tag, u32* output, s32 iterations)
{
	return false;
}

bool zrdr_findbool(_zrdr* reader, const char* tag, bool* output)
{
	_zrdr* rdr = zrdr_findtag_startidx(reader, tag, 1);
	return zrdr_tobool(rdr, output);
}

bool zrdr_toINT(_zrdr* reader, s32* value, s32 index)
{
	if (reader == NULL || reader->type != ZRDR_TYPE::ZRDR_ARRAY || reader->array->integer < index + 1)
	{
		return false;
	}


}

bool zrdr_tobool(_zrdr* reader, bool* output)
{
	if (output == NULL)
	{
		return false;
	}

	if (reader == NULL)
	{
		return false;
	}

	if (reader->type == ZRDR_ARRAY && reader->array->integer > 1)
	{
		reader = reader->array;
	}

	if (reader->type == ZRDR_INTEGER)
	{
		if (reader->integer == 0)
		{
			*output = false;
			return true;
		}
		else
		{
			*output = true;
			return true;
		}
	}

	if (reader->type == ZRDR_STRING)
	{
		if (strcasecmp(reader->string, "true") == 0 || strcasecmp(reader->string, "on"))
		{
			*output = true;
			return true;
		}
		else if (strcasecmp(reader->string, "false") == 0 || strcasecmp(reader->string, "off"))
		{
			*output = false;
			return true;
		}
	}
	
	return false;
}

_zrdr* _zrdr_nexttag(_zrdr* reader, const char* name, size_t size, _zrdr* other)
{
	s32 i = 8;
	s32 count = 1;

	while (true)
	{
		_zrdr* array = reader->array;

		if (array->integer <= count)
		{
			return NULL;
		}

		if (array == other)
		{
			other = NULL;
		}
		else if (array->type == ZRDR_ARRAY)
		{
			return _zrdr_nexttag(array, name, size, other);
		}
		else if (array->type == ZRDR_STRING && other == NULL && strncmp(array->string, name, size) == 0)
		{
			return array + 1;
		}

		i += 8;
		count++;
	}
}

void zrdr_freearray(_zrdr* reader)
{
	_zrdr* array = NULL;

	if (reader->type == ZRDR_REAL)
	{
		reader->real = 0.0f;
	}
	else if (reader->type == ZRDR_ARRAY && reader->integer != 0)
	{
		s32 it = 0;
		s32 shift = 0;

		while (true)
		{
			array = reader->array;
			auto next = array + shift;

			if (next->integer <= it)
			{
				break;
			}

			if (next->type == ZRDR_REAL)
			{
				next->real = 0.0f;
			}
			else if (next->type == ZRDR_ARRAY && next->integer != 0)
			{
				int subshift = 0;

				for (int subit = 0; subit < next->array->integer; subit++)
				{
					zrdr_freearray(array + subshift);
					subshift += 8;
				}

				zfree(array);
				next->real = 0.0f;
			}

			it++;
			shift += 8;
		}

		zfree(array);
		reader->real = 0.0f;
	}
}