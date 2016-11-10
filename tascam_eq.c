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
    Tascam_eq_ports* instance = (Tascam_eq_ports*) calloc(1, sizeof (Tascam_eq_ports));
    tascam_eq_cache* cache = (tascam_eq_cache*) calloc(1, sizeof (tascam_eq_cache));

// init high filter    
    cache->highfreq_name = (char*) malloc(32);
    snprintf(cache->highfreq_name, 32, "name='81 HighFreq',index=");
    cache->highfreq = 5;
    
    cache->highlevel_name = (char*) malloc(32);
    snprintf(cache->highlevel_name, 32, "name='8 High',index=");
    cache->highlevel = 0;
    
// init mid high filter    
    cache->midhighfreq_name = (char*) malloc(32);
    snprintf(cache->midhighfreq_name, 32, "name='71 MHiFreq',index=");
    cache->midhighfreq = 27;
    
    cache->midhighq_name = (char*) malloc(32);
    snprintf(cache->midhighq_name, 32, "name='72 MHiWidth',index=");
    cache->midhighq = 2;
    
    cache->midhighlevel_name = (char*) malloc(32);
    snprintf(cache->midhighlevel_name, 32, "name='7 MHigh',index=");
    cache->midhighlevel = 0;
    
// init mid low filter    
    cache->midlowfreq_name = (char*) malloc(32);
    snprintf(cache->midlowfreq_name, 32, "name='61 MLowFreq',index=");
    cache->midlowfreq = 14;
    
    cache->midlowq_name = (char*) malloc(32);
    snprintf(cache->midlowq_name, 32, "name='62 MLowWidth',index=");
    cache->midlowq = 2;
    
    cache->midlowlevel_name = (char*) malloc(32);
    snprintf(cache->midlowlevel_name, 32, "name='6 MLow',index=");
    cache->midlowlevel = 0;
    
// init low filter    
    cache->lowfreq_name = (char*) malloc(32);
    snprintf(cache->lowfreq_name, 32, "name='51 LowFreq',index=");
    cache->lowfreq = 5;
    
    cache->lowlevel_name = (char*) malloc(32);
    snprintf(cache->lowlevel_name, 32, "name='5 Low',index=");
    cache->lowlevel = 2;
    
// init enabler    
    cache->enable_name = (char*) malloc(32);
    snprintf(cache->enable_name, 32, "name='9 EQ',index=");
    cache->enable = 1;
    
    instance->cache = cache;

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
    Tascam_eq_ports* tascam = (Tascam_eq_ports*) instance;

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
            
        case TASCAM_EQ_INPUT_LEVEL:
            tascam->input_level = (float*) data;
            break;
            
    }
}

extern void
activate_eq(LV2_Handle instance) {
    int i;
    tascam_eq_cache* cache = ((Tascam_eq_ports*) instance)->cache;
    
    for(i=0; i < NUM_CHANNELS; i++) {
        cache->highfreq_elem[i] = get_ctrl_elem(cache->highfreq_name, i);
        cache->highlevel_elem[i] = get_ctrl_elem(cache->highlevel_name, i);
        cache->midhighfreq_elem[i] = get_ctrl_elem(cache->midhighfreq_name, i);
        cache->midhighq_elem[i] = get_ctrl_elem(cache->midhighq_name, i);
        cache->midhighlevel_elem[i] = get_ctrl_elem(cache->midhighlevel_name, i);
        cache->midlowfreq_elem[i] = get_ctrl_elem(cache->midlowfreq_name, i);
        cache->midlowq_elem[i] = get_ctrl_elem(cache->midlowq_name, i);
        cache->midlowlevel_elem[i] = get_ctrl_elem(cache->midlowlevel_name, i);
        cache->lowfreq_elem[i] = get_ctrl_elem(cache->lowfreq_name, i);
        cache->lowlevel_elem[i] = get_ctrl_elem(cache->lowlevel_name, i);
        cache->enable_elem[i] = get_ctrl_elem(cache->enable_name, i);

    }
}

/** Define a macro for converting a gain in dB to a coefficient. */
#define DB_CO(g) ((g) > -90.0f ? powf(10.0f, (g) * 0.05f) : 0.0f)

