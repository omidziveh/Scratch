#include "sensing.h"

bool is_sprite_touching_mouse(const Sprite& sprite, const Stage& stage, int mouseX, int mouseY) {
    float left = stage.x + sprite.x - sprite.width / 2.0f;
    float top = stage.y + sprite.y - sprite.height / 2.0f;

    return mouseX >= left && mouseX <= left + sprite.width &&
           mouseY >= top && mouseY <= top + sprite.height;
}

bool is_sprite_touching_left_edge(const Sprite& sprite, const Stage& stage) {
    return (sprite.x - sprite.width / 2.0f) <= 0;
}

bool is_sprite_touching_right_edge(const Sprite& sprite, const Stage& stage) {
    return (sprite.x + sprite.width / 2.0f) >= stage.width;
}

bool is_sprite_touching_top_edge(const Sprite& sprite, const Stage& stage) {
    return (sprite.y - sprite.height / 2.0f) <= 0;
}

bool is_sprite_touching_bottom_edge(const Sprite& sprite, const Stage& stage) {
    return (sprite.y + sprite.height / 2.0f) >= stage.height;
}

bool is_sprite_touching_edge(const Sprite& sprite, const Stage& stage) {
    return is_sprite_touching_left_edge(sprite, stage) ||
           is_sprite_touching_right_edge(sprite, stage) ||
           is_sprite_touching_top_edge(sprite, stage) ||
           is_sprite_touching_bottom_edge(sprite, stage);
}

void bounce_off_edge(Sprite& sprite, const Stage& stage) {
    float halfW = sprite.width / 2.0f;
    float halfH = sprite.height / 2.0f;

    if (sprite.x - halfW < 0) {
        sprite.x = halfW;
        sprite.angle = 360.0f - sprite.angle;
    }
    if (sprite.x + halfW > stage.width) {
        sprite.x = stage.width - halfW;
        sprite.angle = 360.0f - sprite.angle;
    }
    if (sprite.y - halfH < 0) {
        sprite.y = halfH;
        sprite.angle = 180.0f - sprite.angle;
    }
    if (sprite.y + halfH > stage.height) {
        sprite.y = stage.height - halfH;
        sprite.angle = 180.0f - sprite.angle;
    }

    while (sprite.angle < 0) sprite.angle += 360.0f;
    while (sprite.angle >= 360.0f) sprite.angle -= 360.0f;
}
