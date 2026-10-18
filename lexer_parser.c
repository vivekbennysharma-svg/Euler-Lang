#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<stdbool.h>

//token types
typedef enum {
    TOK_EOF,
    TOK_NUMBER,
    TOK_IDENTIFIER,
    TOK_PLUS,
    TOK_MINUS,
    TOK_MULTIPLY,
    TOK_DIVIDE,
    TOK_CARET,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_ASSIGN,
    TOK_SEMICOLON,
    TOK_COMMA
} TokenType;

//atomic token structure
typedef struct {
    TokenType type;
    char value[64];
} Token;

//lexer structure
typedef struct {
    const char* input;
    size_t pos;
} Lexer;

//AST node types
typedef enum {
    NODE_NUMBER,
    NODE_IDENTIFIER,
    NODE_BINARY_OP,
    NODE_CALL,
    NODE_LET,
    NODE_PRINT,
    NODE_EXPR_STMT,
    NODE_PROGRAM
} ASTNodeType;

//AST node structure
typedef struct ASTNode {
    ASTNodeType type;
    char* value;
    struct ASTNode* left;
    struct ASTNode* right;
    struct ASTNode** children;
    int child_count;
} ASTNode;

//parser structure
typedef struct {
    Lexer lexer;
    Token current;
} Parser;

#define MAX_SYMBOLS 256
static char symbol_table[MAX_SYMBOLS][64];
static int symbol_count = 0;

//    ||FUNCTION PROTOTYPES||
//File reader
char* read_file_to_string(const char* filepath);

//Lexer functions
void lexer_init(Lexer* lexer, const char* src);
void lexer_skip_whitespace(Lexer* lexer);
Token lexer_next_token(Lexer* lexer);

//AST Engine functions
ASTNode* ast_create_node(ASTNodeType type, const char* value);
void ast_add_child(ASTNode* parent, ASTNode* child);
void ast_free(ASTNode* node);

//Parser functions
void parser_advance(Parser* parser);
ASTNode* parse_factor(Parser* parser);
ASTNode* parse_term(Parser* parser);
ASTNode* parse_power(Parser* parser);
ASTNode* parse_expr(Parser* parser);
ASTNode* parse_statement(Parser* parser);
ASTNode* parse_program(Parser* parser);

//Code Generator functions
void codegen_expr(const ASTNode* node);
void codegen_program(const ASTNode* root);

//Symbol tracking
static void add_symbol(const char* name);
static bool is_defined_symbol(const char* name);