extern void
run_eq(LV2_Handle instance, uint32_t n_samples) {
    const Tascam_eq_ports* tascam = (const Tascam_eq_ports*) instance;
    tascam_eq_cache* cache = tascam->cache;
    
    int bChannelChanged = 0;
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
    const int _enable = (int) *(tascam->enable);
    float* meter = tascam->input_level;
    const float* const input = tascam->input;
    float* const output = tascam->output;
    
    
    
    if(_channel != cache->channel) {
        fprintf(stdout, "_channle changed\n" );
        cache->channel = _channel;
        bChannelChanged = 1;
    }
    if(_channel != -1 ) {
    
        *meter = getMeterFloat(_channel);
        
        if( _enable != cache->enable ) {
            fprintf(stdout, "_enable changed\n" );
            cache->enable = _enable;
            setElemInteger(cache->enable_elem[_channel], _enable);
        }
        
        if ((_highlevel != cache->highlevel  ) /* || bChannelChanged */  ) {
            fprintf(stdout, "_highlevel changed\n" );
            cache->highlevel = _highlevel;
            setElemInteger(cache->highlevel_elem[_channel], _highlevel + 12);
        }

        if ((_highfreq != cache->highfreq ) /* || bChannelChanged */  ) {
            fprintf(stdout, "_highfreq changed\n" );
            cache->highfreq = _highfreq;
            setElemInteger(cache->highfreq_elem[_channel], _highfreq);
        }

        if ((_midhighfreq != cache->midhighfreq ) /* || bChannelChanged */  ) {
            fprintf(stdout, "_midhighfreq changed\n" );
            cache->midhighfreq = _midhighfreq;
            setElemInteger(cache->midhighfreq_elem[_channel], _midhighfreq );
        }

        if ((_midhighq != cache->midhighq ) /* || bChannelChanged */  ) {
            fprintf(stdout, "_midhighq changed\n" );
            cache->midhighq = _midhighq;
            setElemInteger(cache->midhighq_elem[_channel], _midhighq );
        }

        if ((_midhighlevel != cache->midhighlevel ) /* || bChannelChanged */  ) {
            fprintf(stdout, "_midhighlevel(%d) changed to %d\n", _channel, _midhighlevel );
            cache->midhighlevel = _midhighlevel;
            setElemInteger(cache->midhighlevel_elem[_channel], _midhighlevel + 12);
        }

        if ((_midlowfreq != cache->midlowfreq ) /* || bChannelChanged */  ) {
            fprintf(stdout, "_midlowfreq changed\n" );
            cache->midlowfreq = _midlowfreq;
            setElemInteger(cache->midlowfreq_elem[_channel], _midlowfreq );
        }

        if ((_midlowq != cache->midlowq ) /* || bChannelChanged */  ) {
            fprintf(stdout, "_midlowq changed\n" );
            cache->midlowq = _midlowq;
            setElemInteger(cache->midlowq_elem[_channel], _midlowq );
        }

        if ((_midlowlevel != cache->midlowlevel ) /* || bChannelChanged */  ) {
            fprintf(stdout, "_midlowlevel changed\n" );
            cache->midlowlevel = _midlowlevel;
            setElemInteger(cache->midlowlevel_elem[_channel] , _midlowlevel + 12);
        }

        if ((_lowlevel != cache->lowlevel ) /* || bChannelChanged */  ) {
            fprintf(stdout, "_lowlevel(%d) changed %d\n", _channel, _lowlevel );
            cache->lowlevel = _lowlevel;
            setElemInteger(cache->lowlevel_elem[_channel], _lowlevel + 12);
        }

        if ((_lowfreq != cache->lowfreq ) /* || bChannelChanged */  ) {
            fprintf(stdout, "_lowfreq changed\n" );
            cache->lowfreq = _lowfreq;
            setElemInteger(cache->lowfreq_elem[_channel] , _lowfreq);
        }
    }
}

extern void
deactivate_eq(LV2_Handle instance) {
    fprintf(stdout, "tascam_lv2: deactivate_eq\n");
}

extern void
cleanup_eq(LV2_Handle instance) {
    fprintf(stdout, "tascam_lv2: cleanup_eq\n");

    tascam_eq_cache* cache = ((Tascam_eq_ports*) instance)->cache;
    if (cache) {
        sfree(&cache->highfreq_name);
        sfree(&cache->highlevel_name);
        sfree(&cache->midhighfreq_name);
        sfree(&cache->midhighq_name);
        sfree(&cache->midhighlevel_name);
        sfree(&cache->midlowfreq_name);
        sfree(&cache->midlowq_name);
        sfree(&cache->midlowlevel_name);
        sfree(&cache->lowfreq_name);
        sfree(&cache->lowlevel_name);
        sfree(&cache->enable_name);
        cache = 0;
    }
    free(instance);
    close_device();
}

