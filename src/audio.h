#ifndef AUDIO_H_
#define AUDIO_H_

enum { AU_STOPPED, AU_PLAYING };

enum {
	AU_CUR = 0x7000,
	AU_VOLUP = 0x7100,
	AU_VOLDN = 0x7200
};

struct au_module {
	char *name;
	void *impl;
};

struct au_stream;

int au_init(void);
void au_shutdown(void);

struct au_module *au_load_module(const char *fname);
void au_free_module(struct au_module *mod);

int au_play_module(struct au_module *mod);
void au_update(void);
int au_stop_module(struct au_module *mod);
int au_module_state(struct au_module *mod);
int au_player_pos(void);

struct au_stream *au_open_stream(const char *fname);
void au_close_stream(struct au_stream *au);

int au_play_stream(struct au_stream *au);
void au_update_stream(void);
int au_stop_stream(void);
int au_stream_state(void);
long au_stream_pos(void);

int au_volume(int vol);
int au_sfx_volume(int vol);
int au_music_volume(int vol);

/* pay no attention to the man behind the curtain */
#define AU_VOLADJ(vol, newvol) \
	do { \
		int d; \
		switch(newvol & 0xff00) { \
		case AU_CUR: \
			return (vol); \
		case AU_VOLUP: \
			d = newvol & 0xff; \
			(newvol) = (vol) + (d ? d : 16); \
			if((newvol) >= 256) (newvol) = 255; \
			break; \
		case AU_VOLDN: \
			d = newvol & 0xff; \
			(newvol) = (vol) - (d ? d : 16); \
			if((newvol) < 0) (newvol) = 0; \
			break; \
		default: \
			break; \
		} \
	} while(0)

#endif	/* AUDIO_H_ */
