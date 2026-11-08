#ifndef AU_MP3_H_
#define AU_MP3_H_

#include "audio.h"
#include "mad.h"
#include "szint.h"

#define MAX_FRAME_SAMPLES	1152

struct au_stream {
	char *name;
	unsigned char *data;
	unsigned int size, offs;

	int state;

	unsigned int rate, nchan;
	unsigned int playpos_bytes;

	struct mad_stream madstr;
	struct mad_frame madfrm;
	struct mad_synth madsyn;

	void *aout;

	int16_t pcm[MAX_FRAME_SAMPLES * 2];
	int pcm_offs, pcm_size;
};

int mp3_load(struct au_stream *au, const char *fname);
void mp3_destroy(struct au_stream *au);
int mp3_decode_frame(struct au_stream *au);

#endif	/* AU_MP3_H_ */
