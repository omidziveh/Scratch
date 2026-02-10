#include "operators.h"
#include "logger.h"
#include <cmath>
#include <limits>

float op_add(float a, float b) {
    return a + b;
}

float op_sub(float a, float b) {
    return a - b;
}

float op_div(float a, float b, bool& success) {
    if (std::abs(b) < std::numeric_limits<float>::epsilon()) {
        log_error("Division by zero: " + std::to_string(a) + " / " + std::to_string(b));
        success = false;
        return 0.0f;
    }
    success = true;
    return a / b;
}

float evaluate_operator(const std::string& opType, float a, float b, bool& success) {
    success = true;
    if (opType == "add") return op_add(a, b);
    if (opType == "sub") return op_sub(a, b);
    if (opType == "div") return op_div(a, b, success);

    log_warning("Unknown operator: " + opType);
    success = false;
    return 0.0f;
}
