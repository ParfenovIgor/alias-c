#include <syntax.h>
#include <exception.h>
#include <process.h>
#include <vector.h>
#include <posix.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct Node                 *syntax_process_block               (struct TokenStream *ts, struct Settings *st, bool braces);
struct TypeNode             *syntax_process_type                (struct TokenStream *ts, struct Settings *st) ;
struct Node                 *syntax_process_expression          (struct TokenStream *ts, struct Settings *st);
struct Node                 *syntax_process_primary             (struct TokenStream *ts, struct Settings *st);
struct FunctionSignature    *syntax_process_function_signature  (struct TokenStream *ts, struct Settings *st);
struct Node                 *syntax_process_statement           (struct TokenStream *ts, struct Settings *st);

struct Node *init_node(struct TokenStream *ts) {
    struct Node *node = (struct Node*)_malloc(sizeof(struct Node));
    node->line_begin = tokenstream_get(ts).line_begin;
    node->position_begin = tokenstream_get(ts).position_begin;
    node->filename = _strdup(tokenstream_get(ts).filename);
    return node;
}

void finish_node(struct Node *node, struct TokenStream *ts) {
    node->line_end = tokenstream_get_prev(ts).line_end;
    node->position_end = tokenstream_get_prev(ts).position_end;
}

void check_next(struct TokenStream *ts, enum TokenType type, const char *error) {
    if (tokenstream_get(ts).type != type) {
        error_syntax(error, tokenstream_get(ts));
    }
}

void pass_next(struct TokenStream *ts, enum TokenType type, const char *error) {
    if (tokenstream_get(ts).type != type) {
        error_syntax(error, tokenstream_get(ts));
    }
    tokenstream_next(ts);
}

struct Node *syntax_process_block(struct TokenStream *ts, struct Settings *st, bool braces) {
    struct Node *node = init_node(ts);
    struct Block *this = (struct Block*)_malloc(sizeof(struct Block));
    node->node_ptr = this;
    this->statement_list = vnew();
    node->node_type = NodeBlock;

    bool one_statement = false;
    if (braces) {
        if (tokenstream_get(ts).type == TokenBraceOpen) {
            tokenstream_next(ts);
        }
        else {
            one_statement = true;
        }
    }

    if (tokenstream_get(ts).type == TokenDot) {
        tokenstream_next(ts);
        check_next(ts, TokenIdentifier, "Identifier expected in label");
        this->label = tokenstream_get(ts).value_string;
        tokenstream_next(ts);
    }
    else {
        this->label = NULL;
    }

    while (tokenstream_get(ts).type != TokenEof && tokenstream_get(ts).type != TokenBraceClose) {
        if (tokenstream_get(ts).type == TokenSemicolon) {
            tokenstream_next(ts);
            continue;
        }
        vpush(&this->statement_list, syntax_process_statement(ts, st));
        if (one_statement) break;
    }
    if (braces && !one_statement) {
        if (tokenstream_get(ts).type != TokenBraceClose) {
            error_syntax("} expected after block", tokenstream_get(ts));
        }
        tokenstream_next(ts);
    }
    finish_node(node, ts);
    return node;
}

struct TypeNode *syntax_process_type(struct TokenStream *ts, struct Settings *st) {
    struct TypeNode *node = (struct TypeNode*)_malloc(sizeof(struct TypeNode));
    node->degree = 0;
    pass_next(ts, TokenSharp, "# expected in type definition");
    if (tokenstream_get(ts).type == TokenInteger) {
        node->degree = tokenstream_get(ts).value_int;
        tokenstream_next(ts);
    }
    check_next(ts, TokenIdentifier, "Identifier expected in type definition");
    const char *typestr = tokenstream_get(ts).value_string;
    tokenstream_next(ts);
        
