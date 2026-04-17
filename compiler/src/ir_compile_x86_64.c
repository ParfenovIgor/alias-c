#include <ir_compile_x86_64.h>
#include <stdio.h>
#include <string.h>
#include <posix.h>

int min(int a, int b) {
    if (a <= b) return a;
    return b;
}

const char *get_reg(enum Register reg) {
    switch (reg) {
        case REGNONE: {
            _panic("Value is not in register");
        }
        case RAX: return "rax";
        case RBX: return "rbx";
        case RCX: return "rcx";
        case RDX: return "rdx";
        case RDI: return "rdi";
        case RSI: return "rsi";
        case RBP: return "rbp";
        case RSP: return "rsp";
        case R8 : return "r8";
        case R9 : return "r9";
        case R10: return "r10";
        case R11: return "r11";
        case R12: return "r12";
        case R13: return "r13";
        case R14: return "r14";
        case R15: return "r15";
    }
}

const char *get_reg_byte(enum Register reg) {
    switch (reg) {
        case REGNONE: {
            _panic("Value is not in register");
        }
        case RAX: return "al";
        case RBX: return "bl";
        case RCX: return "cl";
        case RDX: return "dl";
        case RDI: return "dil";
        case RSI: return "sil";
        case RBP: return "blp";
        case RSP: return "spl";
        case R8 : return "r8b";
        case R9 : return "r9b";
        case R10: return "r10b";
        case R11: return "r11b";
        case R12: return "r12b";
        case R13: return "r13b";
        case R14: return "r14b";
        case R15: return "r15b";
    }
}

void to_reg(struct IRNode *node, enum Register reg, int fd_text) {
    if (node->spill) {
        if (type_size(node->type) == 1) {
            _fputs2(fd_text, "    mov ", get_reg_byte(reg));
        }
        if (type_size(node->type) == 8) {
            _fputs2(fd_text, "    mov ", get_reg(reg));
        }
        _fputsi(fd_text, ", [rbp + ", node->stack_phase, "]\n");
        node->reg = reg;
    }
    else if (node->node_type == IRNodeGlobal) {
        struct IRGlobal *global = node->node_ptr;
        if (type_size(node->type) == 1) {
            _fputs2(fd_text, "    mov ", get_reg_byte(reg));
        }
        if (type_size(node->type) == 8) {
            _fputs2(fd_text, "    mov ", get_reg(reg));
        }
        _fputs3(fd_text, ", ", global->name, "\n");
        node->reg = reg;
    }
    else if (node->node_type == IRNodeConst) {
        struct IRConst *_const = node->node_ptr;
        if (type_size(node->type) == 1) {
            _fputs2(fd_text, "    mov ", get_reg_byte(reg));
        }
        if (type_size(node->type) == 8) {
            _fputs2(fd_text, "    mov ", get_reg(reg));
        }
        _fputsi(fd_text, ", ", _const->value, "\n");
        node->reg = reg;
    }
}

void to_reg_const(struct IRNode *node, enum Register reg, int fd_text) {
    if (node->node_type == IRNodeConst) {
        _fputs2(fd_text, "    mov ", get_reg(reg));
        _fputsi(fd_text, ", [rbp + ", node->stack_phase, "]\n");
        node->reg = reg;
    }
}

void from_reg(struct IRNode *node, int fd_text) {
    if (node->spill) {
        _fputsi(fd_text, "    mov [rbp + ", node->stack_phase, "], ");
        if (type_size(node->type) == 1) {
            _fputs2(fd_text, get_reg_byte(node->reg), "\n");
        }
        if (type_size(node->type) == 8) {
            _fputs2(fd_text, get_reg(node->reg), "\n");
        }
        node->reg = REGNONE;
    }
    else if (node->node_type == IRNodeGlobal) {
        node->reg = REGNONE;
    }
    else if (node->node_type == IRNodeConst) {
        node->reg = REGNONE;
    }
}

void from_reg_const(struct IRNode *node) {
    if (node->node_type == IRNodeConst) {
        node->reg = REGNONE;
    }
}

void free_reg(struct IRNode *node) {
    if (node->spill) {
        node->reg = REGNONE;
    }
    else if (node->node_type == IRNodeGlobal) {
        node->reg = REGNONE;
    }
    else if (node->node_type == IRNodeConst) {
        node->reg = REGNONE;
    }
}

