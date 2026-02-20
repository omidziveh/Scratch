#ifndef OPERATORS_H
#define OPERATORS_H

#include <string>

float op_add(float a, float b);
float op_sub(float a, float b);
float op_mul(float a, float b);
float op_div(float a, float b, bool& success);

float op_abs(float a);
float op_floor(float a);
float op_ceil(float a);
float op_sqrt(float a, bool& success);
float op_sin(float degrees);
float op_cos(float degrees);
float op_mod(float a, float b, bool& success);

float op_and(float a, float b);
float op_or(float a, float b);
float op_not(float a);
float op_xor(float a, float b);

float op_gt(float a, float b);
float op_lt(float a, float b);
float op_eq(float a, float b);

float op_str_len(const std::string& s);
std::string op_str_char(const std::string& s, float index);
std::string op_str_concat(const std::string& a, const std::string& b);

float op_round(float a);
float op_tan(float degrees);
float op_asin(float a, bool& success);
float op_acos(float a, bool& success);
float op_atan(float a);
float op_ln(float a, bool& success);
float op_log(float a, bool& success);
float op_e_pow(float a);
float op_ten_pow(float a);
float op_random(float min_val, float max_val);

float evaluate_operator(const std::string& opType, float a, float b, bool& success);

#endif
