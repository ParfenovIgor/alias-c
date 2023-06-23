#include <iostream>
#include "ast.h"
#include "compile.h"
#include "settings.h"

namespace AST {

int findInLocal(std::string &identifier, CPContext &context) {
    for (int i = (int)context.variable_stack.size() - 1; i >= 0; i--) {
        if (context.variable_stack[i] == identifier) {
            return i;
        }
    }
    return -1;
}

int findInArguments(std::string &identifier, CPContext &context) {
    for (int i = 0; i < (int)context.variable_arguments.size(); i++) {
        if (context.variable_arguments[i] == identifier) {
            return i;
        }
    }
    return -1;
}

Type getVariableType(std::string id, Node *node, CPContext &context) {
    for (int i = (int)context.variable_stack.size() - 1; i >= 0; i--) {
        if (context.variable_stack[i] == id) {
            return context.variable_stack_type[i];
        }
    }
    for (int i = 0; i < (int)context.variable_arguments.size(); i++) {
        if (context.variable_arguments[i] == id) {
            return context.variable_arguments_type[i];
        }
    }
    std::cout << "Error: identifier not found" << std::endl;
    exit(1);
}

int findFunctionIndex(std::string &identifier, CPContext &context) {
    for (int i = (int)context.function_stack.size() - 1; i >= 0; i--) {
        if (context.function_stack[i].first == identifier) {
            return context.function_stack[i].second;
        }
    }
    std::cout << "Error: function identifier not found" << std::endl;
    exit(1);
}

int findPhase(std::string &identifier, CPContext &context) {
    int idx = findInLocal(identifier, context);
    if (idx != -1) {
        return -(idx + 1) * 4;
    }
    else {
        idx = findInArguments(identifier, context);
        if (idx == -1) {
            std::cout << "Error: identifier not found" << std::endl;
            exit(1);
        }
        return (idx + 2) * 4;
    }
}

void Compile(std::shared_ptr <Node> node, std::ostream &out) {
    out << "; " << node->filename << " " << node->line_begin + 1 << ":" << node->position_begin + 1 << " -> program\n";
    out << "global main\n";
    out << "extern malloc\n";
    out << "extern free\n";
    out << "section .text\n";
    if (Settings::GetTopMain()) {
        out << "main:\n";
    }
    out << "push ebp\n";
    out << "mov ebp, esp\n";
    CPContext context;
    node->Compile(out, context);
    out << "leave\n";
    out << "ret\n";
}

void Block::Compile(std::ostream &out, CPContext &context) {
    out << "; " << filename << " " << line_begin + 1 << ":" << position_begin + 1 << " -> block\n";
    size_t old_variable_stack_size = context.variable_stack.size();
    size_t old_function_stack_size = context.function_stack.size();
    for (auto i = statement_list.begin(); i != statement_list.end(); i++) {
        (*i)->Compile(out, context);
    }
    out << "add esp, " << 4 * (context.variable_stack.size() - old_variable_stack_size) << "\n";
    while (context.variable_stack.size() > old_variable_stack_size)
        context.variable_stack.pop_back();
    while (context.variable_stack_type.size() > old_variable_stack_size)
        context.variable_stack_type.pop_back();
    while (context.function_stack.size() > old_function_stack_size)
        context.function_stack.pop_back();
}

void Asm::Compile(std::ostream &out, CPContext &context) {
    out << "; " << filename << " " << line_begin + 1 << ":" << position_begin + 1 << " -> asm\n";
    out << code << "\n";
}

void If::Compile(std::ostream &out, CPContext &context) {
    out << "; " << filename << " " << line_begin + 1 << ":" << position_begin + 1 << " -> if\n";
    branch_list[0].first->Compile(out, context);
    int idx = context.branch_index;
    context.branch_index++;
    out << "cmp [esp - 4], dword 0\n";
    out << "je _if_else" << idx << "\n";
    branch_list[0].second->Compile(out, context);
    out << "jmp _if_end" << idx << "\n";
    out << "_if_else" << idx << ":\n";
    if (else_body) {
        else_body->Compile(out, context);
    }
    out << "_if_end" << idx << ":\n";
}

void While::Compile(std::ostream &out, CPContext &context) {
    out << "; " << filename << " " << line_begin + 1 << ":" << position_begin + 1 << " -> while\n";
    int idx = context.branch_index;
    context.branch_index++;
    out << "_while" << idx << ":\n";
    expression->Compile(out, context);
    out << "cmp [esp - 4], dword 0\n";
    out << "je _while_end" << idx << "\n";
    block->Compile(out, context);
    out << "jmp _while" << idx << "\n";
    out << "_while_end" << idx << ":\n";
}

void FunctionDefinition::Compile(std::ostream &out, CPContext &context) {
    std::string identifier, identifier_end;
    int index;
    if (external) {
        identifier = name;
        identifier_end = "_funend" + name;
        index = -1;
    }
    else {
        identifier = "_fun" + std::to_string(context.function_index);
        identifier_end = "_funend" + std::to_string(context.function_index);
        index = context.function_index;
        context.function_index++;
    }

    out << "; " << filename << " " << line_begin + 1 << ":" << position_begin + 1 << " -> function definition\n";
    if (external) {
        out << "global " << identifier << "\n";
    }
    out << "jmp " << identifier_end << "\n";
    out << identifier << ":\n";
    out << "push ebp\n";
    out << "mov ebp, esp\n";
    out << "sub esp, " << (signature->identifiers.size() + 2) * 4 << "\n";
    
    std::vector <std::string> variable_stack = context.variable_stack;
    std::vector <std::string> variable_arguments = context.variable_arguments;
    context.variable_stack.clear();
    context.variable_arguments.clear();
    context.variable_arguments_type.clear();
    context.function_stack.push_back({name, index});
    for (int i = 0; i < (int)metavariables.size(); i++) {
        context.variable_arguments.push_back(metavariables[i]);
        context.variable_arguments_type.push_back(Type::Int);
    }
    for (int i = 0; i < (int)signature->identifiers.size(); i++) {
        context.variable_arguments.push_back(signature->identifiers[i]);
        context.variable_arguments_type.push_back(signature->types[i]);
    }
    body->Compile(out, context);
    context.variable_stack = variable_stack;
    context.variable_arguments = variable_arguments;

    out << "leave\n";
    out << "ret\n";
    out << identifier_end << ":\n";
}

void Prototype::Compile(std::ostream &out, CPContext &context) {
    out << "; " << filename << " " << line_begin + 1 << ":" << position_begin + 1 << " -> prototype\n";
    out << "extern " << name << "\n";
    context.function_stack.push_back({name, -1});
}

void Definition::Compile(std::ostream &out, CPContext &context) {
    out << "; " << filename << " " << line_begin + 1 << ":" << position_begin + 1 << " -> definition\n";
    context.variable_stack.push_back(identifier);
    context.variable_stack_type.push_back(type);
    out << "sub esp, 4\n";
}

void Assignment::Compile(std::ostream &out, CPContext &context) {
    if (getVariableType(identifier, this, context) == Type::Ptr) {
        if (auto _addition = std::dynamic_pointer_cast <AST::Addition> (value)) {
            auto _identifier = std::dynamic_pointer_cast <AST::Identifier> (_addition->left);
            if (_identifier && getVariableType(_identifier->identifier, this, context) == Type::Ptr) {
                int line_begin = _addition->right->line_begin;
                int position_begin = _addition->right->position_begin;
                int line_end = _addition->right->line_end;
                int position_end = _addition->right->position_end;
                std::string filename = _addition->right->filename;

                auto _multiplication = std::make_shared <AST::Multiplication> ();
                _multiplication->line_begin = line_begin;
                _multiplication->position_begin = position_begin;
                _multiplication->line_end = line_end;
                _multiplication->position_end = position_end;
                _multiplication->filename = filename;

                auto _integer  = std::make_shared <AST::Integer> ();
                _integer->line_begin = line_begin;
                _integer->position_begin = position_begin;
                _integer->line_end = line_end;
                _integer->position_end = position_end;
                _integer->filename = filename;
                
                _multiplication->left = _addition->right;
                _addition->right = _multiplication;
                _integer->value = 4;
                _multiplication->right = _integer;
            }
        }
    }

    out << "; " << filename << " " << line_begin + 1 << ":" << position_begin + 1 << " -> assignment\n";
    value->Compile(out, context);
    int phase = findPhase(identifier, context);
    out << "mov eax, [esp - 4]\n";
    out << "mov [ebp + " << phase << "], eax\n";
}

void Movement::Compile(std::ostream &out, CPContext &context) {
    out << "; " << filename << " " << line_begin + 1 << ":" << position_begin + 1 << " -> movement\n";
    value->Compile(out, context);
    int phase = findPhase(identifier, context);
    out << "mov eax, [esp - 4]\n";
    out << "mov ebx, [ebp + " << phase << "]\n";
    out << "mov [ebx], eax\n";
}

void MovementString::Compile(std::ostream &out, CPContext &context) {
    out << "; " << filename << " " << line_begin + 1 << ":" << position_begin + 1 << " -> movement string\n";
    int idx = context.branch_index;
    context.branch_index++;
    out << "jmp _strbufend" << idx << "\n";
    out << "_strbuf" << idx << " db ";
    for (int i = 0; i < (int)value.size(); i++) {
        out << (int)value[i];
        if (i + 1 != (int)value.size()) {
            out << ",";
        }
        else {
            out << "\n";
        }
    }
    out << "_strbufend" << idx << ":\n";
    out << "mov esi, _strbuf" << idx << "\n";
    int phase = findPhase(identifier, context);
    out << "mov edi, [ebp + " << phase << "]\n";
    out << "mov ecx, " << (int)value.size() << "\n";
    out << "rep movsb\n";
}

void Assumption::Compile(std::ostream &out, CPContext &context) {
    out << "; " << filename << " " << line_begin + 1 << ":" << position_begin + 1 << " -> assumption\n";

    int ind_error = context.branch_index++;
    out << "jmp aftererror" << ind_error << "\n";
    std::string error = "Assumption fault in file " + filename + " on line " + std::to_string(line_begin + 1) + " position " + std::to_string(position_begin + 1);
    out << "error" << ind_error << " db \"" << error << "\", 0xA\n";
    out << "aftererror" << ind_error << ":\n";

    int phase = findPhase(identifier, context);
    int idx = context.branch_index++;
    left->Compile(out, context);
    out << "mov eax, [ebp + " << phase << "]\n";
    out << "sub eax, [esp - 4]\n";
    out << "jl " << "_set1_" << idx << "\n";
    out << "jmp _setend" << idx << "\n";
    out << "_set1_" << idx << ":\n";
    out << "mov eax, 4\n";
    out << "mov ebx, 1\n";
    out << "mov ecx, error" << ind_error << "\n";
    out << "mov edx, " << error.size() + 1 << "\n";
    out << "int 0x80\n";
    out << "mov eax, 1\n";
    out << "mov ebx, 1\n";
    out << "int 0x80\n";
    out << "_setend" << idx << ":\n";

    idx = context.branch_index++;
    right->Compile(out, context);
    out << "mov eax, [esp - 4]\n";
    out << "sub eax, [ebp + " << phase << "]\n";
    out << "jl " << "_set1_" << idx << "\n";
    out << "jmp _setend" << idx << "\n";
    out << "_set1_" << idx << ":\n";
    out << "mov eax, 4\n";
    out << "mov ebx, 1\n";
    out << "mov ecx, error" << ind_error << "\n";
    out << "mov edx, " << error.size() + 1 << "\n";
    out << "int 0x80\n";
    out << "mov eax, 1\n";
    out << "mov ebx, 1\n";
    out << "int 0x80\n";
    out << "_setend" << idx << ":\n";

    statement->Compile(out, context);
}

void Identifier::Compile(std::ostream &out, CPContext &context) {
    out << "; " << filename << " " << line_begin + 1 << ":" << position_begin + 1 << " -> identifier\n";
    int phase = findPhase(identifier, context);
    out << "mov eax, [ebp + " << phase << "]\n";
    out << "mov [esp - 4], eax\n";
}

void Integer::Compile(std::ostream &out, CPContext &context) {
    out << "; " << filename << " " << line_begin + 1 << ":" << position_begin + 1 << " -> integer\n";
    out << "mov [esp - 4], dword " << value << "\n";
}

void Alloc::Compile(std::ostream &out, CPContext &context) {
    int line_begin = expression->line_begin;
    int position_begin = expression->position_begin;
    int line_end = expression->line_end;
    int position_end = expression->position_end;
    std::string filename = expression->filename;

    auto _multiplication = std::make_shared <AST::Multiplication> ();
    _multiplication->line_begin = line_begin;
    _multiplication->position_begin = position_begin;
    _multiplication->line_end = line_end;
    _multiplication->position_end = position_end;
    _multiplication->filename = filename;

    auto _integer  = std::make_shared <AST::Integer> ();
    _integer->line_begin = line_begin;
    _integer->position_begin = position_begin;
    _integer->line_end = line_end;
    _integer->position_end = position_end;
    _integer->filename = filename;

    _integer->value = 4;
    _multiplication->left = expression;
    _multiplication->right = _integer;
    expression = _multiplication;

    out << "; " << filename << " " << line_begin + 1 << ":" << position_begin + 1 << " -> alloc\n";
    expression->Compile(out, context);
    out << "push dword [esp - 4]\n";
    out << "call malloc\n";
    out << "add esp, 4\n";
    out << "mov [esp - 4], eax\n";
}

void Free::Compile(std::ostream &out, CPContext &context) {
    out << "; " << filename << " " << line_begin + 1 << ":" << position_begin + 1 << " -> free\n";
    arg->Compile(out, context);
    out << "push dword [esp - 4]\n";
    out << "call free\n";
    out << "add esp, 4\n";
}

void FunctionCall::Compile(std::ostream &out, CPContext &context) {
    out << "; " << filename << " " << line_begin + 1 << ":" << position_begin + 1 << " -> function call\n";
    for (int i = (int)arguments.size() - 1; i >= 0; i--) {
        int phase = findPhase(arguments[i], context);
        out << "push dword [ebp + " << phase << "]\n";
    }
    for (int i = (int)metavariables.size() - 1; i >= 0; i--) {
        metavariables[i].second->Compile(out, context);
        out << "push dword [esp - 4]\n";
    }
    int idx = findFunctionIndex(identifier, context);
    if (idx == -1) {
        out << "call " << identifier << "\n";
    }
    else {
        out << "call _fun" << idx << "\n";
    }
    out << "add esp, " << (int)(arguments.size() + metavariables.size()) * 4 << "\n";
    for (int i = (int)arguments.size() - 1; i >= 0; i--) {
        int phase = findPhase(arguments[i], context);
        out << "mov eax, [esp - " << (((int)arguments.size() - i) * 4) << "]\n";
        out << "mov [ebp + " << phase << "], eax\n";
    }
}

void Dereference::Compile(std::ostream &out, CPContext &context) {
    out << "; " << filename << " " << line_begin + 1 << ":" << position_begin + 1 << " -> dereference\n";
    arg->Compile(out, context);
    out << "mov eax, [esp - 4]\n";
    out << "mov ebx, [eax]\n";
    out << "mov [esp - 4], ebx\n";
}

void Addition::Compile(std::ostream &out, CPContext &context) {
    out << "; " << filename << " " << line_begin + 1 << ":" << position_begin + 1 << " -> addition\n";
    left->Compile(out, context);
    out << "sub esp, 4\n";
    context.variable_stack.push_back("__junk");
    right->Compile(out, context);
    out << "add esp, 4\n";
    context.variable_stack.pop_back();
    out << "mov eax, [esp - 4]\n";
    out << "add eax, [esp - 8]\n";
    out << "mov [esp - 4], eax\n";
}

void Subtraction::Compile(std::ostream &out, CPContext &context) {
    out << "; " << filename << " " << line_begin + 1 << ":" << position_begin + 1 << " -> subtraction\n";
    left->Compile(out, context);
    out << "sub esp, 4\n";
    context.variable_stack.push_back("__junk");
    right->Compile(out, context);
    out << "add esp, 4\n";
    context.variable_stack.pop_back();
    out << "mov eax, [esp - 4]\n";
    out << "sub eax, [esp - 8]\n";
    out << "mov [esp - 4], eax\n";
}

void Multiplication::Compile(std::ostream &out, CPContext &context) {
    out << "; " << filename << " " << line_begin + 1 << ":" << position_begin + 1 << " -> multiplication\n";
    left->Compile(out, context);
    out << "sub esp, 4\n";
    context.variable_stack.push_back("__junk");
    right->Compile(out, context);
    out << "add esp, 4\n";
    context.variable_stack.pop_back();
    out << "mov eax, [esp - 4]\n";
    out << "mov edx, [esp - 8]\n";
    out << "mul edx\n";
    out << "mov [esp - 4], eax\n";
}

void Division::Compile(std::ostream &out, CPContext &context) {
    out << "; " << filename << " " << line_begin + 1 << ":" << position_begin + 1 << " -> division\n";
    left->Compile(out, context);
    out << "sub esp, 4\n";
    context.variable_stack.push_back("__junk");
    right->Compile(out, context);
    out << "add esp, 4\n";
    context.variable_stack.pop_back();
    out << "mov eax, [esp - 4]\n";
    out << "mov edx, 0\n";
    out << "div dword [esp - 8]\n";
    out << "mov [esp - 4], eax\n";
}

void Less::Compile(std::ostream &out, CPContext &context) {
    out << "; " << filename << " " << line_begin + 1 << ":" << position_begin + 1 << " -> less\n";
    left->Compile(out, context);
    out << "sub esp, 4\n";
    context.variable_stack.push_back("__junk");
    right->Compile(out, context);
    out << "add esp, 4\n";
    context.variable_stack.pop_back();
    out << "mov eax, [esp - 4]\n";
    out << "sub eax, [esp - 8]\n";
    int idx = context.branch_index++;
    out << "jl " << "_set1_" << idx << "\n";
    out << "mov [esp - 4], dword 0\n";
    out << "jmp _setend" << idx << "\n";
    out << "_set1_" << idx << ":\n";
    out << "mov [esp - 4], dword 1\n";
    out << "_setend" << idx << ":\n";
}

void Equal::Compile(std::ostream &out, CPContext &context) {
    out << "; " << filename << " " << line_begin + 1 << ":" << position_begin + 1 << " -> equal\n";
    left->Compile(out, context);
    out << "sub esp, 4\n";
    context.variable_stack.push_back("__junk");
    right->Compile(out, context);
    out << "add esp, 4\n";
    context.variable_stack.pop_back();
    out << "mov eax, [esp - 4]\n";
    out << "sub eax, [esp - 8]\n";
    int idx = context.branch_index++;
    out << "jz " << "_set1_" << idx << "\n";
    out << "mov [esp - 4], dword 0\n";
    out << "jmp _setend" << idx << "\n";
    out << "_set1_" << idx << ":\n";
    out << "mov [esp - 4], dword 1\n";
    out << "_setend" << idx << ":\n";
}

}