void ir_compile_value_x86_64(struct Vector *values_list, struct IRNode *value, int fd_text) {
    if (value->reg != REGNONE) {
        switch (value->reg) {
            case RAX: _fputs(fd_text, "rax"); break;
            case RBX: _fputs(fd_text, "rbx"); break;
            case RCX: _fputs(fd_text, "rcx"); break;
            case RDX: _fputs(fd_text, "rdx"); break;
            case RDI: _fputs(fd_text, "rdi"); break;
            case RSI: _fputs(fd_text, "rsi"); break;
            case RBP: _fputs(fd_text, "rbp"); break;
            case RSP: _fputs(fd_text, "rsp"); break;
            case R8 : _fputs(fd_text, "r8"); break;
            case R9 : _fputs(fd_text, "r9"); break;
            case R10: _fputs(fd_text, "r10"); break;
            case R11: _fputs(fd_text, "r11"); break;
            case R12: _fputs(fd_text, "r12"); break;
            case R13: _fputs(fd_text, "r13"); break;
            case R14: _fputs(fd_text, "r14"); break;
            case R15: _fputs(fd_text, "r15"); break;
        }
    }
    else if (value->node_type == IRNodeConst) {
        struct IRConst *_value = value->node_ptr;
        _fputi(fd_text, _value->value);
    }
    else if (value->node_type == IRNodeGlobal) {
        struct IRGlobal *_value = value->node_ptr;
        if (!(value->type->node_type == TypeNodeFunction && value->type->degree == 0)) {
            _fputs3(fd_text, "", _value->name, "");
        }
        else {
            _fputs(fd_text, _value->name);
        }
    }
    else {
        if (value->spill) {
            _fputsi(fd_text, "rbp + ", value->stack_phase, "");
        }
    }
}

void ir_compile_block_x86_64(struct Vector *blocks_list, struct IRBlock *block, int fd_text) {
    int sz = vsize(blocks_list);
    int res = -1;
    for (int i = 0; i < sz; i++) {
        if (blocks_list->ptr[i] == block) {
            res = i;
            break;
        }
    }

    if (res == -1) {
        res = sz;
        vpush(blocks_list, block);
    }

    _fputsi(fd_text, "._b", res, "");
}

void ir_compile_binary_instr_x86_64(const char *instr, struct IRNode *left, struct IRNode *right, int fd_text) {
    _fputs3(fd_text, "    ", instr, " qword ");
    if (left->reg != REGNONE) {
        _fputs(fd_text, get_reg(left->reg));
    }
    else {
        if (left->node_type == IRNodeConst) {
            struct IRConst *_const = left->node_ptr;
            _fputi(fd_text, _const->value);
        }
        else if(left->node_type == IRNodeGlobal) {
            struct IRGlobal *_value = left->node_ptr;
            _fputs(fd_text, _value->name);
        }
        else {
            _fputsi(fd_text, "[rbp + ", left->stack_phase, "]");
        }
    }
    _fputs(fd_text, ", ");
    if (right->reg != REGNONE) {
        _fputs(fd_text, get_reg(right->reg));
    }
    else {
        if (right->node_type == IRNodeConst) {
            struct IRConst *_const = right->node_ptr;
            _fputi(fd_text, _const->value);
        }
        else if(right->node_type == IRNodeGlobal) {
            struct IRGlobal *_value = right->node_ptr;
            _fputs(fd_text, _value->name);
        }
        else {
            _fputsi(fd_text, "[rbp + ", right->stack_phase, "]");
        }
    }
    _fputs(fd_text, "\n");
}

void ir_compile_binary_instr2_x86_64(const char *instr, const char *left, struct IRNode *right, int fd_text) {
    _fputs3(fd_text, "    ", instr, " qword ");
    _fputs2(fd_text, left, ", ");
    if (right->reg != REGNONE) {
        _fputs(fd_text, get_reg(right->reg));
    }
    else {
        if (right->node_type == IRNodeConst) {
            struct IRConst *_const = right->node_ptr;
            _fputi(fd_text, _const->value);
        }
        else if(right->node_type == IRNodeGlobal) {
            struct IRGlobal *_value = right->node_ptr;
            _fputs(fd_text, _value->name);
        }
        else {
            _fputsi(fd_text, "[rbp + ", right->stack_phase, "]");
        }
    }
    _fputs(fd_text, "\n");
}

