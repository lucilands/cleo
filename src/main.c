#include "song.h"
#include <ncurses.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>

#include <id3tag.h>


static volatile sig_atomic_t g_running = 1;

static void handle_exit_signal(int signum) {
    (void)signum;
    g_running = 0;
}

typedef struct {
    ma_engine engine;
    bool playing;

    song_t song;
} playback_t;


void init_playback(playback_t *playback, const char *path) {
    ma_result result = 0;

    result = ma_engine_init(NULL, &playback->engine);
    if (result != MA_SUCCESS) {
        exit(result);
    }

    init_song(&playback->song, path, &playback->engine);
}

void render_progressbar(WINDOW *area, const playback_t *playback) {
    wclear(area);
    int w, h;
    getmaxyx(area, h, w);
    (void)h;

    float percentage = playback->song.progress / playback->song.length;
    box(area, 0, 0);

    char buffer[256] = {0};
    int meta_length = snprintf(buffer, 256, "%im %is ", (int)playback->song.progress / 60,
                                         (int)playback->song.progress % 60);
    int bar_fill = percentage * (w - 3 - meta_length);

    if (playback->song.title && playback->song.artist) mvwprintw(area, 0, 1, "%s - %s", playback->song.title, playback->song.artist);
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
        song_update(&playback.song);

        render_progressbar(progressbar, &playback);

        wrefresh(stdscr);
        switch (c) {
            case ' ':
                song_toggle(&playback.song);
                break;

            case KEY_LEFT:
                song_skip(&playback.song, -1.0f);
                break;

            case KEY_RIGHT:
                song_skip(&playback.song, 1.0f);
                break;

            case KEY_SLEFT:
                song_skip(&playback.song, -10.0f);
                break;

            case KEY_SRIGHT:
                song_skip(&playback.song, 10.0f);
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

    delete_song(&playback.song);

    ma_engine_uninit(&playback.engine);
    delwin(progressbar);
    endwin();
    return 0;
}
