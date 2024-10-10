#include <syntax.h>
#include <exception.h>
#include <process.h>
#include <vector.h>
#include <posix.h>
#include <stdlib.h>
#include <string.h>

struct Node *init_node(struct TokenStream *ts) {
    struct Node *node = (struct Node*)_malloc(sizeof(struct Node));
    node->line_begin = TokenStream_GetToken(ts).line_begin;
    node->position_begin = TokenStream_GetToken(ts).position_begin;
    node->filename = _strdup(TokenStream_GetToken(ts).filename);
    return node;
}

void check_next(struct TokenStream *ts, enum TokenType type, const char *error) {
    if (TokenStream_GetToken(ts).type != type) {
        SyntaxError(error, TokenStream_GetToken(ts));
    }
}

void pass_next(struct TokenStream *ts, enum TokenType type, const char *error) {
    if (TokenStream_GetToken(ts).type != type) {
        SyntaxError(error, TokenStream_GetToken(ts));
    }
    TokenStream_NextToken(ts);
}

struct Node *Syntax_ProcessBlock(struct TokenStream *ts, bool braces) {
    struct Node *node = (struct Node*)_malloc(sizeof(struct Node));
    struct Block *this = (struct Block*)_malloc(sizeof(struct Block));
    node->node_ptr = this;
    this->statement_list = (struct Node**)_malloc(sizeof(struct Node*));
    this->statement_list[0] = NULL;
    node->node_type = NodeBlock;
    node->line_begin = TokenStream_GetToken(ts).line_begin;
    node->position_begin = TokenStream_GetToken(ts).position_begin;
    node->filename = _strdup(TokenStream_GetToken(ts).filename);
    if (braces) {
        TokenStream_NextToken(ts);
    }
    while (TokenStream_GetToken(ts).type != TokenEof && TokenStream_GetToken(ts).type != TokenBraceClose) {
        if (TokenStream_GetToken(ts).type == TokenSemicolon) {
            TokenStream_NextToken(ts);
            continue;
        }
        if (TokenStream_GetToken(ts).type == TokenInclude) {
            TokenStream_NextToken(ts);
            check_next(ts, TokenString, "String literal expected in include");
            const char *filename = TokenStream_GetToken(ts).value_string;
            TokenStream_NextToken(ts);
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
                this->statement_list = (struct Node**)push_back((void**)this->statement_list, *ptr);
                ptr++;
            }
            _free(inc_block->statement_list);
            _free(inc_block);
            _free(_node);
            continue;
        }
        this->statement_list = (struct Node**)push_back((void**)this->statement_list, Syntax_ProcessStatement(ts));
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
        struct As *this = (struct As*)_malloc(sizeof(struct As));
        node->node_ptr = this;
        node->node_type = NodeAs;
        node->line_begin = res->line_begin;
        node->position_begin = res->position_begin;
        node->filename = _strdup(res->filename);
        this->expression = res;
        TokenStream_NextToken(ts);
        this->type = Syntax_ProcessType(ts);
        node->line_end = TokenStream_GetToken(ts).line_end;
        node->position_end = TokenStream_GetToken(ts).position_end;
        TokenStream_NextToken(ts);
        return node;
    }
    return res;
}

struct Node *Syntax_ProcessPrimary(struct TokenStream *ts) {
    struct Node *node = init_node(ts);

