libdir = ${prefix}/lib/lv2/tascam.lv2
datadir = $(prefix)/lib/lv2/tascam.lv2

libtascam_ladir = src
lib_LTLIBRARIES = libtascam.la
libtascam_la_SOURCES = src/tascam.c src/tascam_alsa.c src/tascam_comp.c src/tascam_comp_stereo.c src/tascam_eq.c src/tascam_eq_stereo.c

data_DATA = data/manifest.ttl data/tascam_comp_stereo.ttl data/tascam_comp.ttl data/tascam_eq_stereo.ttl data/tascam_eq.ttl
dist_data_DATA = data/manifest.ttl data/tascam_comp_stereo.ttl  data/tascam_comp.ttl  data/tascam_eq_stereo.ttl  data/tascam_eq.ttl

libtascam_la_CFLAGS = -O2 ${ALSA_CFLAGS}
libtascam_la_LIBADD = ${ALSA_LIBS}