    if (!_strcmp(typestr, "V")) {
        struct TypeVoid *this = (struct TypeVoid*)_malloc(sizeof(struct TypeVoid));
        node->node_type = TypeNodeVoid;
        node->node_ptr = this;
    }
    else if (!_strcmp(typestr, "I")) {
        struct TypeInt *this = (struct TypeInt*)_malloc(sizeof(struct TypeInt));
        node->node_type = TypeNodeInt;
        node->node_ptr = this;
    }
    else if (!_strcmp(typestr, "C")) {
        struct TypeChar *this = (struct TypeChar*)_malloc(sizeof(struct TypeChar));
        node->node_type = TypeNodeChar;
        node->node_ptr = this;
    }
    else if (!_strcmp(typestr, "S")) {
        struct TypeStruct *this = (struct TypeStruct*)_malloc(sizeof(struct TypeStruct));
        node->node_type = TypeNodeStruct;
        node->node_ptr = this;
        this->names = vnew();
        this->types = vnew();
        pass_next(ts, TokenBraceOpen, "{ expected in struct type definition");
        while (true) {
            if (tokenstream_get(ts).type == TokenBraceClose) break;
            check_next(ts, TokenIdentifier, "Identifier expected in field of struct type definition");
            vpush(&this->names, _strdup(tokenstream_get(ts).value_string));
            tokenstream_next(ts);
            pass_next(ts, TokenColon, ": expected in field of struct type definition");
            vpush(&this->types, syntax_process_type(ts, st));
            if (tokenstream_get(ts).type == TokenBraceClose) break;
            pass_next(ts, TokenComma, ", or } expected after field of struct type definition");
        }
        tokenstream_next(ts);
    }        
    else if (!_strcmp(typestr, "F")) {
        struct TypeFunction *this = (struct TypeFunction*)_malloc(sizeof(struct TypeFunction));
        node->node_type = TypeNodeFunction;
        node->node_ptr = this;
        this->types = vnew();
        pass_next(ts, TokenParenthesisOpen, "( expected in function type definition");
        while (tokenstream_get(ts).type != TokenParenthesisClose) {
            vpush(&this->types, syntax_process_type(ts, st));
        }
        pass_next(ts, TokenParenthesisClose, ") expected in function type definition");
        pass_next(ts, TokenGetField, "-> expected in function type definition");
        this->return_type = syntax_process_type(ts, st);
    }
    else {
        struct TypeIdentifier *this = (struct TypeIdentifier*)_malloc(sizeof(struct TypeIdentifier));
        node->node_type = TypeNodeIdentifier;
        node->node_ptr = this;
        this->identifier = _strdup(typestr);
    }

    return node;
}

bool next_is_operation(struct TokenStream *ts) {
    return (
        tokenstream_get(ts).type == TokenPlus  ||
        tokenstream_get(ts).type == TokenMinus ||
        tokenstream_get(ts).type == TokenMult  ||
        tokenstream_get(ts).type == TokenDiv   ||
        tokenstream_get(ts).type == TokenLess  ||
        tokenstream_get(ts).type == TokenEqual);
}

int operation_priority(enum TokenType *operation) {
    if (*operation == TokenMult ||
        *operation == TokenDiv) {
        return 2;
    }
    if (*operation == TokenPlus ||
        *operation == TokenMinus) {
        return 3;
    }
    if (*operation == TokenLess ||
        *operation == TokenEqual) {
        return 4;
    }
    return 5;
}

struct Node *process_operation(struct Vector *primaries, struct Vector *operations, struct TokenStream *ts) {
    struct Node *root = (struct Node*)_malloc(sizeof(struct Node));
    root->node_ptr = NULL;
    enum TokenType *token = vback(operations);
    int sz = vsize(primaries);
    if (sz < 2) {
        error_syntax("Incorrect structure of the expression", tokenstream_get(ts));
    }
    struct Node *left = primaries->ptr[sz - 2];
    struct Node *right = primaries->ptr[sz - 1];
    root->line_begin = left->line_begin;
    root->position_begin = left->position_begin;
    root->line_end = right->line_end;
    root->position_end = right->position_end;
    root->filename = _strdup(left->filename);

