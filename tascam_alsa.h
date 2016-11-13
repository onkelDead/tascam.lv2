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


#ifndef TASCAM_ALSA_H
#define TASCAM_ALSA_H

#include <alsa/asoundlib.h>

#include <pthread.h>
#include <unistd.h>
#include <mqueue.h>


typedef struct {
    int last_value;
    int new_value;
    char* name;
    snd_hctl_elem_t* elem;
    
} control_cache;

typedef struct {
    int channel;
    int num_controls;
    control_cache controls[12];
    
} channel_cache;


int get_alsa_cardnum();
int open_device();
void close_device();

channel_cache* get_eq_channel_cache(int channel_index);

//void setInteger(const char* name, int channel, int value);
//int getInteger(const char* name);

snd_hctl_elem_t* get_ctrl_elem(const char* name);
void get_ctrl_elem_name(const char* name, int index, char* result[], size_t size);
int setElemInteger(snd_hctl_elem_t *elem, int value);
void setInteger(snd_hctl_t *hctl, const char* name, int value);
int getIntegers(snd_hctl_elem_t *elem, int vals[], int count);

float getMeterFloat(int index);

#endif /* TASCAM_ALSA_H */

