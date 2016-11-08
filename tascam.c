#include <math.h>
#include <stdlib.h>

#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

#include "tascam.h"
#include "tascam_eq.h"
#include "tascam_comp.h"


LV2_SYMBOL_EXPORT
const LV2_Descriptor*
lv2_descriptor(uint32_t index)
{
	switch (index) {
	case 0:  
		return &descriptor_eq;
	case 1:  
		return &descriptor_comp;
	default: 
		return NULL;
	}
}

const void*
extension_data(const char* uri)
{
	return NULL;
}

