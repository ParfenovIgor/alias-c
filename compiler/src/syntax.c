#include <syntax.h>
#include <exception.h>
#include <process.h>
#include <vector.h>
#include <posix.h>
#include <stdlib.h>
#include <string.h>

struct Node *Syntax_ProcessBlock(struct TokenStream *ts, bool braces) {
    struct Node *node = (struct Node*)_malloc(sizeof(struct Node));
    struct Block *block = (struct Block*)_malloc(sizeof(struct Block));
    node->node_ptr = block;
    block->statement_list = (struct Node**)_malloc(sizeof(struct Node*));
    block->statement_list[0] = NULL;
    node->node_type = NodeBlock;
    node->line_begin = TokenStream_GetToken(ts).line_begin;
    node->position_begin = TokenStream_GetToken(ts).position_begin;
    node->filename = _strdup(TokenStream_GetToken(ts).filename);
    if (braces) {
        TokenStream_NextToken(ts);
    }
    while (TokenStream_GetToken(ts).type != TokenEof && TokenStream_GetToken(ts).type != TokenBraceClose) {
        if (TokenStream_GetToken(ts).type == TokenInclude) {
            const char *filename = TokenStream_GetToken(ts).value_string;
            int fd = posix_open(filename, 0, 0);
            if (fd <= 0) {
                const char *buffer = concat("Could not open file ", filename);
                SyntaxError(buffer, TokenStream_GetToken(ts));
            }
            else {
                posix_close(fd);
            }
            struct Node *_node = Parse(filename);
            struct Block *inc_block = (struct Block*)_node->node_ptr;
            struct Node **ptr = inc_block->statement_list;
            while (*ptr != NULL) {
                block->statement_list = (struct Node**)push_back((void**)block->statement_list, *ptr);
                ptr++;
            }
            _free(inc_block->statement_list);
            TokenStream_NextToken(ts);
            continue;
        }
        struct Node *_statement = Syntax_ProcessStatement(ts);
        if (_statement) {
            block->statement_list = (struct Node**)push_back((void**)block->statement_list, _statement);
        }
    }
    if (braces && TokenStream_GetToken(ts).type != TokenBraceClose) {
        SyntaxError("} expected after block", TokenStream_GetToken(ts));
    }
    node->line_end = TokenStream_GetToken(ts).line_end;
    node->position_end = TokenStream_GetToken(ts).position_end;
    return node;
}

struct Type *Syntax_ProcessType(struct TokenStream *ts) {
    struct Type *type = (struct Type*)_malloc(sizeof(struct Type));
    if (TokenStream_GetToken(ts).type != TokenLess) {
        SyntaxError("< expected in type", TokenStream_GetToken(ts));
    }
    TokenStream_NextToken(ts);
    if (TokenStream_GetToken(ts).type != TokenIdentifier) {
        SyntaxError("Identifier expected in type", TokenStream_GetToken(ts));
    }
    type->identifier = _strdup(TokenStream_GetToken(ts).value_string);
    TokenStream_NextToken(ts);
    if (TokenStream_GetToken(ts).type != TokenComma) {
        SyntaxError(", expected in type", TokenStream_GetToken(ts));
    }
    TokenStream_NextToken(ts);
    if (TokenStream_GetToken(ts).type != TokenInteger) {
        SyntaxError("Integer expected in type", TokenStream_GetToken(ts));
    }
    type->degree = TokenStream_GetToken(ts).value_int;
    TokenStream_NextToken(ts);
    if (TokenStream_GetToken(ts).type != TokenGreater) {
        SyntaxError("> expected in type", TokenStream_GetToken(ts));
    }
    return type;
}

