#include <math.h>
#include <stdlib.h>

#include "tascam.h"

#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

#define TASCAM_URI_COMP "http://www.paraair.de/plugins/tascam_comp"

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

