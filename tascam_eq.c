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
#include "tascam_alsa.h"

#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

#define TASCAM_URI "http://www.paraair.de/plugins/tascam_eq"

typedef struct {
    int channel;
    
    int highfreq;
    char *highfreq_name;
    snd_hctl_elem_t* highfreq_elem[16];
    int highlevel;
    char *highlevel_name;
    snd_hctl_elem_t* highlevel_elem[16];
    
    int midhighfreq;
    char *midhighfreq_name;
    snd_hctl_elem_t* midhighfreq_elem[16];
    int midhighq;
    char *midhighq_name;
    snd_hctl_elem_t* midhighq_elem[16];
    int midhighlevel;
    char *midhighlevel_name;
    snd_hctl_elem_t* midhighlevel_elem[16];
    
    int midlowfreq;
    char *midlowfreq_name;
    snd_hctl_elem_t* midlowfreq_elem[16];
    int midlowq;
    char *midlowq_name;
    snd_hctl_elem_t* midlowq_elem[16];
    int midlowlevel;
    char *midlowlevel_name;
    snd_hctl_elem_t* midlowlevel_elem[16];
    
    int lowfreq;
    char *lowfreq_name;
    snd_hctl_elem_t* lowfreq_elem[16];
    int lowlevel;
    char *lowlevel_name;
    snd_hctl_elem_t* lowlevel_elem[16];
    
} tascam_eq;

//tascam_eq *eq = 0;

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

void sfree(char** s) {
    if( *s )
        free(*s);
    *s = 0;
}

extern LV2_Handle
instantiate_eq(const LV2_Descriptor* descriptor,
        double rate,
        const char* bundle_path,
        const LV2_Feature * const* features) {
    int i;

    fprintf(stdout, "tascam.lv2: instantiate_eq bundle:%s\n", bundle_path);
    Tascam_eq* instance = (Tascam_eq*) calloc(1, sizeof (Tascam_eq));

    tascam_eq* eq = (tascam_eq*) calloc(1, sizeof (tascam_eq));

    for (i = 0; i < NUM_CHANNELS; i++) {

    }
    eq->highfreq_name = (char*) malloc(32);
    snprintf(eq->highfreq_name, 32, "name='81 HighFreq',index=");
    eq->highlevel_name = (char*) malloc(32);
    snprintf(eq->highlevel_name, 32, "name='8 High',index=");
    eq->midhighfreq_name = (char*) malloc(32);
    snprintf(eq->midhighfreq_name, 32, "name='71 MHiFreq',index=");
    eq->midhighq_name = (char*) malloc(32);
    snprintf(eq->midhighq_name, 32, "name='72 MHiWidth',index=");
    eq->midhighlevel_name = (char*) malloc(32);
    snprintf(eq->midhighlevel_name, 32, "name='7 MHigh',index=");
    eq->midlowfreq_name = (char*) malloc(32);
    snprintf(eq->midlowfreq_name, 32, "name='61 MLowFreq',index=");
    eq->midlowq_name = (char*) malloc(32);
    snprintf(eq->midlowq_name, 32, "name='62 MLowWidth',index=");
    eq->midlowlevel_name = (char*) malloc(32);
    snprintf(eq->midlowlevel_name, 32, "name='6 MLow',index=");
    eq->lowfreq_name = (char*) malloc(32);
    snprintf(eq->lowfreq_name, 32, "name='51 LowFreq',index=");
    eq->lowlevel_name = (char*) malloc(32);
    snprintf(eq->lowlevel_name, 32, "name='5 Low',index=");

    instance->cache = (void*) eq;

    if (cardnum == -1) {
        cardnum = get_alsa_cardnum();
        if (cardnum == -1)
            return 0;
    }
    open_device();

    return (LV2_Handle) instance;
}

