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
#include "tascam.h"
#include "tascam_eq_stereo.h"


#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

#define TASCAM_URI "http://www.paraair.de/plugins/tascam_eq_stereo"


static channel_cache* cache_left;
static channel_cache* cache_right;

LV2_Descriptor descriptor_eq_stereo = {
    TASCAM_EQ_STEREO_URI,
    instantiate_eq_stereo,
    connect_port_eq_stereo,
    activate_eq_stereo,
    run_eq_stereo,
    deactivate_eq_stereo,
    cleanup_eq_stereo,
    extension_data
};

static LV2_Handle
instantiate_eq_stereo(const LV2_Descriptor* descriptor, double rate, const char* bundle_path, const LV2_Feature * const* features) {

    int i;

    fprintf(stdout, "tascam.lv2: instantiate_eq bundle:%s\n", bundle_path);
    Tascam_eq_stereo_ports* instance = (Tascam_eq_stereo_ports*) calloc(1, sizeof (Tascam_eq_stereo_ports));

    if (open_device())
        return NULL;

    return (LV2_Handle) instance;
}

static void
connect_port_eq_stereo(LV2_Handle instance,
        uint32_t port,
        void* data) {
    Tascam_eq_stereo_ports* tascam = (Tascam_eq_stereo_ports*) instance;

    switch ((PortIndex_eq_stereo) port) {
        case TASCAM_EQ_STEREO_CHANNEL:
            tascam->channel = (const float*) data;
            break;

        case TASCAM_EQ_STEREO_INPUT_RIGHT:
            tascam->input_right = (const float*) data;
            break;

        case TASCAM_EQ_STEREO_OUTPUT_RIGHT:
            tascam->output_right = (float*) data;
            break;

        case TASCAM_EQ_STEREO_INPUT_LEFT:
            tascam->input_left = (const float*) data;
            break;

        case TASCAM_EQ_STEREO_OUTPUT_LEFT:
            tascam->output_left = (float*) data;
            break;

        case TASCAM_EQ_STEREO_HIGH_FREQ:
            tascam->highfreq = (const float*) data;
            break;

        case TASCAM_EQ_STEREO_HIGH_LEVEL:
            tascam->highlevel = (const float*) data;
            break;

        case TASCAM_EQ_STEREO_MIDHIGH_FREQ:
            tascam->midhighfreq = (const float*) data;
            break;

        case TASCAM_EQ_STEREO_MIDHIGH_Q:
            tascam->midhighq = (const float*) data;
            break;

        case TASCAM_EQ_STEREO_MIDHIGH_LEVEL:
            tascam->midhighlevel = (const float*) data;
            break;

        case TASCAM_EQ_STEREO_MIDLOW_FREQ:
            tascam->midlowfreq = (const float*) data;
            break;

        case TASCAM_EQ_STEREO_MIDLOW_Q:
            tascam->midlowq = (const float*) data;
            break;

        case TASCAM_EQ_STEREO_MIDLOW_LEVEL:
            tascam->midlowlevel = (const float*) data;
            break;

        case TASCAM_EQ_STEREO_LOW_FREQ:
            tascam->lowfreq = (const float*) data;
            break;

        case TASCAM_EQ_STEREO_LOW_LEVEL:
            tascam->lowlevel = (const float*) data;
            break;

        case TASCAM_EQ_STEREO_ENABLE:
            tascam->enable = (const float*) data;
            break;

        case TASCAM_EQ_STEREO_INPUT_LEVEL_LEFT:
            tascam->input_level_left = (float*) data;
            break;

        case TASCAM_EQ_STEREO_INPUT_LEVEL_RIGHT:
            tascam->input_level_right = (float*) data;
            break;
    }
}

static void
activate_eq_stereo(LV2_Handle instance) {

}

/** Define a macro for converting a gain in dB to a coefficient. */
#define DB_CO(g) ((g) > -90.0f ? powf(10.0f, (g) * 0.05f) : 0.0f)

