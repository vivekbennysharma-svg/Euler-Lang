#include "euler_runtime.h"

static EulerExpr* alloc_node(ExprKind kind) {
    EulerExpr* node = (EulerExpr*)malloc(sizeof(EulerExpr));
    if (!node) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    node->kind = kind;
    node->val = 0.0;
    node->name[0] = '\0';
    node->left = NULL;
    node->right = NULL;
    return node;
}

EulerExpr* euler_num(double val) {
    EulerExpr* node = alloc_node(EXPR_NUM);
    node->val = val;
    return node;
}

EulerExpr* euler_var(const char* name) {
    EulerExpr* node = alloc_node(EXPR_VAR);
    strncpy(node->name, name, sizeof(node->name) - 1);
    node->name[sizeof(node->name) - 1] = '\0';
    return node;
}

EulerExpr* euler_add(EulerExpr* left, EulerExpr* right) {
    EulerExpr* node = alloc_node(EXPR_ADD);
    node->left = left;
    node->right = right;
    return node;
}

EulerExpr* euler_sub(EulerExpr* left, EulerExpr* right) {
    EulerExpr* node = alloc_node(EXPR_SUB);
    node->left = left;
    node->right = right;
    return node;
}

EulerExpr* euler_mul(EulerExpr* left, EulerExpr* right) {
    EulerExpr* node = alloc_node(EXPR_MUL);
    node->left = left;
    node->right = right;
    return node;
}

EulerExpr* euler_div(EulerExpr* left, EulerExpr* right) {
    EulerExpr* node = alloc_node(EXPR_DIV);
    node->left = left;
    node->right = right;
    return node;
}

EulerExpr* euler_pow(EulerExpr* left, EulerExpr* right) {
    EulerExpr* node = alloc_node(EXPR_POW);
    node->left = left;
    node->right = right;
    return node;
}

EulerExpr* euler_sin(EulerExpr* expr) {
    EulerExpr* node = alloc_node(EXPR_SIN);
    node->left = expr;
    return node;
}

EulerExpr* euler_cos(EulerExpr* expr) {
    EulerExpr* node = alloc_node(EXPR_COS);
    node->left = expr;
    return node;
}

static EulerExpr* clone_expr(const EulerExpr* expr) {
    if (!expr) return NULL;

    EulerExpr* new_expr = alloc_node(expr->kind);
    new_expr->val = expr->val;
    strncpy(new_expr->name, expr->name, sizeof(new_expr->name) - 1);
    new_expr->name[sizeof(new_expr->name) - 1] = '\0';
    new_expr->left = clone_expr(expr->left);
    new_expr->right = clone_expr(expr->right);
    return new_expr;
}

EulerExpr* euler_diff(EulerExpr* expr, EulerExpr* var) {
    if (!expr || !var) return NULL;


    const char* target_var = var->name;
    switch (expr->kind) {
        case EXPR_NUM:
            return euler_num(0.0);
        case EXPR_VAR:
            if(strcmp(expr->name, target_var) == 0) {
                return euler_num(1.0);
            } else {
                return euler_num(0.0);
            }
        case EXPR_ADD:
            return euler_add(euler_diff(expr->left, var), euler_diff(expr->right, var));
        case EXPR_SUB:
            return euler_sub(euler_diff(expr->left, var), euler_diff(expr->right, var));
        case EXPR_MUL: {
            EulerExpr* left_diff = euler_diff(expr->left, var);
            EulerExpr* right_diff = euler_diff(expr->right, var);
            EulerExpr* left_clone = clone_expr(expr->left);
            EulerExpr* right_clone = clone_expr(expr->right);
            EulerExpr* term1 = euler_mul(left_diff, right_clone);
            EulerExpr* term2 = euler_mul(left_clone, right_diff);
            return euler_add(term1, term2);
        }
        case EXPR_DIV: {
            EulerExpr* left_diff = euler_diff(expr->left, var);
            EulerExpr* right_diff = euler_diff(expr->right, var);
            EulerExpr* left_clone = clone_expr(expr->left);
            EulerExpr* right_clone = clone_expr(expr->right);
            EulerExpr* numerator1 = euler_mul(left_diff, right_clone);
            EulerExpr* numerator2 = euler_mul(left_clone, right_diff);
            EulerExpr* numerator = euler_sub(numerator1, numerator2);
            EulerExpr* denominator = euler_pow(right_clone, euler_num(2.0));
            return euler_div(numerator, denominator);
        }
        case EXPR_POW: {
            if (expr->right && expr->right->kind == EXPR_NUM) {
                double exponent = expr->right->val;
                EulerExpr* new_exponent = euler_num(exponent - 1);
                EulerExpr* base_clone = clone_expr(expr->left);
                EulerExpr* base_diff = euler_diff(expr->left, var);
                EulerExpr* term1 = euler_mul(euler_num(exponent), euler_pow(base_clone, new_exponent));
                return euler_mul(term1, base_diff);
            } else {
                fprintf(stderr, "Differentiation of non-constant exponent not implemented\n");
                exit(EXIT_FAILURE);
            }
        }
        case EXPR_SIN: {
            EulerExpr* inner_diff = euler_diff(expr->left, var);
            EulerExpr* cos_expr = euler_cos(clone_expr(expr->left));
            return euler_mul(cos_expr, inner_diff);
        }
        case EXPR_COS: {
            EulerExpr* inner_diff = euler_diff(expr->left, var);
            EulerExpr* sin_expr = euler_sin(clone_expr(expr->left));
            EulerExpr* neg_sin_expr = euler_mul(euler_num(-1.0), sin_expr);
            return euler_mul(neg_sin_expr, inner_diff);
        }
        default:
            return euler_num(0.0);
    }
}

void euler_print_expr(const EulerExpr* expr) {
    if (!expr) return;

    switch (expr->kind) {
        case EXPR_NUM:
            printf("%g", expr->val);
            break;
        case EXPR_VAR:
            printf("%s", expr->name);
            break;
        case EXPR_ADD:
            printf("(");
            euler_print_expr(expr->left);
            printf(" + ");
            euler_print_expr(expr->right);
            printf(")");
            break;
        case EXPR_SUB:
            printf("(");
            euler_print_expr(expr->left);
            printf(" - ");
            euler_print_expr(expr->right);
            printf(")");
            break;
        case EXPR_MUL:
            printf("(");
            euler_print_expr(expr->left);
            printf(" * ");
            euler_print_expr(expr->right);
            printf(")");
            break;
        case EXPR_DIV:
            printf("(");
            euler_print_expr(expr->left);
            printf(" / ");
            euler_print_expr(expr->right);
            printf(")");
            break;
        case EXPR_POW:
            printf("(");
            euler_print_expr(expr->left);
            printf(" ^ ");
            euler_print_expr(expr->right);
            printf(")");
            break;
        case EXPR_SIN:
            printf("sin(");
            euler_print_expr(expr->left);
            printf(")");
            break;
        case EXPR_COS:
            printf("cos(");
            euler_print_expr(expr->left);
            printf(")");
            break;
    }
}

void euler_free_expr(EulerExpr* expr) {
    if (!expr) return;

    euler_free_expr(expr->left);
    euler_free_expr(expr->right);
    free(expr);
}
