#include "token.h"
#include <syntax.h>
#include <exception.h>
#include <process.h>
#include <vector.h>
#include <posix.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct Node                 *syntax_process_module              (struct TokenStream *ts, struct Settings *st);
struct Node                 *syntax_process_block               (struct TokenStream *ts, struct Settings *st);
struct TypeNode             *syntax_process_type                (struct TokenStream *ts, struct Settings *st);
struct Node                 *syntax_process_expression          (struct TokenStream *ts, struct Settings *st);
struct Node                 *syntax_process_primary             (struct TokenStream *ts, struct Settings *st);
struct FunctionSignature    *syntax_process_function_signature  (struct TokenStream *ts, struct Settings *st);
struct Node                 *syntax_process_statement           (struct TokenStream *ts, struct Settings *st, bool top_level);

#define Alloc(T, x) struct T *x = (struct T*)_malloc(sizeof(struct T));
#define Get() (tokenstream_get(ts))
#define Prev() (tokenstream_get_prev(ts))
#define Next() (tokenstream_next(ts))
#define NextIs(x) (Get().type == (x))
#define Expect(x, msg) if (!NextIs(x)) error_syntax(msg, Get());
#define ExpectNext(x, msg) if (!NextIs(x)) error_syntax(msg, Get()); Next();

struct Node *annotate_node(struct Token begin_token, struct Node *node, struct Token end_token) {
    node->line_begin = begin_token.line_begin;
    node->position_begin = begin_token.position_begin;
    node->filename = begin_token.filename;
    node->line_end = end_token.line_end;
    node->position_end = end_token.position_end;
    return node;
}

struct TypeNode *annotate_type_node(struct Token begin_token, struct TypeNode *node, struct Token end_token) {
    node->line_begin = begin_token.line_begin;
    node->position_begin = begin_token.position_begin;
    node->filename = begin_token.filename;
    node->line_end = end_token.line_end;
    node->position_end = end_token.position_end;
    return node;
}

struct Node *syntax_process_module(struct TokenStream *ts, struct Settings *st) {
    struct Token start = Get();
    struct Vector statement_list = vnew();

    while (!NextIs(TokenEof)) {
        struct Node *node = syntax_process_statement(ts, st, true);
        if (node) {
            vpush(&statement_list, node);
        }
    }
    return annotate_node(start, create_module(statement_list), Prev());
}

struct Node *syntax_process_block(struct TokenStream *ts, struct Settings *st) {
    struct Token start = Get();
    struct Vector statement_list = vnew();
    struct Vector reverse_statement_list = vnew();

    bool one_statement = true;
    if (NextIs(TokenBraceOpen)) {
        one_statement = false;
        Next();
    }

    const char *label = NULL;
    if (NextIs(TokenDot)) {
        Next();
        Expect(TokenIdentifier, "Identifier expected in label");
        label = Get().value_string;
        Next();
    }

    while (!NextIs(TokenEof) && !NextIs(TokenBraceClose)) {
        bool defer = false;
        if (NextIs(TokenDefer)) {
            Next();
            defer = true;
        }
        struct Node *node = syntax_process_statement(ts, st, false);
        if (node) {
            if (!defer)
                vpush(&statement_list, node);
            else
                vpush(&reverse_statement_list, node);
        }
        if (one_statement) break;
    }
    if (!one_statement) {
        ExpectNext(TokenBraceClose, "} expected after block");
    }
    int sz = vsize(&reverse_statement_list);
    for (int i = sz - 1; i >= 0; i--) {
        vpush(&statement_list, reverse_statement_list.ptr[i]);
    }

    return annotate_node(start, create_block(statement_list, label), Prev());
}

struct TypeNode *syntax_process_type(struct TokenStream *ts, struct Settings *st) {
    struct Token start = Get();
    struct TypeNode *node;
    int degree = 0;
    ExpectNext(TokenSharp, "# expected in type definition");
    if (NextIs(TokenInteger)) {
        degree = Get().value_int;
        Next();
    }
    Expect(TokenIdentifier, "Identifier expected in type definition");
    const char *typestr = Get().value_string;
    Next();
        
