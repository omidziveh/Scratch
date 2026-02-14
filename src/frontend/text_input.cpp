#include "text_input.h"
#include "../utils/logger.h"
#include "block_utils.h"
#include <algorithm>

int get_arg_count(BlockType type) {
    switch (type) {
        case CMD_MOVE:
        case CMD_TURN:
        case CMD_SET_X:
        case CMD_SET_Y:
        case CMD_CHANGE_X:
        case CMD_CHANGE_Y:
        case CMD_REPEAT:
        case CMD_WAIT:
        case CMD_SAY:
        case CMD_SWITCH_COSTUME:
        case CMD_SET_SIZE:
        case CMD_CHANGE_SIZE:
            return 1;
        case CMD_GOTO:
            return 2;
        case CMD_IF:
            return 1;
        case CMD_NEXT_COSTUME:
        case CMD_SHOW:
        case CMD_HIDE:
        case CMD_START:
        case CMD_EVENT_CLICK:
        case CMD_NONE:
        default:
            return 0;
    }
}

SDL_Rect get_arg_box_rect(const Block& block, int arg_index) {
    SDL_Rect rect;
    int total_args = get_arg_count(block.type);

    if (total_args == 0) {
        rect = {0, 0, 0, 0};
        return rect;
    }

    std::string label = block_get_label(block.type);

    int paren_count = 0;
    int char_pos = 0;
    for (size_t i = 0; i < label.size(); i++) {
        if (label[i] == '(' || label[i] == '[' || label[i] == '<') {
            if (paren_count == arg_index) {
                char_pos = (int)i;
                break;
            }
            paren_count++;
        }
    }

    int text_x_offset = char_pos * 8; 

    rect.x = (int)block.x + 8 + text_x_offset;
    rect.y = (int)block.y + ARG_BOX_Y_OFFSET;
    rect.w = ARG_BOX_WIDTH;
    rect.h = ARG_BOX_HEIGHT;

    return rect;
}

int try_click_arg(const Block& block, int mx, int my) {
    int count = get_arg_count(block.type);
    for (int i = 0; i < count; i++) {
        SDL_Rect r = get_arg_box_rect(block, i);
        if (r.w == 0) continue;
        if (mx >= r.x && mx <= r.x + r.w &&
            my >= r.y && my <= r.y + r.h) {
            return i;
        }
    }
    return -1;
}

static Block* find_block_by_id(std::vector<Block>& blocks, int id) {
    for (auto& b : blocks) {
        if (b.id == id) return &b;
    }
    return nullptr;
}

void begin_editing(TextInputState& state, Block& block, int arg_index) {
    state.active = true;
    state.block_id = block.id;
    state.arg_index = arg_index;

    if (arg_index < (int)block.args.size()) {
        state.buffer = block.args[arg_index];
    } else {
        state.buffer = "";
    }

    state.cursor_pos = (int)state.buffer.size();
    state.blink_timer = SDL_GetTicks();
    state.cursor_visible = true;

    SDL_StartTextInput();
    log_info("Begin editing block #" + std::to_string(block.id) +
             " arg[" + std::to_string(arg_index) + "] = \"" + state.buffer + "\"");
}

void commit_editing(TextInputState& state, std::vector<Block>& blocks) {
    if (!state.active) return;

    Block* block = find_block_by_id(blocks, state.block_id);
    if (block) {
        while ((int)block->args.size() <= state.arg_index) {
            block->args.push_back("");
        }
        block->args[state.arg_index] = state.buffer;

        log_info("Committed block #" + std::to_string(block->id) +
                 " arg[" + std::to_string(state.arg_index) + "] = \"" + state.buffer + "\"");
    }

    state.active = false;
    state.block_id = -1;
    state.buffer.clear();
    state.cursor_pos = 0;

    SDL_StopTextInput();
}

void cancel_editing(TextInputState& state) {
    if (!state.active) return;

    log_debug("Cancelled editing block #" + std::to_string(state.block_id));

    state.active = false;
    state.block_id = -1;
    state.buffer.clear();
    state.cursor_pos = 0;

    SDL_StopTextInput();
}

void on_text_input(TextInputState& state, const char* text) {
    if (!state.active) return;

    std::string input(text);
    state.buffer.insert(state.cursor_pos, input);
    state.cursor_pos += (int)input.size();

    state.blink_timer = SDL_GetTicks();
    state.cursor_visible = true;
}

void on_key_input(TextInputState& state, SDL_Keycode key, std::vector<Block>& blocks) {
    if (!state.active) return;

    switch (key) {
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            commit_editing(state, blocks);
            break;

        case SDLK_ESCAPE:
            cancel_editing(state);
            break;

        case SDLK_BACKSPACE:
            if (state.cursor_pos > 0) {
                state.buffer.erase(state.cursor_pos - 1, 1);
                state.cursor_pos--;
            }
            break;

        case SDLK_DELETE:
            if (state.cursor_pos < (int)state.buffer.size()) {
                state.buffer.erase(state.cursor_pos, 1);
            }
            break;

        case SDLK_LEFT:
            if (state.cursor_pos > 0) {
                state.cursor_pos--;
            }
            break;

        case SDLK_RIGHT:
            if (state.cursor_pos < (int)state.buffer.size()) {
                state.cursor_pos++;
            }
            break;

        case SDLK_HOME:
            state.cursor_pos = 0;
            break;

        case SDLK_END:
            state.cursor_pos = (int)state.buffer.size();
            break;

        default:
            break;
    }

    state.blink_timer = SDL_GetTicks();
    state.cursor_visible = true;
}

void tick_cursor(TextInputState& state) {
    if (!state.active) return;

    Uint32 now = SDL_GetTicks();
    if (now - state.blink_timer >= CURSOR_BLINK_MS) {
        state.cursor_visible = !state.cursor_visible;
        state.blink_timer = now;
    }
}