    if (TokenStream_GetToken(ts).type == TokenInteger) {
        struct Integer *this = (struct Integer*)_malloc(sizeof(struct Integer));
        node->node_ptr = this;
        node->node_type = NodeInteger;
        this->value = TokenStream_GetToken(ts).value_int;
        node->line_end = TokenStream_GetToken(ts).line_end;
        node->position_end = TokenStream_GetToken(ts).position_end;
        TokenStream_NextToken(ts);
        return node;
    }
    if (TokenStream_GetToken(ts).type == TokenChar) {
        struct Char *this = (struct Char*)_malloc(sizeof(struct Char));
        node->node_ptr = this;
        node->node_type = NodeChar;
        this->value = TokenStream_GetToken(ts).value_int;
        node->line_end = TokenStream_GetToken(ts).line_end;
        node->position_end = TokenStream_GetToken(ts).position_end;
        TokenStream_NextToken(ts);
        return node;
    }
    if (TokenStream_GetToken(ts).type == TokenString) {
        struct String *this = (struct String*)_malloc(sizeof(struct String));
        node->node_ptr = this;
        node->node_type = NodeString;
        this->value = _strdup(TokenStream_GetToken(ts).value_string);
        node->line_end = TokenStream_GetToken(ts).line_end;
        node->position_end = TokenStream_GetToken(ts).position_end;
        TokenStream_NextToken(ts);
        return node;
    }
    if (TokenStream_GetToken(ts).type == TokenBracketOpen) {
        struct Array *this = (struct Array*)_malloc(sizeof(struct Array));
        node->node_ptr = this;
        node->node_type = NodeArray;
        this->values = (struct Node**)_malloc(sizeof(struct Node*));
        this->values[0] = NULL;
        TokenStream_NextToken(ts);
        while (TokenStream_GetToken(ts).type != TokenBracketClose) {
            this->values = (struct Node**)push_back((void**)this->values, Syntax_ProcessExpression(ts));
            if (TokenStream_GetToken(ts).type == TokenComma) {
                TokenStream_NextToken(ts);
            }
        }
        node->line_end = TokenStream_GetToken(ts).line_end;
        node->position_end = TokenStream_GetToken(ts).position_end;
        TokenStream_NextToken(ts);
        return node;
    }
    if (TokenStream_GetToken(ts).type == TokenCaret) {
        struct Sizeof *this = (struct Sizeof*)_malloc(sizeof(struct Sizeof));
        node->node_ptr = this;
        node->node_type = NodeSizeof;
        TokenStream_NextToken(ts);
        this->type = Syntax_ProcessType(ts);
        node->line_end = TokenStream_GetToken(ts).line_end;
        node->position_end = TokenStream_GetToken(ts).position_end;
        TokenStream_NextToken(ts);
        return node;
    }

