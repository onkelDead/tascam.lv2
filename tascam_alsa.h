/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   tascam_alsa.h
 * Author: root
 *
 * Created on November 8, 2016, 2:31 PM
 */

#ifndef TASCAM_ALSA_H
#define TASCAM_ALSA_H

#include <alsa/asoundlib.h>

#include <pthread.h>
#include <unistd.h>
#include <mqueue.h>

extern int cardnum;

int get_alsa_cardnum();
int open_device();
void close_device();
void setInteger(const char* name, int channel, int value);
int getInteger(const char* name);

snd_hctl_elem_t* get_ctrl_elem(const char* name, int index);
void setElemInteger(snd_hctl_elem_t *elem, int value);

#endif /* TASCAM_ALSA_H */

