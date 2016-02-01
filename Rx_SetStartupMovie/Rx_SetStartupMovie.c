/**
 * Copyright (C) 2016 Jessica James.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Written by Jessica James <jessica.aj@outlook.com>
 */

#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h> // wcstombs
#include <wchar.h> // wchar_t, wcscmp
#include <stdio.h> // fopen, fclose, rename
#include <string.h> // strcpy, strcmp
#include <stdbool.h> // bool

/** Directory that movies are stored in */
#define MOVIES_DIRECTORY "..\\..\\..\\UDKGame\\Movies\\"

 /** Movie file extension */
#define MOVIE_FILE_EXTENSION ".bik"
const char movie_file_extension[] = MOVIE_FILE_EXTENSION;

/** Name of the movie which is loaded by UDK */
const char movie_filename[] = MOVIES_DIRECTORY "UDKFrontEnd.udk_loading" MOVIE_FILE_EXTENSION;

/** Prefix of the loading screens which we are to use */
const char movie_prefix[] = MOVIES_DIRECTORY "LoadingScreen_";

/** Name of the file which will contain the name of the most recently loaded level */
const char last_loaded_level_filename[] = MOVIES_DIRECTORY "LastLoaded.txt";

/** Name of the front end map */
const wchar_t frontend_level_name[] = L"RenX-FrontEndMap";

int read_file_to_str(FILE *file, char *str, size_t str_size)
{
	size_t length;
	int chr;

	if (str_size == 0)
		return 0;

	length = 0;
	while ((chr = fgetc(file)) != EOF && --str_size != 0)
	{
		*str = chr;
		++str, ++length;
	}
	*str = '\0';

	return length;
}

__declspec(dllexport) void SetStartupMovie(wchar_t *in_leaving_level, wchar_t *in_loading_level)
{
	FILE *file;
	size_t leaving_level_length;
	size_t loading_level_length;
	char leaving_level[256];
	char loading_level[256];
	char loading_level_movie_filename[256];
	char leaving_level_movie_filename[256];

	if (wcscmp(in_leaving_level, in_loading_level) == 0)
		return; // Nothing to change.

	// Copy in_loading_level to character array (interactions with files require this)
	loading_level_length = wcstombs(loading_level, in_loading_level, sizeof(loading_level));
	if (loading_level_length == sizeof(loading_level))
		--loading_level_length;
	loading_level[loading_level_length] = '\0';

	if (wcscmp(in_leaving_level, frontend_level_name) == 0)
	{
		// We're leaving the front-end map. Verify that the default loading screen isn't out of place.
		file = fopen(movie_prefix, "rb");
		if (file != NULL)
		{
			fclose(file);

			// Default movie is out of place; check what the last loaded level was
			file = fopen(last_loaded_level_filename, "rb");
			if (file != NULL)
			{
				leaving_level_length = read_file_to_str(file, leaving_level, sizeof(leaving_level) / sizeof(char));
				if (leaving_level_length == sizeof(leaving_level))
					--leaving_level_length;
				leaving_level[leaving_level_length] = '\0';
				fclose(file);
			}
			else // Last loaded level somehow wasn't stored; just force the default one into place
			{
				remove(loading_level);
				rename(movie_prefix, movie_filename);
				return;
			}

			if (strcmp(leaving_level, loading_level) == 0)
				return; // The loading movie we want is already in postion; leave it.

			goto post_init_leaving_Level;
		}
	}
	// else

	// Copy in_leaving_level to character array (interactions with files require this)
	leaving_level_length = wcstombs(leaving_level, in_leaving_level, sizeof(leaving_level));
	if (leaving_level_length == sizeof(leaving_level))
		--leaving_level_length;
	leaving_level[leaving_level_length] = '\0';

post_init_leaving_Level:

	// Store leaving_level in case the game crashes/exits later.
	file = fopen(last_loaded_level_filename, "wb");
	if (file != NULL)
	{
		fputs(loading_level, file);
		fclose(file);
	}

	strcpy(loading_level_movie_filename, movie_prefix);
	strcpy(loading_level_movie_filename + sizeof(movie_prefix) / sizeof(char) - 1, loading_level);
	strcpy(loading_level_movie_filename + sizeof(movie_prefix) / sizeof(char) - 1 + loading_level_length, movie_file_extension);

	file = fopen(loading_level_movie_filename, "rb");
	if (file != NULL) // level-specific movie exists; move it into place
	{
		fclose(file);

		// check if default movie is in position
		file = fopen(movie_prefix, "rb");
		if (file != NULL)
		{
			fclose(file);

			// A level-specific movie is currently in position; move it
			strcpy(leaving_level_movie_filename, movie_prefix);
			strcpy(leaving_level_movie_filename + sizeof(movie_prefix) / sizeof(char) - 1, leaving_level);
			strcpy(leaving_level_movie_filename + sizeof(movie_prefix) / sizeof(char) - 1 + leaving_level_length, movie_file_extension);
			rename(movie_filename, leaving_level_movie_filename);
		}
		else // The default movie is currently in position; move it.
			rename(movie_filename, movie_prefix);

		// Move level-specific movie into place
		rename(loading_level_movie_filename, movie_filename);
	}
	else // level-specific movie doesn't exist; ensure that current movie is default
	{	
		// check if default movie is in position
		file = fopen(movie_prefix, "rb");
		if (file != NULL)
		{
			fclose(file);

			// A level-specific movie is currently in position; move it
			strcpy(leaving_level_movie_filename, movie_prefix);
			strcpy(leaving_level_movie_filename + sizeof(movie_prefix) / sizeof(char) - 1, leaving_level);
			strcpy(leaving_level_movie_filename + sizeof(movie_prefix) / sizeof(char) - 1 + leaving_level_length, movie_file_extension);
			rename(movie_filename, leaving_level_movie_filename);

			// Move the default movie into position
			rename(movie_prefix, movie_filename);
		}
		// else // Default movie is already in position; nothing to do
	}
}
