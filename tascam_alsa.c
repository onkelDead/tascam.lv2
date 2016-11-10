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

#include <unistd.h>

#include "tascam_alsa.h"

int found = 0;
int cardnum = -1;

snd_hctl_t *hctl = 0;
int ref_counter = 0;

snd_ctl_elem_value_t *control_int = 0;

pthread_t thread;

int b_shutdown = 1;

int meters[19];

void* doSomeThing(void *arg) {
    snd_ctl_elem_id_t *id;
    snd_ctl_elem_id_alloca(&id);
    snd_hctl_elem_t *elem;
    
    int err = snd_ctl_ascii_elem_id_parse(id, "name='Z Meter'");
    if (err) {
        fprintf(stderr, "Wrong control identifier: name='Z Meter' (%d)\n", err);
        return 0;
    }

    elem = snd_hctl_find_elem(hctl, id);    
    
    do {
//        fprintf(stdout, "doSomeThing\n");
        usleep(10000);
        getIntegers(elem, meters, 19 );
    } while (!b_shutdown);
    return 0;
}


int get_alsa_cardnum() {
    int card = -1;
    int err = 0;
    char name[32];
    snd_ctl_t *handle;
    snd_ctl_card_info_t *cinfo;   
    
    if( found )
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
        if( strcmp("US16x08", snd_ctl_card_info_get_id(cinfo)) == 0 ) {
            cardnum = card;
            return cardnum;
        }
        else {
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
    if( cardnum == -1 ) {
        fprintf(stderr, "tascam.lv2: No proper sound card found.");
        return -1;
    }
    found = 1;
    return cardnum;
}

int open_device() {
    int err = 0;
    char name[32];
    fprintf(stdout, "tascam.lv2: open_device\n");
    if( ref_counter == 0 ) {
    
        sprintf(name, "hw:%d", cardnum);
        if ((err = snd_hctl_open(&hctl, name, 0)) < 0) {
            fprintf(stderr, "Control %s open error: %s\n", name, snd_strerror(err));
            return -1;
        }
        if ((err = snd_hctl_load(hctl)) < 0) {
            fprintf(stderr, "Control %s load error: %s\n", name, snd_strerror(err));
            return -1;
        }
        
        snd_ctl_elem_value_alloca(&control_int);        
        
        b_shutdown = 0;                
        pthread_create(&thread, NULL, doSomeThing, NULL);
        
        fprintf(stdout, "tascam.lv2: device opened\n");
    }
    else
        fprintf(stdout, "tascam.lv2: ref_counter = %d\n", ref_counter);
    
    
    ref_counter++;
    return 0;
}

void close_device() {
    if( --ref_counter == 0) {
        fprintf(stdout, "tascam_lv2: close_device\n");
//
//        if( control_int ) {
//            snd_ctl_elem_value_free(control_int);
//            control_int = 0;
//        }
        
        b_shutdown = 1;
        pthread_join(thread, NULL);
        
        if( hctl ) {
            snd_hctl_close(hctl);
            hctl = 0;
        }
        
        
        
    }
}

snd_hctl_elem_t* get_ctrl_elem(const char* name, int index) {
    snd_ctl_elem_id_t *id;
    snd_ctl_elem_id_alloca(&id);
    snd_hctl_elem_t *elem;    
    char num[32];
    
    sprintf(num, "%s%d", name, index);

    int err = snd_ctl_ascii_elem_id_parse(id, num);
    elem = snd_hctl_find_elem(hctl, id);
    return elem;
}

int setElemInteger(snd_hctl_elem_t *elem, int value) {
    int err;

    snd_ctl_elem_value_set_integer(control_int, 0, value);
    if ((err = snd_hctl_elem_write(elem, control_int)) < 0) {
        fprintf(stderr, "Control %s element read error: %s\n", "hw:0", snd_strerror(err));
        return -1;
    }
    return 0;
}

int getIntegers(snd_hctl_elem_t *elem, int vals[], int count) {
    int val = 0, err;

    snd_ctl_elem_value_t *control;
    snd_ctl_elem_value_alloca(&control);
    if ((err = snd_hctl_elem_read(elem, control)) < 0) {
        fprintf(stderr, "Control %s element read error: %s\n", "hw:0", snd_strerror(err));
        return -5;
    }
    for( val = 0; val < count; val++)
        vals[val] = snd_ctl_elem_value_get_integer(control, val);

    return val;
}

float getMeterFloat(int index) {
    return ((float)meters[index]) / 0x7fff;
}


#if 0
void setInteger(const char* name, int channel, int value) {
    snd_ctl_elem_id_t *id;
    snd_ctl_elem_id_alloca(&id);
    snd_hctl_elem_t *elem;    

    int err = snd_ctl_ascii_elem_id_parse(id, name);
    elem = snd_hctl_find_elem(hctl, id);
    if (elem) {
        snd_ctl_elem_value_t *control_int;
        snd_ctl_elem_value_alloca(&control_int);
        snd_ctl_elem_value_set_integer(control_int, 0, value);
        if ((err = snd_hctl_elem_write(elem, control_int)) < 0) {
            fprintf(stderr, "Control %s element read error: %s\n", "hw:0", snd_strerror(err));
            return;
        }
    }
}

int getInteger(const char* name) {
    int val = 0;
    snd_ctl_elem_id_t *id;
    snd_ctl_elem_id_alloca(&id);
    snd_hctl_elem_t *elem;  
    
    int err = snd_ctl_ascii_elem_id_parse(id, name);
    if (err) {
        fprintf(stderr, "Wrong control identifier: %s (%d)\n", name, err);
        return -1;
    }
    elem = snd_hctl_find_elem(hctl, id);
    if (elem) {
        snd_ctl_elem_value_t *control_int;
        snd_ctl_elem_value_alloca(&control_int);
        if ((err = snd_hctl_elem_read(elem, control_int)) < 0) {
            fprintf(stderr, "Control %s element read error: %s\n", "hw:0", snd_strerror(err));
            return -5;
        }
        val = snd_ctl_elem_value_get_integer(control_int, 0);

    }
    return val;

}    
#endif
