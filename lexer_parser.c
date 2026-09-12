#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <ctype.h>

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
    char* value;
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
    NODE_CALL
} ASTNodeType;

//AST node structure
typedef struct ASTNode {
    ASTNodeType type;
    char* value;
    struct ASTNode* left;
    struct ASTNode* right;
} ASTNode;

//parser structure
typedef struct {
    Lexer lexer;
    Token current;
} Parser;

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

//Code Generator functions
void codegen_expr(const ASTNode* node);
void codegen_program(const ASTNode* root);

//    ||FUNCTION IMPLEMENTATIONS||
char* read_file_to_string(const char* filepath) {
    FILE* file = fopen(filepath, "r");
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

Token lexer_next_token(Lexer* lexer) {
    lexer_skip_whitespace(lexer);

    char current_char = lexer->input[lexer->pos];
    if (current_char == '\0') {
        return (Token){TOK_EOF, NULL};
    }

    if (isdigit(current_char)) {
        size_t start = lexer->pos;
        while (isdigit(lexer->input[lexer->pos])) {
            lexer->pos++;
        }
        size_t length = lexer->pos - start;
        char* value = (char*)malloc(length + 1);
        strncpy(value, lexer->input + start, length);
        value[length] = '\0';
        return (Token){TOK_NUMBER, value};
    }

    if (isalpha(current_char)) {
        size_t start = lexer->pos;
        while (isalnum(lexer->input[lexer->pos])) {
            lexer->pos++;
        }
        size_t length = lexer->pos - start;
        char* value = (char*)malloc(length + 1);
        strncpy(value, lexer->input + start, length);
        value[length] = '\0';
        return (Token){TOK_IDENTIFIER, value};
    }

    switch (current_char) {
        case '+':
            lexer->pos++;
            return (Token){TOK_PLUS, strdup("+")};
        case '-':
            lexer->pos++;
            return (Token){TOK_MINUS, strdup("-")};
        case '*':
            lexer->pos++;
            return (Token){TOK_MULTIPLY, strdup("*")};
        case '/':
            lexer->pos++;
            return (Token){TOK_DIVIDE, strdup("/")};
        case '^':
            lexer->pos++;
            return (Token){TOK_CARET, strdup("^")};
        case '(':
            lexer->pos++;
            return (Token){TOK_LPAREN, strdup("(")};
        case ')':
            lexer->pos++;
            return (Token){TOK_RPAREN, strdup(")")};
        case '=':
            lexer->pos++;
            return (Token){TOK_ASSIGN, strdup("=")};
        case ';':
            lexer->pos++;
            return (Token){TOK_SEMICOLON, strdup(";")};
        case ',':
            lexer->pos++;
            return (Token){TOK_COMMA, strdup(",")};
    }

    fprintf(stderr, "Unknown character: %c\n", current_char);
    exit(EXIT_FAILURE);
}

ASTNode* ast_create_node(ASTNodeType type, const char* value) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = type;
    node->value = strdup(value);
    node->left = NULL;
    node->right = NULL;
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
    free(node->value);
    ast_free(node->left);
    ast_free(node->right);
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
        parser_advance(parser);
        return ast_create_node(NODE_IDENTIFIER, token.value);
    } else if (token.type == TOK_LPAREN) {
        parser_advance(parser);
        ASTNode* node = parse_expr(parser);
        if (parser->current.type != TOK_RPAREN) {
            fprintf(stderr, "Expected ')'\n");
            exit(EXIT_FAILURE);
        }
        parser_advance(parser);
        return node;
    } else {
        fprintf(stderr, "Unexpected token: %s\n", token.value);
        exit(EXIT_FAILURE);
    }
}

ASTNode* parse_term(Parser* parser) {
    ASTNode* node = parse_factor(parser);
    while (parser->current.type == TOK_MULTIPLY || parser->current.type == TOK_DIVIDE) {
        Token token = parser->current;
        parser_advance(parser);
        ASTNode* new_node = ast_create_node(NODE_BINARY_OP, token.value);
        ast_add_child(new_node, node);
        ast_add_child(new_node, parse_factor(parser));
        node = new_node;
    }
    return node;
}

ASTNode* parse_power(Parser* parser) {
    ASTNode* node = parse_term(parser);
    while (parser->current.type == TOK_CARET) {
        Token token = parser->current;
        parser_advance(parser);
        ASTNode* new_node = ast_create_node(NODE_BINARY_OP, token.value);
        ast_add_child(new_node, node);
        ast_add_child(new_node, parse_term(parser));
        node = new_node;
    }
    return node;
}

ASTNode* parse_expr(Parser* parser) {
    ASTNode* node = parse_power(parser);
    while (parser->current.type == TOK_PLUS || parser->current.type == TOK_MINUS) {
        Token token = parser->current;
        parser_advance(parser);
        ASTNode* new_node = ast_create_node(NODE_BINARY_OP, token.value);
        ast_add_child(new_node, node);
        ast_add_child(new_node, parse_power(parser));
        node = new_node;
    }
    return node;
}

void codegen_expr(const ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case NODE_NUMBER:
            printf("%s", node->value);
            break;
        case NODE_IDENTIFIER:
            printf("%s", node->value);
            break;
        case NODE_BINARY_OP:
            printf("(");
            codegen_expr(node->left);
            printf(" %s ", node->value);
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
    printf("    EulerExpr* res = ;\n");
    if (!root) return;
    codegen_expr(root);
    printf(";\n");
    printf("    euler_print_expr(res);\n");
    printf("    printf(\"\\n\");\n");
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

    ASTNode* root = parse_expr(&parser);
    codegen_program(root);

    ast_free(root);
    free(src);

    return 0;
}