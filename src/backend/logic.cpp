#include "logic.h"
#include "../utils/logger.h"
#include "file_io.h"

void connect_blocks(Block* top, Block* bottom) {
    if (!top) return;

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
    bottom->parent = top;
}

void connect_inner(Block* container, Block* child) {
    if (!container) return;

    if (!child) {
        container->inner = nullptr;
        return;
    }
    
    container->inner = child;
    child->parent = container;
}

Block* disconnect_inner(Block* block) {
    if (!block || !block->inner) return nullptr;

    Block* disconnected = block->inner;
    
    block->inner = nullptr;
    disconnected->parent = nullptr;

    return disconnected;
}

Block* disconnect_from_parent(Block* block) {
    if (!block || !block->parent) return nullptr;

    Block* parent = block->parent;

    if (parent->next == block) {
        parent->next = nullptr;
    } else if (parent->inner == block) {
        parent->inner = nullptr;
    }
    
    block->parent = nullptr;
    return block;
}

Block* disconnect_next(Block* block) {
    if (!block || !block->next) return nullptr;

    Block* disconnected = block->next;
    block->next = nullptr;
    disconnected->parent = nullptr;

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

Block* get_first_block(Block* block) {
    if (!block) return nullptr;

    while (block->parent) {
        block = block->parent;
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