    if (!_strcmp(typestr, "V")) {
        node = create_type_void(degree);
    }
    else if (!_strcmp(typestr, "I")) {
        node = create_type_int(degree);
    }
    else if (!_strcmp(typestr, "C")) {
        node = create_type_char(degree);
    }
    else if (!_strcmp(typestr, "S")) {
        struct Vector names = vnew();
        struct Vector types = vnew();
        ExpectNext(TokenBraceOpen, "{ expected in struct type definition");
        while (true) {
            if (NextIs(TokenBraceClose)) break;
            Expect(TokenIdentifier, "Identifier expected in field of struct type definition");
            vpush(&names, _strdup(Get().value_string));
            Next();
            ExpectNext(TokenColon, ": expected in field of struct type definition");
            vpush(&types, syntax_process_type(ts, st));
            if (NextIs(TokenBraceClose)) break;
            ExpectNext(TokenComma, ", or } expected after field of struct type definition");
        }
        Next();
        node = create_type_struct(degree, names, types);
    }        
    else if (!_strcmp(typestr, "F")) {
        struct Vector types = vnew();
        ExpectNext(TokenParenthesisOpen, "( expected in function type definition");
        while (!NextIs(TokenParenthesisClose)) {
            vpush(&types, syntax_process_type(ts, st));
        }
        ExpectNext(TokenParenthesisClose, ") expected in function type definition");
        ExpectNext(TokenGetField, "-> expected in function type definition");
        struct TypeNode *return_type = syntax_process_type(ts, st);
        node = create_type_function(degree, types, return_type);
    }
    else {
        node = create_type_identifier(degree, _strdup(typestr));
    }

    return annotate_type_node(start, node, Prev());
}

bool next_is_operation(struct TokenStream *ts) {
    return (
        NextIs(TokenAnd)                ||
        NextIs(TokenOr)                 ||
        NextIs(TokenNot)                ||
        NextIs(TokenAmpersand)          ||
        NextIs(TokenPipe)               ||
        NextIs(TokenCaret)              ||
        NextIs(TokenTilde)              ||
        NextIs(TokenBitwiseShiftLeft)   ||
        NextIs(TokenBitwiseShiftRight)  ||
        NextIs(TokenPlus)               ||
        NextIs(TokenMinus)              ||
        NextIs(TokenMult)               ||
        NextIs(TokenDiv)                ||
        NextIs(TokenMod)                ||
        NextIs(TokenLess)               ||
        NextIs(TokenGreater)            ||
        NextIs(TokenEqual)              ||
        NextIs(TokenLessEqual)          ||
        NextIs(TokenGreaterEqual)       ||
        NextIs(TokenNotEqual));
}

int operation_priority(struct Token *token) {
    if (token->type == -TokenMinus              ||
        token->type == -TokenTilde              ||
        token->type == -TokenNot            ) {
        return 2;
    }
    if (token->type == TokenMult                ||
        token->type == TokenDiv                 ||
        token->type == TokenMod             ) {
        return 3;
    }
    if (token->type == TokenPlus                ||
        token->type == TokenMinus           ) {
        return 4;
    }
    if (token->type == TokenBitwiseShiftLeft    ||
        token->type == TokenBitwiseShiftRight    ) {
        return 5;
    }
    if (token->type == TokenLess                ||
        token->type == TokenGreater             ||
        token->type == TokenLessEqual           ||
        token->type == TokenGreaterEqual    ) {
        return 6;
    }
    if (
        token->type == TokenEqual               ||
        token->type == TokenNotEqual        ) {
        return 7;
    }
    if (token->type == TokenAmpersand) {
        return 8;
    }
    if (token->type == TokenCaret) {
        return 9;
    }
    if (token->type == TokenPipe) {
        return 10;
    }
    if (token->type == TokenAnd) {
        return 11;
    }
    if (token->type == TokenOr) {
        return 12;
    }
    if (token->type == TokenParenthesisOpen) {
        return 13;
    }
    error_syntax("Incorrect structure of the expression", *token);
}

struct Node *process_operation(struct Vector *primaries, struct Vector *operations, struct TokenStream *ts) {
    struct Token *token = vback(operations);

