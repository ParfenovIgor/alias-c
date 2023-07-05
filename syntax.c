#include "common.h"
#include "syntax.h"
#include "exception.h"
#include "process.h"

#include <stdio.h>

struct Node **push_back(struct Node **a, struct Node *node) {
    int sz = 0;
    for (struct Node **b = a; *b != NULL; b++) {
        sz++;
    }
    struct Node **a_new = (struct Node**)_malloc((sz + 2) * sizeof(struct Node*));
    for (int i = 0; i < sz; i++) {
        a_new[i] = a[i];
    }
    a_new[sz] = node;
    a_new[sz + 1] = NULL;
    _free(a);
    return a_new;
}

struct Node **pop_back(struct Node **a) {
    struct Node **b = a;
    while (*(b + 1) != NULL) {
        b++;
    }
    *b = NULL;
    return a;
}

int get_size(struct Node **a) {
    int sz = 0;
    for (struct Node **b = a; *b != NULL; b++) {
        sz++;
    }
    return sz;
}

const char **push_back_string(const char **a, const char *node) {
    int sz = 0;
    for (const char **b = a; *b != NULL; b++) {
        sz++;
    }
    const char **a_new = (const char**)_malloc((sz + 2) * sizeof(const char*));
    for (int i = 0; i < sz; i++) {
        a_new[i] = a[i];
    }
    a_new[sz] = node;
    a_new[sz + 1] = NULL;
    _free(a);
    return a_new;
}

struct Token **push_back_token(struct Token **a, struct Token *token) {
    int sz = 0;
    for (struct Token **b = a; *b != NULL; b++) {
        sz++;
    }
    struct Token **a_new = (struct Token**)_malloc((sz + 2) * sizeof(struct Token*));
    for (int i = 0; i < sz; i++) {
        a_new[i] = a[i];
    }
    a_new[sz] = token;
    a_new[sz + 1] = NULL;
    _free(a);
    return a_new;
}

struct Token *get_back_token(struct Token **a) {
    struct Token **b = a;
    while (*(b + 1) != NULL) {
        b++;
    }
    return *b;
}

struct Token **pop_back_token(struct Token **a) {
    struct Token **b = a;
    while (*(b + 1) != NULL) {
        b++;
    }
    *b = NULL;
    return a;
}

struct Node *Syntax_ProcessProgram(struct TokenStream *ts) {
    struct Node *node = (struct Node*)_malloc(sizeof(struct Node));
    struct Block *block = (struct Block*)_malloc(sizeof(struct Block));
    node->node_ptr = block;
    block->statement_list = (struct Node**)_malloc(sizeof(struct Node*));
    block->statement_list[0] = NULL;
    node->node_type = NodeBlock;
    node->line_begin = TokenStream_GetToken(ts).line_begin;
    node->position_begin = TokenStream_GetToken(ts).position_begin;
    node->filename = TokenStream_GetToken(ts).filename;
    while (TokenStream_GetToken(ts).type != TokenEof && TokenStream_GetToken(ts).type != TokenBraceClose) {
        if (TokenStream_GetToken(ts).type == TokenInclude) {
            const char *filename = TokenStream_GetToken(ts).filename;
            FILE *file = fopen(filename, "r");
            if (!file) {
                const char *buffer = concat("Could not open file ", filename);
                SyntaxError(buffer, TokenStream_GetToken(ts));
            }
            struct Node *_node = Parse(filename);
            struct Block *inc_block = (struct Block*)_node->node_ptr;
            struct Node **ptr = inc_block->statement_list;
            while (*ptr != NULL) {
                block->statement_list = push_back(block->statement_list, *ptr);
                ptr++;
            }
            _free(inc_block->statement_list);
            TokenStream_NextToken(ts);
            continue;
        }
        struct Node *_statement = Syntax_ProcessStatement(ts);
        if (_statement) {
            block->statement_list = push_back(block->statement_list, _statement);
        }
    }
    node->line_end = TokenStream_GetToken(ts).line_end;
    node->position_end = TokenStream_GetToken(ts).position_end;
    return node;
}