    struct BinaryOperator *this = (struct BinaryOperator*)_malloc(sizeof(struct BinaryOperator));
    this->left = left;
    this->right = right;
    root->node_ptr = this;

    if (*token == TokenPlus)  root->node_type = NodeAddition;
    if (*token == TokenMinus) root->node_type = NodeSubtraction;
    if (*token == TokenMult)  root->node_type = NodeMultiplication;
    if (*token == TokenDiv)   root->node_type = NodeDivision;
    if (*token == TokenLess)  root->node_type = NodeLess;
    if (*token == TokenEqual) root->node_type = NodeEqual;

    if (root->node_ptr == NULL) {
        error_syntax("Fatal Error", tokenstream_get(ts));
    }

    vpop(primaries);
    vpop(primaries);
    vpush(primaries, root);
    _free(token);
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
    if (tokenstream_get(ts).type == TokenParenthesisOpen) {
        struct Token token = tokenstream_get(ts);
        enum TokenType *token_type = (enum TokenType*)_malloc(sizeof(enum TokenType));
        *token_type = token.type;
        vpush(&operations, token_type);
        tokenstream_next(ts);
        ParenthesisLevel++;
        CurrentState = State_ParenthesisOpen;
    }
    else if (next_is_operation(ts)) {
        struct Token token = tokenstream_get(ts);
        enum TokenType *token_type = (enum TokenType*)_malloc(sizeof(enum TokenType));
        *token_type = token.type;
        vpush(&operations, token_type);
        tokenstream_next(ts);
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
        
        if (tokenstream_get(ts).type == TokenParenthesisOpen) {
            if (CurrentState != State_UnaryOperation &&
                CurrentState != State_BinaryOperation &&
                CurrentState != State_ParenthesisOpen) {
                error_syntax("Unexpected ( in expression", tokenstream_get(ts));
            }
            struct Token token = tokenstream_get(ts);
            enum TokenType *token_type = (enum TokenType*)_malloc(sizeof(enum TokenType));
            *token_type = token.type;
            vpush(&operations, token_type);
            tokenstream_next(ts);
            ParenthesisLevel++;
            CurrentState = State_ParenthesisOpen;
        }
        else if (tokenstream_get(ts).type == TokenParenthesisClose) {
            if (CurrentState != State_Identifier &&
                CurrentState != State_ParenthesisOpen) {
                error_syntax("Unexpected ) in expression", tokenstream_get(ts));
            }
            while (vsize(&operations) != NULL && *((enum TokenType*)vback(&operations)) != TokenParenthesisOpen) {
                process_operation(&primaries, &operations, ts);
            }
            if (vsize(&operations) == NULL || *((enum TokenType*)vback(&operations)) != TokenParenthesisOpen) {
                error_syntax("Unexpected ) in expression", tokenstream_get(ts));
            }
            _free(vback(&operations));
            vpop(&operations);
            tokenstream_next(ts);
            ParenthesisLevel--;
            CurrentState = State_ParenthesisClose;
        }
        else if (next_is_operation(ts)) {
            if (CurrentState == State_UnaryOperation ||
                CurrentState == State_BinaryOperation ||
                CurrentState == State_ParenthesisOpen) {
                error_syntax("Unexpected operator in expression", tokenstream_get(ts));
            }
            else {
                struct Token token = tokenstream_get(ts);
                while (vsize(&operations) &&
                    operation_priority((enum TokenType*)vback(&operations)) <= 
                    operation_priority((enum TokenType*)&token.type)) {
                    process_operation(&primaries, &operations, ts);
                }
                enum TokenType *token_type = (enum TokenType*)_malloc(sizeof(enum TokenType));
                *token_type = token.type;
                vpush(&operations, token_type);
                tokenstream_next(ts);
                CurrentState = State_BinaryOperation;
            }
        }
        else {
            if (CurrentState != State_UnaryOperation &&
                CurrentState != State_BinaryOperation &&
                CurrentState != State_ParenthesisOpen) {
                error_syntax("Unexpected identifier in expression", tokenstream_get(ts));
            }
            vpush(&primaries, syntax_process_primary(ts, st));
            CurrentState = State_Identifier;
        }
    }