    bool first = true;
    if (TokenStream_GetToken(ts).type == TokenIdentifier) {
        struct Identifier *this = (struct Identifier*)_malloc(sizeof(struct Identifier));
        this->identifier = _strdup(TokenStream_GetToken(ts).value_string);
        node->node_ptr = this;
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

            struct FunctionCall *this = (struct FunctionCall*)_malloc(sizeof(struct FunctionCall));
            this->arguments = (struct Node**)_malloc(sizeof(struct Node*));
            this->arguments[0] = NULL;
            node->node_ptr = this;
            node->node_type = NodeFunctionCall;
            check_next(ts, TokenIdentifier, "Identifier expected in function call");
            this->identifier = _strdup(TokenStream_GetToken(ts).value_string);
            TokenStream_NextToken(ts);
            pass_next(ts, TokenParenthesisOpen, "( expected in function call");
            while (true) {
                if (TokenStream_GetToken(ts).type == TokenParenthesisClose) {
                    break;
                }
                this->arguments = (struct Node**)push_back((void**)this->arguments, Syntax_ProcessExpression(ts));
                if (TokenStream_GetToken(ts).type == TokenParenthesisClose) {
                    break;
                }
                pass_next(ts, TokenComma, ", expected in function call");
            }
            node->line_end = TokenStream_GetToken(ts).line_end;
            node->position_end = TokenStream_GetToken(ts).position_end;
            TokenStream_NextToken(ts);
            this->caller = prv_node;
            first = false;
        }
        else if (TokenStream_GetToken(ts).type == TokenGetField && first == false) {
            struct Node *prv_node = node;
            node = (struct Node*)_malloc(sizeof(struct Node));
            node->line_begin = prv_node->line_begin;
            node->position_begin = prv_node->position_begin;
            node->filename = _strdup(prv_node->filename);

            TokenStream_NextToken(ts);

            struct GetField *this = (struct GetField*)_malloc(sizeof(struct GetField));
            node->node_ptr = this;
            node->node_type = NodeGetField;
            this->left = prv_node;
            check_next(ts, TokenIdentifier, "Identifier expected in get struct field expression");
            this->field = _strdup(TokenStream_GetToken(ts).value_string);
            node->line_end = TokenStream_GetToken(ts).line_end;
            node->position_end = TokenStream_GetToken(ts).position_end;
            TokenStream_NextToken(ts);
            if (TokenStream_GetToken(ts).type == TokenAddress) {
                this->address = true;
                TokenStream_NextToken(ts);
            }
            else {
                this->address = false;
            }
        }
        else if (TokenStream_GetToken(ts).type == TokenBracketOpen && first == false) {
            struct Node *prv_node = node;
            node = (struct Node*)_malloc(sizeof(struct Node));
            node->line_begin = prv_node->line_begin;
            node->position_begin = prv_node->position_begin;
            node->filename = _strdup(prv_node->filename);

            TokenStream_NextToken(ts);

            struct Index *this = (struct Index*)_malloc(sizeof(struct Index));
            node->node_ptr = this;
            node->node_type = NodeIndex;
            this->left = prv_node;
            this->right = Syntax_ProcessExpression(ts);
            node->line_end = TokenStream_GetToken(ts).line_end;
            node->position_end = TokenStream_GetToken(ts).position_end;
            pass_next(ts, TokenBracketClose, "] expected in index expression");
            if (TokenStream_GetToken(ts).type == TokenAddress) {
                this->address = true;
                TokenStream_NextToken(ts);
            }
            else {
                this->address = false;
            }
        }
        else if (TokenStream_GetToken(ts).type == TokenDereference && first == false) {
            struct Node *prv_node = node;
            node = (struct Node*)_malloc(sizeof(struct Node));
            node->line_begin = prv_node->line_begin;
            node->position_begin = prv_node->position_begin;
            node->filename = _strdup(prv_node->filename);

            struct Dereference *this = (struct Dereference*)_malloc(sizeof(struct Dereference));
            node->node_ptr = this;
            node->node_type = NodeDereference;
            this->expression = prv_node;
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
    struct FunctionSignature *this = (struct FunctionSignature*)_malloc(sizeof(struct FunctionSignature));
    this->identifiers = (const char**)_malloc(sizeof(const char*));
    this->identifiers[0] = NULL;
    this->types = (struct Type**)_malloc(sizeof(struct Type*));
    this->types[0] = NULL;

    while (true) {
        if (TokenStream_GetToken(ts).type == TokenParenthesisClose) {
            TokenStream_NextToken(ts);
            break;
        }
        check_next(ts, TokenIdentifier, "Identifier expected in argument list");
        this->identifiers = (const char**)push_back((void**)this->identifiers, _strdup(TokenStream_GetToken(ts).value_string));
        TokenStream_NextToken(ts);
        struct Type *type = Syntax_ProcessType(ts);
        TokenStream_NextToken(ts);
        this->types = (struct Type**)push_back((void**)this->types, type);
        bool *is_const = (bool*)_malloc(sizeof(bool));
        if (TokenStream_GetToken(ts).type == TokenParenthesisClose) {
            TokenStream_NextToken(ts);
            break;
        }
        pass_next(ts, TokenComma, ", expected in argument list");
    }
    pass_next(ts, TokenGetField, "-> expected in function definition");
    this->return_type = Syntax_ProcessType(ts);
    return this;
}

struct Node *Syntax_ProcessStatement(struct TokenStream *ts) {
    if (TokenStream_GetToken(ts).type == TokenBraceOpen) {
        struct Node *node = Syntax_ProcessBlock(ts, true);
        TokenStream_NextToken(ts);
        return node;
    }
    if (TokenStream_GetToken(ts).type == TokenIf) {
        struct Node *node = init_node(ts);
        struct If *this = (struct If*)_malloc(sizeof(struct If));
        this->condition_list = (struct Node**)_malloc(sizeof(struct Node*));
        this->condition_list[0] = NULL;
        this->block_list = (struct Node**)_malloc(sizeof(struct Node*));
        this->block_list[0] = NULL;
        this->else_block = NULL;
        node->node_ptr = this;
        node->node_type = NodeIf;
        TokenStream_NextToken(ts);
        pass_next(ts, TokenParenthesisOpen, "( expected in if condition");
        struct Node *_expression = Syntax_ProcessExpression(ts);
        pass_next(ts, TokenParenthesisClose, ") expected in if condition");
        check_next(ts, TokenBraceOpen, "{ expected in if block");
        struct Node *_block = Syntax_ProcessBlock(ts, true);
        this->condition_list = (struct Node**)push_back((void**)this->condition_list, _expression);
        this->block_list = (struct Node**)push_back((void**)this->block_list, _block);
        node->line_end = TokenStream_GetToken(ts).line_end;
        node->position_end = TokenStream_GetToken(ts).position_end;
        TokenStream_NextToken(ts);

        while (TokenStream_GetToken(ts).type == TokenElse) {
            TokenStream_NextToken(ts);

            if (TokenStream_GetToken(ts).type == TokenIf) {
                TokenStream_NextToken(ts);
                pass_next(ts, TokenParenthesisOpen, "( expected in if condition");
                struct Node *_expression = Syntax_ProcessExpression(ts);
                pass_next(ts, TokenParenthesisClose, ") expected in if condition");
                check_next(ts, TokenBraceOpen, "{ expected in if block");
                struct Node *_block = Syntax_ProcessBlock(ts, true);
                this->condition_list = (struct Node**)push_back((void**)this->condition_list, _expression);
                this->block_list = (struct Node**)push_back((void**)this->block_list, _block);
                node->line_end = TokenStream_GetToken(ts).line_end;
                node->position_end = TokenStream_GetToken(ts).position_end;
                TokenStream_NextToken(ts);
            }
            else {
                check_next(ts, TokenBraceOpen, "{ expected in if block");
                struct Node *_block = Syntax_ProcessBlock(ts, true);
                this->else_block = _block;
                node->line_end = TokenStream_GetToken(ts).line_end;
                node->position_end = TokenStream_GetToken(ts).position_end;
                TokenStream_NextToken(ts);
            }
        }

        return node;
    }
    if (TokenStream_GetToken(ts).type == TokenWhile) {
        struct Node *node = init_node(ts);
        struct While *this = (struct While*)_malloc(sizeof(struct While));
        node->node_ptr = this;
        node->node_type = NodeWhile;
        TokenStream_NextToken(ts);
        pass_next(ts, TokenParenthesisOpen, "( expected in while condition");
        struct Node *_expression = Syntax_ProcessExpression(ts);
        pass_next(ts, TokenParenthesisClose, ") expected in while condition");
        check_next(ts, TokenBraceOpen, "{ expected in while block");
        struct Node *_block = Syntax_ProcessBlock(ts, true);
        this->condition = _expression;
        this->block = _block;
        node->line_end = TokenStream_GetToken(ts).line_end;
        node->position_end = TokenStream_GetToken(ts).position_end;
        TokenStream_NextToken(ts);

        return node;
    }
    if (TokenStream_GetToken(ts).type == TokenFunc) {
        struct Node *node = init_node(ts);
        struct FunctionDefinition *this = (struct FunctionDefinition*)_malloc(sizeof(struct FunctionDefinition));
        node->node_ptr = this;
        node->node_type = NodeFunctionDefinition;
        this->external = false;
        TokenStream_NextToken(ts);
        if (TokenStream_GetToken(ts).type == TokenCaret) {
            this->external = true;
            TokenStream_NextToken(ts);
        }
        if (TokenStream_GetToken(ts).type == TokenIdentifier) {
            this->struct_name = _strdup(TokenStream_GetToken(ts).value_string);
            TokenStream_NextToken(ts);
        }
        else {
            this->struct_name = NULL;
        }
        pass_next(ts, TokenDot, ". exprected in function definition");
        check_next(ts, TokenIdentifier, "Identifier exprected in function definition");
        this->name = _strdup(TokenStream_GetToken(ts).value_string);
        TokenStream_NextToken(ts);
        pass_next(ts, TokenParenthesisOpen, "( expected in function definition");
        this->signature = Syntax_ProcessFunctionSignature(ts);
        TokenStream_NextToken(ts);
        check_next(ts, TokenBraceOpen, "{ expected in function block");
        this->block = Syntax_ProcessBlock(ts, true);
        node->line_end = TokenStream_GetToken(ts).line_end;
        node->position_end = TokenStream_GetToken(ts).position_end;
        TokenStream_NextToken(ts);

        return node;
    }
    if (TokenStream_GetToken(ts).type == TokenProto) {
        struct Node *node = init_node(ts);
        struct Prototype *this = (struct Prototype*)_malloc(sizeof(struct Prototype));
        node->node_ptr = this;
        node->node_type = NodePrototype;
        TokenStream_NextToken(ts);
        if (TokenStream_GetToken(ts).type == TokenIdentifier) {
            this->struct_name = _strdup(TokenStream_GetToken(ts).value_string);
            TokenStream_NextToken(ts);
        }
        else {
            this->struct_name = NULL;
        }
        pass_next(ts, TokenDot, ". exprected in function prototype");
        check_next(ts, TokenIdentifier, "Identifier exprected in function prototype");
        this->name = _strdup(TokenStream_GetToken(ts).value_string);
        TokenStream_NextToken(ts);
        pass_next(ts, TokenParenthesisOpen, "( expected in function prototype");
        this->signature = Syntax_ProcessFunctionSignature(ts);
        node->line_end = TokenStream_GetToken(ts).line_end;
        node->position_end = TokenStream_GetToken(ts).position_end;
        TokenStream_NextToken(ts);

        return node;
    }
    if (TokenStream_GetToken(ts).type == TokenStruct) {
        struct Node *node = init_node(ts);
        struct StructDefinition *this = (struct StructDefinition*)_malloc(sizeof(struct StructDefinition));
        node->node_ptr = this;
        node->node_type = NodeStructDefinition;
        this->identifiers = (const char**)_malloc(sizeof(const char*));
        this->identifiers[0] = NULL;
        this->types = (struct Type**)_malloc(sizeof(struct Type*));
        this->types[0] = NULL;
        TokenStream_NextToken(ts);
        check_next(ts, TokenIdentifier, "Struct name expected in struct definition");
        this->name = _strdup(TokenStream_GetToken(ts).value_string);
        TokenStream_NextToken(ts);
        pass_next(ts, TokenBraceOpen, "{ expected in struct definition");
        while (true) {
            if (TokenStream_GetToken(ts).type == TokenBraceClose) {
                break;
            }
            check_next(ts, TokenIdentifier, "Identifier expected in struct definition");
            this->identifiers = (const char**)push_back((void**)this->identifiers, (void*)TokenStream_GetToken(ts).value_string);
            TokenStream_NextToken(ts);
            this->types = (struct Type**)push_back((void**)this->types, Syntax_ProcessType(ts));
            TokenStream_NextToken(ts);
        }
        node->line_end = TokenStream_GetToken(ts).line_end;
        node->position_end = TokenStream_GetToken(ts).position_end;
        TokenStream_NextToken(ts);

        return node;
    }
    if (TokenStream_GetToken(ts).type == TokenDef) {
        struct Node *node = init_node(ts);
        struct Definition *this = (struct Definition*)_malloc(sizeof(struct Definition));
        node->node_ptr = this;
        node->node_type = NodeDefinition;
        TokenStream_NextToken(ts);
        check_next(ts, TokenIdentifier, "Identifier expected in definition statement");
        this->identifier = _strdup(TokenStream_GetToken(ts).value_string);
        TokenStream_NextToken(ts);
        this->type = Syntax_ProcessType(ts);
        node->line_end = TokenStream_GetToken(ts).line_end;
        node->position_end = TokenStream_GetToken(ts).position_end;
        TokenStream_NextToken(ts);
        return node;
    }
    if (TokenStream_GetToken(ts).type == TokenReturn) {
        struct Node *node = init_node(ts);
        struct Return *this = (struct Return*)_malloc(sizeof(struct Return));
        node->node_ptr = this;
        node->node_type = NodeReturn;
        TokenStream_NextToken(ts);
        this->expression = Syntax_ProcessExpression(ts);
        node->line_end = this->expression->line_end;
        node->position_end = this->expression->position_end;
        return node;
    }
    {
        struct Node *node = init_node(ts);
        struct Node *left = Syntax_ProcessExpression(ts);

        if (TokenStream_GetToken(ts).type == TokenAssign) {
            TokenStream_NextToken(ts);
            struct Assignment *this = (struct Assignment*)_malloc(sizeof(struct Assignment));
            node->node_ptr = this;
            node->node_type = NodeAssignment;
            struct Node *right = Syntax_ProcessExpression(ts);
            node->line_end = right->line_end;
            node->position_end = right->position_end;
            this->dst = left;
            this->src = right;
            return node;
        }
        else if (TokenStream_GetToken(ts).type == TokenMove) {
            TokenStream_NextToken(ts);
            struct Movement *this = (struct Movement*)_malloc(sizeof(struct Movement));
            node->node_ptr = this;
            node->node_type = NodeMovement;
            struct Node *right = Syntax_ProcessExpression(ts);
            node->line_end = right->line_end;
            node->position_end = right->position_end;
            this->dst = left;
            this->src = right;
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
