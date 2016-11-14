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

#ifndef TASCAM_COMP_H
#define TASCAM_COMP_H

#include <math.h>
#include <stdlib.h>
#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

#include "tascam_alsa.h"

#define TASCAM_COMP_URI "http://www.paraair.de/plugins/tascam_comp"

typedef enum {
    TASCAM_COMP_ENABLE,
    TASCAM_COMP_TRESHOLD,
    TASCAM_COMP_RATIO,
    TASCAM_COMP_ATTACK,
    TASCAM_COMP_RELEASE,
    TASCAM_COMP_GAIN,
    TASCAM_COMP_CHANNEL,
    TASCAM_COMP_INPUT_LEVEL,
    TASCAM_COMP_COMP_LEVEL,
} PortIndex_comp;


typedef struct {
	// Port buffers
	const float* channel;
        const float* threshold;
        const float* ratio;
        const float* attack;
        const float* release;
        const float* gain;
        const float* enable;
        float* input_level;
        float* comp_level;
	const float* input;
	float*       output;
} Tascam_comp_ports;

extern LV2_Handle
instantiate_comp(const LV2_Descriptor*     descriptor,
            double                    rate,
            const char*               bundle_path,
            const LV2_Feature* const* features);

extern void
connect_port_comp(LV2_Handle instance,
             uint32_t   port,
             void*      data);

extern void
activate_comp(LV2_Handle instance);

extern void
run_comp(LV2_Handle instance, uint32_t n_samples);

extern void
deactivate_comp(LV2_Handle instance);

extern void
cleanup_comp(LV2_Handle instance);

extern const LV2_Descriptor descriptor_comp;

#endif // TASCAM_COMP_H
