#include <SDL2/SDL.h>
#ifdef __linux__
#include <SDL2/SDL_image.h>
#else
#include <SDL_image.h>
#endif
#include <iostream>
#include <vector>
#include <list>
#include <cmath>
#include <fstream>
#include <cstdlib>
#include <cstring>

#include "common/definitions.h"
#include "common/globals.h"
#include "frontend/draw.h"
#include "frontend/input.h"
#include "frontend/palette.h"
#include "frontend/block_utils.h"
#include "frontend/menu.h"
#include "frontend/hover.h"
#include "utils/logger.h"
#include "frontend/text_input.h"
#include "utils/system_logger.h"
#include "backend/block_executor_looks.h"
#include "backend/sound.h"
#include "backend/runtime.h"
#include "frontend/pen.h"
#include <set>
#include "backend/logic.h"
#include "backend/file_io.h"
#include "frontend/background_menu.h"
#include "frontend/costume_editor.h"
#include "frontend/character_panel.h"
#include "frontend/confirm_dialog.h"
#include "frontend/block_highlight.h"
#include <map>
#include "frontend/sound_manager.h"
#include "frontend/sound_manager_integration.h"
#include "backend/custom_blocks.h"

Sprite sprite;
Runtime gRuntime;
Stage stage;
TTF_Font* g_font = nullptr;
ConfirmDialog g_dialog;
MenuAction g_pending_action = MENU_ACTION_NONE;
CostumeEditor g_costume_editor;

static void register_all_definitions(std::list<Block>& blocks) {
    for (Block& b : blocks) {
        if (b.type == CMD_DEFINE_BLOCK) {
            if (!b.args.empty()) {
                custom_blocks_register(b.args[0], &b);
            }
        }
    }
}

void init_program(SDL_Renderer& renderer) {
    syslog_init();
    menu_init();
    pen_init(&renderer);
    init_logger("debug.log");
    log_info("Application started");
    sprite.texture = load_texture(&renderer, "../assets/cat.png");
    if (!sprite.texture) {
        log_warning("Failed to load cat.png - sprite will be invisible");
    }
    cdialog_init(&g_dialog, WINDOW_WIDTH, WINDOW_HEIGHT);
    ceditor_init(&g_costume_editor, &renderer, 50, 50);
    sound_manager_init(&renderer, WINDOW_WIDTH/2 - 100, WINDOW_HEIGHT/2 - 200, 200, 400);
    // sound_manager_set_visible(true);
}

static void save_project(const char* filename,
                         const std::list<Block>& blocks,
                         const Sprite& sprite)
{
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        log_warning("SAVE: Failed to open file for saving");
        return;
    }

    int count = (int)blocks.size();
    file.write(reinterpret_cast<const char*>(&count), sizeof(count));

    for (const auto& b : blocks) {
        file.write(reinterpret_cast<const char*>(&b.id),   sizeof(b.id));
        file.write(reinterpret_cast<const char*>(&b.type), sizeof(b.type));
        file.write(reinterpret_cast<const char*>(&b.x),    sizeof(b.x));
        file.write(reinterpret_cast<const char*>(&b.y),    sizeof(b.y));

        int argCount = (int)b.args.size();
        file.write(reinterpret_cast<const char*>(&argCount), sizeof(argCount));
        for (const auto& arg : b.args) {
            int len = (int)arg.size();
            file.write(reinterpret_cast<const char*>(&len), sizeof(len));
            file.write(arg.c_str(), len);
        }
    }

    int varCount = (int)sprite.variables.size();
    file.write(reinterpret_cast<const char*>(&varCount), sizeof(varCount));
    for (const auto& var : sprite.variables) {
        int len = (int)var.name.size();
        file.write(reinterpret_cast<const char*>(&len), sizeof(len));
        file.write(var.name.c_str(), len);
        
        len = (int)var.value.size();
        file.write(reinterpret_cast<const char*>(&len), sizeof(len));
        file.write(var.value.c_str(), len);
    }

    file.write(reinterpret_cast<const char*>(&sprite.x),     sizeof(sprite.x));
    file.write(reinterpret_cast<const char*>(&sprite.y),     sizeof(sprite.y));
    file.write(reinterpret_cast<const char*>(&sprite.angle), sizeof(sprite.angle));
    file.write(reinterpret_cast<const char*>(&sprite.visible), sizeof(sprite.visible));
    file.write(reinterpret_cast<const char*>(&sprite.currentCostumeIndex),
               sizeof(sprite.currentCostumeIndex));

    file.close();
    log_info("SAVE: Project saved with variables");
}

