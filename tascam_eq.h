#ifndef TASCAM_EQ_H
#define TASCAM_EQ_H

#include <math.h>
#include <stdlib.h>

#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

#include "tascam.h"

#define TASCAM_EQ_URI "http://www.paraair.de/plugins/tascam_eq"

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