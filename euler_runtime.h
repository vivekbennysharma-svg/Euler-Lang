#ifndef EULER_RUNTIME_H
#define EULER_RUNTIME_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include<math.h>

typedef enum {
    EXPR_NUM,
    EXPR_VAR,
    EXPR_ADD,
    EXPR_SUB,
    EXPR_MUL,
    EXPR_DIV,
    EXPR_POW,
    EXPR_SIN,
    EXPR_COS
} ExprKind;

typedef struct EulerExpr {
    ExprKind kind;
    double val;
    char name[32];
    struct EulerExpr* left;
    struct EulerExpr* right;
} EulerExpr;

EulerExpr* euler_num(double val);
EulerExpr* euler_var(const char* name);
EulerExpr* euler_add(EulerExpr* left, EulerExpr* right);
EulerExpr* euler_sub(EulerExpr* left, EulerExpr* right);
EulerExpr* euler_mul(EulerExpr* left, EulerExpr* right);
EulerExpr* euler_div(EulerExpr* left, EulerExpr* right);
EulerExpr* euler_pow(EulerExpr* left, EulerExpr* right);
EulerExpr* euler_sin(EulerExpr* expr);
EulerExpr* euler_cos(EulerExpr* expr);

//Calculus utilities
EulerExpr* euler_diff(EulerExpr* expr, EulerExpr* var);
EulerExpr* euler_simplify(EulerExpr* expr);

void euler_print_expr(const EulerExpr* expr);
void euler_free_expr(EulerExpr* expr);

#endif