static bool load_project(const char* filename,
                          std::list<Block>& blocks,
                          Sprite& sprite,
                          int& next_block_id)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        log_warning("LOAD: Failed to open file for loading");
        return false;
    }

    blocks.clear();
    next_block_id = 1;

    int count = 0;
    file.read(reinterpret_cast<char*>(&count), sizeof(count));
    std::map<int, Block*> idToPointer;
    std::vector<std::pair<Block*, int>> pendingConnectionsNext;
    std::vector<std::pair<Block*, int>> pendingConnectionsInner;
    std::vector<std::pair<Block*, int>> pendingConnectionsParent;

    for (int i = 0; i < count; i++) {
        Block b;
        file.read(reinterpret_cast<char*>(&b.id),   sizeof(b.id));
        file.read(reinterpret_cast<char*>(&b.type), sizeof(b.type));
        file.read(reinterpret_cast<char*>(&b.x),    sizeof(b.x));
        file.read(reinterpret_cast<char*>(&b.y),    sizeof(b.y));

        b.is_running = false;
        b.glow_start_time = 0;
        b.next = nullptr;
        b.inner = nullptr;
        b.parent = nullptr;

        int argCount = 0;
        file.read(reinterpret_cast<char*>(&argCount), sizeof(argCount));
        for (int j = 0; j < argCount; j++) {
            int len = 0;
            file.read(reinterpret_cast<char*>(&len), sizeof(len));
            std::string arg(len, '\0');
            file.read(&arg[0], len);
            b.args.push_back(arg);
        }

        blocks.push_back(b);
        Block& added = blocks.back();
        idToPointer[added.id] = &added;

        if (added.id >= next_block_id) {
            next_block_id = added.id + 1;
        }
    }

    file.read(reinterpret_cast<char*>(&sprite.x),     sizeof(sprite.x));
    file.read(reinterpret_cast<char*>(&sprite.y),     sizeof(sprite.y));
    file.read(reinterpret_cast<char*>(&sprite.angle), sizeof(sprite.angle));
    file.read(reinterpret_cast<char*>(&sprite.visible), sizeof(sprite.visible));
    file.read(reinterpret_cast<char*>(&sprite.currentCostumeIndex),
              sizeof(sprite.currentCostumeIndex));

    if (sprite.currentCostumeIndex >= 0 &&
        sprite.currentCostumeIndex < (int)sprite.costumes.size()) {
        sprite.texture = sprite.costumes[sprite.currentCostumeIndex].texture;
        sprite.width   = sprite.costumes[sprite.currentCostumeIndex].width;
        sprite.height  = sprite.costumes[sprite.currentCostumeIndex].height;
    }

    int varCount = 0;
    file.read(reinterpret_cast<char*>(&varCount), sizeof(varCount));
    sprite.variables.clear();
    for (int i = 0; i < varCount; i++) {
        Variable var;
        int len = 0;
        
        file.read(reinterpret_cast<char*>(&len), sizeof(len));
        var.name.resize(len);
        file.read(&var.name[0], len);
        
        file.read(reinterpret_cast<char*>(&len), sizeof(len));
        var.value.resize(len);
        file.read(&var.value[0], len);
        
        sprite.variables.push_back(var);
    }

    file.close();
    log_info("LOAD: Project loaded with variables");
    return true;
}