    while (vsize(&operations)) {
        process_operation(&primaries, &operations, ts);
    }

    if (vsize(&primaries) != 1) {
        error_syntax("Incorrect expression", tokenstream_get(ts));
    }

    struct Node *res = primaries.ptr[0];
    vdrop(&primaries);
    vdrop(&operations);

    if (tokenstream_get(ts).type == TokenAs) {
        struct Node *node = (struct Node*)_malloc(sizeof(struct Node));
        struct As *this = (struct As*)_malloc(sizeof(struct As));
        node->node_ptr = this;
        node->node_type = NodeAs;
        node->line_begin = res->line_begin;
        node->position_begin = res->position_begin;
        node->filename = _strdup(res->filename);
        this->expression = res;
        tokenstream_next(ts);
        this->type = syntax_process_type(ts, st);
        node->line_end = tokenstream_get(ts).line_end;
        node->position_end = tokenstream_get(ts).position_end;
        
        return node;
    }
    return res;
}

struct Node *syntax_process_primary(struct TokenStream *ts, struct Settings *st) {
    struct Node *node = init_node(ts);

    if (tokenstream_get(ts).type == TokenInteger) {
        struct Integer *this = (struct Integer*)_malloc(sizeof(struct Integer));
        node->node_ptr = this;
        node->node_type = NodeInteger;
        this->value = tokenstream_get(ts).value_int;
        tokenstream_next(ts);
    }
    else if (tokenstream_get(ts).type == TokenChar) {
        struct Char *this = (struct Char*)_malloc(sizeof(struct Char));
        node->node_ptr = this;
        node->node_type = NodeChar;
        this->value = tokenstream_get(ts).value_int;
        tokenstream_next(ts);
    }
    else if (tokenstream_get(ts).type == TokenString) {
        struct String *this = (struct String*)_malloc(sizeof(struct String));
        node->node_ptr = this;
        node->node_type = NodeString;
        this->value = _strdup(tokenstream_get(ts).value_string);
        tokenstream_next(ts);
    }
    else if (tokenstream_get(ts).type == TokenBracketOpen) {
        struct Array *this = (struct Array*)_malloc(sizeof(struct Array));
        node->node_ptr = this;
        node->node_type = NodeArray;
        this->values = vnew();
        tokenstream_next(ts);
        while (tokenstream_get(ts).type != TokenBracketClose) {
            vpush(&this->values, syntax_process_expression(ts, st));
            if (tokenstream_get(ts).type == TokenComma) {
                tokenstream_next(ts);
            }
        }
        tokenstream_next(ts);
    }
    else if (tokenstream_get(ts).type == TokenDot) {
        struct StructInstance *this = (struct StructInstance*)_malloc(sizeof(struct StructInstance));
        node->node_type = NodeStructInstance;
        node->node_ptr = this;
        this->names = vnew();
        this->values = vnew();
        tokenstream_next(ts);
        pass_next(ts, TokenBraceOpen, "{ expected in struct instance definition");
        while (true) {
            if (tokenstream_get(ts).type == TokenBraceClose) break;
            check_next(ts, TokenIdentifier, "Identifier expected in field of struct instance definition");
            vpush(&this->names, _strdup(tokenstream_get(ts).value_string));
            tokenstream_next(ts);
            pass_next(ts, TokenAssign, ":= expected in field of struct instance definition");
            vpush(&this->values, syntax_process_expression(ts, st));
            if (tokenstream_get(ts).type == TokenBraceClose) break;
            pass_next(ts, TokenComma, ", or } expected after field of struct instance definition");
        }
        tokenstream_next(ts);
    }
    else if (tokenstream_get(ts).type == TokenBackslash) {
        struct LambdaFunction *this = (struct LambdaFunction*)_malloc(sizeof(struct LambdaFunction));
        node->node_type = NodeLambdaFunction;
        node->node_ptr = this;
        tokenstream_next(ts);
        pass_next(ts, TokenParenthesisOpen, "( expected in function definition");
        this->signature = syntax_process_function_signature(ts, st);
        this->block = syntax_process_block(ts, st, true);
    } 
    else if (tokenstream_get(ts).type == TokenCaret) {
        struct Sizeof *this = (struct Sizeof*)_malloc(sizeof(struct Sizeof));
        node->node_ptr = this;
        node->node_type = NodeSizeof;
        tokenstream_next(ts);
        this->type = syntax_process_type(ts, st);
    }
    else if (tokenstream_get(ts).type == TokenIdentifier) {
        struct Identifier *this = (struct Identifier*)_malloc(sizeof(struct Identifier));
        this->identifier = _strdup(tokenstream_get(ts).value_string);
        node->node_ptr = this;
        node->node_type = NodeIdentifier;
        tokenstream_next(ts);
        if (tokenstream_get(ts).type == TokenAddress) {
            this->address = true;
            tokenstream_next(ts);
        }
        else {
            this->address = false;
        }
    }
    else if (tokenstream_get(ts).type == TokenBraceOpen) {
        node = syntax_process_block(ts, st, true);
    }
    else {
        error_syntax("Unexpected symbol in primary", tokenstream_get(ts));
    }

