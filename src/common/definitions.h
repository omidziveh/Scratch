#pragma once
#include <vector>
#include <string>
using namespace std;
#define MOVE 0
#define TURN 1
#define SAY 2
#define WAIT 3
#define IF 4
#define REPEAT 5
#define EVENT_CLICK 6

struct Block {
    int id, type;
    float x, y, w, h;
    vector<string> args;
    Block *next, *inner, *parent;
};

struct Sprite {
    float x, y, angle;
    bool pen = false;
    vector<string> costumes;
    int costume = 0;
};
