#include <iostream>
#include <set>
#include <map>
#include "ast.h"
#include "validator.h"
#include "exception.h"

namespace AST {
std::map < std::string, std::vector < std::pair <int, int> > > states_log;

bool operator < (const State &a, const State &b) {
    return a.heap < b.heap;
}

bool operator == (const State &a, const State &b) {
    return a.heap == b.heap;
}

bool operator <(const FunctionSignatureEvaluated &a, const FunctionSignatureEvaluated &b) {
    return a.size_in < b.size_in || (a.size_in == b.size_in && a.size_out < b.size_out);
}

int getVariableIndex(std::string id, Node *node, VLContext &context) {
    for (int i = (int)context.variable_stack.size() - 1; i >= 0; i--) {
        if (context.variable_stack[i] == id) {
            return i;
        }
    }
    throw AliasException("Identifier was not declared in this scope", node);
}

void checkIdentifier(std::string id, Node *node, VLContext &context) {
    for (int i = (int)context.variable_stack.size() - 1; i >= 0; i--) {
        if (context.variable_stack[i] == id) {
            return;
        }
    }
    for (std::pair <std::string,int> p : context.metavariable_stack) {
        if (p.first == id) {
            return;
        }
    }
    throw AliasException("Identifier was not declared in this scope", node);
}

Type getVariableType(std::string id, Node *node, VLContext &context) {
    for (int i = (int)context.variable_stack.size() - 1; i >= 0; i--) {
        if (context.variable_stack[i] == id) {
            return context.variable_type_stack[i];
        }
    }
    throw AliasException("Identifier was not declared in this scope", node);
}

std::shared_ptr <FunctionSignature> getFunctionSignature(std::string id, Node *node, VLContext &context) {
    for (int i = (int)context.function_stack.size() - 1; i >= 0; i--) {
        if (context.function_stack[i] == id) {
            return context.function_signature_stack[i];
        }
    }
    throw AliasException("Identifier was not declared in this scope", node);
}

int getFunctionIndex(std::string id, Node *node, VLContext &context) {
    for (int i = (int)context.function_stack.size() - 1; i >= 0; i--) {
        if (context.function_stack[i] == id) {
            return i;
        }
    }
    throw AliasException("Identifier was not declared in this scope", node);
}

bool EvaluateExpression(std::shared_ptr <Expression> expression, VLContext context, int &result) {
    if (auto _identifier = std::dynamic_pointer_cast <AST::Identifier> (expression)) {
        for (std::pair <std::string,int> p : context.metavariable_stack) {
            if (p.first == _identifier->identifier) {
                result = p.second;
                return true;
            }
        }
        return false;
    }
    if (auto _integer = std::dynamic_pointer_cast <AST::Integer> (expression)) {
        result = _integer->value;
        return true;
    }
    if (auto _addition = std::dynamic_pointer_cast <AST::Addition> (expression)) {
        int left, right;
        bool l = EvaluateExpression(_addition->left, context, left);
        bool r = EvaluateExpression(_addition->right, context, right);
        if (l && r) {
            result = left + right;
            return true;
        }
        else {
            return false;
        }
    }
    if (auto _subtraction = std::dynamic_pointer_cast <AST::Subtraction> (expression)) {
        int left, right;
        bool l = EvaluateExpression(_subtraction->left, context, left);
        bool r = EvaluateExpression(_subtraction->right, context, right);
        if (l && r) {
            result = left - right;
            return true;
        }
        else {
            return false;
        }
    }
    if (auto _multiplication = std::dynamic_pointer_cast <AST::Multiplication> (expression)) {
        int left, right;
        bool l = EvaluateExpression(_multiplication->left, context, left);
        bool r = EvaluateExpression(_multiplication->right, context, right);
        if (l && r) {
            result = left * right;
            return true;
        }
        else {
            return false;
        }
    }
    if (auto _division = std::dynamic_pointer_cast <AST::Division> (expression)) {
        int left, right;
        bool l = EvaluateExpression(_division->left, context, left);
        bool r = EvaluateExpression(_division->right, context, right);
        if (l && r) {
            result = left / right;
            return true;
        }
        else {
            return false;
        }
    }
    if (auto _less = std::dynamic_pointer_cast <AST::Less> (expression)) {
        int left, right;
        bool l = EvaluateExpression(_less->left, context, left);
        bool r = EvaluateExpression(_less->right, context, right);
        if (l && r) {
            result = left < right;
            return true;
        }
        else {
            return false;
        }
    }
    if (auto _equal = std::dynamic_pointer_cast <AST::Equal> (expression)) {
        int left, right;
        bool l = EvaluateExpression(_equal->left, context, left);
        bool r = EvaluateExpression(_equal->right, context, right);
        if (l && r) {
            result = left == right;
            return true;
        }
        else {
            return false;
        }
    }
    return false;
}

std::shared_ptr <FunctionSignatureEvaluated> EvaluateFunctionSignature(std::shared_ptr <FunctionSignature> signature, VLContext context) {
    std::shared_ptr <FunctionSignatureEvaluated> _signature = std::make_shared <FunctionSignatureEvaluated> ();
    _signature->identifiers = signature->identifiers;
    _signature->types = signature->types;
    _signature->is_const = signature->is_const;
    for (auto expr : signature->size_in) {
        if (expr) {
            int value;
            bool good = EvaluateExpression(expr, context, value);
            if (!good) {
                throw AliasException("Could not evaluate compile time constant", expr.get());
            }
            if (value < 0) {
                throw AliasException("Pre condition size has to be non-negative", expr.get());
            }
            _signature->size_in.push_back(value);
        }
        else {
            _signature->size_in.push_back(0);
        }
    }
    for (auto expr : signature->size_out) {
        if (expr) {
            int value;
            bool good = EvaluateExpression(expr, context, value);
            if (!good) {
                throw AliasException("Could not evaluate compile time constant", expr.get());
            }
            if (value < 0) {
                throw AliasException("Post condition size has to be non-negative", expr.get());
            }
            _signature->size_out.push_back(value);
        }
        else {
            _signature->size_out.push_back(0);
        }
    }
    return _signature;
}

void ValidateFunctionDefinition(FunctionDefinition &function, VLContext &context) {
    VLContext _context = context;
    context = VLContext();
    context.function_stack = _context.function_stack;
    context.function_signature_stack = _context.function_signature_stack;
    context.function_pointer_stack = _context.function_pointer_stack;
    context.function_signature_validated = _context.function_signature_validated;
    context.metavariable_stack = _context.metavariable_stack;

    std::shared_ptr <FunctionSignatureEvaluated> signature = EvaluateFunctionSignature(function.signature, context);
    int n = (int)signature->identifiers.size();
    State state;
    for (int i = 0; i < n; i++) {
        if (signature->types[i] == Type::Ptr) {
            if (signature->size_in[i] == 0)
                state.heap.push_back({-1, 0});
            else {
                state.heap.push_back({(int)context.packet_size.size(), 0});
                context.packet_size.push_back(signature->size_in[i]);
            }
        }
        else {
            state.heap.push_back({-1, 0});
        }
    }
    context.states.insert(state);

    for (int i = 0; i < n; i++) {
        context.variable_stack.push_back(signature->identifiers[i]);
        context.variable_type_stack.push_back(signature->types[i]);
        context.variable_is_const_stack.push_back(signature->is_const[i]);
    }

    function.body->Validate(context);

    std::vector <int> packet_num(n, -2);
    for (State state : context.states) {
        for (int i = 0; i < n; i++) {
            if (signature->types[i] == Type::Int) continue;
            if (signature->size_out[i] == 0) {
                packet_num[i] = -1;
                if (state.heap[i].first != -1) {
                    throw AliasException("Function post condition failed", &function);
                }
            }
            else {
                if (state.heap[i].first == -1 || 
                    signature->is_const[i] && (context.packet_size[state.heap[i].first] - state.heap[i].second < signature->size_out[i]) ||
                    !signature->is_const[i] && (state.heap[i].second != 0 || context.packet_size[state.heap[i].first] < signature->size_out[i])) {
                    throw AliasException("Function post condition failed", &function);
                }
                if (packet_num[i] == -2) {
                    packet_num[i] = state.heap[i].first;
                }
                else if (state.heap[i].first != packet_num[i]) {
                    throw AliasException("Function post condition has several packets", &function);
                }
            }
        }
    }
    for (int i : packet_num) {
        if (i >= 0) {
            context.packet_size[i] = 0;
        }
    }
    for (int i : context.packet_size) {
        if (i != 0) {
            throw AliasException("Memory leak", &function);
        }
    }

    context = _context;
}

void printStates(std::set <State> states) {
    std::cout << "{" << std::endl;
    for (State state : states) {
        std::cout << "[ ";
        for (std::pair <int,int> p : state.heap) {
            std::cout << "(" << p.first << ", " << p.second << ") ";
        }
        std::cout << "]" << std::endl;
    }
    std::cout << "}" << std::endl;
    std::cout << "____________________" << std::endl;
}

void PrintStatesLog() {
    std::cout << "States" << std::endl;
    for (auto v : states_log) {
        std::cout << v.first << std::endl;
        std::cout << (int)v.second.size() << std::endl;
        for (std::pair <int, int> p : v.second) {
            std::cout << p.first << ":" << p.second << std::endl;
        }
    }
}

void checkLeak(Node *node, VLContext &context) {
    for (State state : context.states) {
        std::vector <bool> used(context.packet_size.size());
        for (std::pair <int,int> p : state.heap) {
            if (p.first != -1) {
                used[p.first] = true;
            }
        }
        for (int i = 0; i < (int)context.packet_size.size(); i++) {
            if (!used[i] && context.packet_size[i] != 0) {
                throw AliasException("Memory leak", node);
            }
        }
    }
}

void Validate(std::shared_ptr <Node> node) {
    VLContext context;
    context.states.insert(State());
    node->Validate(context);
}

void Block::Validate(VLContext &context) {
    size_t old_variable_stack_size = context.variable_stack.size();
    size_t old_function_stack_size = context.function_stack.size();

    for (auto i = statement_list.begin(); i != statement_list.end(); i++) {
        (*i)->Validate(context);
        states_log[(*i)->filename].push_back({(*i)->line_begin + 1, (int)context.states.size()});
    }

    std::set <State> _states;
    for (State state : context.states) {
        for (int i = 0; i < (int)context.variable_stack.size() - old_variable_stack_size; i++) {
            state.heap.pop_back();
        }
        _states.insert(state);
    }
    context.states = _states;
    checkLeak(this, context);
    while (context.variable_stack.size() > old_variable_stack_size) {
        context.variable_stack.pop_back();
        context.variable_type_stack.pop_back();
        context.variable_is_const_stack.pop_back();
    }
    while (context.function_stack.size() > old_function_stack_size) {
        context.function_stack.pop_back();
        context.function_signature_stack.pop_back();
        context.function_pointer_stack.pop_back();
        context.function_signature_validated.pop_back();
    }
}

void Asm::Validate(VLContext &context) {
}

void If::Validate(VLContext &context) {
    int value;
    bool good = EvaluateExpression(branch_list[0].first, context, value);
    if (good) {
        if (value != 0) {
            branch_list[0].second->Validate(context);
        }
        else if (else_body) {
            else_body->Validate(context);
        }
        return;
    }

    branch_list[0].first->Validate(context);
    std::set <State> _states = context.states;
    int old_cnt_packets1 = (int)context.packet_size.size();
    branch_list[0].second->Validate(context);
    int old_cnt_packets2 = (int)context.packet_size.size();
    std::vector <int> _packet_size = context.packet_size;
    
    for (int i = old_cnt_packets1; i < old_cnt_packets2; i++)
        context.packet_size[i] = 0;

    std::set <State> _states1 = context.states;
    context.states = _states;
    std::set <State> _states2 = context.states;
    if (else_body) {
        else_body->Validate(context);
        _states2 = context.states;
    }

    for (int i = old_cnt_packets1; i < old_cnt_packets2; i++)
        context.packet_size[i] = _packet_size[i];

    context.states.clear();
    for (State state : _states1)
        context.states.insert(state);
    for (State state : _states2)
        context.states.insert(state);

    checkLeak(this, context);
}

void While::Validate(VLContext &context) {
    int n_heap = (int)context.packet_size.size();
    int cnt = 0;
    while (true) {
        cnt++;
        if (cnt == 100) {
            throw AliasException("While loop check limit exceeded", this);
        }
        std::set <State> _states = context.states;
        block->Validate(context);
        if (n_heap != (int)context.packet_size.size()) {
            throw AliasException("Inexpected allocation in while loop", this);
        }
        bool add = false;
        for (State state : context.states) {
            if (_states.find(state) == _states.end())
                add = true;
            _states.insert(state);
        }
        if (!add)
            break;
        context.states = _states;
    }

    checkLeak(this, context);
}

void FunctionDefinition::Validate(VLContext &context) {
    context.function_stack.push_back(name);
    context.function_signature_stack.push_back(signature);
    context.function_pointer_stack.push_back(this);
    context.function_signature_validated.push_back({});

    if (name == "main" && external) {
        ValidateFunctionDefinition(*this, context);
    }
}

void Prototype::Validate(VLContext &context) {
    context.function_stack.push_back(name);
    context.function_signature_stack.push_back(signature);
    context.function_pointer_stack.push_back(nullptr);
    context.function_signature_validated.push_back({});
}

void Definition::Validate(VLContext &context) {
    context.variable_stack.push_back(identifier);
    context.variable_type_stack.push_back(type);
    context.variable_is_const_stack.push_back(false);

    std::set <State> _states;
    for (State state : context.states) {
        state.heap.push_back({-1, 0});
        _states.insert(state);
    }
    context.states = _states;
}

void Assignment::Validate(VLContext &context) {
    int index = getVariableIndex(identifier, this, context);
    if (context.variable_is_const_stack[index]) {
        throw AliasException("Const values can not be changed", this);
    }
    if (getVariableType(identifier, this, context) == Type::Ptr) {
        if (auto _alloc = std::dynamic_pointer_cast <AST::Alloc> (value)) {
            std::set <State> _states;
            for (State state : context.states) {
                state.heap[index] = {(int)context.packet_size.size(), 0};
                _states.insert(state);
            }
            context.states = _states;
            int value;
            bool good = EvaluateExpression(_alloc->expression, context, value);
            if (!good) {
                throw AliasException("Could not evaluate compile time constant", _alloc->expression.get());
            }
            if (value < 0) {
                throw AliasException("Alloc size has to be non negative", _alloc->expression.get());
            }
            context.packet_size.push_back(value);
        }
        else if (auto _addition = std::dynamic_pointer_cast <AST::Addition> (value)) {
            auto _identifier = std::dynamic_pointer_cast <AST::Identifier> (_addition->left);
            if (!_identifier) {
                throw AliasException("Identifier expected in left part of addition in right part of assignment", this);
            }
            int value;
            bool good = EvaluateExpression(_addition->right, context, value);
            if (!good) {
                throw AliasException("Could not evaluate compile time constant", _addition->right.get());
            }
            if (getVariableType(_identifier->identifier, this, context) == Type::Ptr) {
                int index2 = getVariableIndex(_identifier->identifier, this, context);
                std::set <State> _states;
                for (State state : context.states) {
                    if (state.heap[index2].first == -1) {
                        state.heap[index] = {-1, 0};
                    }
                    else {
                        state.heap[index] = {state.heap[index2].first, state.heap[index2].second + value};
                    }
                    _states.insert(state);
                }
                context.states = _states;
            }
            else {
                std::set <State> _states;
                for (State state : context.states) {
                    state.heap[index] = {-1, 0};
                    _states.insert(state);
                }
                context.states = _states;
            }
        }
        else {
            std::set <State> _states;
            for (State state : context.states) {
                state.heap[index] = {-1, 0};
                _states.insert(state);
            }
            context.states = _states;
        }
        checkLeak(this, context);
    }
    else {
        value->Validate(context);
    }
}

void Movement::Validate(VLContext &context) {
    if (getVariableType(identifier, this, context) == Type::Ptr) {
        int index = getVariableIndex(identifier, this, context);
        for (State state : context.states) {
            if (state.heap[index].first == -1 ||
                state.heap[index].second < 0 ||
                state.heap[index].second >= context.packet_size[state.heap[index].first]) {
                throw AliasException("Access violation", this);
            }
        }
    }
    else {
        throw AliasException("Pointer variable expected in left part of movement", this);
    }
    value->Validate(context);
}

void MovementString::Validate(VLContext &context) {
    if (getVariableType(identifier, this, context) == Type::Ptr) {
        int index = getVariableIndex(identifier, this, context);
        int length = ((int)value.size() + 3) / 4;
        for (State state : context.states) {
            if (state.heap[index].first == -1 ||
                state.heap[index].second < 0 ||
                state.heap[index].second + length - 1 >= context.packet_size[state.heap[index].first]) {
                throw AliasException("Access violation", this);
            }
        }
    }
    else {
        throw AliasException("Pointer variable expected in left part of movement", this);
    }
}

void Assumption::Validate(VLContext &context) {
    if (auto _assignment = std::dynamic_pointer_cast <AST::Assignment> (statement)) {
        std::string identifier1 = _assignment->identifier;
        if (auto _addition = std::dynamic_pointer_cast <AST::Addition> (_assignment->value)) {
            auto _identifier2 = std::dynamic_pointer_cast <AST::Identifier> (_addition->left);
            auto _identifier3 = std::dynamic_pointer_cast <AST::Identifier> (_addition->right);
            if (getVariableType(identifier, this, context) == Type::Ptr) {
                throw AliasException("Integer variable expected in assumption", this);
            }
            if (getVariableType(identifier1, this, context) == Type::Int) {
                throw AliasException("Pointer variable expected in left part of assignment", this);
            }
            if (!_identifier2 || getVariableType(_identifier2->identifier, this, context) == Type::Int) {
                throw AliasException("Pointer variable expected in left part of addition in right part of assignment", this);
            }
            if (!_identifier3 || getVariableType(_identifier3->identifier, this, context) == Type::Ptr) {
                throw AliasException("Integer variable expected in right part of addition in right part of assignment", this);
            }
            std::string identifier2 = _identifier2->identifier;
            std::string identifier3 = _identifier3->identifier;
            if (identifier != identifier3) {
                throw AliasException("Right part of addition in right part of assumption is not defined", this);
            }

            int index1 = getVariableIndex(identifier1, this, context);
            int index2 = getVariableIndex(identifier2, this, context);
            std::set <State> _states;
            for (State state : context.states) {
                if (state.heap[index2].first == -1) {
                    state.heap[index1] = {-1, 0};
                    _states.insert(state);
                }
                else {
                    int value;
                    bool good = EvaluateExpression(left, context, value);
                    if (!good) {
                        throw AliasException("Could not evaluate compile time constant", left.get());
                    }
                    state.heap[index1] = {state.heap[index2].first, state.heap[index2].second + value};
                    _states.insert(state);

                    good = EvaluateExpression(right, context, value);
                    if (!good) {
                        throw AliasException("Could not evaluate compile time constant", right.get());
                    }
                    state.heap[index1] = {state.heap[index2].first, state.heap[index2].second + value};
                    _states.insert(state);
                }
            }
            context.states = _states;
        }
        else {
            throw AliasException("Addition expected in right part of assignment", this);
        }
    }
    else {
        throw AliasException("Assignment expected in assumption", this);
    }
}

void Identifier::Validate(VLContext &context) {
    checkIdentifier(identifier, this, context);
}

void Integer::Validate(VLContext &context) {
}

void Alloc::Validate(VLContext &context) {
    throw AliasException("Unexpected alloc outside assignment", this);
}

void Free::Validate(VLContext &context) {
    if (auto _identifier = std::dynamic_pointer_cast <AST::Identifier> (arg)) {
        int index = getVariableIndex(_identifier->identifier, this, context);
        if (context.variable_is_const_stack[index]) {
            throw AliasException("Const values can not be changed", this);
        }
        int packet_id = -1;
        for (State state : context.states) {
            if (state.heap[index].first == -1 || state.heap[index].second != 0) {
                throw AliasException("Access violation", this);
            }
            if (packet_id == -1) {
                packet_id = state.heap[index].first;
            }
            if (packet_id != state.heap[index].first) {
                throw AliasException("Unpredictible free", this);
            }
        }
        context.packet_size[packet_id] = 0;
        std::set <State> _states;
        for (State state : context.states) {
            for (int i = 0; i < (int)state.heap.size(); i++) {
                if (state.heap[i].first == packet_id) {
                    state.heap[i] = {-1, 0};
                }
            }
            _states.insert(state);
        }
        context.states = _states;
    }
    else {
        throw AliasException("Identifier expected in free statement", this);
    }
}

void FunctionCall::Validate(VLContext &context) {
    std::shared_ptr <FunctionSignature> _signature = getFunctionSignature(identifier, this, context);

    std::vector < std::pair <std::string, int> > metavariable_stack;
    for (std::pair <std::string, std::shared_ptr <Expression>> p : metavariables) {
        int value;
        bool good = EvaluateExpression(p.second, context, value);
        if (!good) {
            throw AliasException("Could not evaluate compile time constant", p.second.get());
        }
        metavariable_stack.push_back({p.first, value});
    }
    swap(context.metavariable_stack, metavariable_stack);

    std::shared_ptr <FunctionSignatureEvaluated> signature = EvaluateFunctionSignature(_signature, context);

    int index = getFunctionIndex(identifier, this, context);
    if (context.function_signature_validated[index].find(*signature.get()) == context.function_signature_validated[index].end()){
        if(context.function_pointer_stack[index]) {
            ValidateFunctionDefinition(*context.function_pointer_stack[index], context);
        }
        context.function_signature_validated[index].insert(*signature.get());
    }

    context.metavariable_stack = metavariable_stack;

    if (signature->identifiers.size() != arguments.size()) {
        throw AliasException("Incorrect number of arguments in function call", this);
    }
    
    int n = (int)signature->identifiers.size();
    std::vector <int> packet_num(n, -2);

    for (int i = 0; i < n; i++) {
        if (signature->types[i] != getVariableType(arguments[i], this, context)) {
            throw AliasException("Incorrect type of argument in function call", this);
        }
        if (signature->types[i] == AST::Type::Ptr) {
            int index = getVariableIndex(arguments[i], this, context);
            for (State state : context.states) {
                if (signature->size_in[i] == 0) {
                    packet_num[i] = -1;
                    if (state.heap[index].first != -1) {
                        throw AliasException("Function pre condition failed", this);
                    }
                }
                else {
                    if (state.heap[index].first == -1 || 
                        signature->is_const[i] && (context.packet_size[state.heap[index].first] - state.heap[index].second < signature->size_in[i]) ||
                        !signature->is_const[i] && (state.heap[index].second != 0 || context.packet_size[state.heap[index].first] < signature->size_in[i])) {
                        throw AliasException("Function pre condition failed", this);
                    }
                    if (packet_num[i] == -2) {
                        packet_num[i] = state.heap[index].first;
                    }
                    else if (state.heap[index].first != packet_num[i]) {
                        throw AliasException("Function pre condition has several packets", this);
                    }
                }
            }
        }
    }

    std::vector <int> new_packet(n);
    for (int i = 0; i < n; i++) {
        if (signature->types[i] == Type::Ptr && !signature->is_const[i]) {
            if (packet_num[i] >= 0) {
                context.packet_size[packet_num[i]] = 0;
            }
            new_packet[i] = (int)context.packet_size.size();
            context.packet_size.push_back(signature->size_out[i]);
        }
    }

    std::set <State> _states;
    for (State state : context.states) {
        for (int i = 0; i < n; i++) {
            if (signature->types[i] == Type::Ptr && !signature->is_const[i]) {
                int index = getVariableIndex(arguments[i], this, context);
                for (int j = 0; j < (int)state.heap.size(); j++) {
                    if (j != index && packet_num[i] >= 0 && state.heap[j].first == packet_num[i]) {
                        state.heap[j] = {-1, 0};
                    }
                }
                if (signature->size_out[i] == 0) {
                    state.heap[i] = {-1, 0};
                }
                else {
                    state.heap[index] = {new_packet[i], 0};
                }
            }
        }
        _states.insert(state);
    }
    context.states = _states;
}

void Dereference::Validate(VLContext &context) {
    if (auto _identifier = std::dynamic_pointer_cast <AST::Identifier> (arg)) {
        if (getVariableType(_identifier->identifier, this, context) == Type::Ptr) {
            int index = getVariableIndex(_identifier->identifier, this, context);
            for (State state : context.states) {
                if (state.heap[index].first == -1 ||
                    state.heap[index].second < 0 ||
                    state.heap[index].second >= context.packet_size[state.heap[index].first]) {
                    throw AliasException("Access violation", this);
                }
            }
        }
        else {
            throw AliasException("Dereference operator has to be applied to pointer variable", this);
        }
    }
    else {
        throw AliasException("Identifier expected after dereference operator", this);
    }
}

void Addition::Validate(VLContext &context) {
    left->Validate(context);
    right->Validate(context);
}

void Subtraction::Validate(VLContext &context) {
    left->Validate(context);
    right->Validate(context);
}

void Multiplication::Validate(VLContext &context) {
    left->Validate(context);
    right->Validate(context);
}

void Division::Validate(VLContext &context) {
    left->Validate(context);
    right->Validate(context);
}

void Less::Validate(VLContext &context) {
    left->Validate(context);
    right->Validate(context);
}

void Equal::Validate(VLContext &context) {
    left->Validate(context);
    right->Validate(context);
}

}
