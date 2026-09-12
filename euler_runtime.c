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

static bool is_num(const EulerExpr* expr, double val) {
    return expr && expr->kind == EXPR_NUM && fabs(expr->val - val)<1e-9;
}

EulerExpr* euler_simplify(EulerExpr* expr) {
    if (!expr) return NULL;

    expr->left = euler_simplify(expr->left);
    expr->right = euler_simplify(expr->right);

    if(expr->left && expr->right && expr->left->kind == EXPR_NUM && expr->right->kind == EXPR_NUM) {
        double left_val = expr->left->val;
        double right_val = expr->right->val;
        double res = 0.0;
        bool foldable = true;

        switch (expr->kind) {
            case EXPR_ADD:
                res = left_val + right_val;
                break;
            case EXPR_SUB:
                res = left_val - right_val;
                break;
            case EXPR_MUL:
                res = left_val * right_val;
                break;
            case EXPR_DIV:
                if(fabs(right_val) > 1e-9) res = left_val / right_val;
                else foldable = false;
                break;
            case EXPR_POW:
                res = pow(left_val, right_val);
                break;
            default:
                foldable = false;
                break;
        }
        if(foldable){
            euler_free_expr(expr->left);
            euler_free_expr(expr->right);
            expr->kind = EXPR_NUM;
            expr->val = res;
            expr->left = NULL;
            expr->right = NULL;
            return expr;
        }
    }
    switch (expr->kind) {
        case EXPR_ADD:
            // x + 0 = x
            if(is_num(expr->right, 0.0)){
                EulerExpr* keep = expr->left;
                euler_free_expr(expr->right);
                free(expr);
                return keep;
            }
            // 0 + x = x
            if(is_num(expr->left, 0.0)){
                EulerExpr* keep = expr->right;
                euler_free_expr(expr->left);
                free(expr);
                return keep;
            }
            // c1 + (c2 + x) = (c1 + c2) + x
            if(expr->left && expr->right && expr->left->kind == EXPR_NUM && expr->right->kind == EXPR_ADD){
                if(expr->right->left && expr->right->left->kind == EXPR_NUM){
                    double new_val = expr->left->val + expr->right->left->val;
                    EulerExpr* x = expr->right->right;
                    euler_free_expr(expr->left);
                    euler_free_expr(expr->right->left);
                    free(expr->right);
                    expr->left=euler_num(new_val);
                    expr->right = x;
                    return euler_simplify(expr);
                }
            }
            break;
        case EXPR_SUB:
            // x - 0 = x
            if(is_num(expr->right, 0.0)){
                EulerExpr* keep = expr->left;
                euler_free_expr(expr->right);
                free(expr);
                return keep;
            }
            break;
        case EXPR_MUL:
            // x * 0 = 0 or 0 * x = 0
            if(is_num(expr->left, 0.0)||is_num(expr->right, 0.0)){
                euler_free_expr(expr->left);
                euler_free_expr(expr->right);
                expr->kind = EXPR_NUM;
                expr->val = 0.0;
                expr->left = NULL;
                expr->right = NULL;
                return expr;
            }
            // 1 * x = x
            if(is_num(expr->left, 1.0)){
                EulerExpr* keep = expr->right;
                euler_free_expr(expr->left);
                free(expr);
                return keep;
            }
            // x * 1 = x
            if(is_num(expr->right, 1.0)){
                EulerExpr* keep = expr->left;
                euler_free_expr(expr->right);
                free(expr);
                return keep;
            }
            // c1 * (c2 * x) = (c1 * c2) * x
            if(expr->left && expr->right && expr->left->kind == EXPR_NUM && expr->right->kind == EXPR_MUL){
                if(expr->right->left && expr->right->left->kind == EXPR_NUM){
                    double new_val = expr->left->val * expr->right->left->val;
                    EulerExpr* x = expr->right->right;

                    euler_free_expr(expr->left);
                    euler_free_expr(expr->right->left);
                    free(expr->right);

                    expr->left = euler_num(new_val);
                    expr->right = x;
                    return euler_simplify(expr);
                }
                // c1 * (x * c2) = (c1 * c2) * x
                if(expr->right->right && expr->right->right->kind == EXPR_NUM){
                    double new_val = expr->left->val * expr->right->right->val;
                    EulerExpr* x = expr->right->left;

                    euler_free_expr(expr->left);
                    euler_free_expr(expr->right->right);
                    free(expr->right);

                    expr->left = euler_num(new_val);
                    expr->right = x;
                    return euler_simplify(expr);
                }
            }
            // (c1 * x) * c2 = (c1 * c2) * x
            if(expr->right && expr->left && expr->right->kind == EXPR_NUM && expr->left->kind == EXPR_MUL){
                if(expr->left->left && expr->left->left->kind == EXPR_NUM){
                    double new_val = expr->right->val * expr->left->left->val;
                    EulerExpr* x = expr->left->right;

                    euler_free_expr(expr->right);
                    euler_free_expr(expr->left->left);
                    free(expr->left);

                    expr->right = euler_num(new_val);
                    expr->left = x;
                    return euler_simplify(expr);
                }
                // (x * c1) * c2 = (c1 * c2) * x
                if(expr->left->right && expr->left->right->kind == EXPR_NUM){
                    double new_val = expr->right->val * expr->left->right->val;
                    EulerExpr* x = expr->left->left;

                    euler_free_expr(expr->right);
                    euler_free_expr(expr->left->right);
                    free(expr->left);

                    expr->right = euler_num(new_val);
                    expr->left = x;
                    return euler_simplify(expr);
                }
            }
            break;
        case EXPR_DIV:
            // 0 / x = 0
            if(is_num(expr->left, 0.0) && !is_num(expr->right, 0.0)){
                euler_free_expr(expr->left);
                euler_free_expr(expr->right);
                expr->kind = EXPR_NUM;
                expr->val = 0.0;
                expr->left = NULL;
                expr->right = NULL;
                return expr;
            }
            // x / 1 = x
            if(is_num(expr->right, 1.0)){
                EulerExpr* keep = expr->left;
                euler_free_expr(expr->right);
                free(expr);
                return keep;
            }
            break;
        case EXPR_POW:
            // x ^ 0 = 1
            if(is_num(expr->right, 0.0)){
                euler_free_expr(expr->left);
                euler_free_expr(expr->right);
                expr->kind = EXPR_NUM;
                expr->val = 1.0;
                expr->left = NULL;
                expr->right = NULL;
                return expr;
            }
            // x ^ 1 = x
            if(is_num(expr->right, 1.0)){
                EulerExpr* keep = expr->left;
                euler_free_expr(expr->right);
                free(expr);
                return keep;
            }
            break;
        default:
            break;
    }
    return expr;
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
