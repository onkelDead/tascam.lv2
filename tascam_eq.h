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

#ifndef TASCAM_EQ_H
#define TASCAM_EQ_H

#include <math.h>
#include <stdlib.h>
#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

#include "tascam_alsa.h"

#define TASCAM_EQ_URI "http://www.paraair.de/plugins/tascam_eq"

typedef enum {
    TASCAM_EQ_ENABLE,
    TASCAM_EQ_HIGH_FREQ,
    TASCAM_EQ_HIGH_LEVEL,
    TASCAM_EQ_MIDHIGH_FREQ,
    TASCAM_EQ_MIDHIGH_Q,
    TASCAM_EQ_MIDHIGH_LEVEL,
    TASCAM_EQ_MIDLOW_FREQ,
    TASCAM_EQ_MIDLOW_Q,
    TASCAM_EQ_MIDLOW_LEVEL,
    TASCAM_EQ_LOW_FREQ,
    TASCAM_EQ_LOW_LEVEL,
    TASCAM_EQ_CHANNEL,
    TASCAM_EQ_INPUT_LEVEL,
} PortIndex_eq;

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
    float* input_level;
} Tascam_eq_ports;

extern LV2_Handle 
instantiate_eq(const LV2_Descriptor*     descriptor,
            double                    rate,
            const char*               bundle_path,
            const LV2_Feature* const* features);

extern void
connect_port_eq(LV2_Handle instance,
             uint32_t   port,
             void*      data);

extern void
activate_eq(LV2_Handle instance);

extern void
run_eq(LV2_Handle instance, uint32_t n_samples);

extern void
deactivate_eq(LV2_Handle instance);

extern void
cleanup_eq(LV2_Handle instance);

extern const LV2_Descriptor descriptor_eq;

#endif
