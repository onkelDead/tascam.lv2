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
#include "tascam_comp.h"

#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

const LV2_Descriptor descriptor_comp = {
	TASCAM_URI_COMP,
	instantiate_comp,
	connect_port_comp,
	activate_comp,
	run_comp,
	deactivate_comp,
	cleanup_comp,
	extension_data
};

typedef enum {
	TASCAM_CHANNEL   = 0,
	TASCAM_INPUT  = 1,
	TASCAM_OUTPUT = 2
} PortIndex_comp;

typedef struct {
	// Port buffers
	const float* channel;
	const float* input;
	float*       output;
} Tascam;

extern LV2_Handle
instantiate_comp(const LV2_Descriptor*     descriptor,
            double                    rate,
            const char*               bundle_path,
            const LV2_Feature* const* features)
{
	Tascam* tascam = (Tascam*)calloc(1, sizeof(Tascam));

	return (LV2_Handle)tascam;
}

extern void
connect_port_comp(LV2_Handle instance,
             uint32_t   port,
             void*      data)
{
	Tascam* tascam = (Tascam*)instance;

	switch ((PortIndex_comp)port) {
	case TASCAM_CHANNEL:
		tascam->channel = (const float*)data;
		break;
	case TASCAM_INPUT:
		tascam->input = (const float*)data;
		break;
	case TASCAM_OUTPUT:
		tascam->output = (float*)data;
		break;
	}
}

extern void
activate_comp(LV2_Handle instance)
{
}

/** Define a macro for converting a gain in dB to a coefficient. */
#define DB_CO(g) ((g) > -90.0f ? powf(10.0f, (g) * 0.05f) : 0.0f)

extern void
run_comp(LV2_Handle instance, uint32_t n_samples)
{
	const Tascam* tascam = (const Tascam*)instance;

	const float        channel   = *(tascam->channel);
	const float* const input  = tascam->input;
	float* const       output = tascam->output;

// TODO: DU may be not needed cause no effect
	for (uint32_t pos = 0; pos < n_samples; pos++) {
		output[pos] = input[pos];
	}
}

extern void
deactivate_comp(LV2_Handle instance)
{
}

extern void
cleanup_comp(LV2_Handle instance)
{
	free(instance);
}



