/*
  Copyright 2006-2016 Detlef Urban <onkel@paraair.de>

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THIS SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/** Include standard C headers */
#include <math.h>
#include <stdlib.h>
#include "tascam_eq.h"

#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

#define TASCAM_URI "http://www.paraair.de/plugins/tascam_eq"


int lowlevel = 0;

LV2_Descriptor descriptor_eq = {
	TASCAM_EQ_URI,
	instantiate_eq,
	connect_port_eq,
	activate_eq,
	run_eq,
	deactivate_eq,
	cleanup_eq,
	extension_data
};


typedef enum {
    TASCAM_CHANNEL,
    TASCAM_INPUT,
    TASCAM_OUTPUT,
    TASCAM_EQ_HIGH_FREQ,
    TASCAM_EQ_HIGH_LEVEL,
    TASCAM_EQ_MIDHIGH_FREQ,
    TASCAM_EQ_MIDHIGH_Q,
    TASCAM_EQ_MIDHIGH_LEVEL,
    TASCAM_EQ_MIDLOW_FREQ,
    TASCAM_EQ_MIDLOW_Q,
    TASCAM_EQ_MIDLOW_LEVEL,
    TASCAM_EQ_LOWLEVEL,
    TASCAM_EQ_LOWFREQ,
    TASCAM_EQ_ENABLE
} PortIndex;

typedef struct {
    // Port buffers
    const float* channel;
    const float* highfreq;
    const float* highlevel;
    const float* midhighfreq;
    const float* midhighq;
    const float* midhighlevel;
    const float* midlowfreq;
    const float* midlowq;
    const float* midlowlevel;
    const float* lowfreq;
    const float* lowlevel;
    const float* enable;
    const float* input;
    float* output;
} Tascam_eq;

extern LV2_Handle
instantiate_eq(const LV2_Descriptor* descriptor,
        double rate,
        const char* bundle_path,
        const LV2_Feature * const* features) {

    if( cardnum == -1 ) {
        cardnum = get_alsa_cardnum();
        if( cardnum == -1 ) 
            return NULL;
    }
    
    Tascam_eq* instance = (Tascam_eq*) calloc(1, sizeof (Tascam_eq));

    return (LV2_Handle) instance;
}

extern void
connect_port_eq(LV2_Handle instance,
        uint32_t port,
        void* data) {
    Tascam_eq* tascam = (Tascam_eq*) instance;

    switch ((PortIndex) port) {
        case TASCAM_CHANNEL:
            tascam->channel = (const float*) data;
            break;

        case TASCAM_INPUT:
            tascam->input = (const float*) data;
            break;

        case TASCAM_OUTPUT:
            tascam->output = (float*) data;
            break;

        case TASCAM_EQ_HIGH_FREQ:
            tascam->highfreq = (const float*) data;
            break;

        case TASCAM_EQ_HIGH_LEVEL:
            tascam->highlevel = (const float*) data;
            break;

        case TASCAM_EQ_MIDHIGH_FREQ:
            tascam->midhighfreq = (const float*) data;
            break;

        case TASCAM_EQ_MIDHIGH_Q:
            tascam->midhighq = (const float*) data;
            break;

        case TASCAM_EQ_MIDHIGH_LEVEL:
            tascam->midhighlevel = (const float*) data;
            break;

        case TASCAM_EQ_MIDLOW_FREQ:
            tascam->midlowfreq = (const float*) data;
            break;

        case TASCAM_EQ_MIDLOW_Q:
            tascam->midlowq = (const float*) data;
            break;

        case TASCAM_EQ_MIDLOW_LEVEL:
            tascam->midlowlevel = (const float*) data;
            break;

        case TASCAM_EQ_LOWFREQ:
            tascam->lowfreq = (const float*) data;
            break;

        case TASCAM_EQ_LOWLEVEL:
            tascam->lowlevel = (const float*) data;
            break;

        case TASCAM_EQ_ENABLE:
            tascam->enable = (const float*) data;
            break;
    }
}

extern void
activate_eq(LV2_Handle instance) {
}

/** Define a macro for converting a gain in dB to a coefficient. */
#define DB_CO(g) ((g) > -90.0f ? powf(10.0f, (g) * 0.05f) : 0.0f)

extern void
run_eq(LV2_Handle instance, uint32_t n_samples) {
    const Tascam_eq* tascam = (const Tascam_eq*) instance;

    const float channel = *(tascam->channel);
    const float highlevel = *(tascam->highlevel);
    const float highfreq = *(tascam->highfreq);
    const float midhighlevel = *(tascam->midhighlevel);
    const float midhighfreq = *(tascam->midhighfreq);
    const float midhighq = *(tascam->midhighq);
    const float midlowlevel = *(tascam->midlowlevel);
    const float midlowfreq = *(tascam->midlowfreq);
    const float midlowq = *(tascam->midlowq);
    const float _lowlevel = *(tascam->lowlevel);
    const float lowfreq = *(tascam->lowfreq);
    const float enable = *(tascam->enable);
    const float* const input = tascam->input;
    float* const output = tascam->output;

    if( (int)_lowlevel != lowlevel ) {
        fprintf(stdout, "change lowlevel from %d to %d\n", lowlevel, (int)_lowlevel);
        lowlevel = (int)_lowlevel;
    }
        
    
    // TODO: DU may be not needed cause no effect
    for (uint32_t pos = 0; pos < n_samples; pos++) {
        output[pos] = input[pos];
    }
}

extern void
deactivate_eq(LV2_Handle instance) {
}

extern void
cleanup_eq(LV2_Handle instance) {
    free(instance);
    close_device();
}

