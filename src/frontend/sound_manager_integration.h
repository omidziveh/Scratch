#ifndef SOUND_MANAGER_INTEGRATION_H
#define SOUND_MANAGER_INTEGRATION_H

#include "sound_manager.h"
#include "../backend/sound.h"
#include <vector>
#include <string>

inline std::vector<std::string> get_sound_names_for_dropdown() {
    return sound_project_get_names();
}

inline bool check_sound_exists(const std::string& name) {
    return sound_project_get_by_name(name) != nullptr;
}

inline int get_project_sound_count() {
    return sound_project_count();
}

#endif