    bool unary = false;
    if (token->type < 0) {
        unary = true;
        token->type = -token->type;
    }
    enum NodeType node_type;
    if (token->type == TokenAnd)                node_type = NodeAnd;
    if (token->type == TokenOr)                 node_type = NodeOr;
    if (token->type == TokenNot)                node_type = NodeNot;
    if (token->type == TokenAmpersand)          node_type = NodeBitwiseAnd;
    if (token->type == TokenPipe)               node_type = NodeBitwiseOr;
    if (token->type == TokenCaret)              node_type = NodeBitwiseXor;
    if (token->type == TokenTilde)              node_type = NodeBitwiseNot;
    if (token->type == TokenBitwiseShiftLeft)   node_type = NodeBitwiseShiftLeft;
    if (token->type == TokenBitwiseShiftRight)  node_type = NodeBitwiseShiftRight;
    if (token->type == TokenPlus)               node_type = NodeAddition;
    if (token->type == TokenMinus)              node_type = NodeSubtraction;
    if (token->type == TokenMult)               node_type = NodeMultiplication;
    if (token->type == TokenDiv)                node_type = NodeDivision;
    if (token->type == TokenMod)                node_type = NodeModulo;
    if (token->type == TokenLess)               node_type = NodeLess;
    if (token->type == TokenGreater)            node_type = NodeGreater;
    if (token->type == TokenEqual)              node_type = NodeEqual;
    if (token->type == TokenLessEqual)          node_type = NodeLessEqual;
    if (token->type == TokenGreaterEqual)       node_type = NodeGreaterEqual;
    if (token->type == TokenNotEqual)           node_type = NodeNotEqual;

    struct Node *root;

    int sz = vsize(primaries);
    if (unary) {
        if (token->type != TokenMinus && token->type != TokenNot && token->type != TokenTilde) {
            error_syntax("Incorrect unary operator", *token);
        }
        if (sz < 1) {
            error_syntax("Incorrect structure of the expression", *token);
        }
        struct Node *right = primaries->ptr[sz - 1];
        root = create_binary_operator(node_type, NULL, right);
        root->line_begin = token->line_begin;
        root->position_begin = token->position_begin;
        root->line_end = right->line_end;
        root->position_end = right->position_end;
        root->filename = _strdup(right->filename);
        vpop(primaries);
    }
    else {
        if (sz < 2) {
            error_syntax("Incorrect structure of the expression", *token);
        }
        struct Node *left = primaries->ptr[sz - 2];
        struct Node *right = primaries->ptr[sz - 1];
        root = create_binary_operator(node_type, left, right);
        root->line_begin = left->line_begin;
        root->position_begin = left->position_begin;
        root->line_end = right->line_end;
        root->position_end = right->position_end;
        root->filename = _strdup(right->filename);
        vpop(primaries);
        vpop(primaries);
    }

    if (root->node_ptr == NULL) {
        error_syntax("Fatal Error", *token);
    }

    vpush(primaries, root);
    vpop(operations);
    return root;
}

struct Node *syntax_process_expression(struct TokenStream *ts, struct Settings *st) {
    struct Vector primaries = vnew();
    struct Vector operations = vnew();
    int ParenthesisLevel = 0;
    
    enum State {
        State_Identifier,
        State_UnaryOperation,
        State_BinaryOperation,
        State_ParenthesisOpen,
        State_ParenthesisClose,
    };
    enum State CurrentState;
    if (NextIs(TokenParenthesisOpen)) {
        vpush(&operations, tokenstream_pget(ts));
        Next();
        ParenthesisLevel++;
        CurrentState = State_ParenthesisOpen;
    }
    else if (next_is_operation(ts)) {
        struct Token *token = (struct Token*)_malloc(sizeof(struct Token));
        *token = Get();
        token->type = -token->type;
        vpush(&operations, token);
        Next();
        CurrentState = State_UnaryOperation;
    }
    else {
        vpush(&primaries, syntax_process_primary(ts, st));
        CurrentState = State_Identifier;
    }

