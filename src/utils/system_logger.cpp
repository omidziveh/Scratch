#include "system_logger.h"
#include "../gfx/SDL2_gfxPrimitives.h"
#include <cstring>
#include <string>

static SysLogEntry entries[SYSLOG_MAX_ENTRIES];
static int entry_count = 0;
static int entry_start = 0;   
static bool visible = false;

void syslog_init() {
    entry_count = 0;
    entry_start = 0;
    visible = false;
    memset(entries, 0, sizeof(entries));
}

void syslog_log(int block_id, const std::string& name) {
    int index;
    if (entry_count < SYSLOG_MAX_ENTRIES) {
        index = entry_count;
        entry_count++;
    } else {
        index = entry_start;
        entry_start = (entry_start + 1) % SYSLOG_MAX_ENTRIES;
    }

    entries[index].block_id = block_id;
    entries[index].timestamp = SDL_GetTicks();

    std::string msg = "[" + std::to_string(block_id) + "] " + name;
    strncpy(entries[index].message, msg.c_str(), 63);
    entries[index].message[63] = '\0';
}

void syslog_clear() {
    entry_count = 0;
    entry_start = 0;
    memset(entries, 0, sizeof(entries));
}

void syslog_toggle() {
    visible = !visible;
}

bool syslog_is_visible() {
    return visible;
}

int syslog_get_count() {
    return entry_count;
}

static int get_real_index(int i) {
    return (entry_start + i) % SYSLOG_MAX_ENTRIES;
}

void syslog_render(SDL_Renderer* renderer) {
    if (!visible) return;
    if (!renderer) return;

    int bx = SYSLOG_BOX_X;
    int by = SYSLOG_BOX_Y;
    int bw = SYSLOG_BOX_W;
    int bh = SYSLOG_BOX_H;

    boxRGBA(renderer, bx, by, bx + bw, by + bh, 15, 15, 15, 230);

    rectangleRGBA(renderer, bx, by, bx + bw, by + bh, 0, 255, 0, 255);

    stringRGBA(renderer, bx + 10, by + 8, "=== BLACK BOX ===", 0, 255, 0, 255);

    hlineRGBA(renderer, bx + 5, bx + bw - 5, by + 22, 0, 255, 0, 200);

    if (entry_count == 0) {
        stringRGBA(renderer, bx + 10, by + 50,
                   "No blocks executed", 100, 100, 100, 255);
        return;
    }

    int total = entry_count;
    int show_start = 0;
    if (total > SYSLOG_MAX_VISIBLE) {
        show_start = total - SYSLOG_MAX_VISIBLE;
    }

    int ypos = by + 30;
    for (int i = show_start; i < total; i++) {
        int ri = get_real_index(i);

        Uint32 age = SDL_GetTicks() - entries[ri].timestamp;
        Uint8 alpha = (age > 3000) ? 150 : 255;

        Uint8 green;
        if (i == total - 1) {
            green = 255;
        } else {
            green = 180;
        }

        char display[28];
        strncpy(display, entries[ri].message, 27);
        display[27] = '\0';

        if (strlen(entries[ri].message) > 27) {
            display[24] = '.';
            display[25] = '.';
            display[26] = '.';
            display[27] = '\0';
        }

        stringRGBA(renderer, bx + 10, ypos, display, 0, green, 0, alpha);
        ypos += 18;
    }
    char count_str[32];
    snprintf(count_str, sizeof(count_str), "Total: %d", entry_count);
    stringRGBA(renderer, bx + 10, by + bh - 18, count_str, 0, 200, 0, 255);
}
