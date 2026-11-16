#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "audio.h"
#include "au_mp3.h"
#include "util.h"
#include "cfgopt.h"

static struct au_module *curmod;
static struct au_stream *curstr;
static int vol_master, vol_mus, vol_sfx;

int au_init(void)
{
	curmod = 0;
	vol_master = vol_mus = vol_sfx = 255;

	if(!opt.music) return 0;

	return 0;
}

void au_shutdown(void)
{
	if(opt.music) {
	}
}

struct au_module *au_load_module(const char *fname)
{
	return 0;
}

void au_free_module(struct au_module *mod)
{
}

int au_play_module(struct au_module *mod)
{
	return -1;
}

void au_update(void)
{
	if(curstr) {
		au_update_stream();
	}
}

int au_stop_module(struct au_module *mod)
{
	return -1;
}

int au_module_state(struct au_module *mod)
{
	return AU_STOPPED;
}

int au_player_pos(void)
{
	return -1;
}

/* --- stream playback --- */

struct au_stream *au_open_stream(const char *fname)
{
	struct au_stream *au;

	if(!(au = malloc(sizeof *au))) {
		fprintf(stderr, "au_open_stream: failed to allocate memory\n");
		return 0;
	}

	if(mp3_load(au, fname) == -1) {
		free(au);
		return 0;
	}
	return au;
}

void au_close_stream(struct au_stream *au)
{
	if(au) {
		mp3_destroy(au);
	}
	free(au);
}

int au_play_stream(struct au_stream *au)
{
	if(curstr) {
		au_stop_stream();
		curstr = 0;
	}

	return -1;	/* TODO */

	au->state = AU_PLAYING;

	curstr = au;
	au_update_stream();
	return 0;
}

void au_update_stream(void)
{
	/* TODO */
}

int au_stop_stream(void)
{
	return -1;	/* TODO */
}

int au_stream_state(void)
{
	return AU_STOPPED;	/* TODO */
}

/* --- volume control --- */

int au_volume(int vol)
{
	AU_VOLADJ(vol_master, vol);
	if(vol != vol_master) {
		vol_master = vol;

		au_sfx_volume(vol_sfx);
		au_music_volume(vol_mus);
	}
	return vol_master;
}

int au_sfx_volume(int vol)
{
	AU_VOLADJ(vol_sfx, vol);
	vol_sfx = vol;
	/* TODO */
	return vol_sfx;
}

int au_music_volume(int vol)
{
	AU_VOLADJ(vol_mus, vol);
	vol_mus = vol;

	if(curmod) {
	}
	return vol_mus;
}