void ir_compile_binary_instr3_x86_64(const char *instr, struct IRNode *left, const char *right, int fd_text) {
    _fputs3(fd_text, "    ", instr, " qword ");
    if (left->reg != REGNONE) {
        _fputs(fd_text, get_reg(left->reg));
    }
    else {
        if (left->node_type == IRNodeConst) {
            struct IRConst *_const = left->node_ptr;
            _fputi(fd_text, _const->value);
        }
        else if(left->node_type == IRNodeGlobal) {
            struct IRGlobal *_value = left->node_ptr;
            _fputs(fd_text, _value->name);
        }
        else {
            _fputsi(fd_text, "[rbp + ", left->stack_phase, "]");
        }
    }
    _fputs2(fd_text, ", ", right);
    _fputs(fd_text, "\n");
}

void ir_compile_phi_x86_64(struct Vector *values_list, struct IRBlock *block, struct IRBlock *succ_block, int fd_text) {
    int sz_insts = vsize(&succ_block->value_list);
    for (int i = 0; i < sz_insts; i++) {
        struct IRNode *value = succ_block->value_list.ptr[i];
        if (value->node_type == IRNodePhi) {
            struct IRPhi *phi = value->node_ptr;
            int sz_args = vsize(&phi->values);
            for (int j = 0; j < sz_args; j++) {
                if (block == phi->blocks.ptr[j] && value != phi->values.ptr[j]) {
                    _fputs(fd_text, "; PHI\n");
                    struct IRNode *value2 = phi->values.ptr[j];
                    if (value->spill && value2->spill) to_reg(value, RAX, fd_text);
                    ir_compile_binary_instr_x86_64("mov", value, value2, fd_text);
                    if (value->spill && value2->spill) from_reg(value, fd_text);
                    _fputs(fd_text, "; PHI_END\n");
                }
            }
        }
    }
}

