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


#include <alloca.h>
#include <errno.h>
#include <pthread.h>


#include "tascam.h"
#include "tascam_alsa.h"


//* Id of the Tascam US-16x08 alsa card 
static int cardnum = -1;

//* HCTL handle of the Tascam card 
static snd_hctl_t *hctl = 0;

/** Tascam card reference counter. 
 * Will be incremented for each successfully open_device call an decremented each time the card will be closed.
 * If the reference counter reaches zero, the alsa card will be closed, and all resource gets freed.
 */
static int ref_counter = 0;

//* Global control snd element control value.
static snd_ctl_elem_value_t *control_int = 0;

//* pthread for the cache transfer from LV2 to Alsa card. 
static pthread_t thread;

//* Flag to indicate the thread control (on/off))
static int b_shutdown = 1;

//* EQ control element names.
const char* eq_control_path[] = {
    "name='9 EQ',index=",
    "name='81 HighFreq',index=",
    "name='8 High',index=",
    "name='71 MHiFreq',index=",
    "name='72 MHiWidth',index=",
    "name='7 MHigh',index=",
    "name='61 MLowFreq',index=",
    "name='62 MLowWidth',index=",
    "name='6 MLow',index=",
    "name='51 LowFreq',index=",
    "name='5 Low',index=",
    NULL
};

//* Compressor control element names.
const char* comp_control_path[] = {
    "name='A Comp',index=",
    "name='B Thresh',index=",
    "name='C Ratio',index=",
    "name='D Attack',index=",
    "name='E Release',index=",
    "name='F Gain',index=",
    NULL
};



//* static pointer to the EQ cache
static channel_cache *eq_cache[16];

//* static pointer to the EQ cache
static channel_cache *comp_cache[16];

//* static array of integers to hold the input meter values.
static int meters[35] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

/**
 * Thread function to identify changed cache values. Each change will be posted to Alsa card.
 * Add. this function requests the Tascam meter values and stores them in the array meters.
 * @param arg Not used, should be NULL.
 * @return Always 0
 */
void* doSomeThing(void *arg) {
    snd_ctl_elem_id_t *id;
    snd_ctl_elem_id_alloca(&id);
    static snd_hctl_elem_t *elem;

    int i = 0;
    int j = 0;

    int err = snd_ctl_ascii_elem_id_parse(id, "name='Z Meter'");
    if (err) {
        fprintf(stderr, "Wrong control identifier: name='Z Meter' (%d)\n", err);
        return 0;
    }

    elem = snd_hctl_find_elem(hctl, id);

    do {

        usleep(10000);

        getIntegers(elem, meters, 34);

        for (int i = 0; i < NUM_CHANNELS; i++) {
            for (j = 0; j < eq_cache[i]->num_controls; j++) {
                int new_value = eq_cache[i]->controls[j].new_value;
                int old_value = eq_cache[i]->controls[j].last_value;
                if (new_value != old_value) {
//                    fprintf(stderr, "tascam.lv2:do_something(): change %s %d\n", eq_cache[i]->controls[j].name, new_value);
                    setInteger(hctl, eq_cache[i]->controls[j].name, new_value);
                    eq_cache[i]->controls[j].last_value = new_value;
                    continue;
                }
            }
            for (j = 0; j < comp_cache[i]->num_controls; j++) {
                int new_value = comp_cache[i]->controls[j].new_value;
                int old_value = comp_cache[i]->controls[j].last_value;
                if (new_value != old_value) {
//                    fprintf(stderr, "tascam.lv2:do_something(): change %s %d\n", comp_cache[i]->controls[j].name, new_value);
                    setInteger(hctl, comp_cache[i]->controls[j].name, new_value);
                    comp_cache[i]->controls[j].last_value = new_value;
                    continue;
                }
            }
        }

    } while (!b_shutdown);
    return 0;
}

int get_alsa_cardnum() {
    int card = -1;
    int err = 0;
    char name[32];
    snd_ctl_t *handle;
    snd_ctl_card_info_t *cinfo;

    if (cardnum != -1)
        return cardnum;

    snd_ctl_card_info_alloca(&cinfo);

    cardnum = -1;

    if (snd_card_next(&card) < 0 || card < 0) {
        fprintf(stderr, "tascam.lv2: No sound card found.");
        return -1;
    }
    while (card >= 0) {
        sprintf(name, "hw:%d", card);

        if ((err = snd_ctl_open(&handle, name, 0)) < 0) {
            fprintf(stderr, "tascam.lv2: Control %s open error: %s\n", name, snd_strerror(err));
            goto next_card;
        }
#ifdef DEBUG
        fprintf(stdout, "card opened\n");
#endif
        if ((err = snd_ctl_card_info(handle, cinfo)) < 0) {
            fprintf(stderr, "tascam.lv2: Control hardware info (%i): %s", card, snd_strerror(err));
            snd_ctl_close(handle);
            goto next_card;
        }
#ifdef DEBUG
        fprintf(stdout, "device: [%s] %s\n", snd_ctl_card_info_get_id(cinfo), snd_ctl_card_info_get_name(cinfo));
#endif
        if (strcmp("US16x08", snd_ctl_card_info_get_id(cinfo)) == 0) {
            cardnum = card;
            return cardnum;
        } else {
            snd_ctl_close(handle);
#ifdef DEBUG
            fprintf(stderr, "card closed\n");
#endif
        }
next_card:
        if (snd_card_next(&card) < 0) {
            fprintf(stderr, "tascam.lv2: snd_card_next failed");
            break;
        }
    }
    if (cardnum == -1) {
        fprintf(stderr, "tascam.lv2: No proper sound card found.");
        return -1;
    }
    return cardnum;
}

