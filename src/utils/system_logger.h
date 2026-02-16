#ifndef SYSTEM_LOGGER_H
#define SYSTEM_LOGGER_H
#include "../common/definitions.h" 

#include <string>
#include <SDL2/SDL.h>

#define SYSLOG_MAX_ENTRIES   100
#define SYSLOG_MAX_VISIBLE   15
#define SYSLOG_BOX_X         650
#define SYSLOG_BOX_Y         50
#define SYSLOG_BOX_W         220
#define SYSLOG_BOX_H         320

struct SysLogEntry {
    int block_id;
    char message[64];
    Uint32 timestamp;
};

void syslog_init();
void syslog_log(int block_id, const std::string& name);
void syslog_clear();
void syslog_toggle();
bool syslog_is_visible();
void syslog_render(SDL_Renderer* renderer);
int  syslog_get_count();

#endif