extern void
connect_port_eq(LV2_Handle instance,
        uint32_t port,
        void* data) {
    Tascam_eq* tascam = (Tascam_eq*) instance;

    switch ((PortIndex_eq) port) {
        case TASCAM_EQ_CHANNEL:
            tascam->channel = (const float*) data;
            break;

        case TASCAM_EQ_INPUT:
            tascam->input = (const float*) data;
            break;

        case TASCAM_EQ_OUTPUT:
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
    Tascam_eq* tascam = (Tascam_eq*) instance;
    tascam_eq* eq = (tascam_eq*) tascam->cache;
    int i;
    for(i=0; i < NUM_CHANNELS; i++) {
        eq->highfreq_elem[i] = get_ctrl_elem(eq->highfreq_name, i);
        eq->highlevel_elem[i] = get_ctrl_elem(eq->highlevel_name, i);
        eq->midhighfreq_elem[i] = get_ctrl_elem(eq->midhighfreq_name, i);
        eq->midhighq_elem[i] = get_ctrl_elem(eq->midhighq_name, i);
        eq->midhighlevel_elem[i] = get_ctrl_elem(eq->midhighlevel_name, i);
        eq->midlowfreq_elem[i] = get_ctrl_elem(eq->midlowfreq_name, i);
        eq->midlowq_elem[i] = get_ctrl_elem(eq->midlowq_name, i);
        eq->midlowlevel_elem[i] = get_ctrl_elem(eq->midlowlevel_name, i);
        eq->lowfreq_elem[i] = get_ctrl_elem(eq->lowfreq_name, i);
        eq->lowlevel_elem[i] = get_ctrl_elem(eq->lowlevel_name, i);
    }
}

/** Define a macro for converting a gain in dB to a coefficient. */
#define DB_CO(g) ((g) > -90.0f ? powf(10.0f, (g) * 0.05f) : 0.0f)

extern void
run_eq(LV2_Handle instance, uint32_t n_samples) {
    const Tascam_eq* tascam = (const Tascam_eq*) instance;
    tascam_eq* eq = (tascam_eq*) tascam->cache;
    
    const int _channel = (int) *(tascam->channel);
    const int _highlevel = (int) *(tascam->highlevel);
    const int _highfreq = (int) *(tascam->highfreq);
    const int _midhighlevel = (int) *(tascam->midhighlevel);
    const int _midhighfreq = (int) *(tascam->midhighfreq);
    const int _midhighq = (int) *(tascam->midhighq);
    const int _midlowlevel = (int) *(tascam->midlowlevel);
    const int _midlowfreq = (int) *(tascam->midlowfreq);
    const int _midlowq = (int) *(tascam->midlowq);
    const int _lowlevel = (int) *(tascam->lowlevel);
    const int _lowfreq = (int) *(tascam->lowfreq);
    const float enable = *(tascam->enable);
    const float* const input = tascam->input;
    float* const output = tascam->output;
    
    int bChannelChanged = (_channel != eq->channel);
        
    if ((_highlevel != eq->highlevel  ) || bChannelChanged ) {
        fprintf(stdout, "_highlevel changed\n" );
        eq->highlevel = _highlevel;
        setElemInteger(eq->highlevel_elem[_channel], _highlevel + 12);
    }

    if ((_highfreq != eq->highfreq ) || bChannelChanged ) {
        fprintf(stdout, "_highfreq changed\n" );
        eq->highfreq = _highfreq;
        setElemInteger(eq->highfreq_elem[_channel], _highfreq);
    }

    if ((_midhighfreq != eq->midhighfreq ) || bChannelChanged ) {
        fprintf(stdout, "_midhighfreq changed\n" );
        eq->midhighfreq = _midhighfreq;
        setElemInteger(eq->midhighfreq_elem[_channel], _midhighfreq );
    }

    if ((_midhighq != eq->midhighq ) || bChannelChanged ) {
        fprintf(stdout, "_midhighq changed\n" );
        eq->midhighq = _midhighq;
        setElemInteger(eq->midhighq_elem[_channel], _midhighq );
    }

    if ((_midhighlevel != eq->midhighlevel ) || bChannelChanged ) {
        fprintf(stdout, "_midhighlevel changed\n" );
        eq->midhighlevel = _midhighlevel;
        setElemInteger(eq->midhighlevel_elem[_channel], _midhighlevel + 12);
    }

    if ((_midlowfreq != eq->midlowfreq ) || bChannelChanged ) {
        fprintf(stdout, "_midlowfreq changed\n" );
        eq->midlowfreq = _midlowfreq;
        setElemInteger(eq->midlowfreq_elem[_channel], _midlowfreq );
    }

    if ((_midlowq != eq->midlowq ) || bChannelChanged ) {
        fprintf(stdout, "_midlowq changed\n" );
        eq->midlowq = _midlowq;
        setElemInteger(eq->midlowq_elem[_channel], _midlowq );
    }

    if ((_midlowlevel != eq->midlowlevel ) || bChannelChanged ) {
        fprintf(stdout, "_midlowlevel changed\n" );
        eq->midlowlevel = _midlowlevel;
        setElemInteger(eq->midlowlevel_elem[_channel] , _midlowlevel + 12);
    }

    if ((_lowlevel != eq->lowlevel ) || bChannelChanged ) {
        fprintf(stdout, "_lowlevel changed\n" );
        eq->lowlevel = _lowlevel;
        setElemInteger(eq->lowlevel_elem[_channel], _lowlevel + 12);
    }

    if ((_lowfreq != eq->lowfreq ) || bChannelChanged ) {
        fprintf(stdout, "_lowfreq changed\n" );
        eq->lowfreq = _lowfreq;
        setElemInteger(eq->lowfreq_elem[_channel] , _lowfreq);
    }

    if( bChannelChanged ) {
        eq->channel = _channel;
    }
    return;
}

extern void
deactivate_eq(LV2_Handle instance) {
    fprintf(stdout, "tascam_lv2: deactivate_eq\n");
}

extern void
cleanup_eq(LV2_Handle instance) {
    fprintf(stdout, "tascam_lv2: cleanup_eq\n");

    tascam_eq* eq = (tascam_eq*) ((Tascam_eq*) instance)->cache;
    if (eq) {
        sfree(&eq->highfreq_name);
        sfree(&eq->highlevel_name);
        sfree(&eq->midhighfreq_name);
        sfree(&eq->midhighq_name);
        sfree(&eq->midhighlevel_name);
        sfree(&eq->midlowfreq_name);
        sfree(&eq->midlowq_name);
        sfree(&eq->midlowlevel_name);
        sfree(&eq->lowfreq_name);
        sfree(&eq->lowlevel_name);
        eq = 0;
    }
    free(instance);
    close_device();
}

