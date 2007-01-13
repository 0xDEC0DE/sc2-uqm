/*  alist.c, Copyright (c) 2005 Michael C. Martin */

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope thta it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  Se the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
//#include "libs/reslib.h"
#include "alist.h"
#include "stringbank.h"

alist_entry *
AlistEntry_New (const char *key, const char *value)
{
	alist_entry *e;
	e = malloc (sizeof(alist_entry));
	e->key = key;
	e->value = value;
	e->next = NULL;
	return e;
}

void
AlistEntry_Free (alist_entry *e)
{
	if (e == NULL)
		return;
	AlistEntry_Free (e->next);
	free (e);
}

alist *
Alist_New (void)
{
	alist *result = malloc (sizeof(alist));
	result->first = NULL;
	return result;
}

void
Alist_Free (alist *m) {
	if (m == NULL) return;
	AlistEntry_Free (m->first);
	free (m);
}

alist_entry *
Alist_GetEntry (alist *m, const char *key)
{
	alist_entry *x = m->first;
	while (x != NULL) {
		if (!strcmp (x->key, key))
			return x;
		x = x->next;
	}
	return NULL;
}

const char *
Alist_GetString (alist *m, const char *key)
{
	alist_entry *x = Alist_GetEntry (m, key);
	return x ? x->value : NULL;
}

void
Alist_PutString (alist *m, const char *key, const char *value)
{
	alist_entry *e = Alist_GetEntry (m, key);
	if (e == NULL) {
		e = AlistEntry_New(key, value);
		e->next = m->first;
		m->first = e;
	} else {
		e->value = value;
	}
}

void
Alist_PutAll (alist *dst, alist *src)
{
	alist_entry *e = src->first;
	while (e) {
		Alist_PutString (dst, e->key, e->value);
		e = e->next;
	}
}

alist *
Alist_New_FromFile (FILE *f)
{
	long flen;
	alist *m;
	char *data;

	fseek (f, 0, SEEK_END);
	flen = ftell (f);
	fseek (f, 0, SEEK_SET);

	data = malloc (flen + 1);
	if (!data) {
		return NULL;
	}

	fread (data, 1, flen, f);
	data[flen] = '\0';

	m = Alist_New_FromString (data);
	free (data);
	return m;
}

alist *
Alist_New_FromFilename (const char *fname)
{
	alist *result;
	FILE *f = fopen (fname, "rt");
	if (!f) {
		return NULL;
	}
	result = Alist_New_FromFile (f);
	fclose(f);
	return result;
}

alist *
Alist_New_FromString (char *d)
{
	alist *m = Alist_New ();
	int len, i;
	if (!m) return NULL;

	len = strlen(d);
	i = 0;
	while (i < len) {
		int key_start, key_end, value_start, value_end;
		/* Starting a line: search for non-whitespace */
		while ((i < len) && isspace (d[i])) i++;
		if (i >= len) break;  /* Done parsing! */
		/* If it was a comment, skip to end of comment/file */
		if (d[i] == '#') {
			while ((i < len) && (d[i] != '\n')) i++;
			if (i >= len) break;
			continue; /* Back to keyword search */
		}
		key_start = i;
		/* Find the = on this line */
		while ((i < len) && (d[i] != '=') &&
		       (d[i] != '\n') && (d[i] != '#')) i++;
		if (i >= len) {  /* Bare key at EOF */
			fprintf (stderr, "Warning: Bare keyword at EOF");
			break;
		}
		/* Comments here mean incomplete line too */
		if (d[i] != '=') {
			fprintf (stderr, "Warning: Key without value");
			while ((i < len) && (d[i] != '\n')) i++;
			if (i >= len) break;
			continue; /* Back to keyword search */
		}
		/* Key ends at first whitespace before = , or at key_start*/
		key_end = i;
		while ((key_end > key_start) && isspace (d[key_end-1]))
			key_end--;
		
		/* Consume the = */
		i++;
		/* Value starts at first non-whitespace after = on line... */
		while ((i < len) && (d[i] != '#') && (d[i] != '\n') &&
		       isspace (d[i]))
			i++;
		value_start = i;
		/* Until first non-whitespace before terminator */
		while ((i < len) && (d[i] != '#') && (d[i] != '\n'))
			i++;
		value_end = i;
		while ((value_end > value_start) && isspace (d[value_end-1]))
			value_end--;
		/* Skip past EOL or EOF */
		while ((i < len) && (d[i] != '\n'))
			i++;
		i++;

		/* We now have start and end values for key and value.
		   We terminate the strings for both by writing \0s, then
		   make a new map entry. */
		d[key_end] = '\0';
		d[value_end] = '\0';
		Alist_PutString (m, StringBank_AddOrFindString(d+key_start), 
				  StringBank_AddOrFindString(d+value_start));
	}
	return m;
}

void
Alist_Dump (alist *m, FILE *s, const char *prefix)
{
	alist_entry *e = m->first;
	int prefix_len = 0;
	if (prefix)
		prefix_len = strlen (prefix);
	while (e) {
		if (!prefix || !strncmp (prefix, e->key, prefix_len)) {
			fwrite (e->key, 1, strlen (e->key), s);
			fputc (' ', s);
			fputc ('=', s);
			fputc (' ', s);
			fwrite (e->value, 1, strlen (e->value), s);
			fputc ('\n', s);
		}
		e = e->next;
	}
	return;
}
