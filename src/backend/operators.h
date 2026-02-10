#ifndef OPERATORS_H
#define OPERATORS_H

#include <string>

float op_add(float a, float b);
float op_sub(float a, float b);
float op_div(float a, float b, bool& success);

float evaluate_operator(const std::string& opType, float a, float b, bool& success);

#endif