    while (CurrentState == State_UnaryOperation ||
           CurrentState == State_BinaryOperation ||
           CurrentState == State_ParenthesisOpen ||
           next_is_operation(ts) ||
           ParenthesisLevel != 0) {
        
        if (NextIs(TokenParenthesisOpen)) {
            if (CurrentState != State_UnaryOperation &&
                CurrentState != State_BinaryOperation &&
                CurrentState != State_ParenthesisOpen) {
                error_syntax("Unexpected ( in expression", Get());
            }
            vpush(&operations, tokenstream_pget(ts));
            Next();
            ParenthesisLevel++;
            CurrentState = State_ParenthesisOpen;
        }
        else if (NextIs(TokenParenthesisClose)) {
            if (CurrentState != State_Identifier && CurrentState != State_ParenthesisClose) {
                error_syntax("Unexpected ) in expression", Get());
            }
            while (vsize(&operations) != NULL && ((struct Token*)vback(&operations))->type != TokenParenthesisOpen) {
                process_operation(&primaries, &operations, ts);
            }
            if (vsize(&operations) == NULL || ((struct Token*)vback(&operations))->type != TokenParenthesisOpen) {
                error_syntax("Unexpected ) in expression", Get());
            }
            vpop(&operations);
            Next();
            ParenthesisLevel--;
            CurrentState = State_ParenthesisClose;
        }
        else if (next_is_operation(ts)) {
            struct Token *token = (struct Token*)_malloc(sizeof(struct Token));
            *token = Get();
            if (CurrentState == State_UnaryOperation ||
                CurrentState == State_BinaryOperation ||
                CurrentState == State_ParenthesisOpen) {
                token->type = -token->type;
                CurrentState = State_UnaryOperation;
            }
            else {
                CurrentState = State_BinaryOperation;
            }
            while (vsize(&operations) &&
                operation_priority((struct Token*)vback(&operations)) <= operation_priority(token)) {
                process_operation(&primaries, &operations, ts);
            }
            vpush(&operations, token);
            Next();
        }
        else {
            if (CurrentState != State_UnaryOperation &&
                CurrentState != State_BinaryOperation &&
                CurrentState != State_ParenthesisOpen) {
                error_syntax("Unexpected identifier in expression", Get());
            }
            vpush(&primaries, syntax_process_primary(ts, st));
            CurrentState = State_Identifier;
        }
    }

    while (vsize(&operations)) {
        process_operation(&primaries, &operations, ts);
    }

    if (vsize(&primaries) != 1) {
        error_syntax("Incorrect expression", Get());
    }

    struct Node *res = primaries.ptr[0];    

    if (NextIs(TokenAs)) {
        struct Node *expression = res;
        Next();
        struct TypeNode *type = syntax_process_type(ts, st);
        res = create_as(expression, type);
    }
    return res;
}

struct Node *syntax_process_primary(struct TokenStream *ts, struct Settings *st) {
    struct Token start = Get();
    struct Node *node;