struct Node *Syntax_ProcessBlock(struct TokenStream *ts) {
    struct Node *node = (struct Node*)_malloc(sizeof(struct Node));
    struct Block *block = (struct Block*)_malloc(sizeof(struct Block));
    node->node_ptr = block;
    block->statement_list = (struct Node**)_malloc(sizeof(struct Node*));
    block->statement_list[0] = NULL;
    node->node_type = NodeBlock;
    node->line_begin = TokenStream_GetToken(ts).line_begin;
    node->position_begin = TokenStream_GetToken(ts).position_begin;
    node->filename = TokenStream_GetToken(ts).filename;
    TokenStream_NextToken(ts);
    while (TokenStream_GetToken(ts).type != TokenEof && TokenStream_GetToken(ts).type != TokenBraceClose) {
        if (TokenStream_GetToken(ts).type == TokenInclude) {
            const char *filename = TokenStream_GetToken(ts).filename;
            FILE *file = fopen(filename, "r");
            if (!file) {
                const char *buffer = concat("Could not open file ", filename);
                SyntaxError(buffer, TokenStream_GetToken(ts));
            }
            struct Node *_node = Parse(filename);
            struct Block *inc_block = (struct Block*)_node->node_ptr;
            struct Node **ptr = inc_block->statement_list;
            while (*ptr != NULL) {
                block->statement_list = push_back(block->statement_list, *ptr);
                ptr++;
            }
            _free(inc_block->statement_list);
            TokenStream_NextToken(ts);
            continue;
        }
        struct Node *_statement = Syntax_ProcessStatement(ts);
        if (_statement) {
            block->statement_list = push_back(block->statement_list, _statement);
        }
    }
    if (TokenStream_GetToken(ts).type != TokenBraceClose) {
        SyntaxError("} expected after block", TokenStream_GetToken(ts));
    }
    node->line_end = TokenStream_GetToken(ts).line_end;
    node->position_end = TokenStream_GetToken(ts).position_end;
    return node;
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

int operation_priority(struct Token *operation) {
    if (operation->type == TokenMult ||
        operation->type == TokenDiv) {
        return 1;
    }
    if (operation->type == TokenPlus ||
        operation->type == TokenMinus) {
        return 2;
    }
    if (operation->type == TokenLess ||
        operation->type == TokenEqual) {
        return 3;
    }
    return 4;
}

struct Node *process_operation(struct Node ***primaries, struct Token ***operations) {
    struct Node *root = (struct Node*)_malloc(sizeof(struct Node));
    root->node_ptr = NULL;
    struct Token *token = get_back_token(*operations);
    int sz = get_size(*primaries);
    struct Node *left = *primaries[sz - 2];
    struct Node *right = *primaries[sz - 1];
    root->line_begin = left->line_begin;
    root->position_begin = left->position_begin;
    root->line_end = right->line_end;
    root->position_end = right->position_end;
    root->filename = left->filename;
    
    if (token->type == TokenPlus) {
        struct Addition *addition = (struct Addition*)_malloc(sizeof(struct Addition));
        addition->left = left;
        addition->right = right;
        root->node_ptr = addition;
        root->node_type = NodeAddition;
    }
    if (token->type == TokenMinus) {
        struct Subtraction *subtraction = (struct Subtraction*)_malloc(sizeof(struct Subtraction));
        subtraction->left = left;
        subtraction->right = right;
        root->node_ptr = subtraction;
        root->node_type = NodeSubtraction;
    }
    if (token->type == TokenMult) {
        struct Multiplication *multiplication = (struct Multiplication*)_malloc(sizeof(struct Multiplication));
        multiplication->left = left;
        multiplication->right = right;
        root->node_ptr = multiplication;
        root->node_type = NodeMultiplication;
    }
    if (token->type == TokenDiv) {
        struct Division *division = (struct Division*)_malloc(sizeof(struct Division));
        division->left = left;
        division->right = right;
        root->node_ptr = division;
        root->node_type = NodeDivision;
    }
    if (token->type == TokenLess) {
        struct Less *less = (struct Less*)_malloc(sizeof(struct Less));
        less->left = left;
        less->right = right;
        root->node_ptr = less;
        root->node_type = NodeLess;
    }
    if (token->type == TokenEqual) {
        struct Equal *equal = (struct Equal*)_malloc(sizeof(struct Equal));
        equal->left = left;
        equal->right = right;
        root->node_ptr = equal;
        root->node_type = NodeEqual;
    }

