#include "operators.h"
#include "../utils/logger.h"
#include <cmath>
#include <limits>

float op_add(float a, float b) {
    return a + b;
}

float op_sub(float a, float b) {
    return a - b;
}

float op_mul(float a, float b) {
    return a * b;
}

float op_div(float a, float b, bool& success) {
    if (std::abs(b) < std::numeric_limits<float>::epsilon()) {
        log_error("Division by zero");
        success = false;
        return 0.0f;
    }
    success = true;
    return a / b;
}

float op_mod(float a, float b, bool& success) {
    if (std::abs(b) < std::numeric_limits<float>::epsilon()) {
        log_error("Modulo by zero");
        success = false;
        return 0.0f;
    }
    success = true;
    return std::fmod(a, b);
}

float op_abs(float a) {
    return std::fabs(a);
}

float op_floor(float a) {
    return std::floor(a);
}

float op_ceil(float a) {
    return std::ceil(a);
}

float op_sqrt(float a, bool& success) {
    if (a < 0) {
        log_error("Sqrt of negative number");
        success = false;
        return 0.0f;
    }
    success = true;
    return std::sqrt(a);
}

float op_sin(float degrees) {
    return std::sin(degrees * 3.14159265f / 180.0f);
}

float op_cos(float degrees) {
    return std::cos(degrees * 3.14159265f / 180.0f);
}

float op_and(float a, float b) {
    return (a != 0.0f && b != 0.0f) ? 1.0f : 0.0f;
}
float op_or(float a, float b)  {
    return (a != 0.0f || b != 0.0f) ? 1.0f : 0.0f;
}
float op_not(float a)          {
    return (a == 0.0f) ? 1.0f : 0.0f;
}
float op_xor(float a, float b) {
    return ( (a!=0) != (b!=0) ) ? 1.0f : 0.0f;
}


float op_gt(float a, float b) {
    return (a > b) ? 1.0f : 0.0f;
}
float op_lt(float a, float b) {
    return (a < b) ? 1.0f : 0.0f;
}
float op_eq(float a, float b) {
    return (std::fabs(a - b) < 0.00001f) ? 1.0f : 0.0f;
}

// --- String ---

float op_str_len(const std::string& s) {
    return (float)s.length();
}

std::string op_str_char(const std::string& s, float index) {
    int idx = (int)index - 1;
    if (idx >= 0 && idx < (int)s.length()) {
        return std::string(1, s[idx]);
    }
    return "";
}

std::string op_str_concat(const std::string& a, const std::string& b) {
    return a + b;
}

float evaluate_operator(const std::string& opType, float a, float b, bool& success) {
    success = true;
    if (opType == "add") return op_add(a, b);
    if (opType == "sub") return op_sub(a, b);
    if (opType == "mul") return op_mul(a, b);
    if (opType == "div") return op_div(a, b, success);

    log_warning("Unknown operator: " + opType);
    success = false;
    return 0.0f;
}