static void
run_eq_stereo(LV2_Handle instance, uint32_t n_samples) {
    const Tascam_eq_stereo_ports* tascam = (const Tascam_eq_stereo_ports*) instance;

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
    float* meter_left = tascam->input_level_left;
    float* meter_right = tascam->input_level_right;

    if (_channel != -1) {

        *meter_left = getMeterFloat(_channel);
        *meter_right = getMeterFloat(_channel + 1);
        cache_left = get_eq_channel_cache(_channel);
        cache_right = get_eq_channel_cache(_channel + 1);

        if (_enable != cache_left->controls[TASCAM_EQ_ENABLE].new_value) {
            fprintf(stdout, "_enable changed to %d\n", _enable);
            cache_left->controls[TASCAM_EQ_ENABLE].new_value = _enable;
            cache_right->controls[TASCAM_EQ_ENABLE].new_value = _enable;
            return; 
        }

        if ((_highlevel != cache_left->controls[TASCAM_EQ_HIGH_LEVEL].new_value - 12)) {
            fprintf(stdout, "_highlevel changed\n");
            cache_left->controls[TASCAM_EQ_HIGH_LEVEL].new_value = _highlevel + 12;
            cache_right->controls[TASCAM_EQ_HIGH_LEVEL].new_value = _highlevel + 12;
            return; 
        }

        if ((_highfreq != cache_left->controls[TASCAM_EQ_HIGH_FREQ].new_value)) {
            fprintf(stdout, "_highfreq changed\n");
            cache_left->controls[TASCAM_EQ_HIGH_FREQ].new_value = _highfreq;
            cache_right->controls[TASCAM_EQ_HIGH_FREQ].new_value = _highfreq;
            return; 
        }

        if ((_midhighfreq != cache_left->controls[TASCAM_EQ_MIDHIGH_FREQ].new_value)) {
            fprintf(stdout, "_midhighfreq changed\n");
            cache_left->controls[TASCAM_EQ_MIDHIGH_FREQ].new_value = _midhighfreq;
            cache_right->controls[TASCAM_EQ_MIDHIGH_FREQ].new_value = _midhighfreq;
            return; 
        }

        if ((_midhighq != cache_left->controls[TASCAM_EQ_MIDHIGH_Q].new_value)) {
            fprintf(stdout, "_midhighq changed\n");
            cache_left->controls[TASCAM_EQ_MIDHIGH_Q].new_value = _midhighq;
            cache_right->controls[TASCAM_EQ_MIDHIGH_Q].new_value = _midhighq;
            return; 
        }

        if ((_midhighlevel != cache_left->controls[TASCAM_EQ_MIDHIGH_LEVEL].new_value - 12)) {
            fprintf(stdout, "_midhighlevel(%d) changed to %d\n", _channel, _midhighlevel);
            cache_left->controls[TASCAM_EQ_MIDHIGH_LEVEL].new_value = _midhighlevel + 12;
            cache_right->controls[TASCAM_EQ_MIDHIGH_LEVEL].new_value = _midhighlevel + 12;
            return; 
        }

        if ((_midlowfreq != cache_left->controls[TASCAM_EQ_MIDLOW_FREQ].new_value)) {
            fprintf(stdout, "_midlowfreq changed\n");
            cache_left->controls[TASCAM_EQ_MIDLOW_FREQ].new_value = _midlowfreq;
            cache_right->controls[TASCAM_EQ_MIDLOW_FREQ].new_value = _midlowfreq;
            return; 
        }

        if ((_midlowq != cache_left->controls[TASCAM_EQ_MIDLOW_Q].new_value)) {
            fprintf(stdout, "_midlowq changed\n");
            cache_left->controls[TASCAM_EQ_MIDLOW_Q].new_value = _midlowq;
            cache_right->controls[TASCAM_EQ_MIDLOW_Q].new_value = _midlowq;
            return; 
        }

        if ((_midlowlevel != cache_left->controls[TASCAM_EQ_MIDLOW_LEVEL].new_value - 12)) {
            fprintf(stdout, "_midlowlevel changed\n");
            cache_left->controls[TASCAM_EQ_MIDLOW_LEVEL].new_value = _midlowlevel + 12;
            cache_right->controls[TASCAM_EQ_MIDLOW_LEVEL].new_value = _midlowlevel + 12;
            return; 
        }

        if ((_lowlevel != cache_left->controls[TASCAM_EQ_LOW_LEVEL].new_value - 12)) {
            fprintf(stdout, "_lowlevel(%d) changed %d\n", _channel, _lowlevel);
            cache_left->controls[TASCAM_EQ_LOW_LEVEL].new_value = _lowlevel + 12;
            cache_right->controls[TASCAM_EQ_LOW_LEVEL].new_value = _lowlevel + 12;
            return; 
        }

        if ((_lowfreq != cache_left->controls[TASCAM_EQ_LOW_FREQ].new_value)) {
            fprintf(stdout, "_lowfreq changed\n");
            cache_left->controls[TASCAM_EQ_LOW_FREQ].new_value = _lowfreq;
            cache_right->controls[TASCAM_EQ_LOW_FREQ].new_value = _lowfreq;
            return; 
        }
    }
}

static void
deactivate_eq_stereo(LV2_Handle instance) {
    fprintf(stdout, "tascam_lv2: deactivate_eq\n");
}

static void
cleanup_eq_stereo(LV2_Handle instance) {
    fprintf(stdout, "tascam_lv2: cleanup_eq\n");

    free(instance);
    close_device();
}