    if (NextIs(TokenInteger)) {
        node = create_integer(Get().value_int);
        Next();
    }
    else if (NextIs(TokenChar)) {
        node = create_char(Get().value_int), Get();
        Next();
    }
    else if (NextIs(TokenString)) {
        node = create_string(Get().value_string);
        Next();
    }
    else if (NextIs(TokenBracketOpen)) {
        struct Vector values = vnew();
        while (!NextIs(TokenBracketClose)) {
            vpush(&values, syntax_process_expression(ts, st));
            if (NextIs(TokenComma)) {
                Next();
            }
        }
        node = create_array(values);
        Next();
    }
    else if (NextIs(TokenDot)) {
        struct Vector names = vnew();
        struct Vector values = vnew();
        Next();

        ExpectNext(TokenBraceOpen, "{ expected in struct instance definition");
        while (true) {
            if (NextIs(TokenBraceClose)) break;
            Expect(TokenIdentifier, "Identifier expected in field of struct instance definition");
            vpush(&names, _strdup(Next().value_string));
            ExpectNext(TokenAssign, ":= expected in field of struct instance definition");
            vpush(&values, syntax_process_expression(ts, st));
            if (NextIs(TokenBraceClose)) break;
            Expect(TokenComma, ", or } expected after field of struct instance definition");
            Next();
        }
        node = create_struct_instance(names, values);
        Next();
    }
    else if (NextIs(TokenBackslash)) {
        Next();
        struct FunctionSignature *signature = syntax_process_function_signature(ts, st);
        struct Node *block = syntax_process_expression(ts, st);
        node = create_lambda_function(signature, block);
    }
    else if (NextIs(TokenDereference)) {
        Next();
        node = create_sizeof(syntax_process_type(ts, st));
    }
    else if (NextIs(TokenIdentifier) || NextIs(TokenAt)) {
        const char *identifier;
        bool address = false;
        if (NextIs(TokenIdentifier))
            identifier = _strdup(Get().value_string);
        else
            identifier = _strdup("@");
        Next();
        if (NextIs(TokenAmpersand)) {
            address = true;
            Next();
        }
        node = create_identifier(identifier, address);
    }
    else if (NextIs(TokenBraceOpen)) {
        node = syntax_process_block(ts, st);
    }
    else if (NextIs(TokenIf)) {
        Next();
        struct Vector condition_list = vnew();
        struct Vector block_list = vnew();
        struct Node *else_block = NULL;
        ExpectNext(TokenParenthesisOpen, "( expected in if condition");
        struct Node *_expression = syntax_process_expression(ts, st);
        ExpectNext(TokenParenthesisClose, ") expected in if condition");
        struct Node *_block = syntax_process_expression(ts, st);
        vpush(&condition_list, _expression);
        vpush(&block_list, _block);

        while (NextIs(TokenElse)) {
            Next();
            if (NextIs(TokenIf)) {
                Next();
                ExpectNext(TokenParenthesisOpen, "( expected in if condition");
                struct Node *_expression = syntax_process_expression(ts, st);
                ExpectNext(TokenParenthesisClose, ") expected in if condition");
                struct Node *_block = syntax_process_expression(ts, st);
                vpush(&condition_list, _expression);
                vpush(&block_list, _block);
            }
            else {
                else_block = syntax_process_expression(ts, st);
            }
        }

        node = create_if(condition_list, block_list, else_block);
    }
    else if (NextIs(TokenWhile)) {
        struct Node *else_block = NULL;
        const char *label = NULL;
        Next();
        if (NextIs(TokenDot)) {
            Next();
            Expect(TokenIdentifier, "Identifier expected in label");
            label = Get().value_string;
            Next();
        }
        ExpectNext(TokenParenthesisOpen, "( expected in while condition");
        struct Node *condition = syntax_process_expression(ts, st);
        ExpectNext(TokenParenthesisClose, ") expected in while condition");
        struct Node *block = syntax_process_block(ts, st);
        if (NextIs(TokenElse)) {
            Next();
            else_block = syntax_process_expression(ts, st);
        }

        node = create_while(condition, block, else_block, label);
    }
    else {
        error_syntax("Unexpected symbol in primary", Get());
    }
    annotate_node(start, node, Prev());

    while (true) {
        struct Token start = Get();
        struct Node *prv_node = node;
        if (NextIs(TokenDot)) {
            Next();
            Expect(TokenIdentifier, "Identifier expected in method call");
            const char *function = Get().value_string;
            Next();
            ExpectNext(TokenParenthesisOpen, "( expected in method call");
            struct Vector arguments = vnew();
            while (true) {
                if (NextIs(TokenParenthesisClose)) {
                    break;
                }
                vpush(&arguments, syntax_process_expression(ts, st));
                if (NextIs(TokenParenthesisClose)) {
                    break;
                }
                ExpectNext(TokenComma, ", expected in method call");
            }
            Next();
            node = create_method_call(prv_node, function, arguments);
        }
        else if (NextIs(TokenParenthesisOpen)) {
            Next();
            struct Vector arguments = vnew();
            struct Node *propagate_allocator = NULL;

            if (NextIs(TokenAt)) {
                Next();
                propagate_allocator = syntax_process_expression(ts, st);
                if (!NextIs(TokenParenthesisClose)) {
                    ExpectNext(TokenComma, ", expected in function call");
                }
            }
            while (true) {
                if (NextIs(TokenParenthesisClose)) {
                    break;
                }
                vpush(&arguments, syntax_process_expression(ts, st));
                if (NextIs(TokenParenthesisClose)) {
                    break;
                }
                ExpectNext(TokenComma, ", expected in function call");
            }
            Next();
            node = create_function_call(prv_node, arguments, propagate_allocator);
        }
        else if (NextIs(TokenGetField)) {
            Next();
            Expect(TokenIdentifier, "Identifier expected in get struct field expression");
            const char *field = _strdup(Get().value_string);
            Next();
            bool address = false;
            if (NextIs(TokenAmpersand)) {
                address = true;
                Next();
            }
            node = create_get_field(prv_node, field, address);
        }
        else if (NextIs(TokenBracketOpen)) {
            Next();
            struct Node *right = syntax_process_expression(ts, st);
            ExpectNext(TokenBracketClose, "] expected in index expression");
            bool address = false;
            if (NextIs(TokenAmpersand)) {
                address = true;
                Next();
            }
            node = create_index(prv_node, right, address);
        }
        else if (NextIs(TokenDereference)) {
            Next();
            node = create_dereference(prv_node);
        }
        else {
            break;
        }
        annotate_node(start, node, Prev());
    }

