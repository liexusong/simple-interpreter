#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct Token {
    int type;
    int value;
} Token;

#define TYPE_INTEGER 1
#define TYPE_PLUS    2
#define TYPE_MINUS   3
#define TYPE_MUL     4
#define TYPE_DIV     5
#define TYPE_LPAREN  6
#define TYPE_RPAREN  7
#define TYPE_ERROR   100
#define TYPE_EOF     0

typedef struct Lexer {
    char *codes;
    int length;
    int pos;
} Lexer;

typedef struct Node {
    struct Node *left;
    struct Node *right;
    Token *token;
} Node;

typedef struct Parser {
    Lexer *lexer;
    Token *lastToken;
    Node *root;
} Parser;

#define is_space(c) ((c) == '\t' || (c) == ' ' || (c) == '\r' || (c) == '\n')
#define is_digit(c) ((c) >= '0' && (c) <= '9')

Node *expr(Parser *parser);

Token *newToken(int type, int value)
{
    Token *token = malloc(sizeof(Token));

    if (token) {
        token->type = type;
        token->value = value;
    }

    return token;
}

Lexer *newLexer(char *codes, int length)
{
    Lexer *lexer = malloc(sizeof(Lexer));

    if (lexer) {
        lexer->codes = codes;
        lexer->length = length;
        lexer->pos = 0;
    }

    return lexer;
}

Token *getIntegerToken(Lexer *lexer)
{
    char temp[32];
    int last = 0;

    while (lexer->pos < lexer->length) {
        if (is_digit(lexer->codes[lexer->pos])) {
            temp[last++] = lexer->codes[lexer->pos];
            lexer->pos++;
        } else {
            break;
        }
    }

    temp[last] = 0;

    return newToken(TYPE_INTEGER, atoi(temp));
}

Token *getNextToken(Lexer *lexer)
{
    char lc;

    while (lexer->pos < lexer->length) {

        if (is_space(lexer->codes[lexer->pos])) {
            lexer->pos++;
            continue;
        }

        lc = lexer->codes[lexer->pos];

        if (is_digit(lc)) {
            return getIntegerToken(lexer);
        }

        if (lc == '+') {
            lexer->pos++;
            return newToken(TYPE_PLUS, '+');
        }

        if (lc == '-') {
            lexer->pos++;
            return newToken(TYPE_MINUS, '-');
        }

        if (lc == '*') {
            lexer->pos++;
            return newToken(TYPE_MUL, '*');
        }

        if (lc == '/') {
            lexer->pos++;
            return newToken(TYPE_DIV, '/');
        }

        if (lc == '(') {
            lexer->pos++;
            return newToken(TYPE_LPAREN, '(');
        }

        if (lc == ')') {
            lexer->pos++;
            return newToken(TYPE_RPAREN, ')');
        }

        return newToken(TYPE_ERROR, 0);
    }

    return newToken(TYPE_EOF, 0);
}

Node *newNode(Node *left, Node *right, Token *token)
{
    Node *node = malloc(sizeof(Node));

    if (node) {
        node->left = left;
        node->right = right;
        node->token = token;
    }

    return node;
}

Node *newBinOpNode(Node *left, Node *right, Token *token)
{
    return newNode(left, right, token);
}

Node *newNumNode(Token *token)
{
    return newNode(NULL, NULL, token);
}

Parser *newParser(char *codes)
{
    Lexer *lexer = newLexer(codes, strlen(codes));
    if (!lexer) {
        return NULL;
    }

    Parser *parser = malloc(sizeof(Parser));
    if (parser) {
        parser->lexer = lexer;
        parser->lastToken = getNextToken(parser->lexer);
        parser->root = NULL;
    }

    return parser;
}

int check(Parser *parser, int type)
{
    if (parser->lastToken->type == type) {
        parser->lastToken = getNextToken(parser->lexer);
        return 1;
    }
    return 0;
}

Node *factor(Parser *parser)
{
    Token *token = parser->lastToken;
    Node *node = NULL;

    if (token->type == TYPE_INTEGER) {
        check(parser, TYPE_INTEGER);
        return newNumNode(token);
    } else if (token->type == TYPE_LPAREN) {
        check(parser, TYPE_LPAREN);
        node = expr(parser);
        check(parser, TYPE_RPAREN);
    }

    return node;
}

Node *term(Parser *parser)
{
    Node *node = factor(parser);

    while (parser->lastToken->type == TYPE_MUL
        || parser->lastToken->type == TYPE_DIV)
    {
        Token *token = parser->lastToken;

        if (token->type == TYPE_MUL) {
            check(parser, TYPE_MUL);
        } else {
            check(parser, TYPE_DIV);
        }

        node = newBinOpNode(node, factor(parser), token);
    }

    return node;
}

Node *expr(Parser *parser)
{
    Node *node = term(parser);

    while (parser->lastToken->type == TYPE_PLUS
        || parser->lastToken->type == TYPE_MINUS)
    {
        Token *token = parser->lastToken;

        if (token->type == TYPE_PLUS) {
            check(parser, TYPE_PLUS);
        } else {
            check(parser, TYPE_MINUS);
        }

        node = newBinOpNode(node, term(parser), token);
    }

    return node;
}

void parse(Parser *parser)
{
    parser->root = expr(parser);
}

int nodeValue(Node *node)
{
    if (!node) {
        return 0;
    }

    switch (node->token->type) {
    case TYPE_INTEGER:
        return node->token->value;
    case TYPE_PLUS:
        return nodeValue(node->left) + nodeValue(node->right);
    case TYPE_MINUS:
        return nodeValue(node->left) - nodeValue(node->right);
    case TYPE_MUL:
        return nodeValue(node->left) * nodeValue(node->right);
    case TYPE_DIV:
        return nodeValue(node->left) / nodeValue(node->right);
    }

    return 0;
}

int interpret(Parser *parser)
{
    int result = nodeValue(parser->root);
}

int main()
{
    char buffer[1024] = {0};
    Parser *parser;
    int result;

    scanf("%s", buffer);

    parser = newParser(buffer);

    parse(parser);

    result = interpret(parser);

    printf("Result: %d\n", result);

    return 0;
}
