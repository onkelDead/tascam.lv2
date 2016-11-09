#ifndef TASCAM_EQ_H
#define TASCAM_EQ_H

#include <math.h>
#include <stdlib.h>

#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

#include "tascam.h"

#define TASCAM_EQ_URI "http://www.paraair.de/plugins/tascam_eq"


typedef enum {
    TASCAM_EQ_CHANNEL,
    TASCAM_EQ_INPUT,
    TASCAM_EQ_OUTPUT,
    TASCAM_EQ_HIGH_FREQ,
    TASCAM_EQ_HIGH_LEVEL,
    TASCAM_EQ_MIDHIGH_FREQ,
    TASCAM_EQ_MIDHIGH_Q,
    TASCAM_EQ_MIDHIGH_LEVEL,
    TASCAM_EQ_MIDLOW_FREQ,
    TASCAM_EQ_MIDLOW_Q,
    TASCAM_EQ_MIDLOW_LEVEL,
    TASCAM_EQ_LOWFREQ,
    TASCAM_EQ_LOWLEVEL,
    TASCAM_EQ_ENABLE
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
    const float* input;
    float* output;
    void* cache;
} Tascam_eq;

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

extern LV2_Descriptor descriptor_eq;

#endif