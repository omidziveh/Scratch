#include "logic.h"
#include <iostream>

void connect_blocks(Block* top, Block* bottom) {
    // disconnection
    if (bottom == nullptr) {
        top->next = nullptr;
        return;
    }
    
    // prevent self-connection
    if (top == bottom) {
        log_error(
            "Self connection: " + blocktype_to_string(top->type) + 
            " with " + blocktype_to_string(bottom->type)
        );
        return;
    }
    
    // loop check
    if (would_create_cycle(top, bottom)) {
        log_error(
            "Connection would create circular reference (Top ID: " + 
            std::to_string(top->id) + ", Bottom ID: " + std::to_string(bottom->id) + ")" 
        );
        return;
    }
    
    top->next = bottom;
    
    #ifdef DEBUG_MODE
    log_success("Connected Block #" + std::to_string(top->id) + " -> Block #" + std::to_string(bottom->id));
    #endif
}

Block* disconnect_next(Block* block) {
    if (block == nullptr) {
        return nullptr;
    }
    
    Block* disconnected = block->next;
    block->next = nullptr;
    
    #ifdef DEBUG_MODE
    if (disconnected != nullptr) {
        log_success(
            "Disconnected Block #" + std::to_string(disconnected->id) + 
            " from Block #" + std::to_string(block->id)
        );
    }
    #endif
    
    return disconnected;
}

void insert_block_between(Block* top, Block* middle, Block* bottom) {
    // checks:
    if (top == nullptr || middle == nullptr) {
        log_error("Cannot insert - top or middle block is NULL");
        return;
    }
    
    if (middle == top || middle == bottom) {
        log_error("Cannot insert block into itself");
        return;
    }
    
    if (would_create_cycle(top, middle) || would_create_cycle(middle, bottom)) {
        log_error("Insertion would create circular reference");
        return;
    }
    
    top->next = middle;
    middle->next = bottom;
    
    #ifdef DEBUG_MODE
    log_success(
        "Inserted Block #" + std::to_string(middle->id) 
        + " between Block #" + std::to_string(top->id)
        + " and Block #" + std::to_string(bottom ? bottom->id : -1)
    );
    #endif
}

bool would_create_cycle(Block* top, Block* bottom) {
    if (top == nullptr || bottom == nullptr) {
        return false;
    }
    
    Block* current = bottom;
    int safety_counter = 0;
    const int MAX_DEPTH = 10000;
    
    while (current != nullptr && safety_counter < MAX_DEPTH) {
        if (current == top) {
            return true;
        }
        
        if (current->next == top || current->inner == top) {
            return true;
        }
        
        current = current->next;
        safety_counter++;
    }
    
    return false;
}

Block* get_last_block(Block* block) {
    if (block == nullptr) {
        return nullptr;
    }
    
    Block* current = block;
    while (current->next != nullptr) {
        current = current->next;
    }
    
    return current;
}

int count_chain_length(Block* block) {
    int count = 0;
    Block* current = block;
    
    while (current != nullptr) {
        count++;
        current = current->next;
    }
    
    return count;
}

bool is_in_chain(Block* head, Block* target) {
    if (head == nullptr || target == nullptr) {
        return false;
    }
    
    Block* current = head;
    while (current != nullptr) {
        if (current == target) {
            return true;
        }
        current = current->next;
    }
    
    return false;
}