    return node;
}

struct FunctionSignature *syntax_process_function_signature(struct TokenStream *ts, struct Settings *st) {
    struct Vector identifiers = vnew();
    struct Vector types = vnew();
    ExpectNext(TokenParenthesisOpen, "( expected in function prototype");
    while (true) {
        if (NextIs(TokenParenthesisClose)) {
            Next();
            break;
        }
        Expect(TokenIdentifier, "Identifier expected in argument list");
        vpush(&identifiers, _strdup(Get().value_string));
        Next();
        vpush(&types, syntax_process_type(ts, st));
        if (NextIs(TokenParenthesisClose)) {
            Next();
            break;
        }
        ExpectNext(TokenComma, ", expected in argument list");
    }
    ExpectNext(TokenGetField, "-> expected in function definition");
    struct TypeNode *return_type = syntax_process_type(ts, st);
    return create_function_signature(identifiers, types, return_type);
}

struct Node *syntax_process_statement(struct TokenStream *ts, struct Settings *st, bool top_level) {
    if (NextIs(TokenSemicolon)) {
        Next();
        return NULL;
    }
    if (NextIs(TokenEval)) {
        if (top_level) {
            error_syntax("Unexpected eval statement in top-level declaration", Get());
        }
        Next();
        return syntax_process_expression(ts, st);
    }
    struct Token start = Get();
    struct Node *node;
    if (NextIs(TokenInclude)) {
        if (!top_level) {
            error_syntax("Include has to be a top-level declaration", Get());
        }
        Next();
        const char *include_path = NULL;
        if (NextIs(TokenIdentifier)) {
            const char *include_name = Get().value_string;
            for (int i = 0; i < vsize(&st->include_names); i++) {
                if (!_strcmp(include_name, st->include_names.ptr[i])) {
                    include_path = st->include_paths.ptr[i];
                    break;
                }
            }
            if (!include_path) {
                error_syntax("Include name was not declared", Get());
            }
            Next();
        }
        ExpectNext(TokenDot, ". expected in include");
        Expect(TokenString, "String literal expected in include");
        char *path = _concat(include_path, Get().value_string);
        bool include = true;

        const char *filename = _strrchr(path, '/');
        if (!filename) filename = path;
        else filename++;

        int sz = vsize(&st->included_files);
        for (int i = 0; i < sz; i++) {
            if (_strcmp(filename, st->included_files.ptr[i]) == 0) {
                include = false;
                break;
            }
        }
        if (include) {
            vpush(&st->included_files, (char*)filename);
            int fd = posix_open(path, 0, 0);
            if (fd <= 0) {
                const char *buffer = _concat("Could not open file ", path);
                error_syntax(buffer, Get());
            }
            posix_close(fd);
            struct Node *_node = process_parse(path, st);
            struct Module *module = (struct Module*)_node->node_ptr;
            node = create_include(module->statement_list);
            Next();
        }
        else {
            Next();
            return NULL;
        }
    }
    else if (NextIs(TokenTest)) {
        Next();
        Expect(TokenIdentifier, "Identifier expected in test");
        const char *name = _strdup(Get().value_string);
        Next();
        node = create_test(name, syntax_process_expression(ts, st));
    }
    else if (NextIs(TokenFunc)) {
        struct TypeNode *caller_type = NULL;
        bool external = false;
        bool global = top_level;
        Next();
        if (NextIs(TokenCaret)) {
            external = true;
            Next();
        }
        if (NextIs(TokenSharp)) {
            caller_type = syntax_process_type(ts, st);
        }
        ExpectNext(TokenDot, ". exprected in function definition");
        Expect(TokenIdentifier, "Identifier exprected in function definition");
        const char *name = _strdup(Get().value_string);
        Next();
        struct FunctionSignature *signature = syntax_process_function_signature(ts, st);
        struct Node *block = syntax_process_expression(ts, st);
        node = create_function_definition(caller_type, name, signature, block, external, global);
    }
    else if (NextIs(TokenProto)) {
        struct TypeNode *caller_type = NULL;
        Next();
        if (NextIs(TokenSharp)) {
            caller_type = syntax_process_type(ts, st);
        }
        ExpectNext(TokenDot, ". exprected in function prototype");
        Expect(TokenIdentifier, "Identifier exprected in function prototype");
        const char *name = _strdup(Get().value_string);
        Next();
        struct FunctionSignature *signature = syntax_process_function_signature(ts, st);
        node = create_prototype(caller_type, name, signature);
    }
    else if (NextIs(TokenDef)) {
        if (top_level) {
            Next();
            Expect(TokenIdentifier, "Identifier expected in definition statement");
            const char *identifier = _strdup(Get().value_string);
            Next();

            struct TypeNode *type = NULL;
            struct Node *value = NULL;
            if (NextIs(TokenSharp)) {
                type = syntax_process_type(ts, st);
            }
            if (NextIs(TokenAssign)) {
                Next();
                value = syntax_process_expression(ts, st);
            }
            node = create_global_definition(identifier, type, value);
        }
        else {
            Next();
            Expect(TokenIdentifier, "Identifier expected in definition statement");
            const char *identifier = _strdup(Get().value_string);
            Next();

            struct TypeNode *type = NULL;
            struct Node *value = NULL;
            if (NextIs(TokenSharp)) {
                type = syntax_process_type(ts, st);
            }
            if (NextIs(TokenAssign)) {
                Next();
                value = syntax_process_expression(ts, st);
            }
            node = create_definition(identifier, type, value);
        }
    }
    else if (NextIs(TokenTypedef)) {
        Next();
        Expect(TokenIdentifier, "Identifier expected in type definition statement");
        const char *identifier = _strdup(Get().value_string);
        Next();
        ExpectNext(TokenAssign, ":= expected in type definition statement");
        struct TypeNode *type = syntax_process_type(ts, st);
        node = create_type_definition(identifier, type);
    }
    else if (top_level) {
        error_syntax("Unexpected statement in top-level declaration", Get());
    }
    else if (NextIs(TokenReturn)) {
        Next();
        const char *label = NULL;
        if (NextIs(TokenDot)) {
            Next();
            Expect(TokenIdentifier, "Identifier expected in label");
            label = Get().value_string;
            Next();
        }
        struct Node *expression = syntax_process_expression(ts, st);
        node = create_return(expression, label);
    }
    else if (NextIs(TokenBreak)) {
        Next();
        const char *label = NULL;
        if (NextIs(TokenDot)) {
            Next();
            Expect(TokenIdentifier, "Identifier expected in label");
            label = Get().value_string;
            Next();
        }
        struct Node *expression = syntax_process_expression(ts, st);
        node = create_break(expression, label);
    }
    else if (NextIs(TokenContinue)) {
        Next();
        const char *label = NULL;
        if (NextIs(TokenDot)) {
            Next();
            Expect(TokenIdentifier, "Identifier expected in label");
            label = Get().value_string;
            Next();
        }
        node = create_continue(label);
    }
    else {
        struct Node *left = syntax_process_expression(ts, st);

        if (NextIs(TokenAssign)) {
            Next();
            struct Node *right = syntax_process_expression(ts, st);
            node = create_assignment(left, right);
        }
        else if (NextIs(TokenMove)) {
            Next();
            struct Node *right = syntax_process_expression(ts, st);
            node = create_movement(left, right);
        }
        else {
            error_syntax(":= or <- expected in assignment or movement statement", Get());
        }
    }

    return annotate_node(start, node, Prev());
}

struct Node *syntax_process(struct TokenStream *token_stream, struct Settings *st) {
    token_stream->pos = 0;
    return syntax_process_module(token_stream, st);
}
