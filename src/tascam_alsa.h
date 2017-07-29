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

// EQ control element cache
typedef struct {
    int last_value;
    int new_value;
    char* name;
} control_cache;

// EQ channel cache
typedef struct {
    int channel;
    int num_controls;
    control_cache controls[12];
    
} channel_cache;

/**
 * Get the EQ channel cache which corresponds to a specific channel
 * @param channel_index The index of the channel to be returned.
 * @return A pointer to a structure of type eq_channel_cache.
 */
channel_cache* get_eq_channel_cache(int channel_index);

/**
 * Get the Compressor channel cache which corresponds to a specific channel
 * @param channel_index The index of the channel to be returned.
 * @return A pointer to a structure of type eq_channel_cache.
 */
channel_cache* get_comp_channel_cache(int channel_index);

// alsa device control functions
int get_alsa_cardnum();
int open_device();
void close_device();

/**
 * Request the meter value of the output range meter on the channel identified by parameter channel_index
 * @param channel_index An integer of the channel index to be returned.
 * @return The value of the meter.
 */
float getInputMeterFloat(int channel_index);

/**
 * Request the meter value of the compression range meter on the channel identified by parameter channel_index
 * @param channel_index An integer of the channel index to be returned.
 * @return The value of the compression meter.
 */
float getCompMeterFloat(int channel_index);
/**
 * Creates a complete name of an alsa conrol element used to set or get values.
 * @param name Pointer to a C-type string contains the names stub.
 * @param index An integer value of the channel index to use
 * @param result Pointer to a Pointer of C-type string to hold the result.
 * @param size A size_t variable specified the siz of the result buffer.
 */
void get_ctrl_elem_name(const char* name, int index, char* result[], size_t size);

/**
 * Function to set an integer control value of an opened alsa device.
 * @param hctl Pointer to a structure of type snd_hctl_t, which represents an onpened alsa device.
 * @param name A C-type string contains the complete name of the control element.
 * @param value The int value to be set.
 */
void setInteger(snd_hctl_t *hctl, const char* name, int value);

/**
 * Gets a n array of integer values from an alsa control element.
 * @param elem A pointer to a structure of type snd_hctl_elem_t, which identifies the alsa control.
 * @param vals An array of integers to store the requested values.
 * @param count the count of elements in the array vals
 * @return 0 on success, otherwise -1;
 */
int getIntegers(snd_hctl_elem_t *elem, int vals[], int count);


#endif /* TASCAM_ALSA_H */