static void new_project(std::list<Block>& blocks,
                         Sprite& sprite,
                         int& next_block_id,
                         int& execution_index,
                         bool& is_executing,
                         SDL_Renderer* renderer)
{
    blocks.clear();
    next_block_id = 1;
    execution_index = -1;
    is_executing = false;

    sprite.x = STAGE_X + STAGE_WIDTH / 2.0f;
    sprite.y = STAGE_Y + STAGE_HEIGHT / 2.0f;
    sprite.angle = 0.0f;
    sprite.visible = true;
    sprite.isPenDown = 0;
    sprite.penR = 0;
    sprite.penG = 0;
    sprite.penB = 255;
    sprite.penSize = 1;
    sprite.prevPenX = sprite.x;
    sprite.prevPenY = sprite.y;

    if (!sprite.costumes.empty()) {
        sprite.currentCostumeIndex = 0;
        sprite.texture = sprite.costumes[0].texture;
        sprite.width   = sprite.costumes[0].width;
        sprite.height  = sprite.costumes[0].height;
    }

    pen_clear(renderer);

    log_info("NEW: New project created");
}

void draw_variables(SDL_Renderer* renderer, const Sprite& sprite) {
    if (sprite.variables.empty()) return;

    int y_offset = 10;
    for (const auto& var : sprite.variables) {
        std::string display = var.name + ": " + var.value;
        
        int textW = display.length() * 8 + 10;
        SDL_Rect bg = {STAGE_X + 5, STAGE_Y + y_offset, textW, 18};
        SDL_SetRenderDrawColor(renderer, 255, 140, 26, 200);
        SDL_RenderFillRect(renderer, &bg);
        
        SDL_SetRenderDrawColor(renderer, 200, 100, 10, 255);
        SDL_RenderDrawRect(renderer, &bg);

        draw_text(renderer, bg.x + 5, bg.y + 4, display.c_str(), COLOR_BLACK);
        
        y_offset += 22;
    }
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    srand((unsigned int)time(nullptr));

    int  g_execution_index   = -1;
    bool g_is_executing      = false;
    bool g_step_mode         = false;
    bool g_waiting_for_step  = false;

    

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    if (TTF_Init() == -1) {
        std::cerr << "TTF_Init Error: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

    syslog_init();
    menu_init();

    if (IMG_Init(IMG_INIT_PNG) == 0) {
        std::cerr << "IMG_Init Error: " << IMG_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    if (!sound_init()) {
        log_warning("Sound engine failed to initialize");
    }

    SDL_Window* window = SDL_CreateWindow(
        "Blocky",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        std::cerr << "Window Error: " << SDL_GetError() << std::endl;
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    {
    SDL_Surface* icon_surface = IMG_Load("../assets/logo.png");
    if (icon_surface) {
        SDL_SetWindowIcon(window, icon_surface);
        SDL_FreeSurface(icon_surface);
        std::cout << "Window icon loaded successfully." << std::endl;
    } else {
        std::cerr << "Warning: Could not load icon: " << IMG_GetError() << std::endl;
    }
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "Renderer Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    g_font = TTF_OpenFont("../assets/font.ttf", 14);
    if (!g_font) {
         std::cerr << "Error: No font loaded. Text will not render." << std::endl;
    }

    init_program(*renderer);

    {
        const int COSTUME_COUNT = 10;
        char* costume_files[COSTUME_COUNT];
        for (int i = 0; i < COSTUME_COUNT; i++) {
            std::string path = "../assets/cat" + std::to_string(i + 1) + ".png";
            costume_files[i] = strdup(path.c_str());
        }

        const char* costume_names[] = {
            "costume1", "costume2", "costume3", "costume4", "costume5", 
            "costume6", "costume7", "costume8", "costume9", "costume10"
        };
        int num_costumes = COSTUME_COUNT;

        for (int i = 0; i < num_costumes; i++) {
            SDL_Texture* tex = load_texture(renderer, costume_files[i]);
            SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
            if (tex) {
                int w = 0, h = 0;
                SDL_QueryTexture(tex, nullptr, nullptr, &w, &h);
                sprite.costumes.push_back(Costume(costume_names[i], tex, w, h));
                log_info("Loaded costume: " + std::string(costume_names[i]));
            } else {
                log_warning("Failed to load costume: " + std::string(costume_files[i]));
            }
        }

        for (int i = 0; i < COSTUME_COUNT; i++) {
            free(costume_files[i]);
        }

        if (!sprite.costumes.empty()) {
            sprite.currentCostumeIndex = 0;
            sprite.texture = sprite.costumes[0].texture;
            sprite.width   = sprite.costumes[0].width;
            sprite.height  = sprite.costumes[0].height;
        }
    }

    std::vector<PaletteItem> palette_items;
    init_palette(palette_items);
    init_categories();

    int palette_scroll_offset = 0;
    int palette_max_scroll = get_palette_total_height(palette_items) - PALETTE_HEIGHT + 20;
    if (palette_max_scroll < 0) palette_max_scroll = 0;

    int target_scroll_override = -1;


    Block* program_head = nullptr;
    std::list<Block> blocks;
    int next_block_id = 1;

    stage.renderer = renderer;

    sprite.x = STAGE_X + STAGE_WIDTH / 2;
    sprite.y = STAGE_Y + STAGE_HEIGHT / 2;

    TextInputState text_state;

    int  mouse_x   = 0, mouse_y   = 0;
    bool hover_run  = false;
    bool hover_stop = false;
    bool hover_sound = false;

    bool running = true;
    SDL_Event event;

    std::vector<Runtime> activeRuntimes;

    for (Block& b : blocks) {
        if (b.type == CMD_DEFINE_BLOCK) {
            if (!b.args.empty()) {
                custom_blocks_register(b.args[0], &b);
            }
        }
    }

    register_all_definitions(blocks);

    while (running) {

        while (SDL_PollEvent(&event)) {
            if (sound_manager_handle_event(&event)) {
                continue;
            }
            
            switch (event.type) {

                case SDL_QUIT:
                    running = false;
                    break;

                case SDL_MOUSEWHEEL: {
                    int mx, my;
                    SDL_GetMouseState(&mx, &my);

                    if (mx >= PALETTE_X && mx < PALETTE_X + PALETTE_WIDTH &&
                        my >= PALETTE_Y && my < PALETTE_Y + PALETTE_HEIGHT) {

                        palette_scroll_offset -= event.wheel.y * 30;
                        if (palette_scroll_offset < 0)
                            palette_scroll_offset = 0;
                        if (palette_scroll_offset > palette_max_scroll)
                            palette_scroll_offset = palette_max_scroll;
                    }
                    break;
                }

                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        int mx = event.button.x;
                        int my = event.button.y;

                        if (g_costume_editor.is_open) {
                            ceditor_handle_mouse_down(&g_costume_editor, mx, my, renderer);
                            break;
                        }

                        if (g_dialog.is_open) {
                            CDialogResult res = cdialog_handle_click(&g_dialog, mx, my);
                            if (res == CDLG_YES) {
                                if (g_pending_action == MENU_ACTION_NEW) {
                                    activeRuntimes.clear();
                                    new_project(blocks, sprite, next_block_id, g_execution_index, g_is_executing, renderer);
                                } else if (g_pending_action == MENU_ACTION_EXIT) {
                                    running = false;
                                } else if (g_pending_action == MENU_ACTION_LOAD) {
                                    activeRuntimes.clear();
                                    blocks.clear();
                                    {
                                        load_project("project.scratch", blocks, sprite, next_block_id);
                                        g_execution_index = -1;
                                        g_is_executing = false;
                                        log_info("LOAD: Project loaded successfully");
                                    }
                                }
                            }
                            g_pending_action = MENU_ACTION_NONE;
                            break;
                        }

                        if (my < MENU_BAR_OFFSET) {
                            menu_handle_mouse_down(mx, my);
                            break;
                        }

                        if (menu_is_any_open()) {
                            menu_handle_mouse_down(mx, my);
                            break;
                        }

                        if (mx >= TOOLBAR_WIDTH - 155 && mx <= TOOLBAR_WIDTH - 125 &&
                            my >= TOOLBAR_Y + 5 && my <= TOOLBAR_Y + TOOLBAR_HEIGHT - 5) {
                            sound_manager_set_visible(!sound_manager_is_visible());
                            break;
                        }

                        if (mx >= TOOLBAR_WIDTH - 110 && mx <= TOOLBAR_WIDTH - 80 &&
                            my >= TOOLBAR_Y + 5  && my <= TOOLBAR_Y + TOOLBAR_HEIGHT - 5) {

                            bool is_any_running = false;
                            for (Runtime& rt : activeRuntimes) {
                                if (rt.state == RUNTIME_RUNNING) {
                                    is_any_running = true;
                                    break;
                                }
                            }

                            if (is_any_running) {
                                for (Runtime& rt : activeRuntimes) {
                                    runtime_pause(&rt);
                                }
                                log_info("RUNTIME: Paused");
                            } else {
                                bool was_paused = false;
                                for (Runtime& rt : activeRuntimes) {
                                    if (rt.state == RUNTIME_PAUSED) {
                                        runtime_resume(&rt);
                                        was_paused = true;
                                    }
                                }

                                if (!was_paused) {
                            activeRuntimes.clear();
                            custom_blocks_clear();
                            register_all_definitions(blocks);
                            for (Block& b : blocks) {
                                b.has_executed = false;
                                if (b.type == CMD_START && b.next) {
                                    Runtime rt;
                                    runtime_init(&rt, b.next, &sprite);
                                    runtime_start(&rt);
                                    activeRuntimes.push_back(rt);
                                }
                            }
                            log_info("RUN: Started " +
                                     std::to_string(activeRuntimes.size()) + " runtime(s)");
                                } else {
                                    log_info("RUNTIME: Resumed");
                                }
                            }
                            break;
                        }

                        if (mx >= TOOLBAR_WIDTH - 65 && mx <= TOOLBAR_WIDTH - 35 &&
                            my >= TOOLBAR_Y + 5  && my <= TOOLBAR_Y + TOOLBAR_HEIGHT - 5) {

                            for (Runtime& rt : activeRuntimes) {
                                runtime_stop(&rt);
                            }
                            activeRuntimes.clear();
                            log_info("STOP: All runtimes stopped");
                            break;
                        }

                        bool clicked_sprite = false;
                        if (sprite.visible && sprite.texture) {
                            float halfW = (sprite.width * sprite.scale) / 2.0f;
                            float halfH = (sprite.height * sprite.scale) / 2.0f;
                            
                            if (mx >= sprite.x - halfW && mx <= sprite.x + halfW &&
                                my >= sprite.y - halfH && my <= sprite.y + halfH) {
                                clicked_sprite = true;
                            }
                        }

                        if (clicked_sprite) {
                            register_all_definitions(blocks);
                            for (Block& b : blocks) {
                                if (b.type == CMD_EVENT_CLICK) {
                                    if (b.next) {
                                        Runtime rt;
                                        runtime_init(&rt, b.next, &sprite);
                                        runtime_start(&rt);
                                        activeRuntimes.push_back(rt);
                                        log_info("EVENT: Started script from CMD_EVENT_CLICK");
                                    }
                                }
                            }
                        }

                        if (my >= CATEGORY_BAR_Y &&
                            my <  CATEGORY_BAR_Y + CATEGORY_BAR_HEIGHT &&
                            mx >= PALETTE_X && mx < STAGE_X) {

                            const auto& cats = get_categories();
                            int totalWidth  = STAGE_X;
                            int buttonWidth = totalWidth / (int)cats.size();
                            int clickedIndex = (mx - PALETTE_X) / buttonWidth;

                            if (clickedIndex >= 0 && clickedIndex < (int)cats.size()) {
                                target_scroll_override =
                                    get_category_scroll_target(cats[clickedIndex].category);
                                palette_scroll_offset = target_scroll_override;
                                if (palette_scroll_offset < 0)
                                    palette_scroll_offset = 0;
                                if (palette_scroll_offset > palette_max_scroll)
                                    palette_scroll_offset = palette_max_scroll;
                            }
                            break;
                        }

                        bool clicked_arg = false;
                        for (auto& block : blocks) {
                            if (try_click_arg(block, mx, my, text_state)) {
                                clicked_arg = true;
                                break;
                            }
                        }

                        if (!clicked_arg) {
                            if (text_state.active) {
                                commit_editing(text_state, blocks);
                            }
                            handle_mouse_down(event, blocks, palette_items,
                                              next_block_id, palette_scroll_offset, text_state);
                        }
                    }
                    
                    break;

                case SDL_MOUSEBUTTONUP:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        if (g_costume_editor.is_open) {
                            ceditor_handle_mouse_up(&g_costume_editor, event.button.x, event.button.y, renderer);
                        } else {
                            menu_handle_mouse_up(event.button.x, event.button.y);
                            handle_mouse_up(event, blocks);
                        }
                    }
                    break;

                case SDL_MOUSEMOTION: {
                    int mx = event.motion.x;
                    int my = event.motion.y;
                    mouse_x = mx;
                    mouse_y = my;

                    if (g_costume_editor.is_open) {
                        ceditor_handle_mouse_move(&g_costume_editor, mx, my, renderer);
                    } else {
                        menu_handle_mouse_move(mx, my);

                        hover_sound = (mx >= TOOLBAR_WIDTH - 155 && mx <= TOOLBAR_WIDTH - 125 &&
                        my >= TOOLBAR_Y + 5 && my <= TOOLBAR_Y + TOOLBAR_HEIGHT - 5);

                        hover_run = (mx >= TOOLBAR_WIDTH - 110 && mx <= TOOLBAR_WIDTH - 80 &&
                        my >= TOOLBAR_Y + 5 && my <= TOOLBAR_Y + TOOLBAR_HEIGHT - 5);

                        hover_stop = (mx >= TOOLBAR_WIDTH - 65 && mx <= TOOLBAR_WIDTH - 35 &&
                        my >= TOOLBAR_Y + 5 && my <= TOOLBAR_Y + TOOLBAR_HEIGHT - 5);

                        handle_mouse_motion(event, blocks);
                    }
                    break;
                }

                case SDL_TEXTINPUT:
                    if (!g_costume_editor.is_open) {
                        if (text_state.active) {
                            on_text_input(text_state, event.text.text);
                        }
                    }
                    break;

                case SDL_KEYDOWN:
                    if (g_costume_editor.is_open) {
                        ceditor_handle_key(&g_costume_editor, event.key.keysym.sym, renderer);
                    } else {
                        if (text_state.active) {
                            on_key_input(text_state, event.key.keysym.sym, blocks);
                        } else {
                            SDL_Keycode keycode = event.key.keysym.sym;
                            const char* keyName = SDL_GetKeyName(keycode);
                            std::string keyStr = (keyName) ? keyName : "";
                            
                            for (auto & c: keyStr) c = tolower(c);
                            
                            register_all_definitions(blocks);
                            
                            for (Block& b : blocks) {
                                if (b.type == CMD_EVENT_KEY) {
                                    bool match = false;
                                    
                                    if (!b.args.empty()) {
                                        std::string target = b.args[0];
                                        for (auto & c: target) c = tolower(c); 
                                        
                                        if (target == "any") {
                                            match = true;
                                        }
                                        else if (target == "space" && keycode == SDLK_SPACE) {
                                            match = true;
                                        }
                                        else if (target == "up arrow" && keycode == SDLK_UP) match = true;
                                        else if (target == "down arrow" && keycode == SDLK_DOWN) match = true;
                                        else if (target == "left arrow" && keycode == SDLK_LEFT) match = true;
                                        else if (target == "right arrow" && keycode == SDLK_RIGHT) match = true;
                                        else if (keyStr == target) {
                                            match = true;
                                        }
                                    }
                                    
                                    if (match) {
                                        if (b.next) {
                                            Runtime rt;
                                            runtime_init(&rt, b.next, &sprite);
                                            runtime_start(&rt);
                                            activeRuntimes.push_back(rt);
                                            log_info("EVENT: Started script from CMD_EVENT_KEY (" + b.args[0] + ")");
                                        }
                                    }
                                }
                            }
                                if (event.key.keysym.sym == SDLK_e) {
                                    if (sprite.currentCostumeIndex >= 0 && sprite.currentCostumeIndex < (int)sprite.costumes.size()) {
                                        ceditor_open(&g_costume_editor, sprite.currentCostumeIndex, sprite.costumes[sprite.currentCostumeIndex].texture, renderer);
                                    } else if (sprite.texture) {
                                        ceditor_open(&g_costume_editor, 0, sprite.texture, renderer);
                                    }
                                }
                            if (event.key.keysym.sym == SDLK_l) {
                                syslog_toggle();
                            }
                            if (event.key.keysym.sym == SDLK_F12) {
                                for (Runtime& rt : activeRuntimes) {
                                    rt.stepMode = !rt.stepMode;
                                    rt.waitingForStep = rt.stepMode;
                                }
                                g_step_mode = !g_step_mode;
                                log_info("Step mode: " +
                                        std::string(g_step_mode ? "ON" : "OFF"));
                            }
                            if (event.key.keysym.sym == SDLK_SPACE) {
                                if (g_step_mode && g_waiting_for_step) {
                                    g_waiting_for_step = false;
                                }
                                for (Runtime& rt : activeRuntimes) {
                                    if (rt.stepMode && rt.waitingForStep) {
                                        rt.waitingForStep = false;
                                    }
                                }
                            }
                            if (event.key.keysym.sym == SDLK_DELETE) {
                            }
                        }
                    }
                    break;

                default:
                    break;
            }
        }
        
        if (!g_costume_editor.is_open && g_costume_editor.target_costume_index != -1) {
            SDL_Texture* result = ceditor_get_result(&g_costume_editor);
            if (result) {
                if (g_costume_editor.target_costume_index >= 0 && g_costume_editor.target_costume_index < (int)sprite.costumes.size()) {
                    sprite.costumes[g_costume_editor.target_costume_index].texture = result;
                    sprite.texture = result;
                }
            }
            g_costume_editor.target_costume_index = -1;
        }

        MenuAction action = menu_consume_action();
        switch (action) {
            case MENU_ACTION_NEW:
                g_pending_action = MENU_ACTION_NEW;
                cdialog_show(&g_dialog, "New Project", "Create new project?");
                break;

            case MENU_ACTION_SAVE:
                if (!blocks.empty()) {
                    Block* headToSave = nullptr;
                    for(auto& b : blocks) {
                        if(!b.parent) {
                            headToSave = &b;
                            break;
                        }
                    }
                    if(headToSave) {
                        save_project("project.scratch", blocks, sprite);
                    }
                } else {
                    log_warning("SAVE: No blocks to save");
                }
                break;

            case MENU_ACTION_LOAD:
                g_pending_action = MENU_ACTION_LOAD;
                cdialog_show(&g_dialog, "Load Project", "Load project? Unsaved changes will be lost.");
                break;

            case MENU_ACTION_EXIT:
                g_pending_action = MENU_ACTION_EXIT;
                cdialog_show(&g_dialog, "Exit", "Are you sure you want to exit?");
                break;

            case MENU_ACTION_SYSTEM_LOGGER:
                syslog_toggle();
                break;

            case MENU_ACTION_DEBUG_INFO:
                log_info("DEBUG: Blocks=" + std::to_string(blocks.size()) +
                         " Sprite=(" + std::to_string((int)sprite.x) + "," +
                         std::to_string((int)sprite.y) + ")" +
                         " Angle=" + std::to_string((int)sprite.angle) +
                         " Variables=" + std::to_string(sprite.variables.size()));
                break;

            case MENU_ACTION_ABOUT:
                log_info("ABOUT: Blocky v1.0 - Scratch Clone built with SDL2");
                break;

            case MENU_ACTION_NONE:
            default:
                break;
        }

        tick_cursor(text_state);
        logger_tick();

        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        for (auto it = activeRuntimes.begin(); it != activeRuntimes.end(); ) {
            runtime_tick(&(*it), &stage, mouseX, mouseY);
            if (it->state == RUNTIME_FINISHED || it->state == RUNTIME_STOPPED) {
                it = activeRuntimes.erase(it);
            } else {
                ++it;
            }
        }

        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);

        bool is_program_running = false;
        for (const Runtime& rt : activeRuntimes) {
            if (rt.state == RUNTIME_RUNNING) {
                is_program_running = true;
                break;
            }
        }

        draw_toolbar(renderer, is_program_running);

        const auto& cats = get_categories();
        int selected_cat_index = 0;
        for (size_t i = 0; i < cats.size(); i++) {
            if (palette_scroll_offset >= cats[i].yPosition) {
                selected_cat_index = (int)i;
            }
        }

        draw_category_bar(renderer, cats, selected_cat_index);
        draw_palette(renderer, palette_items, palette_scroll_offset);
        draw_coding_area(renderer);
        draw_stage(renderer, sprite);
        pen_render(renderer);
        draw_variables(renderer, sprite);

        draw_all_blocks(renderer, blocks, text_state);

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

        if (hover_sound) {
            filledCircleRGBA(renderer, TOOLBAR_WIDTH - 140, 50, 15, 255, 255, 255, 50);
        }

        if (hover_run) {
            filledCircleRGBA(renderer, TOOLBAR_WIDTH - 95, 50, 15, 255, 255, 255, 50);
        }

        if (hover_stop) {
            filledCircleRGBA(renderer, TOOLBAR_WIDTH - 50, 50, 15, 255, 255, 255, 50);
        }

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

        if (syslog_is_visible()) {
            syslog_render(renderer);
        }

        menu_render(renderer);

        render_palette_hover(renderer, palette_items, mouse_x, mouse_y,
                             palette_scroll_offset);

        cdialog_render(&g_dialog, renderer);
        
        if (g_costume_editor.is_open) {
            ceditor_render(&g_costume_editor, renderer);
        }

        sound_manager_render();

        SDL_RenderPresent(renderer);
    }

    log_info("Application shutting down...");

    for (Runtime& rt : activeRuntimes) {
        runtime_stop(&rt);
    }
    activeRuntimes.clear();

    sound_manager_cleanup();
    ceditor_destroy(&g_costume_editor);
    pen_shutdown();

    for (auto& c : sprite.costumes) {
        if (c.texture) {
            SDL_DestroyTexture(c.texture);
            c.texture = nullptr;
        }
    }
    sprite.costumes.clear();

    sprite.texture = nullptr;

    close_logger();
    sound_cleanup();
    
    if (g_font) TTF_CloseFont(g_font);
    TTF_Quit();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
