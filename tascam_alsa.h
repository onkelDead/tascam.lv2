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

extern int cardnum;

int get_alsa_cardnum();

#endif /* TASCAM_ALSA_H */