bool next_is_operation(struct TokenStream *ts) {
    return (
        TokenStream_GetToken(ts).type == TokenPlus  ||
        TokenStream_GetToken(ts).type == TokenMinus ||
        TokenStream_GetToken(ts).type == TokenMult  ||
        TokenStream_GetToken(ts).type == TokenDiv   ||
        TokenStream_GetToken(ts).type == TokenLess  ||
        TokenStream_GetToken(ts).type == TokenEqual);
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

struct Node *process_operation(struct Node ***primaries, enum TokenType ***operations, struct TokenStream *ts) {
    struct Node *root = (struct Node*)_malloc(sizeof(struct Node));
    root->node_ptr = NULL;
    enum TokenType *token = (enum TokenType*)get_back((void**)*operations);
    int sz = get_size((void**)*primaries);
    if (sz < 2) {
        SyntaxError("Incorrect structure of the expression", TokenStream_GetToken(ts));
    }
    struct Node *left = (*primaries)[sz - 2];
    struct Node *right = (*primaries)[sz - 1];
    root->line_begin = left->line_begin;
    root->position_begin = left->position_begin;
    root->line_end = right->line_end;
    root->position_end = right->position_end;
    root->filename = _strdup(left->filename);

    if (*token == TokenPlus) {
        struct Addition *addition = (struct Addition*)_malloc(sizeof(struct Addition));
        addition->left = left;
        addition->right = right;
        root->node_ptr = addition;
        root->node_type = NodeAddition;
    }
    if (*token == TokenMinus) {
        struct Subtraction *subtraction = (struct Subtraction*)_malloc(sizeof(struct Subtraction));
        subtraction->left = left;
        subtraction->right = right;
        root->node_ptr = subtraction;
        root->node_type = NodeSubtraction;
    }
    if (*token == TokenMult) {
        struct Multiplication *multiplication = (struct Multiplication*)_malloc(sizeof(struct Multiplication));
        multiplication->left = left;
        multiplication->right = right;
        root->node_ptr = multiplication;
        root->node_type = NodeMultiplication;
    }
    if (*token == TokenDiv) {
        struct Division *division = (struct Division*)_malloc(sizeof(struct Division));
        division->left = left;
        division->right = right;
        root->node_ptr = division;
        root->node_type = NodeDivision;
    }
    if (*token == TokenLess) {
        struct Less *less = (struct Less*)_malloc(sizeof(struct Less));
        less->left = left;
        less->right = right;
        root->node_ptr = less;
        root->node_type = NodeLess;
    }
    if (*token == TokenEqual) {
        struct Equal *equal = (struct Equal*)_malloc(sizeof(struct Equal));
        equal->left = left;
        equal->right = right;
        root->node_ptr = equal;
        root->node_type = NodeEqual;
    }

    if (root->node_ptr == NULL) {
        print_string(STDOUT, "Fatal Error\n");
        posix_exit(3);
    }

    *primaries = (struct Node**)pop_back((void**)*primaries);
    *primaries = (struct Node**)pop_back((void**)*primaries);
    *primaries = (struct Node**)push_back((void**)*primaries, root);
    _free(token);
    *operations = (enum TokenType**)pop_back((void**)*operations);
    return root;
}

struct Node *Syntax_ProcessExpression(struct TokenStream *ts) {
    struct Node **primaries = (struct Node**)_malloc(sizeof(struct Node*));
    primaries[0] = NULL;
    enum TokenType **operations = (enum TokenType**)_malloc(sizeof(enum TokenType*));
    operations[0] = NULL;
    int ParenthesisLevel = 0;
    
    enum State {
        State_Identifier,
        State_UnaryOperation,
        State_BinaryOperation,
        State_ParenthesisOpen,
        State_ParenthesisClose,
    };
    enum State CurrentState;
    if (TokenStream_GetToken(ts).type == TokenParenthesisOpen) {
        struct Token token = TokenStream_GetToken(ts);
        enum TokenType *token_type = (enum TokenType*)_malloc(sizeof(enum TokenType));
        *token_type = token.type;
        operations = (enum TokenType**)push_back((void**)operations, token_type);
        TokenStream_NextToken(ts);
        ParenthesisLevel++;
        CurrentState = State_ParenthesisOpen;
    }
    else if (next_is_operation(ts)) {
        struct Token token = TokenStream_GetToken(ts);
        enum TokenType *token_type = (enum TokenType*)_malloc(sizeof(enum TokenType));
        *token_type = token.type;
        operations = (enum TokenType**)push_back((void**)operations, token_type);
        TokenStream_NextToken(ts);
        CurrentState = State_UnaryOperation;
    }
    else {
        primaries = (struct Node**)push_back((void**)primaries, Syntax_ProcessPrimary(ts));
        CurrentState = State_Identifier;
    }
    
    while (CurrentState == State_UnaryOperation ||
           CurrentState == State_BinaryOperation ||
           CurrentState == State_ParenthesisOpen ||
           next_is_operation(ts) ||
           ParenthesisLevel != 0) {
        
        if (TokenStream_GetToken(ts).type == TokenParenthesisOpen) {
            if (CurrentState != State_UnaryOperation &&
                CurrentState != State_BinaryOperation &&
                CurrentState != State_ParenthesisOpen) {
                SyntaxError("Unexpected ( in expression", TokenStream_GetToken(ts));
            }
            struct Token token = TokenStream_GetToken(ts);
            enum TokenType *token_type = (enum TokenType*)_malloc(sizeof(enum TokenType));
            *token_type = token.type;
            operations = (enum TokenType**)push_back((void**)operations, token_type);
            TokenStream_NextToken(ts);
            ParenthesisLevel++;
            CurrentState = State_ParenthesisOpen;
        }
        else if (TokenStream_GetToken(ts).type == TokenParenthesisClose) {
            if (CurrentState != State_Identifier &&
                CurrentState != State_ParenthesisOpen) {
                SyntaxError("Unexpected ) in expression", TokenStream_GetToken(ts));
            }
            while (*operations != NULL && *((enum TokenType*)get_back((void**)operations)) != TokenParenthesisOpen) {
                process_operation(&primaries, &operations, ts);
            }
            if (*operations == NULL || *((enum TokenType*)get_back((void*)operations)) != TokenParenthesisOpen) {
                SyntaxError("Unexpected ) in expression", TokenStream_GetToken(ts));
            }
            _free(get_back((void*)operations));
            operations = (enum TokenType**)pop_back((void**)operations);
            TokenStream_NextToken(ts);
            ParenthesisLevel--;
            CurrentState = State_ParenthesisClose;
        }
        else if (next_is_operation(ts)) {
            if (CurrentState == State_UnaryOperation ||
                CurrentState == State_BinaryOperation ||
                CurrentState == State_ParenthesisOpen) {
                SyntaxError("Unexpected operator in expression", TokenStream_GetToken(ts));
            }
            else {
                struct Token token = TokenStream_GetToken(ts);
                while (*operations != NULL &&
                    operation_priority((enum TokenType*)get_back((void**)operations)) <= 
                        operation_priority((enum TokenType*)&token.type)) {
                    process_operation(&primaries, &operations, ts);
                }
                enum TokenType *token_type = (enum TokenType*)_malloc(sizeof(enum TokenType));
                *token_type = token.type;
                operations = (enum TokenType**)push_back((void**)operations, token_type);
                TokenStream_NextToken(ts);
                CurrentState = State_BinaryOperation;
            }
        }
        else {
            if (CurrentState != State_UnaryOperation &&
                CurrentState != State_BinaryOperation &&
                CurrentState != State_ParenthesisOpen) {
                SyntaxError("Unexpected identifier in expression", TokenStream_GetToken(ts));
            }
            primaries = (struct Node**)push_back((void**)primaries, Syntax_ProcessPrimary(ts));
            CurrentState = State_Identifier;
        }
    }

    while ((*operations) != NULL) {
        process_operation(&primaries, &operations, ts);
    }

    if (get_size((void**)primaries) != 1) {
        SyntaxError("Incorrect expression", TokenStream_GetToken(ts));
    }

    struct Node *res = primaries[0];
    _free(primaries);
    _free(operations);

    if (TokenStream_GetToken(ts).type == TokenAs) {
        struct Node *node = (struct Node*)_malloc(sizeof(struct Node));
        struct As *_as = (struct As*)_malloc(sizeof(struct As));
        node->node_ptr = _as;
        node->node_type = NodeAs;
        node->line_begin = res->line_begin;
        node->position_begin = res->position_begin;
        node->filename = _strdup(res->filename);
        _as->expression = res;
        TokenStream_NextToken(ts);
        _as->type = Syntax_ProcessType(ts);
        node->line_end = TokenStream_GetToken(ts).line_end;
        node->position_end = TokenStream_GetToken(ts).position_end;
        TokenStream_NextToken(ts);
        return node;
    }
    return res;
}

struct Node *Syntax_ProcessPrimary(struct TokenStream *ts) {
    struct Node *node = (struct Node*)_malloc(sizeof(struct Node));
    node->line_begin = TokenStream_GetToken(ts).line_begin;
    node->position_begin = TokenStream_GetToken(ts).position_begin;
    node->filename = _strdup(TokenStream_GetToken(ts).filename);

    if (TokenStream_GetToken(ts).type == TokenInteger) {
        struct Integer *_integer = (struct Integer*)_malloc(sizeof(struct Integer));
        node->node_ptr = _integer;
        node->node_type = NodeInteger;
        _integer->value = TokenStream_GetToken(ts).value_int;
        node->line_end = TokenStream_GetToken(ts).line_end;
        node->position_end = TokenStream_GetToken(ts).position_end;
        TokenStream_NextToken(ts);
        return node;
    }
    if (TokenStream_GetToken(ts).type == TokenCaret) {
        struct Sizeof *_sizeof = (struct Sizeof*)_malloc(sizeof(struct Sizeof));
        node->node_ptr = _sizeof;
        node->node_type = NodeSizeof;
        TokenStream_NextToken(ts);
        _sizeof->type = Syntax_ProcessType(ts);
        node->line_end = TokenStream_GetToken(ts).line_end;
        node->position_end = TokenStream_GetToken(ts).position_end;
        TokenStream_NextToken(ts);
        return node;
    }

    bool first = true;
    if (TokenStream_GetToken(ts).type == TokenIdentifier) {
        struct Identifier *_identifier = (struct Identifier*)_malloc(sizeof(struct Identifier));
        _identifier->identifier = _strdup(TokenStream_GetToken(ts).value_string);
        node->node_ptr = _identifier;
        node->node_type = NodeIdentifier;
        node->line_end = TokenStream_GetToken(ts).line_end;
        node->position_end = TokenStream_GetToken(ts).position_end;
        TokenStream_NextToken(ts);
        first = false;
    }

    while (true) {
        if (TokenStream_GetToken(ts).type == TokenDot) {
            struct Node *prv_node = node;
            node = (struct Node*)_malloc(sizeof(struct Node));
            node->line_begin = prv_node->line_begin;
            node->position_begin = prv_node->position_begin;
            node->filename = _strdup(prv_node->filename);

            if (first) {
                _free(prv_node);
                prv_node = NULL;
            }
            TokenStream_NextToken(ts);

            struct FunctionCall *function_call = (struct FunctionCall*)_malloc(sizeof(struct FunctionCall));
            function_call->arguments = (struct Node**)_malloc(sizeof(struct Node*));
            function_call->arguments[0] = NULL;
            node->node_ptr = function_call;
            node->node_type = NodeFunctionCall;
            if (TokenStream_GetToken(ts).type != TokenIdentifier) {
                SyntaxError("Identifier expected in function call", TokenStream_GetToken(ts));
            }
            function_call->identifier = _strdup(TokenStream_GetToken(ts).value_string);
            TokenStream_NextToken(ts);
            if (TokenStream_GetToken(ts).type != TokenParenthesisOpen) {
                SyntaxError("( expected in function call", TokenStream_GetToken(ts));
            }
            TokenStream_NextToken(ts);
            while (true) {
                if (TokenStream_GetToken(ts).type == TokenParenthesisClose) {
                    break;
                }
                function_call->arguments = (struct Node**)push_back((void**)function_call->arguments, Syntax_ProcessExpression(ts));
                if (TokenStream_GetToken(ts).type == TokenParenthesisClose) {
                    break;
                }
                if (TokenStream_GetToken(ts).type != TokenComma) {
                    SyntaxError(", expected in function call", TokenStream_GetToken(ts));
                }
                TokenStream_NextToken(ts);
            }
            node->line_end = TokenStream_GetToken(ts).line_end;
            node->position_end = TokenStream_GetToken(ts).position_end;
            TokenStream_NextToken(ts);
            function_call->caller = prv_node;
            first = false;
        }
        else if (TokenStream_GetToken(ts).type == TokenGetField && first == false) {
            struct Node *prv_node = node;
            node = (struct Node*)_malloc(sizeof(struct Node));
            node->line_begin = prv_node->line_begin;
            node->position_begin = prv_node->position_begin;
            node->filename = _strdup(prv_node->filename);

            TokenStream_NextToken(ts);

            struct GetField *_get_field = (struct GetField*)_malloc(sizeof(struct GetField));
            node->node_ptr = _get_field;
            node->node_type = NodeGetField;
            _get_field->left = prv_node;
            if (TokenStream_GetToken(ts).type != TokenIdentifier) {
                SyntaxError("Identifier expected in get struct field expression", TokenStream_GetToken(ts));
            }
            _get_field->field = _strdup(TokenStream_GetToken(ts).value_string);
            node->line_end = TokenStream_GetToken(ts).line_end;
            node->position_end = TokenStream_GetToken(ts).position_end;
            TokenStream_NextToken(ts);
            if (TokenStream_GetToken(ts).type == TokenAddress) {
                _get_field->address = true;
                TokenStream_NextToken(ts);
            }
            else {
                _get_field->address = false;
            }
        }
        else if (TokenStream_GetToken(ts).type == TokenBracketOpen && first == false) {
            struct Node *prv_node = node;
            node = (struct Node*)_malloc(sizeof(struct Node));
            node->line_begin = prv_node->line_begin;
            node->position_begin = prv_node->position_begin;
            node->filename = _strdup(prv_node->filename);

            TokenStream_NextToken(ts);

            struct Index *_index = (struct Index*)_malloc(sizeof(struct Index));
            node->node_ptr = _index;
            node->node_type = NodeIndex;
            _index->left = prv_node;
            _index->right = Syntax_ProcessExpression(ts);
            if (TokenStream_GetToken(ts).type != TokenBracketClose) {
                SyntaxError("] expected in index expression", TokenStream_GetToken(ts));
            }
            node->line_end = TokenStream_GetToken(ts).line_end;
            node->position_end = TokenStream_GetToken(ts).position_end;
            TokenStream_NextToken(ts);
            if (TokenStream_GetToken(ts).type == TokenAddress) {
                _index->address = true;
                TokenStream_NextToken(ts);
            }
            else {
                _index->address = false;
            }
        }
        else if (TokenStream_GetToken(ts).type == TokenDereference && first == false) {
            struct Node *prv_node = node;
            node = (struct Node*)_malloc(sizeof(struct Node));
            node->line_begin = prv_node->line_begin;
            node->position_begin = prv_node->position_begin;
            node->filename = _strdup(prv_node->filename);

            struct Dereference *_dereference = (struct Dereference*)_malloc(sizeof(struct Dereference));
            node->node_ptr = _dereference;
            node->node_type = NodeDereference;
            _dereference->expression = prv_node;
            node->line_end = TokenStream_GetToken(ts).line_end;
            node->position_end = TokenStream_GetToken(ts).position_end;
            TokenStream_NextToken(ts);
        }
        else break;
    }

    if (first) {
        SyntaxError("Undexpected symbol in primary expression", TokenStream_GetToken(ts));
    }
    return node;
}

struct FunctionSignature *Syntax_ProcessFunctionSignature(struct TokenStream *ts) {
    struct FunctionSignature *function_signature = (struct FunctionSignature*)_malloc(sizeof(struct FunctionSignature));
    function_signature->identifiers = (const char**)_malloc(sizeof(const char*));
    function_signature->identifiers[0] = NULL;
    function_signature->types = (struct Type**)_malloc(sizeof(struct Type*));
    function_signature->types[0] = NULL;

    while (true) {
        if (TokenStream_GetToken(ts).type == TokenParenthesisClose) {
            TokenStream_NextToken(ts);
            break;
        }
        if (TokenStream_GetToken(ts).type != TokenIdentifier) {
            SyntaxError("Identifier expected in argument list", TokenStream_GetToken(ts));
        }
        function_signature->identifiers = (const char**)push_back((void**)function_signature->identifiers, _strdup(TokenStream_GetToken(ts).value_string));
        TokenStream_NextToken(ts);
        struct Type *type = Syntax_ProcessType(ts);
        TokenStream_NextToken(ts);
        function_signature->types = (struct Type**)push_back((void**)function_signature->types, type);
        bool *is_const = (bool*)_malloc(sizeof(bool));
        if (TokenStream_GetToken(ts).type == TokenParenthesisClose) {
            TokenStream_NextToken(ts);
            break;
        }
        if (TokenStream_GetToken(ts).type != TokenComma) {
            SyntaxError(", expected in argument list", TokenStream_GetToken(ts));
        }
        TokenStream_NextToken(ts);
    }
    if (TokenStream_GetToken(ts).type != TokenGetField) {
        SyntaxError("-> expected in function definition", TokenStream_GetToken(ts));
    }
    TokenStream_NextToken(ts);
    function_signature->return_type = Syntax_ProcessType(ts);
    return function_signature;
}

struct Node *Syntax_ProcessStatement(struct TokenStream *ts) {
    if (TokenStream_GetToken(ts).type == TokenSemicolon) {
        TokenStream_NextToken(ts);
        return NULL;
    }
    if (TokenStream_GetToken(ts).type == TokenBraceOpen) {
        struct Node *node = Syntax_ProcessBlock(ts, true);
        TokenStream_NextToken(ts);
        return node;
    }
    if (TokenStream_GetToken(ts).type == TokenAsm) {
        struct Node *node = (struct Node*)_malloc(sizeof(struct Node));
        struct Asm *_asm = (struct Asm*)_malloc(sizeof(struct Asm));
        node->node_ptr = _asm;
        node->node_type = NodeAsm;
        node->line_begin = TokenStream_GetToken(ts).line_begin;
        node->position_begin = TokenStream_GetToken(ts).position_begin;
        node->filename = _strdup(TokenStream_GetToken(ts).filename);
        node->line_end = TokenStream_GetToken(ts).line_end;
        node->position_end = TokenStream_GetToken(ts).position_end;
        _asm->code = _strdup(TokenStream_GetToken(ts).value_string);
        TokenStream_NextToken(ts);
        return node;
    }
    if (TokenStream_GetToken(ts).type == TokenIf) {
        struct Node *node = (struct Node*)_malloc(sizeof(struct Node));
        struct If *_if = (struct If*)_malloc(sizeof(struct If));
        _if->condition_list = (struct Node**)_malloc(sizeof(struct Node*));
        _if->condition_list[0] = NULL;
        _if->block_list = (struct Node**)_malloc(sizeof(struct Node*));
        _if->block_list[0] = NULL;
        _if->else_block = NULL;
        node->node_ptr = _if;
        node->node_type = NodeIf;
        node->line_begin = TokenStream_GetToken(ts).line_begin;
        node->position_begin = TokenStream_GetToken(ts).position_begin;
        node->filename = _strdup(TokenStream_GetToken(ts).filename);
        TokenStream_NextToken(ts);
        if (TokenStream_GetToken(ts).type != TokenParenthesisOpen) {
            SyntaxError("( expected in if condition", TokenStream_GetToken(ts));
        }
        TokenStream_NextToken(ts);
        struct Node *_expression = Syntax_ProcessExpression(ts);
        if (TokenStream_GetToken(ts).type != TokenParenthesisClose) {
            SyntaxError(") expected in if condition", TokenStream_GetToken(ts));
        }
        TokenStream_NextToken(ts);
        if (TokenStream_GetToken(ts).type != TokenBraceOpen) {
            SyntaxError("{ expected in if block", TokenStream_GetToken(ts));
        }
        struct Node *_block = Syntax_ProcessBlock(ts, true);
        _if->condition_list = (struct Node**)push_back((void**)_if->condition_list, _expression);
        _if->block_list = (struct Node**)push_back((void**)_if->block_list, _block);
        node->line_end = TokenStream_GetToken(ts).line_end;
        node->position_end = TokenStream_GetToken(ts).position_end;
        TokenStream_NextToken(ts);

        if (TokenStream_GetToken(ts).type == TokenElse) {
            TokenStream_NextToken(ts);
            if (TokenStream_GetToken(ts).type != TokenBraceOpen) {
                SyntaxError("{ expected in if block", TokenStream_GetToken(ts));
            }
            struct Node *_block = Syntax_ProcessBlock(ts, true);
            _if->else_block = _block;
            node->line_end = TokenStream_GetToken(ts).line_end;
            node->position_end = TokenStream_GetToken(ts).position_end;
            TokenStream_NextToken(ts);
        }

        return node;
    }
    if (TokenStream_GetToken(ts).type == TokenWhile) {
        struct Node *node = (struct Node*)_malloc(sizeof(struct Node));
        struct While *_while = (struct While*)_malloc(sizeof(struct While));
        node->node_ptr = _while;
        node->node_type = NodeWhile;
        node->line_begin = TokenStream_GetToken(ts).line_begin;
        node->position_begin = TokenStream_GetToken(ts).position_begin;
        node->filename = _strdup(TokenStream_GetToken(ts).filename);
        TokenStream_NextToken(ts);
        if (TokenStream_GetToken(ts).type != TokenParenthesisOpen) {
            SyntaxError("( expected in while condition", TokenStream_GetToken(ts));
        }
        TokenStream_NextToken(ts);
        struct Node *_expression = Syntax_ProcessExpression(ts);
        if (TokenStream_GetToken(ts).type != TokenParenthesisClose) {
            SyntaxError(") expected in while condition", TokenStream_GetToken(ts));
        }
        TokenStream_NextToken(ts);
        if (TokenStream_GetToken(ts).type != TokenBraceOpen) {
            SyntaxError("{ expected in while block", TokenStream_GetToken(ts));
        }
        struct Node *_block = Syntax_ProcessBlock(ts, true);
        _while->condition = _expression;
        _while->block = _block;
        node->line_end = TokenStream_GetToken(ts).line_end;
        node->position_end = TokenStream_GetToken(ts).position_end;
        TokenStream_NextToken(ts);

        return node;
    }
    if (TokenStream_GetToken(ts).type == TokenFunc) {
        struct Node *node = (struct Node*)_malloc(sizeof(struct Node));
        struct FunctionDefinition *function_definition = (struct FunctionDefinition*)_malloc(sizeof(struct FunctionDefinition));
        node->node_ptr = function_definition;
        node->node_type = NodeFunctionDefinition;
        node->line_begin = TokenStream_GetToken(ts).line_begin;
        node->position_begin = TokenStream_GetToken(ts).position_begin;
        node->filename = _strdup(TokenStream_GetToken(ts).filename);
        function_definition->external = false;
        TokenStream_NextToken(ts);
        if (TokenStream_GetToken(ts).type == TokenCaret) {
            function_definition->external = true;
            TokenStream_NextToken(ts);
        }
        if (TokenStream_GetToken(ts).type == TokenIdentifier) {
            function_definition->struct_name = _strdup(TokenStream_GetToken(ts).value_string);
            TokenStream_NextToken(ts);
        }
        else {
            function_definition->struct_name = NULL;
        }
        if (TokenStream_GetToken(ts).type != TokenDot) {
            SyntaxError(". exprected in function definition", TokenStream_GetToken(ts));
        }
        TokenStream_NextToken(ts);
        if (TokenStream_GetToken(ts).type != TokenIdentifier) {
            SyntaxError("Identifier exprected in function definition", TokenStream_GetToken(ts));
        }
        function_definition->name = _strdup(TokenStream_GetToken(ts).value_string);
        TokenStream_NextToken(ts);
        if (TokenStream_GetToken(ts).type != TokenParenthesisOpen) {
            SyntaxError("( expected in function definition", TokenStream_GetToken(ts));
        }
        TokenStream_NextToken(ts);
        function_definition->signature = Syntax_ProcessFunctionSignature(ts);
        TokenStream_NextToken(ts);
        if (TokenStream_GetToken(ts).type != TokenBraceOpen) {
            SyntaxError("{ expected in function block", TokenStream_GetToken(ts));
        }
        function_definition->block = Syntax_ProcessBlock(ts, true);
        node->line_end = TokenStream_GetToken(ts).line_end;
        node->position_end = TokenStream_GetToken(ts).position_end;
        TokenStream_NextToken(ts);

        return node;
    }
    if (TokenStream_GetToken(ts).type == TokenProto) {
        struct Node *node = (struct Node*)_malloc(sizeof(struct Node));
        struct Prototype *prototype = (struct Prototype*)_malloc(sizeof(struct Prototype));
        node->node_ptr = prototype;
        node->node_type = NodePrototype;
        node->line_begin = TokenStream_GetToken(ts).line_begin;
        node->position_begin = TokenStream_GetToken(ts).position_begin;
        node->filename = _strdup(TokenStream_GetToken(ts).filename);
        TokenStream_NextToken(ts);
        if (TokenStream_GetToken(ts).type == TokenIdentifier) {
            prototype->struct_name = _strdup(TokenStream_GetToken(ts).value_string);
            TokenStream_NextToken(ts);
        }
        else {
            prototype->struct_name = NULL;
        }
        if (TokenStream_GetToken(ts).type != TokenDot) {
            SyntaxError(". exprected in function prototype", TokenStream_GetToken(ts));
        }
        TokenStream_NextToken(ts);
        if (TokenStream_GetToken(ts).type != TokenIdentifier) {
            SyntaxError("Identifier exprected in function prototype", TokenStream_GetToken(ts));
        }
        prototype->name = _strdup(TokenStream_GetToken(ts).value_string);
        TokenStream_NextToken(ts);
        if (TokenStream_GetToken(ts).type != TokenParenthesisOpen) {
            SyntaxError("( expected in function prototype", TokenStream_GetToken(ts));
        }
        TokenStream_NextToken(ts);
        prototype->signature = Syntax_ProcessFunctionSignature(ts);
        node->line_end = TokenStream_GetToken(ts).line_end;
        node->position_end = TokenStream_GetToken(ts).position_end;
        TokenStream_NextToken(ts);

        return node;
    }
    if (TokenStream_GetToken(ts).type == TokenStruct) {
        struct Node *node = (struct Node*)_malloc(sizeof(struct Node));
        struct StructDefinition *struct_definition = (struct StructDefinition*)_malloc(sizeof(struct StructDefinition));
        node->node_ptr = struct_definition;
        node->node_type = NodeStructDefinition;
        node->line_begin = TokenStream_GetToken(ts).line_begin;
        node->position_begin = TokenStream_GetToken(ts).position_begin;
        node->filename = _strdup(TokenStream_GetToken(ts).filename);
        struct_definition->identifiers = (const char**)_malloc(sizeof(const char*));
        struct_definition->identifiers[0] = NULL;
        struct_definition->types = (struct Type**)_malloc(sizeof(struct Type*));
        struct_definition->types[0] = NULL;
        TokenStream_NextToken(ts);
        if (TokenStream_GetToken(ts).type != TokenIdentifier) {
            SyntaxError("Struct name expected in struct definition", TokenStream_GetToken(ts));
        }
        struct_definition->name = _strdup(TokenStream_GetToken(ts).value_string);
        TokenStream_NextToken(ts);
        if (TokenStream_GetToken(ts).type != TokenBraceOpen) {
            SyntaxError("{ expected in struct definition", TokenStream_GetToken(ts));
        }
        TokenStream_NextToken(ts);
        while (true) {
            if (TokenStream_GetToken(ts).type == TokenBraceClose) {
                break;
            }
            if (TokenStream_GetToken(ts).type != TokenIdentifier) {
                SyntaxError("Identifier expected in struct definition", TokenStream_GetToken(ts));
            }
            struct_definition->identifiers = (const char**)push_back((void**)struct_definition->identifiers, (void*)TokenStream_GetToken(ts).value_string);
            TokenStream_NextToken(ts);
            struct_definition->types = (struct Type**)push_back((void**)struct_definition->types, Syntax_ProcessType(ts));
            TokenStream_NextToken(ts);
        }
        node->line_end = TokenStream_GetToken(ts).line_end;
        node->position_end = TokenStream_GetToken(ts).position_end;
        TokenStream_NextToken(ts);

        return node;
    }
    if (TokenStream_GetToken(ts).type == TokenDef) {
        struct Node *node = (struct Node*)_malloc(sizeof(struct Node));
        struct Definition *definition = (struct Definition*)_malloc(sizeof(struct Definition));
        node->node_ptr = definition;
        node->node_type = NodeDefinition;
        node->line_begin = TokenStream_GetToken(ts).line_begin;
        node->position_begin = TokenStream_GetToken(ts).position_begin;
        node->filename = _strdup(TokenStream_GetToken(ts).filename);
        TokenStream_NextToken(ts);
        if (TokenStream_GetToken(ts).type != TokenIdentifier) {
            SyntaxError("Identifier expected in definition statement", TokenStream_GetToken(ts));
        }
        definition->identifier = _strdup(TokenStream_GetToken(ts).value_string);
        TokenStream_NextToken(ts);
        definition->type = Syntax_ProcessType(ts);
        node->line_end = TokenStream_GetToken(ts).line_end;
        node->position_end = TokenStream_GetToken(ts).position_end;
        TokenStream_NextToken(ts);
        return node;
    }
    if (TokenStream_GetToken(ts).type == TokenReturn) {
        struct Node *node = (struct Node*)_malloc(sizeof(struct Node));
        struct Return *_return = (struct Return*)_malloc(sizeof(struct Return));
        node->node_ptr = _return;
        node->node_type = NodeReturn;
        node->line_begin = TokenStream_GetToken(ts).line_begin;
        node->position_begin = TokenStream_GetToken(ts).position_begin;
        node->filename = _strdup(TokenStream_GetToken(ts).filename);
        TokenStream_NextToken(ts);
        _return->expression = Syntax_ProcessExpression(ts);
        node->line_end = _return->expression->line_end;
        node->position_end = _return->expression->position_end;
        return node;
    }
    {
        struct Node *node = (struct Node*)_malloc(sizeof(struct Node));
        node->line_begin = TokenStream_GetToken(ts).line_begin;
        node->position_begin = TokenStream_GetToken(ts).position_begin;
        node->filename = _strdup(TokenStream_GetToken(ts).filename);
        struct Node *left = Syntax_ProcessExpression(ts);

        if (TokenStream_GetToken(ts).type == TokenAssign) {
            TokenStream_NextToken(ts);
            struct Assignment *_assignment = (struct Assignment*)_malloc(sizeof(struct Assignment));
            node->node_ptr = _assignment;
            node->node_type = NodeAssignment;
            struct Node *right = Syntax_ProcessExpression(ts);
            node->line_end = right->line_end;
            node->position_end = right->position_end;
            _assignment->dst = left;
            _assignment->src = right;
            return node;
        }
        else if (TokenStream_GetToken(ts).type == TokenMove) {
            TokenStream_NextToken(ts);
            struct Movement *_movement = (struct Movement*)_malloc(sizeof(struct Movement));
            node->node_ptr = _movement;
            node->node_type = NodeMovement;
            struct Node *right = Syntax_ProcessExpression(ts);
            node->line_end = right->line_end;
            node->position_end = right->position_end;
            _movement->dst = left;
            _movement->src = right;
            return node;
        }
        else {
            SyntaxError(":= or <- expected in assignment or movement statement", TokenStream_GetToken(ts));
        }
    }
}

struct Node *Syntax_Process(struct TokenStream *token_stream) {
    token_stream->pos = 0;
    return Syntax_ProcessBlock(token_stream, false);
}