void ir_compile_x86_64(struct IRBuilder *builder, const char *filename_compile_output) {
    int fd[2];
    posix_pipe(fd);
    int fd_text = fd[1];
    int fd_text_out = fd[0];

    _fputs(fd_text, "section .data\n");

    int sz_globalvar = vsize(&builder->globalvar_list);
    for (int i = 0; i < sz_globalvar; i++) {
        struct IRGlobalVar *globalvar = builder->globalvar_list.ptr[i];
        if (globalvar->type == IRGlobalVarFunction) continue;
        _fputs(fd_text, globalvar->name);
        _fputs(fd_text, ": ");
        if (globalvar->type == IRGlobalVarInt) {
            int value = (long)globalvar->value;
            _fputsi(fd_text, "dq ", value, "\n");
        }
        if (globalvar->type == IRGlobalVarString) {
            const char *value = globalvar->value;
            _fputs(fd_text, "db ");
            for (int i = 0; i < _strlen(value); i++) {
                _fputi(fd_text, value[i]);
                _fputs(fd_text, ", ");
            }
            _fputs(fd_text, "0\n");
        }
    }
    _fputs(fd_text, "\n\nsection .text\n");

    int sz_functions = vsize(&builder->function_list);

    for (int i = 0; i < sz_functions; i++) {
        struct IRFunction *function = builder->function_list.ptr[i];
        struct Vector values_list = vnew();
        struct Vector blocks_list = vnew();

        enum Register cur_register = R8;
        int cur_stack_phase = -32;

        bool have_call = false;
        int sz_blocks = vsize(&function->block_list);
        if (!sz_blocks) {
            _fputs3(fd_text, "extern ", function->name_generate, "\n");
            continue;
        }
        for (int j = 0; j < sz_blocks; j++) {
            struct IRBlock *block = function->block_list.ptr[j];
            int sz_insts = vsize(&block->value_list);
            for (int k = 0; k < sz_insts; k++) {
                struct IRNode *value = block->value_list.ptr[k];
                enum IRNodeType type = value->node_type;
                if (type == IRNodeCall) {
                    have_call = true;
                }
            }
        }
        
        _fputs3(fd_text, "global ", function->name_generate, "\n");
        _fputs2(fd_text, function->name_generate, ":\n");

        int sz_args = vsize(&function->arg_list);
        for (int j = 0; j < sz_args; j++) {
            struct IRNode *arg = function->arg_list.ptr[j];
            if (have_call) {
                if (j == 0) {
                    _fputs(fd_text, "    mov r8, rdi\n");
                }
                if (j == 1) {
                    _fputs(fd_text, "    mov r9, rsi\n");
                }
                if (j == 2) {
                    _fputs(fd_text, "    mov r10, rdx\n");
                }
                if (j == 3) {
                    _fputs(fd_text, "    mov r11, rcx\n");
                }
                arg->reg = cur_register;
                cur_register++;
            }
            else {
                if (j == 0) arg->reg = RDI;
                if (j == 1) arg->reg = RSI;
                if (j == 2) arg->reg = RDX;
                if (j == 3) arg->reg = RCX;
            }
        }
        
        _fputs(fd_text, "    push rbp\n");
        _fputs(fd_text, "    mov rbp, rsp\n");

        for (int j = 0; j < sz_blocks; j++) {
            struct IRBlock *block = function->block_list.ptr[j];

            int sz_insts = vsize(&block->value_list);
            for (int k = 0; k < sz_insts; k++) {
                struct IRNode *value = block->value_list.ptr[k];
                enum IRNodeType type = value->node_type;

                if (type != IRNodeConst && 
                    type != IRNodeAlloca && type != IRNodeStore && 
                    type != IRNodeBr && type != IRNodeCondBr && type != IRNodeRet &&
                    !(type == IRNodeCall && value->type->size == 0)) {
                    if (cur_register <= R15) {
                        value->reg = cur_register;
                        cur_register++;
                    }
                    else {
                        value->reg = REGNONE;
                        value->spill = true;
                        value->stack_phase = cur_stack_phase - type_size(value->type);
                        cur_stack_phase -= type_size(value->type);
                    }
                }
                if (type == IRNodeAlloca) {
                    value->spill = true;
                    struct IRAlloca *value_alloca = (struct IRAlloca*)value->node_ptr;
                    int data_phase = cur_stack_phase - value_alloca->size;
                    int alloca_phase = data_phase - type_size(value->type);
                    cur_stack_phase = alloca_phase;
                    _fputsi(fd_text, "    lea rax, [rbp + ", data_phase, "]\n");
                    _fputsi(fd_text, "    mov [rbp + ", alloca_phase, "], rax\n");
                    value->stack_phase = alloca_phase;
                }
            }
        }

        for (enum Register reg = R12; reg <= R15; reg++) {
            _fputs3(fd_text, "    push ", get_reg(reg), "\n");
        }
        _fputsi(fd_text, "    add rsp, ", cur_stack_phase, "\n");

        for (int j = 0; j < sz_blocks; j++) {
            struct IRBlock *block = function->block_list.ptr[j];
            ir_compile_block_x86_64(&blocks_list, block, fd_text);
            _fputs(fd_text, ":\n");

            int sz_insts = vsize(&block->value_list);
            for (int k = 0; k < sz_insts; k++) {
                struct IRNode *value = block->value_list.ptr[k];
                enum IRNodeType type = value->node_type;

                if (type == IRNodeConst || type == IRNodeAlloca) continue;

                if (type == IRNodePhi) {
                    continue;
                }
                else if (type == IRNodeGEP) {
                    _fputs(fd_text, "; GEP\n");
                    struct IRGEP *gep = value->node_ptr;
                    
                    to_reg(value, RDX, fd_text);
                    to_reg(gep->base, RCX, fd_text);
                    to_reg(gep->index, RAX, fd_text);
                    if (gep->size == 1 || gep->size == 2 || gep->size == 4 || gep->size == 8 || gep->size == 16) {
                        _fputs3(fd_text, "    lea ", get_reg(value->reg), ", [");
                        _fputs3(fd_text, get_reg(gep->base->reg), " + ", get_reg(gep->index->reg));
                        _fputsi(fd_text, " * ", gep->size, "]\n");
                    }
                    else {
                        _fputs3(fd_text, "    mov rax, ", get_reg(gep->index->reg), "\n");
                        _fputsi(fd_text, "    mov rdi, ", gep->size, "\n");
                        _fputs(fd_text, "    mul rdi\n");
                        _fputs3(fd_text, "    lea ", get_reg(value->reg), ", [");
                        _fputs2(fd_text, get_reg(gep->base->reg), " + rax]\n");
                    }
                    from_reg(value, fd_text);
                    from_reg(gep->base, fd_text);
                    free_reg(gep->index);
                }
                else if (type == IRNodeSGEP) {
                    _fputs(fd_text, "; SGEP\n");
                    struct IRSGEP *sgep = value->node_ptr;
                    to_reg(value, RAX, fd_text);
                    to_reg(sgep->instance, RDX, fd_text);
                    _fputs3(fd_text, "    lea ", get_reg(value->reg), ", [");
                    _fputs(fd_text, get_reg(sgep->instance->reg));
                    _fputsi(fd_text, " + ", sgep->phase, "]\n");
                    from_reg(value, fd_text);
                    from_reg(sgep->instance, fd_text);
                }
                else if (type == IRNodeLoad) {
                    _fputs(fd_text, "; LOAD\n");
                    struct IRLoad *load = value->node_ptr;
                    int sz = load->size;
                    if (sz > 8) {
                        for (int i = 0; i < sz; i++) {
                            _fputs(fd_text, "    ((char*)");
                            ir_compile_value_x86_64(&values_list, value, fd_text);
                            _fputsi(fd_text, ")[", i, "] = (char)");
                            ir_compile_value_x86_64(&values_list, load->src, fd_text);
                            _fputsi(fd_text, "[", i, "]; // load");
                            if (i + 1 < sz) _fputs(fd_text, "\n");
                        }
                    }
                    else {
                        to_reg(value, RCX, fd_text);
                        to_reg(load->src, RDX, fd_text);

                        if (sz == 1) {
                            _fputs3(fd_text, "    mov byte al, [", get_reg(load->src->reg), "]\n");
                            _fputs3(fd_text, "    mov byte ", get_reg_byte(value->reg), ", al\n");
                        }
                        if (sz == 8) {
                            _fputs3(fd_text, "    mov qword rax, [", get_reg(load->src->reg), "]\n");
                            _fputs3(fd_text, "    mov qword ", get_reg(value->reg), ", rax\n");
                        }

                        from_reg(value, fd_text);
                        from_reg(load->src, fd_text);
                    }
                }
                else if (type == IRNodeStore) {
                    _fputs(fd_text, "; STORE\n");
                    struct IRStore *store = value->node_ptr;
                    int sz = store->size;
                    if (sz > 8) {
                        _fputsi(fd_text, "    mov rdi, [rbp + ", store->dst->stack_phase, "]\n");
                        _fputsi(fd_text, "    mov rsi, [rbp + ", store->src->stack_phase, "]\n");
                        _fputsi(fd_text, "    mov rcx, ", sz, "\n");
                        _fputs(fd_text, "    repe movsb\n");
                    }
                    else {
                        to_reg(store->dst, RAX, fd_text);
                        to_reg(store->src, RDX, fd_text);
                        if (sz == 1) {
                            _fputs(fd_text, "    mov byte [");
                            _fputs2(fd_text, get_reg(store->dst->reg), "], ");
                            _fputs2(fd_text, get_reg_byte(store->src->reg), "\n");
                        }
                        else if (sz == 8) {
                            _fputs(fd_text, "    mov qword [");
                            _fputs2(fd_text, get_reg(store->dst->reg), "], ");
                            _fputs2(fd_text, get_reg(store->src->reg), "\n");
                        }
                        from_reg(store->dst, fd_text);
                        from_reg(store->src, fd_text);
                    }
                }
                else if (type == IRNodeCall) {
                    _fputs(fd_text, "; CALL\n");
                    struct IRCall *call = value->node_ptr;
                    int sz_args = vsize(&call->arguments);
                    for (int arg = 0; arg < sz_args; arg++) {
                        enum Register reg;
                        switch (arg) {
                            case 0: reg = RDI; break;
                            case 1: reg = RSI; break;
                            case 2: reg = RDX; break;
                            case 3: reg = RCX; break;
                            default: _panic("Too many arguments");
                        }
                        struct IRNode *value_arg = call->arguments.ptr[arg];
                        to_reg(value_arg, reg, fd_text);
                        _fputs3(fd_text, "    mov ", get_reg(reg), ", ");
                        _fputs2(fd_text, get_reg(value_arg->reg), "\n");
                        free_reg(value_arg);
                    }
                    
                    for (enum Register reg = R8; reg <= R11; reg++) {
                        _fputs3(fd_text, "    push ", get_reg(reg), "\n");
                    }

                    to_reg(call->function, RAX, fd_text);
                    _fputs3(fd_text, "    call ", get_reg(call->function->reg), "\n");
                    free_reg(call->function);

                    for (enum Register reg = R11; reg >= R8; reg--) {
                        _fputs3(fd_text, "    pop ", get_reg(reg), "\n");
                    }

                    if (value->type->size != 0) {
                        ir_compile_binary_instr3_x86_64("mov", value, "rax", fd_text);
                        _fputsi(fd_text, "; CALL FINISH ", value->reg, "\n");
                    }
                }
                else if (type == IRNodeBr) {
                    struct IRBr *br = value->node_ptr;
                    ir_compile_phi_x86_64(&values_list, block, br->block, fd_text);
                    _fputs(fd_text, "    jmp ");
                    ir_compile_block_x86_64(&blocks_list, br->block, fd_text);
                    _fputs(fd_text, "\n");
                    break;
                }
                else if (type == IRNodeCondBr) {
                    struct IRCondBr *condbr = value->node_ptr;
                    if (condbr->condition->node_type == IRNodeConst) {
                        struct IRConst *_const = condbr->condition->node_ptr;
                        if (_const->value == 0) {
                            _fputs(fd_text, "    jmp ");
                            ir_compile_block_x86_64(&blocks_list, condbr->block_else, fd_text);
                            _fputs(fd_text, "\n");
                        }
                        else {
                            _fputs(fd_text, "    jmp ");
                            ir_compile_block_x86_64(&blocks_list, condbr->block_then, fd_text);
                            _fputs(fd_text, "\n");
                        }
                    }
                    else {
                        ir_compile_binary_instr3_x86_64("cmp", condbr->condition, "0", fd_text);
                        ir_compile_phi_x86_64(&values_list, block, condbr->block_then, fd_text);
                        _fputs(fd_text, "    jne ");
                        ir_compile_block_x86_64(&blocks_list, condbr->block_then, fd_text);
                        _fputs(fd_text, "\n");
                        ir_compile_phi_x86_64(&values_list, block, condbr->block_else, fd_text);
                        _fputs(fd_text, "    jmp ");
                        ir_compile_block_x86_64(&blocks_list, condbr->block_else, fd_text);
                        _fputs(fd_text, "\n");
                    }
                    break;
                }
                else if (type == IRNodeRet) {
                    struct IRRet *ret = value->node_ptr;
                    if (ret->value) {
                        ir_compile_binary_instr2_x86_64("mov", "rax", ret->value, fd_text);
                    }
                    _fputsi(fd_text, "    sub rsp, ", cur_stack_phase, "\n");
                    for (enum Register reg = R15; reg >= R12; reg--) {
                        _fputs3(fd_text, "    pop ", get_reg(reg), "\n");
                    }
                    _fputs(fd_text, "    leave\n");
                    _fputs(fd_text, "    ret\n");
                    break;
                }
                else if (type == IRNodeAnd ||
                    type == IRNodeOr ||
                    type == IRNodeNot ||
                    type == IRNodeBitwiseAnd ||
                    type == IRNodeBitwiseOr ||
                    type == IRNodeBitwiseXor ||
                    type == IRNodeBitwiseNot ||
                    type == IRNodeBitwiseShiftLeft ||
                    type == IRNodeBitwiseShiftRight ||
                    type == IRNodeAddition ||
                    type == IRNodeSubtraction ||
                    type == IRNodeMultiplication ||
                    type == IRNodeDivision ||
                    type == IRNodeModulo ||
                    type == IRNodeLess ||
                    type == IRNodeGreater ||
                    type == IRNodeEqual ||
                    type == IRNodeLessEqual ||
                    type == IRNodeGreaterEqual ||
                    type == IRNodeNotEqual
                ) {
                    struct IRBinaryOperator *binary_operator = value->node_ptr;
                    switch (type) {
                        case IRNodeAnd: {
                            to_reg(value, RAX, fd_text);
                            to_reg(binary_operator->left, RCX, fd_text);
                            to_reg(binary_operator->right, RDX, fd_text);
                            _fputs(fd_text, "    xor rdi, rdi\n");
                            _fputs(fd_text, "    xor rsi, rsi\n");
                            _fputs3(fd_text, "    cmp ", get_reg(binary_operator->left->reg), ", 0\n");
                            _fputs(fd_text, "    setne dil\n");
                            _fputs3(fd_text, "    cmp ", get_reg(binary_operator->right->reg), ", 0\n");
                            _fputs(fd_text, "    setne sil\n");
                            _fputs(fd_text, "    and rdi, rsi\n");
                            _fputs3(fd_text, "    mov ", get_reg(value->reg), ", rdi\n");
                            from_reg(value, fd_text);
                            free_reg(binary_operator->left);
                            free_reg(binary_operator->right);
                        } break;
                        case IRNodeOr: {
                            to_reg(value, RAX, fd_text);
                            to_reg(binary_operator->left, RCX, fd_text);
                            to_reg(binary_operator->right, RDX, fd_text);
                            _fputs(fd_text, "    xor rdi, rdi\n");
                            _fputs(fd_text, "    xor rsi, rsi\n");
                            _fputs3(fd_text, "    cmp ", get_reg(binary_operator->left->reg), ", 0\n");
                            _fputs(fd_text, "    setne dil\n");
                            _fputs3(fd_text, "    cmp ", get_reg(binary_operator->right->reg), ", 0\n");
                            _fputs(fd_text, "    setne sil\n");
                            _fputs(fd_text, "    or rdi, rsi\n");
                            _fputs3(fd_text, "    mov ", get_reg(value->reg), ", rdi\n");
                            from_reg(value, fd_text);
                            free_reg(binary_operator->left);
                            free_reg(binary_operator->right);
                        } break;
                        case IRNodeNot:                 _fputs(fd_text, "!"); break;
                        case IRNodeBitwiseAnd: {
                            to_reg(value, RAX, fd_text);
                            to_reg(binary_operator->left, RCX, fd_text);
                            to_reg(binary_operator->right, RDX, fd_text);
                            ir_compile_binary_instr_x86_64("mov", value, binary_operator->left, fd_text);
                            ir_compile_binary_instr_x86_64("and", value, binary_operator->right, fd_text);
                            from_reg(value, fd_text);
                            free_reg(binary_operator->left);
                            free_reg(binary_operator->right);
                        } break;
                        case IRNodeBitwiseOr:           _fputs(fd_text, "or"); break;
                        case IRNodeBitwiseXor:          _fputs(fd_text, "xor"); break;
                        case IRNodeBitwiseNot:          _fputs(fd_text, "not"); break;
                        case IRNodeBitwiseShiftLeft:    _fputs(fd_text, "sal"); break;
                        case IRNodeBitwiseShiftRight:   _fputs(fd_text, "sar"); break;
                        case IRNodeAddition: {
                            to_reg(value, RAX, fd_text);
                            ir_compile_binary_instr_x86_64("mov", value, binary_operator->left, fd_text);
                            ir_compile_binary_instr_x86_64("add", value, binary_operator->right, fd_text);
                            from_reg(value, fd_text);
                        } break;
                        case IRNodeSubtraction: {
                            if (binary_operator->left) {
                                to_reg(value, RAX, fd_text);
                                ir_compile_binary_instr_x86_64("mov", value, binary_operator->left, fd_text);
                                ir_compile_binary_instr_x86_64("sub", value, binary_operator->right, fd_text);
                                from_reg(value, fd_text);
                            }
                            else {
                                to_reg(value, RAX, fd_text);
                                ir_compile_binary_instr_x86_64("mov", value, binary_operator->right, fd_text);
                                _fputs3(fd_text, "    neg ", get_reg(value->reg), "\n");
                                from_reg(value, fd_text);
                            }
                        } break;
                        case IRNodeMultiplication: {
                            to_reg(binary_operator->left, RAX, fd_text);
                            to_reg(binary_operator->right, RDX, fd_text);
                            to_reg(value, RCX, fd_text);
                            _fputs3(fd_text, "    mov rax, ", get_reg(binary_operator->left->reg), "\n");
                            _fputs3(fd_text, "    mul ", get_reg(binary_operator->right->reg), "\n");
                            _fputs3(fd_text, "    mov ", get_reg(value->reg), ", rax\n");
                            from_reg(value, fd_text);
                            free_reg(binary_operator->left);
                            free_reg(binary_operator->right);
                        } break;
                        case IRNodeDivision: {
                            to_reg(value, RCX, fd_text);
                            to_reg(binary_operator->left, RAX, fd_text);
                            to_reg(binary_operator->right, RDI, fd_text);
                            _fputs3(fd_text, "    mov rax, ", get_reg(binary_operator->left->reg), "\n");
                            _fputs(fd_text, "    xor rdx, rdx\n");
                            _fputs3(fd_text, "    div ", get_reg(binary_operator->right->reg), "\n");
                            _fputs3(fd_text, "    mov ", get_reg(value->reg), ", rax\n");
                            from_reg(value, fd_text);
                            free_reg(binary_operator->left);
                            free_reg(binary_operator->right);
                        } break;
                        case IRNodeModulo: {
                            to_reg(value, RCX, fd_text);
                            to_reg(binary_operator->left, RAX, fd_text);
                            to_reg(binary_operator->right, RDI, fd_text);
                            _fputs3(fd_text, "    mov rax, ", get_reg(binary_operator->left->reg), "\n");
                            _fputs(fd_text, "    xor rdx, rdx\n");
                            _fputs3(fd_text, "    div ", get_reg(binary_operator->right->reg), "\n");
                            _fputs3(fd_text, "    mov ", get_reg(value->reg), ", rdx\n");
                            from_reg(value, fd_text);
                            free_reg(binary_operator->left);
                            free_reg(binary_operator->right);
                        } break;
                        case IRNodeLess: {
                            _fputs(fd_text, "    xor rax, rax\n");
                            to_reg(binary_operator->left, RCX, fd_text);
                            ir_compile_binary_instr_x86_64("cmp", binary_operator->left, binary_operator->right, fd_text);
                            from_reg(binary_operator->left, fd_text);
                            _fputs(fd_text, "    setl al\n");
                            ir_compile_binary_instr3_x86_64("mov", value, "rax", fd_text);
                        } break;
                        case IRNodeGreater: {
                            _fputs(fd_text, "    xor rax, rax\n");
                            to_reg(binary_operator->left, RCX, fd_text);
                            ir_compile_binary_instr_x86_64("cmp", binary_operator->left, binary_operator->right, fd_text);
                            from_reg(binary_operator->left, fd_text);
                            _fputs(fd_text, "    setg al\n");
                            ir_compile_binary_instr3_x86_64("mov", value, "rax", fd_text);
                        } break;
                        case IRNodeEqual: {
                            _fputs(fd_text, "    xor rax, rax\n");
                            to_reg(binary_operator->left, RCX, fd_text);
                            ir_compile_binary_instr_x86_64("cmp", binary_operator->left, binary_operator->right, fd_text);
                            from_reg(binary_operator->left, fd_text);
                            _fputs(fd_text, "    sete al\n");
                            ir_compile_binary_instr3_x86_64("mov", value, "rax", fd_text);
                        } break;
                        case IRNodeLessEqual:           _fputs(fd_text, "<="); break;
                        case IRNodeGreaterEqual:        _fputs(fd_text, ">="); break;
                        case IRNodeNotEqual:            _fputs(fd_text, "!="); break;
                    };
                }
                else {
                    _panic("Uncompilable value");
                }
            }
        }

        _fputs(fd_text, "\n");
    }

    if (builder->testing) {
        _fputs(fd_text, "extern posix_exit\n");
        _fputs(fd_text, "global test:\n");
        _fputs(fd_text, "test:\n");
        int sz = vsize(&builder->test_names);

        if (sz) {
            for (int i = 0; i < sz; i++) {
                _fputs3(fd_text, "    call ", builder->test_names.ptr[i], "\n");
                _fputs(fd_text, "    test rax, 0\n");
                _fputs(fd_text, "    jnz _testfail\n");
            }
            _fputs(fd_text, "    mov rdi, 0\n");
            _fputs(fd_text, "    call posix_exit\n");
            _fputs(fd_text, "    mov rdi, 1\n");
            _fputs(fd_text, "_testfail:\n");
            _fputs(fd_text, "    call posix_exit\n");
        }
        else {
            _fputs(fd_text, "    mov rdi, 0\n");
            _fputs(fd_text, "    call posix_exit\n");
        }
    }

    posix_close(fd_text);
    char *str_text = read_file_descriptor(fd_text_out);
    posix_close(fd_text_out);
    if (filename_compile_output) {
        write_file(filename_compile_output, str_text);
    }
}
