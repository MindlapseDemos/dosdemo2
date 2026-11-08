#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "assfile/assfile.h"
#include "audio.h"
#include "au_mp3.h"
#include "util.h"
#include "mad.h"


int mp3_load(struct au_stream *au, const char *fname)
{
	int i;
	long len;
	struct ass_file *fp;
	struct mad_header hdr;

	if(!(au->name = strdup(fname))) {
		fprintf(stderr, "mp3_load: failed to allocate name\n");
		return -1;
	}

	if(!(fp = ass_fopen(fname, "rb"))) {
		fprintf(stderr, "mp3_load: failed to open: %s: %s\n", fname, strerror(errno));
		free(au->name);
		return -1;
	}
	ass_fseek(fp, 0, SEEK_END);
	len = ass_ftell(fp);
	ass_fseek(fp, 0, SEEK_SET);

	if(!(au->data = calloc(1, len + MAD_BUFFER_GUARD))) {
		fprintf(stderr, "mp3_load: failed to allocate stream data\n");
		ass_fclose(fp);
		free(au->name);
		return -1;
	}
	ass_fread(au->data, 1, len, fp);
	ass_fclose(fp);

	au->offs = 0;
	au->size = len;

	mad_stream_init(&au->madstr);
	mad_frame_init(&au->madfrm);
	mad_synth_init(&au->madsyn);
	au->madstr.options |= MAD_OPTION_IGNORECRC;
	/* au->madstr.options |= MAD_OPTION_HALFSAMPLERATE */

	mad_header_init(&hdr);
	mad_stream_buffer(&au->madstr, au->data, au->size + MAD_BUFFER_GUARD);

	for(i=0; i<1024; i++) {
		if(mad_header_decode(&hdr, &au->madstr) == 0) {
			au->rate = hdr.samplerate;
			au->offs = au->madstr.this_frame - au->data;
			break;
		}
		if(!MAD_RECOVERABLE(au->madstr.error)) {
			free(au->data);
			free(au->name);
			return -1;
		}
	}

	return 0;
}

void mp3_destroy(struct au_stream *au)
{
	mad_synth_finish(&au->madsyn);
	mad_frame_finish(&au->madfrm);
	mad_stream_finish(&au->madstr);

	free(au->name);
	free(au->data);
}

static INLINE int scale_sample(mad_fixed_t sample)
{
	/* round */
	sample += (1L << (MAD_F_FRACBITS - 16));
	/* clip */
	if(sample >= MAD_F_ONE) {
		sample = MAD_F_ONE - 1;
	} else if(sample < -MAD_F_ONE) {
		sample = -MAD_F_ONE;
	}
	/* quantize */
	return sample >> (MAD_F_FRACBITS + 1 - 16);
}

int mp3_decode_frame(struct au_stream *au)
{
	unsigned int i, len;
	const mad_fixed_t *left, *right;
	int16_t *outptr;

	for(i=0; i<256; i++) {
		if(au->madstr.next_frame >= au->data + au->size) {
			return -1;
		}
		if(mad_frame_decode(&au->madfrm, &au->madstr) == 0) {
			break;
		}
		if(au->madstr.error == MAD_ERROR_BUFLEN || !MAD_RECOVERABLE(au->madstr.error)) {
			return -1;
		}
	}
	if(i >= 256) return 0;

	mad_synth_frame(&au->madsyn, &au->madfrm);

	len = au->madsyn.pcm.length;
	if(len > MAX_FRAME_SAMPLES) {
		len = MAX_FRAME_SAMPLES;
	}

	left = au->madsyn.pcm.samples[0];
	right = au->madsyn.pcm.samples[1];
	outptr = au->pcm;

	for(i=0; i<len; i++) {
		outptr[0] = scale_sample(left[i]);
		outptr[1] = scale_sample(right[i]);
		outptr += 2;
	}

	au->pcm_offs = 0;
	au->pcm_size = len * 4;	/* 2 bytes per sample, 2 channels */
	return 1;
}