//    ||FUNCTION IMPLEMENTATIONS||
char* read_file_to_string(const char* filepath) {
    FILE* file = fopen(filepath, "rb");
    if (!file) {
        perror("Failed to open file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = (char*)malloc(length + 1);
    if (!buffer) {
        perror("Failed to allocate memory");
        fclose(file);
        return NULL;
    }

    fread(buffer, 1, length, file);
    buffer[length] = '\0';

    fclose(file);
    return buffer;
}

void lexer_init(Lexer* lexer, const char* src) {
    lexer->input = src;
    lexer->pos = 0;
}

void lexer_skip_whitespace(Lexer* lexer) {
    while (lexer->input[lexer->pos]!= '\0' && isspace((unsigned char)lexer->input[lexer->pos])) {
        lexer->pos++;
    }
}
Token lexer_next_token(Lexer* lexer) {
    lexer_skip_whitespace(lexer);

    Token tok;
    memset(&tok, 0, sizeof(tok));
    memset(tok.value, 0, sizeof(tok.value));

    char current_char = lexer->input[lexer->pos];
    if (current_char == '\0') {
        tok.type = TOK_EOF;
        return tok;
    }

    if (isdigit(current_char)) {
        size_t idx = 0;
        while ((isdigit((unsigned char)lexer->input[lexer->pos]) || lexer->input[lexer->pos] == '.') && idx < 63){
            tok.value[idx++] = lexer->input[lexer->pos++];
        }
        tok.type = TOK_NUMBER;
        return tok;
    }

    if (isalpha(current_char)) {
        size_t idx = 0;
        while ((isalnum((unsigned char)lexer->input[lexer->pos]) || lexer->input[lexer->pos] == '_') && idx < 63) {
            tok.value[idx++] = lexer->input[lexer->pos++];
        }
        tok.type = TOK_IDENTIFIER;
        return tok;
    }

    lexer->pos++;
    tok.value[0] = current_char;
    tok.value[1] = '\0';

    switch (current_char) {
        case '+': tok.type = TOK_PLUS; return tok;
        case '-': tok.type = TOK_MINUS; return tok;
        case '*': tok.type = TOK_MULTIPLY; return tok;
        case '/': tok.type = TOK_DIVIDE; return tok;
        case '^': tok.type = TOK_CARET; return tok;
        case '(': tok.type = TOK_LPAREN; return tok;
        case ')': tok.type = TOK_RPAREN; return tok;
        case '=': tok.type = TOK_ASSIGN; return tok;
        case ';': tok.type = TOK_SEMICOLON; return tok;
        case ',': tok.type = TOK_COMMA; return tok;
        default:
            fprintf(stderr, "Unknown character: %c\n", current_char);
            exit(EXIT_FAILURE);
    }
}

ASTNode* ast_create_node(ASTNodeType type, const char* value) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = type;
    node->value = strdup(value);
    node->left = NULL;
    node->right = NULL;
    node->children = NULL;
    node->child_count = 0;
    return node;
}

void ast_add_child(ASTNode* parent, ASTNode* child) {
    if (!parent->left) {
        parent->left = child;
    } else if (!parent->right) {
        parent->right = child;
    } else {
        fprintf(stderr, "Cannot add more than two children to a binary node\n");
        exit(EXIT_FAILURE);
    }
}

void ast_free(ASTNode* node) {
    if (!node) return;
    if(node->value) {
        free(node->value);
    }
    ast_free(node->left);
    ast_free(node->right);
    for(int i = 0; i < node->child_count; i++) {
        ast_free(node->children[i]);
    }
    if(node->children) {
        free(node->children);
    }
    free(node);
}

void parser_advance(Parser* parser) {
    parser->current = lexer_next_token(&parser->lexer);
}

ASTNode* parse_factor(Parser* parser) {
    Token token = parser->current;
    if (token.type == TOK_NUMBER) {
        parser_advance(parser);
        return ast_create_node(NODE_NUMBER, token.value);
    } else if (token.type == TOK_IDENTIFIER) {
        char name[64];
        strncpy(name, token.value, sizeof(name));
        parser_advance(parser);
        
        if(parser->current.type == TOK_LPAREN) {
            parser_advance(parser);
            ASTNode* call_node = ast_create_node(NODE_CALL, name);

            if(parser->current.type != TOK_RPAREN) {
                do {
                    ASTNode* arg = parse_expr(parser);
                    call_node->children = (ASTNode**)realloc(call_node->children, sizeof(ASTNode*) * (call_node->child_count + 1));
                    call_node->children[call_node->child_count++] = arg;
                    if(parser->current.type == TOK_COMMA) {
                        parser_advance(parser);
                    } else {
                        break;
                    }
                } while (1);
            }

            if (parser->current.type != TOK_RPAREN) {
                fprintf(stderr, "Expected ')'\n");
                exit(EXIT_FAILURE);
            }
            parser_advance(parser);
            return call_node;
        }
        return ast_create_node(NODE_IDENTIFIER, name);
    }
    if (token.type == TOK_LPAREN) {
        parser_advance(parser);
        ASTNode* node = parse_expr(parser);
        if (parser->current.type != TOK_RPAREN) {
            fprintf(stderr, "Expected ')'\n");
            exit(EXIT_FAILURE);
        }
        parser_advance(parser);
        return node;
    }
    if(token.type == TOK_MINUS){
        parser_advance(parser);
        ASTNode* zero = ast_create_node(NODE_NUMBER, "0");
        ASTNode* sub = ast_create_node(NODE_BINARY_OP, "-");
        ast_add_child(sub, zero);
        ast_add_child(sub, parse_factor(parser));
        return sub;
    }
    fprintf(stderr, "Unexpected token: %s\n", token.value);
    exit(EXIT_FAILURE);
}

ASTNode* parse_term(Parser* parser) {
    ASTNode* node = parse_power(parser);

    while (parser->current.type == TOK_MULTIPLY || parser->current.type == TOK_DIVIDE) {
        Token token = parser->current;
        parser_advance(parser);
        ASTNode* new_node = ast_create_node(NODE_BINARY_OP, token.value);
        ast_add_child(new_node, node);
        ast_add_child(new_node, parse_power(parser));
        node = new_node;
    }
    return node;
}

ASTNode* parse_power(Parser* parser) {
    ASTNode* node = parse_factor(parser);
    if (parser->current.type == TOK_CARET) {
        Token token = parser->current;
        parser_advance(parser);
        ASTNode* new_node = ast_create_node(NODE_BINARY_OP, token.value);
        ast_add_child(new_node, node);
        ast_add_child(new_node, parse_power(parser));
        node = new_node;
    }
    return node;
}

ASTNode* parse_expr(Parser* parser) {
    ASTNode* node = parse_term(parser);
    while (parser->current.type == TOK_PLUS || parser->current.type == TOK_MINUS) {
        Token token = parser->current;
        parser_advance(parser);
        ASTNode* new_node = ast_create_node(NODE_BINARY_OP, token.value);
        ast_add_child(new_node, node);
        ast_add_child(new_node, parse_term(parser));
        node = new_node;
    }
    return node;
}

ASTNode* parse_statement(Parser* parser){
    if(parser->current.type == TOK_IDENTIFIER && strcmp(parser->current.value, "let") == 0) {
        parser_advance(parser);
        if(parser->current.type != TOK_IDENTIFIER) {
            fprintf(stderr, "Expected identifier after 'let'\n");
            exit(EXIT_FAILURE);
        }
        char var_name[64];
        strncpy(var_name, parser->current.value, sizeof(var_name));
        parser_advance(parser);
        if(parser->current.type != TOK_ASSIGN) {
            fprintf(stderr, "Expected '=' after identifier in let statement\n");
            exit(EXIT_FAILURE);
        }
        parser_advance(parser);

        ASTNode* expr = parse_expr(parser);
        if(parser->current.type == TOK_SEMICOLON){
            parser_advance(parser);
        }

        ASTNode* stmt = ast_create_node(NODE_LET, var_name);
        stmt->left = expr;
        return stmt;
    }

    if(parser->current.type == TOK_IDENTIFIER && strcmp(parser->current.value, "print")==0){
        parser_advance(parser);
        if(parser->current.type != TOK_LPAREN){
            fprintf(stderr, "Expected '(' after 'print'\n");
            exit(EXIT_FAILURE);
        }
        parser_advance(parser);
        ASTNode* expr = parse_expr(parser);
        if(parser->current.type != TOK_RPAREN){
            fprintf(stderr, "Expected ')' after print expression\n");
            exit(EXIT_FAILURE);
        }
        parser_advance(parser);
        if(parser->current.type == TOK_SEMICOLON){
            parser_advance(parser);
        }

        ASTNode* stmt = ast_create_node(NODE_PRINT, "print");
        stmt->left = expr;
        return stmt;
    }

    ASTNode* expr = parse_expr(parser);
    if(parser->current.type == TOK_SEMICOLON){
        parser_advance(parser);
    }
    ASTNode* stmt = ast_create_node(NODE_EXPR_STMT, NULL);
    stmt->left = expr;
    return stmt;
}

ASTNode* parse_program(Parser* parser) {
    ASTNode* program = ast_create_node(NODE_PROGRAM, NULL);
    while (parser->current.type != TOK_EOF) {
        ASTNode* stmt = parse_statement(parser);
        program->children = (ASTNode**)realloc(program->children, sizeof(ASTNode*) * (program->child_count + 1));
        program->children[program->child_count++] = stmt;
    }
    return program;
}

void codegen_expr(const ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case NODE_NUMBER:
            if(strchr(node->value, '.') != NULL) printf("euler_num(%s)", node->value);
            else printf("euler_num(%s.0)", node->value);
            break;
        case NODE_IDENTIFIER:
            if(is_defined_symbol(node->value)){
                printf("%s", node->value);
            } else {
                printf("euler_var(\"%s\")", node->value);
            }
            break;
        case NODE_CALL:
            printf("euler_%s(", node->value);
            for(int i=0; i<node->child_count; i++){
                if(i>0) printf(", ");
                codegen_expr(node->children[i]);
            }
            printf(")");
            break;
        case NODE_BINARY_OP:
            if(strcmp(node->value, "+")==0) printf("euler_add(");
            else if(strcmp(node->value, "-")==0) printf("euler_sub(");
            else if(strcmp(node->value, "*")==0) printf("euler_mul(");
            else if(strcmp(node->value, "/")==0) printf("euler_div(");
            else if(strcmp(node->value, "^")==0) printf("euler_pow(");
            codegen_expr(node->left);
            printf(", ");
            codegen_expr(node->right);
            printf(")");
            break;
        default:
            fprintf(stderr, "Unknown AST node type\n");
            exit(EXIT_FAILURE);
    }
}

