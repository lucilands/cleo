#include <stdlib.h>
#include <stdio.h>

#include "song.h"
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include <id3tag.h>

const char* get_miniaudio_error_string(ma_result result) {
    switch (result) {
        case MA_SUCCESS:                 return "Success";
        case MA_ERROR:                   return "Generic error";
        case MA_INVALID_ARGS:            return "Invalid arguments";
        case MA_OUT_OF_MEMORY:           return "Out of memory";
        case MA_OUT_OF_RANGE:            return "Out of range";
        case MA_ACCESS_DENIED:           return "Access denied";
        case MA_DOES_NOT_EXIST:          return "Does not exist";
        case MA_ALREADY_EXISTS:          return "Already exists";
        case MA_TOO_MANY_OPEN_FILES:     return "Too many open files";
        case MA_NOT_IMPLEMENTED:         return "Not implemented";
        case MA_DEVICE_ALREADY_INITIALIZED: return "Device already initialized";
        case MA_DEVICE_NOT_INITIALIZED:  return "Device not initialized";
        case MA_DEVICE_NOT_STARTED:      return "Device not started";
        case MA_DEVICE_NOT_STOPPED:      return "Device not stopped";
        case MA_FAILED_TO_INIT_BACKEND:  return "Failed to initialize backend";
        case MA_FAILED_TO_OPEN_BACKEND_DEVICE: return "Failed to open backend device";
        default:                         return "Unknown error code";
    }
}

id3_utf8_t *get_id3tag(struct id3_tag *tag, const char *frameno) {
    struct id3_frame *frame = id3_tag_findframe(tag, frameno, 0);
    if (!frame) return NULL;

    union id3_field *field = id3_frame_field(frame, 1);
    if (!field) return NULL;
    unsigned int nstrings = id3_field_getnstrings(field);        
    for (unsigned int i = 0; i < nstrings; i++) {
        const id3_ucs4_t *ucs4_str = id3_field_getstrings(field, i);
                
        if (ucs4_str) {
            return id3_ucs4_utf8duplicate(ucs4_str);
        }
    }

    return NULL;
}

void init_song(song_t *song, const char * restrict path, ma_engine *engine) {
    ma_result result = ma_sound_init_from_file(engine, path, 0,NULL, NULL, &song->sound);
    if (result) {
        fprintf(stderr, "Error: Failed to load song %s: %s", path, get_miniaudio_error_string(result));
        exit(1);
    }

    ma_sound_get_length_in_seconds(&song->sound, (float*)&song->length);
    struct id3_file *file = id3_file_open(path, ID3_FILE_MODE_READONLY);
    struct id3_tag *tag = id3_file_tag(file);

    song->artist = get_id3tag(tag, ID3_FRAME_ARTIST);
    song->title = get_id3tag(tag, ID3_FRAME_TITLE);

    id3_file_close(file);
}

void delete_song(song_t *song) {
    ma_sound_uninit(&song->sound);
    free((void*)song->title);
    free((void*)song->artist);
}

void song_play(song_t *song) {
    ma_sound_start(&song->sound);
}

void song_pause(song_t *song) {
    ma_sound_stop(&song->sound);
}

void song_toggle(song_t *song) {
    if (song->playing) {
        song_pause(song);
        song->playing = false;
        return;
    }
    song_play(song);
    song->playing = true;
}

void song_skip(song_t *song, float delta) {
    if (song->progress + delta <= 0 || song->progress + delta >= song->length) return;
    ma_sound_seek_to_second(&song->sound, song->progress + delta);
}

void song_update(song_t *song) {
    ma_sound_get_cursor_in_seconds(&song->sound, &song->progress);
}
