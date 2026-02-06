#include "logic.h"
#include "logger.h"
#include "file_io.h"

void connect_blocks(Block* top, Block* bottom) {
    if (!bottom) {
        top->next = nullptr;
        return;
    }

    if (top == bottom) {
        log_error("Cannot connect block to itself");
        return;
    }

    if (would_create_cycle(top, bottom)) {
        log_error("Connection would create cycle");
        return;
    }

    top->next = bottom;
}

Block* disconnect_next(Block* block) {
    if (!block) return nullptr;

    Block* disconnected = block->next;
    block->next = nullptr;
    return disconnected;
}

void insert_block_between(Block* top, Block* middle, Block* bottom) {
    if (!top || !middle) {
        log_error("Cannot insert - null block");
        return;
    }

    if (middle == top || middle == bottom) {
        log_error("Cannot insert block into itself");
        return;
    }

    if (would_create_cycle(top, middle) || would_create_cycle(middle, bottom)) {
        log_error("Insertion would create cycle");
        return;
    }

    top->next = middle;
    middle->next = bottom;
}

bool would_create_cycle(Block* top, Block* bottom) {
    if (!top || !bottom) return false;

    Block* current = bottom;
    int counter = 0;

    while (current && counter < 10000) {
        if (current == top) return true;
        if (current->next == top || current->inner == top) return true;
        current = current->next;
        counter++;
    }

    return false;
}

Block* get_last_block(Block* block) {
    if (!block) return nullptr;

    while (block->next) {
        block = block->next;
    }
    return block;
}

int count_chain_length(Block* block) {
    int count = 0;
    while (block) {
        count++;
        block = block->next;
    }
    return count;
}
