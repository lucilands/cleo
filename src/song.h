#ifndef _SONG_H
#define _SONG_H
#include "miniaudio.h"
#include <id3tag.h>
#include <stdbool.h>


typedef struct {
    ma_sound sound;
    const float length;
    float progress;
    bool playing;

    const id3_utf8_t *title;
    const id3_utf8_t *artist;
} song_t;

void init_song(song_t *song, const char * restrict path, ma_engine *engine);
void delete_song(song_t *song);

void song_play(song_t *song);
void song_pause(song_t *song);
void song_toggle(song_t *song);
void song_skip(song_t *song, float delta);
void song_update(song_t *song);

#endif //_SONG_H
