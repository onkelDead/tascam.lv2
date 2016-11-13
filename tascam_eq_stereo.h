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

#ifndef TASCAM_EQ_STEREO_H
#define TASCAM_EQ_STEREO_H

#include <math.h>
#include <stdlib.h>
#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

#include "tascam_alsa.h"


#define TASCAM_EQ_STEREO_URI "http://www.paraair.de/plugins/tascam_eq_stereo"


typedef enum {
    TASCAM_EQ_STEREO_ENABLE,
    TASCAM_EQ_STEREO_HIGH_FREQ,
    TASCAM_EQ_STEREO_HIGH_LEVEL,
    TASCAM_EQ_STEREO_MIDHIGH_FREQ,
    TASCAM_EQ_STEREO_MIDHIGH_Q,
    TASCAM_EQ_STEREO_MIDHIGH_LEVEL,
    TASCAM_EQ_STEREO_MIDLOW_FREQ,
    TASCAM_EQ_STEREO_MIDLOW_Q,
    TASCAM_EQ_STEREO_MIDLOW_LEVEL,
    TASCAM_EQ_STEREO_LOW_FREQ,
    TASCAM_EQ_STEREO_LOW_LEVEL,
    TASCAM_EQ_STEREO_CHANNEL,
    TASCAM_EQ_STEREO_INPUT_LEVEL_LEFT,
    TASCAM_EQ_STEREO_INPUT_LEVEL_RIGHT,
    TASCAM_EQ_STEREO_INPUT_LEFT,
    TASCAM_EQ_STEREO_INPUT_RIGHT,
    TASCAM_EQ_STEREO_OUTPUT_LEFT,
    TASCAM_EQ_STEREO_OUTPUT_RIGHT
} PortIndex_eq_stereo;

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
    float* input_level_left;
    float* input_level_right;
    const float* input_left;
    const float* input_right;
    float* output_left;
    float* output_right;

} Tascam_eq_stereo_ports;

static LV2_Handle 
instantiate_eq_stereo(const LV2_Descriptor*     descriptor,
            double                    rate,
            const char*               bundle_path,
            const LV2_Feature* const* features);

static void
connect_port_eq_stereo(LV2_Handle instance,
             uint32_t   port,
             void*      data);

static void
activate_eq_stereo(LV2_Handle instance);

static void
run_eq_stereo(LV2_Handle instance, uint32_t n_samples);

static void
deactivate_eq_stereo(LV2_Handle instance);

static void
cleanup_eq_stereo(LV2_Handle instance);

extern LV2_Descriptor descriptor_eq_stereo;

#endif