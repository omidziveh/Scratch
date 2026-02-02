#pragma once
#include <string>
using namespace std;
#define MOVE 0
#define TURN 1
#define GOTO 2
#define REPEAT 3
#define IF 4
#define WAIT 5

#define CATEGORY_MOTION 0
#define CATEGORY_CONTROL 1
#define CATEGORY_LOOKS 2

struct Sprite {
    int x;
    int y;
    int width;
    int height;
    string image_path;
};

struct Block {
    int type;          
    int category;      
    int x;             
    int y;              
    int width;         
    int height;         
    int param1;         
    int param2;         
    string text;      
};