    if (root->node_ptr == NULL) {
        SyntaxError("Binary operator expected in expression", *token);
    }

    *primaries = pop_back(*primaries);
    *primaries = pop_back(*primaries);
    *primaries = push_back(*primaries, root);
    *operations = pop_back_token(*operations);
    return root;
}

struct Node *Syntax_ProcessExpression(struct TokenStream *ts) {
    struct Node **primaries = (struct Node**)_malloc(sizeof(struct Node*));
    primaries[0] = NULL;
    struct Token **operations = (struct Token**)_malloc(sizeof(struct Token*));
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
        operations = push_back_token(operations, &token);
        TokenStream_NextToken(ts);
        ParenthesisLevel++;
        CurrentState = State_ParenthesisOpen;
    }
    else if (next_is_operation(ts)) {
        struct Token token = TokenStream_GetToken(ts);
        operations = push_back_token(operations, &token);
        TokenStream_NextToken(ts);
        CurrentState = State_UnaryOperation;
    }
    else {
        primaries = push_back(primaries, Syntax_ProcessPrimary(ts));
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
            operations = push_back_token(operations, &token);
            TokenStream_NextToken(ts);
            ParenthesisLevel++;
            CurrentState = State_ParenthesisOpen;
        }
        else if (TokenStream_GetToken(ts).type == TokenParenthesisClose) {
            if (CurrentState != State_Identifier &&
                CurrentState != State_ParenthesisOpen) {
                SyntaxError("Unexpected ) in expression", TokenStream_GetToken(ts));
            }
            while (*operations != NULL && get_back_token(operations)->type != TokenParenthesisOpen) {
                process_operation(&primaries, &operations);
            }
            if (*operations == NULL || get_back_token(operations)->type != TokenParenthesisOpen) {
                SyntaxError("Unexpected ) in expression", TokenStream_GetToken(ts));
            }
            operations = pop_back_token(operations);
            TokenStream_NextToken(ts);
            ParenthesisLevel--;
            CurrentState = State_ParenthesisClose;
        }
        else if (next_is_operation(ts)) {
            if (CurrentState == State_UnaryOperation ||
                CurrentState == State_BinaryOperation ||
                CurrentState == State_ParenthesisOpen) {
                struct Token token = TokenStream_GetToken(ts);
                while (*operations != NULL &&
                    operation_priority(get_back_token(operations)) <= operation_priority(&token)) {
                    process_operation(&primaries, &operations);
                }
                operations = push_back_token(operations, &token);
                TokenStream_NextToken(ts);
                CurrentState = State_UnaryOperation;
            }
            else {
                struct Token token = TokenStream_GetToken(ts);
                while (*operations != NULL &&
                    operation_priority(get_back_token(operations)) <= operation_priority(&token)) {
                    process_operation(&primaries, &operations);
                }
                operations = push_back_token(operations, &token);
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
            primaries = push_back(primaries, Syntax_ProcessPrimary(ts));
            CurrentState = State_Identifier;
        }
    }

    while (*operations != NULL) {
        process_operation(&primaries, &operations);
    }

    if (get_size(primaries) != 1) {
        SyntaxError("Incorrect expression", TokenStream_GetToken(ts));
    }

    struct Node *res = primaries[0];
    _free(primaries);
    _free(operations);
    return res;
}

