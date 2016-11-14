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
#include "tascam_comp.h"

#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

const LV2_Descriptor descriptor_comp = {
    TASCAM_COMP_URI,
    instantiate_comp,
    connect_port_comp,
    activate_comp,
    run_comp,
    deactivate_comp,
    cleanup_comp,
    extension_data
};

extern LV2_Handle
instantiate_comp(const LV2_Descriptor* descriptor,
        double rate,
        const char* bundle_path,
        const LV2_Feature * const* features) {

    //    fprintf(stdout, "tascam.lv2: instantiate_eq bundle:%s\n", bundle_path);
    Tascam_comp_ports* instance = (Tascam_comp_ports*) calloc(1, sizeof (Tascam_comp_ports));

    if (open_device())
        return NULL;

    return (LV2_Handle) instance;
}

extern void
connect_port_comp(LV2_Handle instance,
        uint32_t port,
        void* data) {
    Tascam_comp_ports* tascam = (Tascam_comp_ports*) instance;

    switch ((PortIndex_comp) port) {
        case TASCAM_COMP_CHANNEL:
            tascam->channel = (const float*) data;
            break;

        case TASCAM_COMP_ENABLE:
            tascam->enable = (const float*) data;
            break;

        case TASCAM_COMP_TRESHOLD:
            tascam->threshold = (const float*) data;
            break;

        case TASCAM_COMP_RATIO:
            tascam->ratio = (const float*) data;
            break;

        case TASCAM_COMP_ATTACK:
            tascam->attack = (const float*) data;
            break;

        case TASCAM_COMP_RELEASE:
            tascam->release = (const float*) data;
            break;

        case TASCAM_COMP_GAIN:
            tascam->gain = (const float*) data;
            break;

        case TASCAM_COMP_INPUT_LEVEL:
            tascam->input_level = (float*) data;
            break;

        case TASCAM_COMP_COMP_LEVEL:
            tascam->comp_level = (float*) data;
            break;

    }
}

extern void
activate_comp(LV2_Handle instance) {
    fprintf(stdout, "tascam_lv2: activate_comp\n");
}

extern void
run_comp(LV2_Handle instance, uint32_t n_samples) {

    const Tascam_comp_ports* tascam = (const Tascam_comp_ports*) instance;

    const int _channel = (int) *(tascam->channel);
    const int _threshold = (int) *(tascam->threshold);
    const int _ratio = (int) *(tascam->ratio);
    const int _attack = (int) *(tascam->attack);
    const int _release = (int) *(tascam->release);
    const int _gain = (int) *(tascam->gain);
    const int _enable = (int) *(tascam->enable);

    float* meter = tascam->input_level;
    float* comp = tascam->comp_level;

    if (_channel != -1) {

        *meter = getInputMeterFloat(_channel);
        *comp = getCompMeterFloat(_channel);
        channel_cache* cache = get_comp_channel_cache(_channel);

        if (_enable != cache->controls[TASCAM_COMP_ENABLE].new_value) {
            fprintf(stdout, "_enable changed to %d\n", _enable);
            cache->controls[TASCAM_COMP_ENABLE].new_value = _enable;
        }

        if ((_threshold != cache->controls[TASCAM_COMP_TRESHOLD].new_value - 32)) {
            fprintf(stdout, "_threshold changed\n");
            cache->controls[TASCAM_COMP_TRESHOLD].new_value = _threshold + 32;
        }

        if ((_ratio != cache->controls[TASCAM_COMP_RATIO].new_value)) {
            fprintf(stdout, "_ratio changed\n");
            cache->controls[TASCAM_COMP_RATIO].new_value = _ratio;
        }

        if ((_attack != cache->controls[TASCAM_COMP_ATTACK].new_value + 2)) {
            fprintf(stdout, "_attack changed\n");
            cache->controls[TASCAM_COMP_ATTACK].new_value = _attack - 2;
        }

        if ((_release / 10 != (cache->controls[TASCAM_COMP_RELEASE].new_value ) )) {
            fprintf(stdout, "_release changed\n");
            cache->controls[TASCAM_COMP_RELEASE].new_value = _release / 10;
        }

        if ((_gain != cache->controls[TASCAM_COMP_GAIN].new_value)) {
            fprintf(stdout, "_gain changed\n");
            cache->controls[TASCAM_COMP_GAIN].new_value = _gain;
        }
    }
}

extern void
deactivate_comp(LV2_Handle instance) {
    fprintf(stdout, "tascam_lv2: deactivate_comp\n");
}

extern void
cleanup_comp(LV2_Handle instance) {
    fprintf(stdout, "tascam_lv2: cleanup_comp\n");

    free((Tascam_comp_ports*) instance);
    close_device();
}



