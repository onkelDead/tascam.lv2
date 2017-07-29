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
#include "tascam_eq.h"

#include "lv2/lv2plug.in/ns/lv2core/lv2.h"



const LV2_Descriptor descriptor_eq = {
	TASCAM_EQ_URI,
	instantiate_eq,
	connect_port_eq,
	activate_eq,
	run_eq,
	deactivate_eq,
	cleanup_eq,
	extension_data
};

extern LV2_Handle
instantiate_eq(const LV2_Descriptor* descriptor,
	double rate,
	const char* bundle_path,
	const LV2_Feature * const* features)
{

	Tascam_eq_ports* instance = (Tascam_eq_ports*) calloc(1, sizeof(Tascam_eq_ports));

	if (open_device())
		return NULL;

	return(LV2_Handle) instance;
}

extern void
connect_port_eq(LV2_Handle instance,
	uint32_t port,
	void* data)
{
	Tascam_eq_ports* tascam = (Tascam_eq_ports*) instance;

	switch ((PortIndex_eq) port) {
	case TASCAM_EQ_CHANNEL:
		tascam->channel = (const float*) data;
		break;

	case TASCAM_EQ_HIGH_FREQ:
		tascam->highfreq = (const float*) data;
		break;

	case TASCAM_EQ_HIGH_LEVEL:
		tascam->highlevel = (const float*) data;
		break;

	case TASCAM_EQ_MIDHIGH_FREQ:
		tascam->midhighfreq = (const float*) data;
		break;

	case TASCAM_EQ_MIDHIGH_Q:
		tascam->midhighq = (const float*) data;
		break;

	case TASCAM_EQ_MIDHIGH_LEVEL:
		tascam->midhighlevel = (const float*) data;
		break;

	case TASCAM_EQ_MIDLOW_FREQ:
		tascam->midlowfreq = (const float*) data;
		break;

	case TASCAM_EQ_MIDLOW_Q:
		tascam->midlowq = (const float*) data;
		break;

	case TASCAM_EQ_MIDLOW_LEVEL:
		tascam->midlowlevel = (const float*) data;
		break;

	case TASCAM_EQ_LOW_FREQ:
		tascam->lowfreq = (const float*) data;
		break;

	case TASCAM_EQ_LOW_LEVEL:
		tascam->lowlevel = (const float*) data;
		break;

	case TASCAM_EQ_ENABLE:
		tascam->enable = (const float*) data;
		break;

	case TASCAM_EQ_INPUT_LEVEL:
		tascam->input_level = (float*) data;
		break;

	}
}

extern void
activate_eq(LV2_Handle instance)
{

}

extern void
run_eq(LV2_Handle instance, uint32_t n_samples)
{

	const Tascam_eq_ports* tascam = (const Tascam_eq_ports*) instance;

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

	float* meter = tascam->input_level;


	if (_channel != -1) {

		*meter = getInputMeterFloat(_channel);
		channel_cache* cache = get_eq_channel_cache(_channel);

		if (_enable != cache->controls[TASCAM_EQ_ENABLE].new_value) {
			//            fprintf(stdout, "_enable (%p) %d to %d\n", (void*)cache, _channel, _enable );
			cache->controls[TASCAM_EQ_ENABLE].new_value = _enable;
			return;
		}

		if ((_highlevel != cache->controls[TASCAM_EQ_HIGH_LEVEL].new_value - 12)) {
			//            fprintf(stdout, "_highlevel changed\n" );
			cache->controls[TASCAM_EQ_HIGH_LEVEL].new_value = _highlevel + 12;
			return;
		}

		if ((_highfreq != cache->controls[TASCAM_EQ_HIGH_FREQ].new_value)) {
			//            fprintf(stdout, "_highfreq changed\n" );
			cache->controls[TASCAM_EQ_HIGH_FREQ].new_value = _highfreq;
			return;
		}

		if ((_midhighfreq != cache->controls[TASCAM_EQ_MIDHIGH_FREQ].new_value)) {
			//            fprintf(stdout, "_midhighfreq changed\n" );
			cache->controls[TASCAM_EQ_MIDHIGH_FREQ].new_value = _midhighfreq;
			return;
		}

		if ((_midhighq != cache->controls[TASCAM_EQ_MIDHIGH_Q].new_value) /* || bChannelChanged */) {
			//            fprintf(stdout, "_midhighq changed\n" );
			cache->controls[TASCAM_EQ_MIDHIGH_Q].new_value = _midhighq;
			return;
		}

		if ((_midhighlevel != cache->controls[TASCAM_EQ_MIDHIGH_LEVEL].new_value - 12) /* || bChannelChanged */) {
			//            fprintf(stdout, "_midhighlevel(%d) changed to %d\n", _channel, _midhighlevel );
			cache->controls[TASCAM_EQ_MIDHIGH_LEVEL].new_value = _midhighlevel + 12;
			return;
		}

		if ((_midlowfreq != cache->controls[TASCAM_EQ_MIDLOW_FREQ].new_value) /* || bChannelChanged */) {
			//            fprintf(stdout, "_midlowfreq changed\n" );
			cache->controls[TASCAM_EQ_MIDLOW_FREQ].new_value = _midlowfreq;
			return;
		}

		if ((_midlowq != cache->controls[TASCAM_EQ_MIDLOW_Q].new_value) /* || bChannelChanged */) {
			//            fprintf(stdout, "_midlowq changed\n" );
			cache->controls[TASCAM_EQ_MIDLOW_Q].new_value = _midlowq;
			return;
		}

		if ((_midlowlevel != cache->controls[TASCAM_EQ_MIDLOW_LEVEL].new_value - 12) /* || bChannelChanged */) {
			//            fprintf(stdout, "_midlowlevel changed\n" );
			cache->controls[TASCAM_EQ_MIDLOW_LEVEL].new_value = _midlowlevel + 12;
			return;
		}

		if ((_lowlevel != cache->controls[TASCAM_EQ_LOW_LEVEL].new_value - 12) /* || bChannelChanged */) {
			//            fprintf(stdout, "_lowlevel(%d) changed %d\n", _channel, _lowlevel );
			cache->controls[TASCAM_EQ_LOW_LEVEL].new_value = _lowlevel + 12;
			return;
		}

		if ((_lowfreq != cache->controls[TASCAM_EQ_LOW_FREQ].new_value) /* || bChannelChanged */) {
			//            fprintf(stdout, "_lowfreq changed\n" );
			cache->controls[TASCAM_EQ_LOW_FREQ].new_value = _lowfreq;
			return;
		}
	}
}

extern void
deactivate_eq(LV2_Handle instance)
{
	//    fprintf(stdout, "tascam_lv2: deactivate_eq\n");
}

extern void
cleanup_eq(LV2_Handle instance)
{
	//    fprintf(stdout, "tascam_lv2: cleanup_eq\n");

	free((Tascam_eq_ports*) instance);
	close_device();
}

