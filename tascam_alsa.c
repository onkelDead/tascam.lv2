/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   tascam_alsa.c
 * Author: root
 *
 * Created on November 8, 2016, 2:33 PM
 */

#include <alloca.h>
#include <errno.h>
#include "tascam_alsa.h"

int found = 0;
int cardnum = -1;

snd_hctl_t *hctl = 0;
int ref_counter = 0;


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

void setElemInteger(snd_hctl_elem_t *elem, int value) {
    int err;
    if (elem) {
        snd_ctl_elem_value_t *control;
        snd_ctl_elem_value_alloca(&control);
        snd_ctl_elem_value_set_integer(control, 0, value);
        if ((err = snd_hctl_elem_write(elem, control)) < 0) {
            fprintf(stderr, "Control %s element read error: %s\n", "hw:0", snd_strerror(err));
            return;
        }
    }
    
}
void setInteger(const char* name, int channel, int value) {
    snd_ctl_elem_id_t *id;
    snd_ctl_elem_id_alloca(&id);
    snd_hctl_elem_t *elem;    

    int err = snd_ctl_ascii_elem_id_parse(id, name);
    elem = snd_hctl_find_elem(hctl, id);
    if (elem) {
        snd_ctl_elem_value_t *control;
        snd_ctl_elem_value_alloca(&control);
        snd_ctl_elem_value_set_integer(control, 0, value);
        if ((err = snd_hctl_elem_write(elem, control)) < 0) {
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
        snd_ctl_elem_value_t *control;
        snd_ctl_elem_value_alloca(&control);
        if ((err = snd_hctl_elem_read(elem, control)) < 0) {
            fprintf(stderr, "Control %s element read error: %s\n", "hw:0", snd_strerror(err));
            return -5;
        }
        val = snd_ctl_elem_value_get_integer(control, 0);

    }
    return val;

}    
