/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   tascam.h
 * Author: root
 *
 * Created on November 8, 2016, 2:45 PM
 */

#ifndef TASCAM_H
#define TASCAM_H

#define NUM_CHANNELS 16

#include "tascam_eq.h"
#include "tascam_comp.h"
#include "tascam_alsa.h"

extern const void*
extension_data(const char* uri);

#endif /* TASCAM_H */

