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
#include "tascam_comp_stereo.h"

#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

static channel_cache* cache_left;
static channel_cache* cache_right;

const LV2_Descriptor descriptor_comp_stereo = {
    TASCAM_COMP_STEREO_URI,
    instantiate_comp_stereo,
    connect_port_comp_stereo,
    activate_comp_stereo,
    run_comp_stereo,
    deactivate_comp_stereo,
    cleanup_comp_stereo,
    extension_data
};

extern LV2_Handle
instantiate_comp_stereo(const LV2_Descriptor* descriptor,
        double rate,
        const char* bundle_path,
        const LV2_Feature * const* features) {

    //    fprintf(stdout, "tascam.lv2: instantiate_eq bundle:%s\n", bundle_path);
    Tascam_comp_ports_stereo* instance = (Tascam_comp_ports_stereo*) calloc(1, sizeof (Tascam_comp_ports_stereo));

    if (open_device())
        return NULL;

    return (LV2_Handle) instance;
}

extern void
connect_port_comp_stereo(LV2_Handle instance,
        uint32_t port,
        void* data) {
    Tascam_comp_ports_stereo* tascam = (Tascam_comp_ports_stereo*) instance;

    switch ((PortIndex_comp) port) {
        case TASCAM_COMP_STEREO_CHANNEL:
            tascam->channel = (const float*) data;
            break;

        case TASCAM_COMP_STEREO_ENABLE:
            tascam->enable = (const float*) data;
            break;

        case TASCAM_COMP_STEREO_TRESHOLD:
            tascam->threshold = (const float*) data;
            break;

        case TASCAM_COMP_STEREO_RATIO:
            tascam->ratio = (const float*) data;
            break;

        case TASCAM_COMP_STEREO_ATTACK:
            tascam->attack = (const float*) data;
            break;

        case TASCAM_COMP_STEREO_RELEASE:
            tascam->release = (const float*) data;
            break;

        case TASCAM_COMP_STEREO_GAIN:
            tascam->gain = (const float*) data;
            break;

        case TASCAM_COMP_STEREO_INPUT_LEVEL_LEFT:
            tascam->input_level_left = (float*) data;
            break;

        case TASCAM_COMP_STEREO_INPUT_LEVEL_RIGHT:
            tascam->input_level_right = (float*) data;
            break;

        case TASCAM_COMP_STEREO_COMP_LEVEL_LEFT:
            tascam->comp_level_left = (float*) data;
            break;
            
        case TASCAM_COMP_STEREO_COMP_LEVEL_RIGHT:
            tascam->comp_level_right = (float*) data;
            break;

    }
}

extern void
activate_comp_stereo(LV2_Handle instance) {
    fprintf(stdout, "tascam_lv2: activate_comp_stereo\n");
}

extern void
run_comp_stereo(LV2_Handle instance, uint32_t n_samples) {

    const Tascam_comp_ports_stereo* tascam = (const Tascam_comp_ports_stereo*) instance;

    const int _channel = (int) *(tascam->channel);
    const int _threshold = (int) *(tascam->threshold);
    const int _ratio = (int) *(tascam->ratio);
    const int _attack = (int) *(tascam->attack);
    const int _release = (int) *(tascam->release);
    const int _gain = (int) *(tascam->gain);
    const int _enable = (int) *(tascam->enable);

    float* meter_left = tascam->input_level_left;
    float* meter_right = tascam->input_level_right;
    float* comp_left = tascam->comp_level_left;
    float* comp_right = tascam->comp_level_right;

    if (_channel != -1) {

        *meter_left = getInputMeterFloat(_channel);
        *meter_right = getInputMeterFloat(_channel + 1);
        *comp_left = getCompMeterFloat(_channel);
        *comp_right = getCompMeterFloat(_channel + 1);
        
        cache_left = get_comp_channel_cache(_channel);
        cache_right = get_comp_channel_cache(_channel + 1);

        if (_enable != cache_left->controls[TASCAM_COMP_STEREO_ENABLE].new_value) {
            fprintf(stdout, "_enable changed to %d\n", _enable);
            cache_left->controls[TASCAM_COMP_STEREO_ENABLE].new_value = _enable;
            cache_right->controls[TASCAM_COMP_STEREO_ENABLE].new_value = _enable;
        }

        if ((_threshold != cache_left->controls[TASCAM_COMP_STEREO_TRESHOLD].new_value - 32)) {
            fprintf(stdout, "_threshold changed\n");
            cache_left->controls[TASCAM_COMP_STEREO_TRESHOLD].new_value = _threshold + 32;
            cache_right->controls[TASCAM_COMP_STEREO_TRESHOLD].new_value = _threshold + 32;
        }

        if ((_ratio != cache_left->controls[TASCAM_COMP_STEREO_RATIO].new_value)) {
            fprintf(stdout, "_ratio changed\n");
            cache_left->controls[TASCAM_COMP_STEREO_RATIO].new_value = _ratio;
            cache_right->controls[TASCAM_COMP_STEREO_RATIO].new_value = _ratio;
        }

        if ((_attack != cache_left->controls[TASCAM_COMP_STEREO_ATTACK].new_value + 2)) {
            fprintf(stdout, "_attack changed\n");
            cache_left->controls[TASCAM_COMP_STEREO_ATTACK].new_value = _attack - 2;
            cache_right->controls[TASCAM_COMP_STEREO_ATTACK].new_value = _attack - 2;
        }

        if ((_release != (cache_left->controls[TASCAM_COMP_STEREO_RELEASE].new_value ) * 10)) {
            fprintf(stdout, "_release changed\n");
            cache_left->controls[TASCAM_COMP_STEREO_RELEASE].new_value = _release / 10;
            cache_right->controls[TASCAM_COMP_STEREO_RELEASE].new_value = _release / 10;
        }

        if ((_gain != cache_left->controls[TASCAM_COMP_STEREO_GAIN].new_value)) {
            fprintf(stdout, "_gain changed\n");
            cache_left->controls[TASCAM_COMP_STEREO_GAIN].new_value = _gain;
            cache_right->controls[TASCAM_COMP_STEREO_GAIN].new_value = _gain;
        }
    }
}

extern void
deactivate_comp_stereo(LV2_Handle instance) {
    fprintf(stdout, "tascam_lv2: deactivate_comp_stereo\n");
}

extern void
cleanup_comp_stereo(LV2_Handle instance) {
    fprintf(stdout, "tascam_lv2: cleanup_comp_stereo\n");

    free((Tascam_comp_ports*) instance);
    close_device();
}