void codegen_program(const ASTNode* root) {
    printf("#include <stdio.h>\n");
    printf("#include \"euler_runtime.h\"\n\n");
    printf("int main() {\n");
    
    if(root && root->type == NODE_PROGRAM) {
        for(int i=0; i<root->child_count; i++){
            ASTNode* stmt = root->children[i];
            if(stmt->type == NODE_LET) {
                printf("EulerExpr* %s = euler_simplify(", stmt->value);
                codegen_expr(stmt->left);
                printf(");\n");
                add_symbol(stmt->value);
            } else if(stmt->type == NODE_PRINT) {
                printf("     {\n");
                printf("         EulerExpr* _p = euler_simplify(");
                codegen_expr(stmt->left);
                printf(");\n");
                printf("         euler_print_expr(_p);\n");
                printf("         printf(\"\\n\");\n");
                printf("    }\n");
            } else if(stmt->type == NODE_EXPR_STMT) {
                printf("    {\n");
                printf("        EulerExpr* _e = euler_simplify(");
                codegen_expr(stmt->left);
                printf(");\n");
                printf("        euler_print_expr(_e);\n");
                printf("        printf(\"\\n\");\n");
                printf("    }\n");
            }
        }
    }

    printf("    return 0;\n");
    printf("}\n");
}

//    ||MAIN FUNCTION||
int main(int argc, char* argv[]) {
    if (argc < 2)  return 1;

    char* src = read_file_to_string(argv[1]);

    Parser parser;
    lexer_init(&parser.lexer, src);
    parser_advance(&parser);

    ASTNode* root = parse_program(&parser);
    codegen_program(root);

    ast_free(root);
    free(src);

    return 0;
}

static void add_symbol(const char* name){
    if(symbol_count < MAX_SYMBOLS){
        strncpy(symbol_table[symbol_count], name, sizeof(symbol_table[symbol_count])-1);
        symbol_table[symbol_count][sizeof(symbol_table[symbol_count])-1]='\0';
        symbol_count++;
    }
}

static bool is_defined_symbol(const char* name){
    for(int i=0;i<symbol_count;i++){
        if(strcmp(symbol_table[i],name)==0){
            return true;
        }
    }
    return false;
}