    while (true) {
        if (tokenstream_get(ts).type == TokenParenthesisOpen) {
            struct Node *prv_node = node;
            node = (struct Node*)_malloc(sizeof(struct Node));
            node->line_begin = prv_node->line_begin;
            node->position_begin = prv_node->position_begin;
            node->filename = _strdup(prv_node->filename);
            tokenstream_next(ts);

            struct FunctionCall *this = (struct FunctionCall*)_malloc(sizeof(struct FunctionCall));
            node->node_ptr = this;
            node->node_type = NodeFunctionCall;
            this->function = prv_node;
            this->arguments = vnew();
            while (true) {
                if (tokenstream_get(ts).type == TokenParenthesisClose) {
                    break;
                }
                vpush(&this->arguments, syntax_process_expression(ts, st));
                if (tokenstream_get(ts).type == TokenParenthesisClose) {
                    break;
                }
                pass_next(ts, TokenComma, ", expected in function call");
            }
            tokenstream_next(ts);
            finish_node(node, ts);
        }
        else if (tokenstream_get(ts).type == TokenGetField) {
            struct Node *prv_node = node;
            node = (struct Node*)_malloc(sizeof(struct Node));
            node->line_begin = prv_node->line_begin;
            node->position_begin = prv_node->position_begin;
            node->filename = _strdup(prv_node->filename);

            tokenstream_next(ts);

            struct GetField *this = (struct GetField*)_malloc(sizeof(struct GetField));
            node->node_ptr = this;
            node->node_type = NodeGetField;
            this->left = prv_node;
            check_next(ts, TokenIdentifier, "Identifier expected in get struct field expression");
            this->field = _strdup(tokenstream_get(ts).value_string);
            tokenstream_next(ts);
            if (tokenstream_get(ts).type == TokenAddress) {
                this->address = true;
                tokenstream_next(ts);
            }
            else {
                this->address = false;
            }
            finish_node(node, ts);
        }
        else if (tokenstream_get(ts).type == TokenBracketOpen) {
            struct Node *prv_node = node;
            node = (struct Node*)_malloc(sizeof(struct Node));
            node->line_begin = prv_node->line_begin;
            node->position_begin = prv_node->position_begin;
            node->filename = _strdup(prv_node->filename);

            tokenstream_next(ts);

            struct Index *this = (struct Index*)_malloc(sizeof(struct Index));
            node->node_ptr = this;
            node->node_type = NodeIndex;
            this->left = prv_node;
            this->right = syntax_process_expression(ts, st);
            pass_next(ts, TokenBracketClose, "] expected in index expression");
            if (tokenstream_get(ts).type == TokenAddress) {
                this->address = true;
                tokenstream_next(ts);
            }
            else {
                this->address = false;
            }
            finish_node(node, ts);
        }
        else if (tokenstream_get(ts).type == TokenDereference) {
            struct Node *prv_node = node;
            node = (struct Node*)_malloc(sizeof(struct Node));
            node->line_begin = prv_node->line_begin;
            node->position_begin = prv_node->position_begin;
            node->filename = _strdup(prv_node->filename);

            struct Dereference *this = (struct Dereference*)_malloc(sizeof(struct Dereference));
            node->node_ptr = this;
            node->node_type = NodeDereference;
            this->expression = prv_node;
            tokenstream_next(ts);
            finish_node(node, ts);
        }
        else break;
    }

