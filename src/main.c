#include <ncurses.h>
#include <signal.h>
#include <stdbool.h>
#include <wchar.h>

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

static volatile sig_atomic_t g_running = 1;

static void handle_exit_signal(int signum) {
    (void)signum;
    g_running = 0;
}


typedef struct {
    ma_engine engine;
    ma_sound sound;

    float progress;
    float length;
    bool playing;
} playback_t;


void init_playback(playback_t *playback, const char *path) {
    ma_result result = 0;

    result = ma_engine_init(NULL, &playback->engine);
    if (result != MA_SUCCESS) {
        exit(result);
    }

    result = ma_sound_init_from_file(&playback->engine, path, 0,
                                     NULL, NULL, &playback->sound);
    if (result != MA_SUCCESS) {
        ma_engine_uninit(&playback->engine);
        exit(result);
    }

    ma_sound_get_length_in_seconds(&playback->sound, &playback->length);
}

void render_progressbar(WINDOW *area, const playback_t *playback) {
    wclear(area);
    int w, h;
    getmaxyx(area, h, w);
    (void)h;

    float percentage = playback->progress / playback->length;
    box(area, 0, 0);

    char buffer[256] = {0};
    int meta_length = snprintf(buffer, 256, "%im %is ", (int)playback->progress / 60,
                                         (int)playback->progress % 60);
    int bar_fill = percentage * (w - 3 - meta_length);

    mvwprintw(area, 1, 1, "%s", buffer);
    for (int i = 0; i < bar_fill; i++) {
        mvwprintw(area, 1, i+1+meta_length, "#");
    }
    wrefresh(area);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Error: %s requires atleast one argument\n", argv[0]);
        fprintf(stderr, "Usage: %s [AUDIOFILE]\n", argv[0]);
        return -1;
    }

    signal(SIGINT, handle_exit_signal);
    signal(SIGTERM, handle_exit_signal);

    initscr();
    keypad(stdscr, TRUE);
    noecho();
    cbreak();
    timeout(100);
    curs_set(0);

    playback_t playback = {0};
    init_playback(&playback, argv[1]);

    WINDOW *progressbar = subwin(stdscr, 3, COLS, LINES-3, 0);

    int c = '\0';
    while (g_running && (c = getch()) != 'q') {
        ma_sound_get_cursor_in_seconds(&playback.sound, &playback.progress);

        render_progressbar(progressbar, &playback);

        wrefresh(stdscr);
        switch (c) {
            case ' ':
                if (!playback.playing) {
                    ma_sound_start(&playback.sound);
                    playback.playing = true;
                    break;
                }
                ma_sound_stop(&playback.sound);
                playback.playing = false;
                break;

            case KEY_LEFT:
                ma_sound_seek_to_second(&playback.sound, playback.progress - 1.0f);
                break;

            case KEY_RIGHT:
                ma_sound_seek_to_second(&playback.sound, playback.progress + 1.0f);
                break;

            case KEY_SLEFT:
                ma_sound_seek_to_second(&playback.sound, playback.progress - 10.0f);
                break;

            case KEY_SRIGHT:
                ma_sound_seek_to_second(&playback.sound, playback.progress + 10.0f);
                break;

            case KEY_RESIZE:
                clear();
                refresh();

                wresize(progressbar, 3, COLS); 
                mvwin(progressbar, LINES - 3, 0);

                wrefresh(progressbar);
                render_progressbar(progressbar, &playback);

            default:
                break;
        }
    }

    ma_sound_uninit(&playback.sound);
    ma_engine_uninit(&playback.engine);
    delwin(progressbar);
    endwin();
    return 0;
}