int open_device() {
    int err = 0;
    int i = 0, j;
    char name[32];

//    fprintf(stdout, "tascam.lv2: open_device()\n");

    if (ref_counter == 0) {

        if (cardnum == -1) {
            cardnum = get_alsa_cardnum();
            if (cardnum == -1)
                return -1;
        }

        sprintf(name, "hw:%d", cardnum);
        if ((err = snd_hctl_open(&hctl, name, 0)) < 0) {
            fprintf(stderr, "Control %s open error: %s\n", name, snd_strerror(err));
            return -1;
        }
        if ((err = snd_hctl_load(hctl)) < 0) {
            fprintf(stderr, "Control %s load error: %s\n", name, snd_strerror(err));
            return -1;
        }

        for (j = 0; j < NUM_CHANNELS; j++) {
            eq_cache[j] = calloc(1, sizeof (channel_cache));
            eq_cache[j]->channel = j;
            i = 0;
            while (eq_control_path[i]) {
                eq_cache[j]->controls[i].name = (char*) malloc(32);
                get_ctrl_elem_name(eq_control_path[i], j, &(eq_cache[j]->controls[i].name), 32);
                eq_cache[j]->controls[i].last_value = eq_cache[j]->controls[i++].new_value = -1;
            }
            eq_cache[j]->num_controls = i;
            
            comp_cache[j] = calloc(1, sizeof(channel_cache));
            comp_cache[j]->channel = j;
            i = 0;
            while (comp_control_path[i]) {
                comp_cache[j]->controls[i].name = (char*) malloc(32);
                get_ctrl_elem_name(comp_control_path[i], j, &(comp_cache[j]->controls[i].name), 32);
                comp_cache[j]->controls[i].last_value = comp_cache[j]->controls[i++].new_value = -1;
            }
            comp_cache[j]->num_controls = i;
        }

        snd_ctl_elem_value_alloca(&control_int);

        b_shutdown = 0;
        pthread_create(&thread, NULL, doSomeThing, NULL);

//        fprintf(stdout, "tascam.lv2: device opened\n");
    } 
//    else
//        fprintf(stdout, "tascam.lv2: ref_counter = %d\n", ref_counter);

    ref_counter++;
    return 0;
}

channel_cache* get_eq_channel_cache(int channel_index) {
    return eq_cache[channel_index];
}

channel_cache* get_comp_channel_cache(int channel_index) {
    return comp_cache[channel_index];
}

void close_device() {
    int i = 0, j;
    if (--ref_counter == 0) {
//        fprintf(stdout, "tascam_lv2: close_device\n");

        b_shutdown = 1;
        pthread_join(thread, NULL);

        if (hctl) {
            snd_hctl_close(hctl);
            hctl = 0;
        }
        
        for (j = 0; j < NUM_CHANNELS; j++) {
            i = 0;
            while (eq_control_path[i]) {
                free(eq_cache[j]->controls[i++].name);
            }
            free(eq_cache[j]);
        }        
    }
}

snd_hctl_elem_t* get_ctrl_elem(const char* name) {
    snd_ctl_elem_id_t *id;
    snd_ctl_elem_id_alloca(&id);
    snd_hctl_elem_t *elem;

    int err = snd_ctl_ascii_elem_id_parse(id, name);
    elem = snd_hctl_find_elem(hctl, id);
    return elem;
}

void get_ctrl_elem_name(const char* name, int index, char* result[], size_t size) {
    snd_ctl_elem_id_t *id;
    snd_ctl_elem_id_alloca(&id);
    snd_hctl_elem_t *elem;

    snprintf(*result, size, "%s%d", name, index);
    return;
}

int setElemInteger(snd_hctl_elem_t *elem, int value) {
    int err;

    snd_ctl_elem_value_set_integer(control_int, 0, value);
    if ((err = snd_hctl_elem_write(elem, control_int)) < 0) {
        fprintf(stderr, "Control %s element write error: %s\n", "hw:0", snd_strerror(err));
        return -1;
    }
    return 0;
}

void setInteger(snd_hctl_t *hctl, const char* name, int value) {
    snd_ctl_elem_id_t *id;
    snd_ctl_elem_id_alloca(&id);
    snd_hctl_elem_t *elem;

    int err = snd_ctl_ascii_elem_id_parse(id, name);
    elem = snd_hctl_find_elem(hctl, id);
    if (elem) {
        fflush(stdout);
        snd_ctl_elem_value_t *control;
        snd_ctl_elem_value_alloca(&control);
        snd_ctl_elem_value_set_integer(control, 0, value);
        if ((err = snd_hctl_elem_write(elem, control)) < 0) {
            fprintf(stderr, "Control %s element read error: %s\n", "hw:0", snd_strerror(err));
            return;
        }
    }
}

int getIntegers(snd_hctl_elem_t *elem, int vals[], int count) {
    int val = 0, err;

    snd_ctl_elem_value_t *control;
    snd_ctl_elem_value_alloca(&control);
    if ((err = snd_hctl_elem_read(elem, control)) < 0) {
        fprintf(stderr, "Control %s element read error: %s\n", "hw:0", snd_strerror(err));
        return -1;
    }
    else {
        for (val = 0; val < count; val++)
            vals[val] = snd_ctl_elem_value_get_integer(control, val);
    }
    return 0;
}


float getInputMeterFloat(int channel_index) {
    return ((float) meters[channel_index]) / 0x7fff ;
}


float getCompMeterFloat(int channel_index) {
    return -((float) meters[channel_index + 18]) / 0x7fff;
}