    finish_node(node, ts);
    return node;
}

struct FunctionSignature *syntax_process_function_signature(struct TokenStream *ts, struct Settings *st) {
    struct FunctionSignature *this = (struct FunctionSignature*)_malloc(sizeof(struct FunctionSignature));
    this->identifiers = vnew();
    this->types = vnew();

    while (true) {
        if (tokenstream_get(ts).type == TokenParenthesisClose) {
            tokenstream_next(ts);
            break;
        }
        check_next(ts, TokenIdentifier, "Identifier expected in argument list");
        vpush(&this->identifiers, _strdup(tokenstream_get(ts).value_string));
        tokenstream_next(ts);
        vpush(&this->types, syntax_process_type(ts, st));
        bool *is_const = (bool*)_malloc(sizeof(bool));
        if (tokenstream_get(ts).type == TokenParenthesisClose) {
            tokenstream_next(ts);
            break;
        }
        pass_next(ts, TokenComma, ", expected in argument list");
    }
    pass_next(ts, TokenGetField, "-> expected in function definition");
    this->return_type = syntax_process_type(ts, st);
    return this;
}

struct Node *syntax_process_statement(struct TokenStream *ts, struct Settings *st) {
    if (tokenstream_get(ts).type == TokenBraceOpen) {
        struct Node *node = syntax_process_block(ts, st, true);
        return node;
    }
    struct Node *node = init_node(ts);
    if (tokenstream_get(ts).type == TokenInclude) {
        struct Include *this = (struct Include*)_malloc(sizeof(struct Include));
        node->node_ptr = this;
        node->node_type = NodeInclude;

        tokenstream_next(ts);
        const char *include_path = "";
        if (tokenstream_get(ts).type == TokenIdentifier) {
            const char *include_name = tokenstream_get(ts).value_string;
            for (int i = 0; i < vsize(&st->include_names); i++) {
                if (!_strcmp(include_name, st->include_names.ptr[i])) {
                    include_path = st->include_paths.ptr[i];
                    break;
                }
            }
            if (_strlen(include_path) == 0) {
                error_syntax("Include name was not declared", tokenstream_get(ts));
            }
            tokenstream_next(ts);
        }
        pass_next(ts, TokenDot, ". expected in include");
        check_next(ts, TokenString, "String literal expected in include");
        char *filename = concat(include_path, tokenstream_get(ts).value_string);
        tokenstream_next(ts);
        int fd = posix_open(filename, 0, 0);
        if (fd <= 0) {
            const char *buffer = concat("Could not open file ", filename);
            error_syntax(buffer, tokenstream_get(ts));
        }
        else {
            posix_close(fd);
        }
        struct Node *_node = process_parse(filename, st);
        struct Block *inc_block = (struct Block*)_node->node_ptr;
        this->statement_list = inc_block->statement_list;
        _free(filename);
        _free(inc_block);
        _free(_node);
    }
    else if (tokenstream_get(ts).type == TokenTest) {
        struct Test *this = (struct Test*)_malloc(sizeof(struct Test));
        node->node_ptr = this;
        node->node_type = NodeTest;
        tokenstream_next(ts);
        check_next(ts, TokenIdentifier, "Identifier expected in test");
        this->name = _strdup(tokenstream_get(ts).value_string);
        tokenstream_next(ts);
        this->block = syntax_process_block(ts, st, true);
    }
    else if (tokenstream_get(ts).type == TokenIf) {
        struct If *this = (struct If*)_malloc(sizeof(struct If));
        this->condition_list = vnew();
        this->block_list = vnew();
        this->else_block = NULL;
        node->node_ptr = this;
        node->node_type = NodeIf;
        tokenstream_next(ts);
        pass_next(ts, TokenParenthesisOpen, "( expected in if condition");
        struct Node *_expression = syntax_process_expression(ts, st);
        pass_next(ts, TokenParenthesisClose, ") expected in if condition");
        struct Node *_block = syntax_process_block(ts, st, true);
        vpush(&this->condition_list, _expression);
        vpush(&this->block_list, _block);

        while (tokenstream_get(ts).type == TokenElse) {
            tokenstream_next(ts);

            if (tokenstream_get(ts).type == TokenIf) {
                tokenstream_next(ts);
                pass_next(ts, TokenParenthesisOpen, "( expected in if condition");
                struct Node *_expression = syntax_process_expression(ts, st);
                pass_next(ts, TokenParenthesisClose, ") expected in if condition");
                struct Node *_block = syntax_process_block(ts, st, true);
                vpush(&this->condition_list, _expression);
                vpush(&this->block_list, _block);
            }
            else {
                struct Node *_block = syntax_process_block(ts, st, true);
                this->else_block = _block;
            }
        }
    }
    else if (tokenstream_get(ts).type == TokenWhile) {
        struct While *this = (struct While*)_malloc(sizeof(struct While));
        node->node_ptr = this;
        node->node_type = NodeWhile;
        tokenstream_next(ts);
        if (tokenstream_get(ts).type == TokenDot) {
            tokenstream_next(ts);
            check_next(ts, TokenIdentifier, "Identifier expected in label");
            this->label = tokenstream_get(ts).value_string;
            tokenstream_next(ts);
        }
        else {
            this->label = NULL;
        }
        pass_next(ts, TokenParenthesisOpen, "( expected in while condition");
        struct Node *_expression = syntax_process_expression(ts, st);
        pass_next(ts, TokenParenthesisClose, ") expected in while condition");
        struct Node *_block = syntax_process_block(ts, st, true);
        this->condition = _expression;
        this->block = _block;
    }
    else if (tokenstream_get(ts).type == TokenFunc) {
        struct FunctionDefinition *this = (struct FunctionDefinition*)_malloc(sizeof(struct FunctionDefinition));
        node->node_ptr = this;
        node->node_type = NodeFunctionDefinition;
        this->external = false;
        tokenstream_next(ts);
        if (tokenstream_get(ts).type == TokenCaret) {
            this->external = true;
            tokenstream_next(ts);
        }
        if (tokenstream_get(ts).type == TokenIdentifier) {
            this->struct_name = _strdup(tokenstream_get(ts).value_string);
            tokenstream_next(ts);
        }
        else {
            this->struct_name = NULL;
        }
        pass_next(ts, TokenDot, ". exprected in function definition");
        check_next(ts, TokenIdentifier, "Identifier exprected in function definition");
        this->name = _strdup(tokenstream_get(ts).value_string);
        tokenstream_next(ts);
        pass_next(ts, TokenParenthesisOpen, "( expected in function definition");
        this->signature = syntax_process_function_signature(ts, st);
        this->block = syntax_process_block(ts, st, true);
    }
    else if (tokenstream_get(ts).type == TokenProto) {
        struct Prototype *this = (struct Prototype*)_malloc(sizeof(struct Prototype));
        node->node_ptr = this;
        node->node_type = NodePrototype;
        tokenstream_next(ts);
        if (tokenstream_get(ts).type == TokenIdentifier) {
            this->struct_name = _strdup(tokenstream_get(ts).value_string);
            tokenstream_next(ts);
        }
        else {
            this->struct_name = NULL;
        }
        pass_next(ts, TokenDot, ". exprected in function prototype");
        check_next(ts, TokenIdentifier, "Identifier exprected in function prototype");
        this->name = _strdup(tokenstream_get(ts).value_string);
        tokenstream_next(ts);
        pass_next(ts, TokenParenthesisOpen, "( expected in function prototype");
        this->signature = syntax_process_function_signature(ts, st);
    }
    else if (tokenstream_get(ts).type == TokenDef) {
        struct Definition *this = (struct Definition*)_malloc(sizeof(struct Definition));
        node->node_ptr = this;
        node->node_type = NodeDefinition;
        tokenstream_next(ts);
        check_next(ts, TokenIdentifier, "Identifier expected in definition statement");
        this->identifier = _strdup(tokenstream_get(ts).value_string);
        tokenstream_next(ts);
        if (tokenstream_get(ts).type == TokenSharp) {
            this->type = syntax_process_type(ts, st);
        }
        else {
            this->type = NULL;
        }
        pass_next(ts, TokenAssign, ":= expected in definition statement");
        this->value = syntax_process_expression(ts, st);
    }
    else if (tokenstream_get(ts).type == TokenTypedef) {
        struct TypeDefinition *this = (struct TypeDefinition*)_malloc(sizeof(struct TypeDefinition));
        node->node_ptr = this;
        node->node_type = NodeTypeDefinition;
        tokenstream_next(ts);
        check_next(ts, TokenIdentifier, "Identifier expected in type definition statement");
        this->identifier = _strdup(tokenstream_get(ts).value_string);
        tokenstream_next(ts);
        pass_next(ts, TokenAssign, ":= expected in type definition statement");
        this->type = syntax_process_type(ts, st);
    }
    else if (tokenstream_get(ts).type == TokenReturn) {
        struct Return *this = (struct Return*)_malloc(sizeof(struct Return));
        node->node_ptr = this;
        node->node_type = NodeReturn;
        tokenstream_next(ts);
        this->expression = syntax_process_expression(ts, st);
    }
    else if (tokenstream_get(ts).type == TokenBreak) {
        struct Break *this = (struct Break*)_malloc(sizeof(struct Break));
        node->node_ptr = this;
        node->node_type = NodeBreak;
        tokenstream_next(ts);
        if (tokenstream_get(ts).type == TokenDot) {
            tokenstream_next(ts);
            check_next(ts, TokenIdentifier, "Identifier expected in label");
            this->label = tokenstream_get(ts).value_string;
            tokenstream_next(ts);
        }
        else {
            this->label = NULL;
        }
        this->expression = syntax_process_expression(ts, st);
    }
    else {
        struct Node *left = syntax_process_expression(ts, st);

        if (tokenstream_get(ts).type == TokenAssign) {
            tokenstream_next(ts);
            struct Assignment *this = (struct Assignment*)_malloc(sizeof(struct Assignment));
            node->node_ptr = this;
            node->node_type = NodeAssignment;
            struct Node *right = syntax_process_expression(ts, st);
            this->dst = left;
            this->src = right;
        }
        else if (tokenstream_get(ts).type == TokenMove) {
            tokenstream_next(ts);
            struct Movement *this = (struct Movement*)_malloc(sizeof(struct Movement));
            node->node_ptr = this;
            node->node_type = NodeMovement;
            struct Node *right = syntax_process_expression(ts, st);
            this->dst = left;
            this->src = right;
        }
        else {
            error_syntax(":= or <- expected in assignment or movement statement", tokenstream_get(ts));
        }
    }

    finish_node(node, ts);
    return node;
}

struct Node *syntax_process(struct TokenStream *token_stream, struct Settings *st) {
    token_stream->pos = 0;
    return syntax_process_block(token_stream, st, false);
}
