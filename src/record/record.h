/*
 * 2019 Tarpeeksi Hyvae Soft /
 * VCS
 *
 */

#ifndef RECORD_H
#define RECORD_H

void krecord_initialize_recording(const char *const filename, const uint width, const uint height, const uint frameRate);

bool krecord_is_recording(void);

void krecord_record_new_frame(void);

void krecord_finalize_recording(void);

#endif