struct Node *Syntax_ProcessPrimary(struct TokenStream *ts) {
    struct Node *node = (struct Node*)_malloc(sizeof(struct Node));
    node->line_begin = TokenStream_GetToken(ts).line_begin;
    node->position_begin = TokenStream_GetToken(ts).position_begin;
    node->filename = TokenStream_GetToken(ts).filename;
    if (TokenStream_GetToken(ts).type == TokenDereference) {
        struct Dereference *_dereference = (struct Dereference*)_malloc(sizeof(struct Dereference));
        node->node_ptr = _dereference;
        node->node_type = NodeDereference;
        TokenStream_NextToken(ts);
        struct Node *_expression = Syntax_ProcessExpression(ts);
        _dereference->arg = _expression;
        node->line_end = _expression->line_end;
        node->position_end = _expression->position_end;
        return node;
    }
    if (TokenStream_GetToken(ts).type == TokenIdentifier) {
        struct Identifier *_identifier = (struct Identifier*)_malloc(sizeof(struct Identifier));
        node->node_ptr = _identifier;
        node->node_type = NodeIdentifier;
        _identifier->identifier = TokenStream_GetToken(ts).value_string;
        node->line_end = TokenStream_GetToken(ts).line_end;
        node->position_end = TokenStream_GetToken(ts).position_end;
        TokenStream_NextToken(ts);
        return node;
    }
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
    if (TokenStream_GetToken(ts).type == TokenAlloc) {
        struct Alloc *_alloc = (struct Alloc*)_malloc(sizeof(struct Alloc));
        node->node_ptr = _alloc;
        node->node_type = NodeAlloc;
        TokenStream_NextToken(ts);
        if (TokenStream_GetToken(ts).type != TokenParenthesisOpen) {
            SyntaxError("( expected in alloc expression", TokenStream_GetToken(ts));
        }
        TokenStream_NextToken(ts);
        _alloc->expression = Syntax_ProcessExpression(ts);
        if (TokenStream_GetToken(ts).type != TokenParenthesisClose) {
            SyntaxError(") expected in alloc expression", TokenStream_GetToken(ts));
        }
        node->line_end = TokenStream_GetToken(ts).line_end;
        node->position_end = TokenStream_GetToken(ts).position_end;
        TokenStream_NextToken(ts);
        return node;
    }
    SyntaxError("Identifier expected in primary expression", TokenStream_GetToken(ts));
}

