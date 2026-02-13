#include "sensing.h"
#include "../utils/logger.h"
#include <cmath>

bool is_sprite_touching_mouse(const Sprite& sprite, const Stage& stage, int mouseX, int mouseY) {
    float halfW = (sprite.width * sprite.scale) / 2.0f;
    float halfH = (sprite.height * sprite.scale) / 2.0f;

    float left   = sprite.x - halfW;
    float right  = sprite.x + halfW;
    float top    = sprite.y - halfH;
    float bottom = sprite.y + halfH;

    return (mouseX >= left && mouseX <= right && mouseY >= top && mouseY <= bottom);
}

bool is_sprite_touching_edge(const Sprite& sprite, const Stage& stage) {
    return is_sprite_touching_left_edge(sprite, stage)
        || is_sprite_touching_right_edge(sprite, stage)
        || is_sprite_touching_top_edge(sprite, stage)
        || is_sprite_touching_bottom_edge(sprite, stage);
}

bool is_sprite_touching_left_edge(const Sprite& sprite, const Stage& stage) {
    float halfW = (sprite.width * sprite.scale) / 2.0f;
    return (sprite.x - halfW) <= stage.x;
}

bool is_sprite_touching_right_edge(const Sprite& sprite, const Stage& stage) {
    float halfW = (sprite.width * sprite.scale) / 2.0f;
    return (sprite.x + halfW) >= (stage.x + stage.width);
}

bool is_sprite_touching_top_edge(const Sprite& sprite, const Stage& stage) {
    float halfH = (sprite.height * sprite.scale) / 2.0f;
    return (sprite.y - halfH) <= stage.y;
}

bool is_sprite_touching_bottom_edge(const Sprite& sprite, const Stage& stage) {
    float halfH = (sprite.height * sprite.scale) / 2.0f;
    return (sprite.y + halfH) >= (stage.y + stage.height);
}

void bounce_off_edge(Sprite& sprite, const Stage& stage) {
    float halfW = (sprite.width * sprite.scale) / 2.0f;
    float halfH = (sprite.height * sprite.scale) / 2.0f;

    if (sprite.x - halfW < stage.x) {
        sprite.x = stage.x + halfW;
        sprite.direction = 180.0f - sprite.direction;
    }
    if (sprite.x + halfW > stage.x + stage.width) {
        sprite.x = stage.x + stage.width - halfW;
        sprite.direction = 180.0f - sprite.direction;
    }
    if (sprite.y - halfH < stage.y) {
        sprite.y = stage.y + halfH;
        sprite.direction = -sprite.direction;
    }
    if (sprite.y + halfH > stage.y + stage.height) {
        sprite.y = stage.y + stage.height - halfH;
        sprite.direction = -sprite.direction;
    }

    while (sprite.direction < 0) sprite.direction += 360.0f;
    while (sprite.direction >= 360.0f) sprite.direction -= 360.0f;

    sprite.angle = sprite.direction;
    while (sprite.angle < 0) sprite.angle += 360.0f;
    while (sprite.angle >= 360.0f) sprite.angle -= 360.0f;
}

void clamp_sprite_to_stage(Sprite& sprite, const Stage& stage) {
    float halfW = (sprite.width * sprite.scale) / 2.0f;
    float halfH = (sprite.height * sprite.scale) / 2.0f;

    float minX = stage.x + halfW;
    float maxX = stage.x + stage.width - halfW;
    float minY = stage.y + halfH;
    float maxY = stage.y + stage.height - halfH;

    if (sprite.x < minX) sprite.x = minX;
    if (sprite.x > maxX) sprite.x = maxX;
    if (sprite.y < minY) sprite.y = minY;
    if (sprite.y > maxY) sprite.y = maxY;
}