struct Node *Syntax_ProcessStatement(struct TokenStream *ts) {
    if (TokenStream_GetToken(ts).type == TokenSemicolon) {
        TokenStream_NextToken(ts);
        return NULL;
    }
    if (TokenStream_GetToken(ts).type == TokenBraceOpen) {
        struct Node *node = Syntax_ProcessBlock(ts);
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
        node->filename = TokenStream_GetToken(ts).filename;
        node->line_end = TokenStream_GetToken(ts).line_end;
        node->position_end = TokenStream_GetToken(ts).position_end;
        _asm->code = TokenStream_GetToken(ts).value_string;
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
        node->filename = TokenStream_GetToken(ts).filename;
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
        struct Node *_block = Syntax_ProcessBlock(ts);
        _if->condition_list = push_back(_if->condition_list, _expression);
        _if->block_list = push_back(_if->block_list, _block);
        node->line_end = TokenStream_GetToken(ts).line_end;
        node->position_end = TokenStream_GetToken(ts).position_end;
        TokenStream_NextToken(ts);

        if (TokenStream_GetToken(ts).type == TokenElse) {
            TokenStream_NextToken(ts);
            if (TokenStream_GetToken(ts).type != TokenBraceOpen) {
                SyntaxError("{ expected in if block", TokenStream_GetToken(ts));
            }
            struct Node *_block = Syntax_ProcessBlock(ts);
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
        node->filename = TokenStream_GetToken(ts).filename;
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
        struct Node *_block = Syntax_ProcessBlock(ts);
        _while->condition = _expression;
        _while->block = _block;
        node->line_end = TokenStream_GetToken(ts).line_end;
        node->position_end = TokenStream_GetToken(ts).position_end;
        TokenStream_NextToken(ts);

        return node;
    }
    /*if (ts.GetToken().type == TokenType::Func) {
        std::shared_ptr <AST::FunctionDefinition> function_definition = std::make_shared <AST::FunctionDefinition> ();
        function_definition->line_begin = ts.GetToken().line_begin;
        function_definition->position_begin = ts.GetToken().position_begin;
        ts.NextToken();
        if (ts.GetToken().type == TokenType::Caret) {
            function_definition->external = true;
            ts.NextToken();
        }
        if (ts.GetToken().type != TokenType::Identifier) {
            throw AliasException("Identifier exprected in function definition", ts.GetToken());
        }
        function_definition->name = ts.GetToken().value_string;
        ts.NextToken();
        if (ts.GetToken().type == TokenType::BracketOpen) {
            ts.NextToken();
            while (true) {
                if (ts.GetToken().type == TokenType::BracketClose) {
                    ts.NextToken();
                    break;
                }
                if (ts.GetToken().type != TokenType::Identifier) {
                    throw AliasException("Idenfier expected in metavariable list", ts.GetToken());
                }
                function_definition->metavariables.push_back(ts.GetToken().value_string);
                ts.NextToken();
                if (ts.GetToken().type == TokenType::BracketClose) {
                    ts.NextToken();
                    break;
                }
                if (ts.GetToken().type != TokenType::Comma) {
                    throw AliasException(", expected in metavariables list", ts.GetToken());
                }
                ts.NextToken();
            }
        }
        if (ts.GetToken().type != TokenType::ParenthesisOpen) {
            throw AliasException("( expected in function definition", ts.GetToken());
        }
        ts.NextToken();
        std::shared_ptr <AST::FunctionSignature> function_signature = std::make_shared <AST::FunctionSignature> ();
        while (true) {
            if (ts.GetToken().type == TokenType::ParenthesisClose) {
                ts.NextToken();
                break;
            }
            if (ts.GetToken().type != TokenType::Identifier) {
                throw AliasException("Idenfier expected in argument list", ts.GetToken());
            }
            function_signature->identifiers.push_back(ts.GetToken().value_string);
            ts.NextToken();
            if (ts.GetToken().type != TokenType::Int && ts.GetToken().type != TokenType::Ptr) {
                throw AliasException("Type expected in argument list", ts.GetToken());
            }
            if (ts.GetToken().type == TokenType::Int) {
                function_signature->types.push_back(AST::Type::Int);
                ts.NextToken();
                if (ts.GetToken().type == TokenType::Const) {
                    function_signature->is_const.push_back(true);
                    ts.NextToken();
                }
                else {
                    function_signature->is_const.push_back(false);
                }
                function_signature->size_in.push_back(0);
                function_signature->size_out.push_back(0);
            }
            else {
                function_signature->types.push_back(AST::Type::Ptr);
                ts.NextToken();
                if (ts.GetToken().type == TokenType::Const) {
                    function_signature->is_const.push_back(true);
                    ts.NextToken();
                }
                else {
                    function_signature->is_const.push_back(false);
                }
                auto _expression1 = ProcessExpression(ts);
                if (ts.GetToken().type != TokenType::Colon) {
                    throw AliasException(": expected in argument list", ts.GetToken());
                }
                ts.NextToken();
                auto _expression2 = ProcessExpression(ts);
                function_signature->size_in.push_back(_expression1);
                function_signature->size_out.push_back(_expression2);
            }
            if (ts.GetToken().type == TokenType::ParenthesisClose) {
                ts.NextToken();
                break;
            }
            if (ts.GetToken().type != TokenType::Comma) {
                throw AliasException(", expected in argument list", ts.GetToken());
            }
            ts.NextToken();
        }
        function_definition->signature = function_signature;
        if (ts.GetToken().type != TokenType::BraceOpen) {
            throw AliasException("{ expected in function block", ts.GetToken());
        }
        std::shared_ptr <AST::Block> _block = ProcessBlock(ts);
        function_definition->body = _block;
        function_definition->line_end = ts.GetToken().line_end;
        function_definition->position_end = ts.GetToken().position_end;
        function_definition->filename = ts.GetToken().filename;
        ts.NextToken();

        return function_definition;
    }
    if (ts.GetToken().type == TokenType::Proto) {
        std::shared_ptr <AST::Prototype> prototype = std::make_shared <AST::Prototype> ();
        prototype->line_begin = ts.GetToken().line_begin;
        prototype->position_begin = ts.GetToken().position_begin;
        ts.NextToken();
        if (ts.GetToken().type != TokenType::Identifier) {
            throw AliasException("Identifier exprected in function prototype", ts.GetToken());
        }
        prototype->name = ts.GetToken().value_string;
        ts.NextToken();
        if (ts.GetToken().type == TokenType::BracketOpen) {
            ts.NextToken();
            while (true) {
                if (ts.GetToken().type == TokenType::BracketClose) {
                    ts.NextToken();
                    break;
                }
                if (ts.GetToken().type != TokenType::Identifier) {
                    throw AliasException("Idenfier expected in metavariable list", ts.GetToken());
                }
                prototype->metavariables.push_back(ts.GetToken().value_string);
                ts.NextToken();
                if (ts.GetToken().type == TokenType::BracketClose) {
                    ts.NextToken();
                    break;
                }
                if (ts.GetToken().type != TokenType::Comma) {
                    throw AliasException(", expected in metavariables list", ts.GetToken());
                }
                ts.NextToken();
            }
        }
        if (ts.GetToken().type != TokenType::ParenthesisOpen) {
            throw AliasException("( expected in function prototype", ts.GetToken());
        }
        ts.NextToken();
        std::shared_ptr <AST::FunctionSignature> function_signature = std::make_shared <AST::FunctionSignature> ();
        while (true) {
            if (ts.GetToken().type == TokenType::ParenthesisClose) {
                break;
            }
            if (ts.GetToken().type != TokenType::Identifier) {
                throw AliasException("Idenfier expected in argument list", ts.GetToken());
            }
            function_signature->identifiers.push_back(ts.GetToken().value_string);
            ts.NextToken();
            if (ts.GetToken().type != TokenType::Int && ts.GetToken().type != TokenType::Ptr) {
                throw AliasException("Type expected in argument list", ts.GetToken());
            }
            if (ts.GetToken().type == TokenType::Int) {
                function_signature->types.push_back(AST::Type::Int);
                ts.NextToken();
                function_signature->size_in.push_back(0);
                function_signature->size_out.push_back(0);
            }
            else {
                function_signature->types.push_back(AST::Type::Ptr);
                ts.NextToken();
                auto _expression1 = ProcessExpression(ts);
                if (ts.GetToken().type != TokenType::Colon) {
                    throw AliasException(": expected in argument list", ts.GetToken());
                }
                ts.NextToken();
                auto _expression2 = ProcessExpression(ts);
                function_signature->size_in.push_back(_expression1);
                function_signature->size_out.push_back(_expression2);
            }
            if (ts.GetToken().type == TokenType::ParenthesisClose) {
                break;
            }
            if (ts.GetToken().type != TokenType::Comma) {
                throw AliasException(", expected in argument list", ts.GetToken());
            }
            ts.NextToken();
        }
        prototype->signature = function_signature;                    
        prototype->line_end = ts.GetToken().line_end;
        prototype->position_end = ts.GetToken().position_end;
        prototype->filename = ts.GetToken().filename;
        ts.NextToken();

        return prototype;
    }*/
    if (TokenStream_GetToken(ts).type == TokenDef) {
        struct Node *node = (struct Node*)_malloc(sizeof(struct Node));
        struct Definition *definition = (struct Definition*)_malloc(sizeof(struct Definition));
        node->node_ptr = definition;
        node->node_type = NodeWhile;
        node->line_begin = TokenStream_GetToken(ts).line_begin;
        node->position_begin = TokenStream_GetToken(ts).position_begin;
        node->filename = TokenStream_GetToken(ts).filename;
        TokenStream_NextToken(ts);
        if (TokenStream_GetToken(ts).type != TokenIdentifier) {
            SyntaxError("Identifier expected in definition statement", TokenStream_GetToken(ts));
        }
        definition->identifier = TokenStream_GetToken(ts).value_string;
        TokenStream_NextToken(ts);
        if (TokenStream_GetToken(ts).type != TokenInt && TokenStream_GetToken(ts).type != TokenPtr) {
            SyntaxError("Type expected in definition statement", TokenStream_GetToken(ts));
        }
        if (TokenStream_GetToken(ts).type == TokenInt) {
            definition->type = TypeInt;
        }
        else {
            definition->type = TypePtr;
        }
        node->line_end = TokenStream_GetToken(ts).line_end;
        node->position_end = TokenStream_GetToken(ts).position_end;
        TokenStream_NextToken(ts);
        return node;
    }
    if (TokenStream_GetToken(ts).type == TokenAssume) {
        struct Node *node = (struct Node*)_malloc(sizeof(struct Node));
        struct Assumption *assumption = (struct Assumption*)_malloc(sizeof(struct Assumption));
        node->node_ptr = assumption;
        node->node_type = NodeAssumption;
        node->line_begin = TokenStream_GetToken(ts).line_begin;
        node->position_begin = TokenStream_GetToken(ts).position_begin;
        node->filename = TokenStream_GetToken(ts).filename;
        TokenStream_NextToken(ts);
        if (TokenStream_GetToken(ts).type != TokenParenthesisOpen) {
            SyntaxError("( expected in assume condition", TokenStream_GetToken(ts));
        }
        TokenStream_NextToken(ts);
        if (TokenStream_GetToken(ts).type != TokenIdentifier) {
            SyntaxError("Identifier expected in assume condition", TokenStream_GetToken(ts));
        }
        assumption->identifier = TokenStream_GetToken(ts).value_string;
        TokenStream_NextToken(ts);
        assumption->left = Syntax_ProcessExpression(ts);
        if (TokenStream_GetToken(ts).type != TokenColon) {
            SyntaxError(": expected in assume condition", TokenStream_GetToken(ts));
        }
        TokenStream_NextToken(ts);
        assumption->right = Syntax_ProcessExpression(ts);
        if (TokenStream_GetToken(ts).type != TokenParenthesisClose) {
            SyntaxError(") expected in assume condition", TokenStream_GetToken(ts));
        }
        TokenStream_NextToken(ts);
        struct Node *_statement = Syntax_ProcessStatement(ts);
        assumption->statement = _statement;
        node->line_end = _statement->line_end;
        node->position_end =_statement->position_end;
        return node;
    }
    if (TokenStream_GetToken(ts).type == TokenFree) {
        struct Node *node = (struct Node*)_malloc(sizeof(struct Node));
        struct Free *_free = (struct Free*)_malloc(sizeof(struct Free));
        node->node_ptr = _free;
        node->node_type = NodeFree;
        node->line_begin = TokenStream_GetToken(ts).line_begin;
        node->position_begin = TokenStream_GetToken(ts).position_begin;
        node->filename = TokenStream_GetToken(ts).filename;
        TokenStream_NextToken(ts);
        if (TokenStream_GetToken(ts).type != TokenParenthesisOpen) {
            SyntaxError("( expected in free statement", TokenStream_GetToken(ts));
        }
        TokenStream_NextToken(ts);
        _free->expression = Syntax_ProcessExpression(ts);
        if (TokenStream_GetToken(ts).type != TokenParenthesisClose) {
            SyntaxError(") expected in free expression", TokenStream_GetToken(ts));
        }
        node->line_end = TokenStream_GetToken(ts).line_end;
        node->position_end = TokenStream_GetToken(ts).position_end;
        TokenStream_NextToken(ts);
        return node;
    }
    if (TokenStream_GetToken(ts).type == TokenCall) {
        struct Node *node = (struct Node*)_malloc(sizeof(struct Node));
        struct FunctionCall *function_call = (struct FunctionCall*)_malloc(sizeof(struct FunctionCall));
        node->node_ptr = function_call;
        node->node_type = NodeFunctionCall;
        node->line_begin = TokenStream_GetToken(ts).line_begin;
        node->position_begin = TokenStream_GetToken(ts).position_begin;
        node->filename = TokenStream_GetToken(ts).filename;
        TokenStream_NextToken(ts);
        if (TokenStream_GetToken(ts).type != TokenIdentifier) {
            SyntaxError("Identifier expected in function call", TokenStream_GetToken(ts));
        }
        function_call->identifier = TokenStream_GetToken(ts).value_string;
        TokenStream_NextToken(ts);
        if (TokenStream_GetToken(ts).type == TokenBracketOpen) {
            TokenStream_NextToken(ts);
            while (true) {
                if (TokenStream_GetToken(ts).type == TokenBracketClose) {
                    TokenStream_NextToken(ts);
                    break;
                }
                if (TokenStream_GetToken(ts).type != TokenIdentifier) {
                    SyntaxError("Identifier expected in metavariable list", TokenStream_GetToken(ts));
                }
                function_call->metavariable_name = push_back_string(function_call->metavariable_name, TokenStream_GetToken(ts).value_string);
                TokenStream_NextToken(ts);
                if (TokenStream_GetToken(ts).type != TokenEqual) {
                    SyntaxError("= expected in metavariable list", TokenStream_GetToken(ts));
                }
                TokenStream_NextToken(ts);
                function_call->metavariable_value = push_back(function_call->metavariable_value, Syntax_ProcessExpression(ts));
                if (TokenStream_GetToken(ts).type == TokenBracketClose) {
                    TokenStream_NextToken(ts);
                    break;
                }
                if (TokenStream_GetToken(ts).type != TokenComma) {
                    SyntaxError(", expected in metavariables list", TokenStream_GetToken(ts));
                }
                TokenStream_NextToken(ts);
            }
        }
        if (TokenStream_GetToken(ts).type != TokenParenthesisOpen) {
            SyntaxError("( expected in function call", TokenStream_GetToken(ts));
        }
        TokenStream_NextToken(ts);
        while (true) {
            if (TokenStream_GetToken(ts).type == TokenParenthesisClose) {
                break;
            }
            if (TokenStream_GetToken(ts).type == TokenIdentifier) {
                function_call->arguments = push_back_string(function_call->arguments, TokenStream_GetToken(ts).value_string);
            }
            else {
                SyntaxError("Identifier expected in function call", TokenStream_GetToken(ts));
            }
            TokenStream_NextToken(ts);
            if (TokenStream_GetToken(ts).type == TokenParenthesisClose) {
                break;
            }
            if (TokenStream_GetToken(ts).type != TokenComma) {
                SyntaxError(", expectred in function call", TokenStream_GetToken(ts));
            }
            TokenStream_NextToken(ts);
        }
        node->line_end = TokenStream_GetToken(ts).line_end;
        node->position_end = TokenStream_GetToken(ts).position_end;
        TokenStream_NextToken(ts);
        return node;
    }
    if (TokenStream_GetToken(ts).type == TokenIdentifier) {
        struct Node *node = (struct Node*)_malloc(sizeof(struct Node));
        node->line_begin = TokenStream_GetToken(ts).line_begin;
        node->position_begin = TokenStream_GetToken(ts).position_begin;
        node->filename = TokenStream_GetToken(ts).filename;
        const char *identifier = TokenStream_GetToken(ts).value_string;
        TokenStream_NextToken(ts);
        if (TokenStream_GetToken(ts).type != TokenAssign && TokenStream_GetToken(ts).type != TokenMove) {
            SyntaxError(":= or <- expected in assignment or movement statement", TokenStream_GetToken(ts));
        }

        if (TokenStream_GetToken(ts).type == TokenAssign) {
            struct Assignment *assignment = (struct Assignment*)_malloc(sizeof(struct Assignment));
            node->node_ptr = assignment;
            node->node_type = NodeAssignment;
            assignment->identifier = identifier;
            TokenStream_NextToken(ts);
            assignment->value = Syntax_ProcessExpression(ts);
            node->line_end = assignment->value->line_end;
            node->position_end = assignment->value->position_end;
            return node;
        }
        else if (TokenStream_GetToken(ts).type == TokenMove) {
            TokenStream_NextToken(ts);
            if (TokenStream_GetToken(ts).type == TokenString) {
                struct MovementString *movement_string = (struct MovementString*)_malloc(sizeof(struct MovementString));
                node->node_ptr = movement_string;
                node->node_type = NodeMovementString;
                node->line_end = TokenStream_GetToken(ts).line_end;
                node->position_end = TokenStream_GetToken(ts).position_end;
                movement_string->identifier = identifier;
                movement_string->value = TokenStream_GetToken(ts).value_string;
                TokenStream_NextToken(ts);
                return node;
            }
            else {
                struct Movement *movement = (struct Movement*)_malloc(sizeof(struct Movement));
                node->node_ptr = movement;
                node->node_type = NodeMovement;
                movement->identifier = identifier;
                movement->value = Syntax_ProcessExpression(ts);
                node->line_end = movement->value->line_end;
                node->position_end = movement->value->position_end;
                return node;
            }
        }
    }
    SyntaxError("Statement expected", TokenStream_GetToken(ts));
}

struct Node *Syntax_Process(struct TokenStream *token_stream) {
    return Syntax_ProcessProgram(token_stream);
}
