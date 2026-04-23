#include <ir_compile_x86_64.h>
#include <stdio.h>
#include <string.h>
#include <posix.h>

const char *get_reg_qword(enum Register reg) {
    switch (reg) {
        case REGNONE: _panic("Value is not in register");
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
        case REGNONE: _panic("Value is not in register");
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

const char *get_reg(enum Register reg, int size) {
    switch (size) {
        case 1: return get_reg_byte(reg);
        case 8: return get_reg_qword(reg);
        default: _panic("Unexpected register size");
    }
}

const char *get_qual(int size) {
    switch (size) {
        case 1: return "byte";
        case 8: return "qword";
        default: _panic("Unexpected value size");
    }
}

void to_reg(struct IRNode *node, enum Register reg, int fd_text) {
    if (node->spill || node->node_type == IRNodeGlobal || node->node_type == IRNodeConst) {
        node->reg = reg;
        _fputs3(fd_text, "    mov ", get_reg(reg, type_size(node->type)), ", ");

        if (node->spill) {
            _fputsi(fd_text, "[rbp + ", node->stack_phase, "]\n");
        }
        if (node->node_type == IRNodeGlobal) {
            struct IRGlobal *global = node->node_ptr;
            _fputs2(fd_text, global->name, "\n");
        }
        if (node->node_type == IRNodeConst) {
            struct IRConst *_const = node->node_ptr;
            _fputsi(fd_text, "", _const->value, "\n");
        }
    }
}

void occupy_reg(struct IRNode *node, enum Register reg) {
    if (node->spill) {
        node->reg = reg;
    }
}

void from_reg(struct IRNode *node, int fd_text) {
    if (node->spill) {
        _fputsi(fd_text, "    mov [rbp + ", node->stack_phase, "], ");
        _fputs2(fd_text, get_reg(node->reg, type_size(node->type)), "\n");
        node->reg = REGNONE;
    }
    else if (node->node_type == IRNodeGlobal) {
        node->reg = REGNONE;
    }
    else if (node->node_type == IRNodeConst) {
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

void ir_compile_block_x86_64(struct IRBlock *block, int fd_text) {
    _fputsi(fd_text, "._b", block->idx, "");
}

void print_value(struct IRNode *value, int fd_text) {
    if (value->reg != REGNONE) {
        _fputs(fd_text, get_reg(value->reg, type_size(value->type)));
    }
    else {
        if (value->node_type == IRNodeConst) {
            struct IRConst *_const = value->node_ptr;
            _fputi(fd_text, _const->value);
        }
        else if(value->node_type == IRNodeGlobal) {
            struct IRGlobal *_value = value->node_ptr;
            _fputs(fd_text, _value->name);
        }
        else {
            _fputsi(fd_text, "[rbp + ", value->stack_phase, "]");
        }
    }
}

void prefix(const char *instr, int size, int fd_text) {
    _fputs3(fd_text, "    ", instr, " ");
    _fputs2(fd_text, get_qual(size), " ");
}

void uninstr_v(const char *instr, struct IRNode *arg, int fd_text) {
    prefix(instr, type_size(arg->type), fd_text);
    print_value(arg, fd_text);
    _fputs(fd_text, "\n");
}

void uninstr_r(const char *instr, enum Register arg, int size, int fd_text) {
    prefix(instr, size, fd_text);
    _fputs2(fd_text, get_reg(arg, size), "\n");
}

void bininstr_vv(const char *instr, struct IRNode *left, struct IRNode *right, int fd_text) {
    prefix(instr, type_size(left->type), fd_text);
    print_value(left, fd_text);
    _fputs(fd_text, ", ");
    print_value(right, fd_text);
    _fputs(fd_text, "\n");
}

void bininstr_rv(const char *instr, enum Register left, struct IRNode *right, int fd_text) {
    prefix(instr, type_size(right->type), fd_text);
    _fputs2(fd_text, get_reg(left, type_size(right->type)), ", ");
    print_value(right, fd_text);
    _fputs(fd_text, "\n");
}

void bininstr_vr(const char *instr, struct IRNode *left, enum Register right, int fd_text) {
    prefix(instr, type_size(left->type), fd_text);
    print_value(left, fd_text);
    _fputs2(fd_text, ", ", get_reg(right, type_size(left->type)));
    _fputs(fd_text, "\n");
}

void bininstr_vc(const char *instr, struct IRNode *left, const char *right, int fd_text) {
    prefix(instr, type_size(left->type), fd_text);
    print_value(left, fd_text);
    _fputs2(fd_text, ", ", right);
    _fputs(fd_text, "\n");
}

void bininstr_rr(const char *instr, enum Register left, enum Register right, int size, int fd_text) {
    prefix(instr, size, fd_text);
    _fputs3(fd_text, get_reg(left, size), ", ", get_reg(right, size));
    _fputs(fd_text, "\n");
}

void ir_compile_phi_x86_64(struct IRBlock *block, struct IRBlock *succ_block, int fd_text) {
    int sz_insts = vsize(&succ_block->value_list);
    for (int i = 0; i < sz_insts; i++) {
        struct IRNode *value = succ_block->value_list.ptr[i];
        if (value->node_type == IRNodePhi) {
            struct IRPhi *phi = value->node_ptr;
            int sz_args = vsize(&phi->values);
            for (int j = 0; j < sz_args; j++) {
                if (block == phi->blocks.ptr[j] && value != phi->values.ptr[j]) {
                    struct IRNode *src = phi->values.ptr[j];
                    if (value->spill && src->spill) {
                        occupy_reg(value, RAX);
                        _fputs3(fd_text, "    mov ", get_reg(value->reg, type_size(value->type)), ", ");
                        _fputsi(fd_text, "[rbp + ", src->stack_phase, "]\n");
                        from_reg(value, fd_text);
                    }
                    else {
                        bininstr_vv("mov", value, src, fd_text);
                    }
                }
            }
        }
    }
}

enum Register call_reg(int n) {
    switch (n) {
        case 0: return RDI;
        case 1: return RSI;
        case 2: return RDX;
        case 3: return RCX;
        default: _panic("Unsupported number of arguments"); 
    }
}

int reg_status[10000];

enum Register find_register(struct IRNode *node) {
    int left = node->block->idx;
    int right = node->right_bound;
    int sz = vsize(&node->uses);
    for (int i = 0; i < sz; i++) {
        struct IRNode *use = node->uses.ptr[i];
        if (left > use->block->idx) left = use->block->idx;
        if (right < use->block->idx) right = use->block->idx;
    }
    if (node->node_type == IRNodePhi) {
        int cnt_pred = vsize(&node->block->pred_list);
        for (int i = 0; i < cnt_pred; i++) {
            struct IRBlock *pred = node->block->pred_list.ptr[i];
            if (left > pred->idx) left = pred->idx;
            if (right < pred->idx) right = pred->idx;
        }
    }
    int mask = 0;
    for (int i = left; i <= right; i++) {
        mask |= reg_status[i];
    }
    for (enum Register reg = R8; reg <= R15; reg++) {
        if (!(mask & (1 << reg))) {
            for (int i = left; i <= right; i++) {
                reg_status[i] |= (1 << reg);
            }
            return reg;
        }
    }
    return REGNONE;
}

long calculate_priority(struct IRNode *value) {
    long priority = 1;
    for (int i = 0; i < value->loop_degree; i++) priority *= 10;
    int cnt_uses = vsize(&value->uses);
    for (int u = 0; u < cnt_uses; u++) {
        struct IRNode *use = value->uses.ptr[u];
        long x = 1;
        for (int i = 0; i < use->loop_degree; i++) {
            x *= 10;
        }
        priority += x;
    }
    return priority;
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
            reg_status[j] = 0;
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

        uninstr_r("push", RBP, 8, fd_text);
        bininstr_rr("mov", RBP, RSP, 8, fd_text);

        for (enum Register reg = R12; reg <= R15; reg++) {
            uninstr_r("push", reg, 8, fd_text);
        }
        
        struct Vector values_priority = vnew();
        struct Vector values_idx = vnew();

        int sz_args = vsize(&function->arg_list);
        for (int j = 0; j < sz_args; j++) {
            struct IRNode *value = function->arg_list.ptr[j];
            vpush(&values_priority, (void*)calculate_priority(value));
            vpush(&values_idx, value);
        }

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
                    vpush(&values_priority, (void*)calculate_priority(value));
                    vpush(&values_idx, value);
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

        int cnt_values = vsize(&values_priority);
        for (int i = 0; i < cnt_values; i++) {
            for (int j = 0; j + 1 < cnt_values; j++) {
                if ((long)values_priority.ptr[j] < (long)values_priority.ptr[j + 1]) {
                    void *tmp = values_priority.ptr[j];
                    values_priority.ptr[j] = values_priority.ptr[j + 1];
                    values_priority.ptr[j + 1] = tmp;
                    tmp = values_idx.ptr[j];
                    values_idx.ptr[j] = values_idx.ptr[j + 1];
                    values_idx.ptr[j + 1] = tmp;
                }
            }
        }

        for (int i = 0; i < cnt_values; i++) {
            struct IRNode *value = values_idx.ptr[i];
            enum Register reg = find_register(value);
            value->reg = reg;
            if (reg == REGNONE) {
                value->spill = true;
                value->stack_phase = cur_stack_phase - type_size(value->type);
                cur_stack_phase -= type_size(value->type);
            }
            if (value->node_type == IRNodeArg) {
                int idx = -1;
                for (int j = 0; j < sz_args; j++) {
                    if (function->arg_list.ptr[j] == value) {
                        idx = j;
                        break;
                    }
                }
                _assert(idx != -1);
                if (reg == REGNONE) {
                    _fputsi(fd_text, "    mov [rbp + ", value->stack_phase, "], ");
                    _fputs2(fd_text, get_reg(call_reg(idx), 8), "\n");
                }
                else {
                    _fputs3(fd_text, "    mov ", get_reg(reg, 8), ", ");
                    _fputs2(fd_text, get_reg(call_reg(idx), 8), "\n");
                }
            }
        }

        _fputsi(fd_text, "    add rsp, ", cur_stack_phase, "\n");

        for (int j = 0; j < sz_blocks; j++) {
            struct IRBlock *block = function->block_list.ptr[j];
            ir_compile_block_x86_64(block, fd_text);
            _fputs(fd_text, ":\n");

            int sz_insts = vsize(&block->value_list);
            for (int k = 0; k < sz_insts; k++) {
                struct IRNode *value = block->value_list.ptr[k];
                enum IRNodeType type = value->node_type;

                if (type == IRNodeAlloca) continue;

                /* _fputsi(fd_text, "; %", value->idx, " : ");
                int sz = vsize(&value->uses);
                for (int kk = 0; kk < sz; kk++) {
                    struct IRNode *use = value->uses.ptr[kk];
                    _fputsi(fd_text, "%", use->idx, ", ");
                }
                _fputs(fd_text, "\n"); */

                if (type == IRNodePhi) {
                    continue;
                }
                else if (type == IRNodeGEP) {
                    struct IRGEP *gep = value->node_ptr;
                    
                    occupy_reg(value, RDX);
                    to_reg(gep->base, RCX, fd_text);
                    to_reg(gep->index, RAX, fd_text);
                    if (gep->size == 1 || gep->size == 2 || gep->size == 4 || gep->size == 8 || gep->size == 16) {
                        _fputs3(fd_text, "    lea ", get_reg(value->reg, type_size(value->type)), ", [");
                        _fputs3(fd_text, get_reg(gep->base->reg, type_size(gep->base->type)), " + ", get_reg(gep->index->reg, type_size(gep->index->type)));
                        _fputsi(fd_text, " * ", gep->size, "]\n");
                    }
                    else {
                        bininstr_rr("mov", RAX, gep->index->reg, type_size(gep->index->type), fd_text);
                        _fputsi(fd_text, "    mov rdi, ", gep->size, "\n");
                        uninstr_r("mul", RDI, 8, fd_text);
                        _fputs3(fd_text, "    lea ", get_reg(value->reg, type_size(value->type)), ", [");
                        _fputs2(fd_text, get_reg(gep->base->reg, type_size(gep->base->type)), " + rax]\n");
                    }
                    from_reg(value, fd_text);
                    free_reg(gep->base);
                    free_reg(gep->index);
                }
                else if (type == IRNodeSGEP) {
                    struct IRSGEP *sgep = value->node_ptr;
                    occupy_reg(value, RAX);
                    to_reg(sgep->instance, RDX, fd_text);
                    _fputs3(fd_text, "    lea ", get_reg(value->reg, type_size(value->type)), ", [");
                    _fputs(fd_text, get_reg(sgep->instance->reg, type_size(sgep->instance->type)));
                    _fputsi(fd_text, " + ", sgep->phase, "]\n");
                    from_reg(value, fd_text);
                    free_reg(sgep->instance);
                }
                else if (type == IRNodeLoad) {
                    struct IRLoad *load = value->node_ptr;
                    int sz = load->size;

                    occupy_reg(value, RCX);
                    to_reg(load->src, RDX, fd_text);

                    if (sz == 1) {
                        _fputs3(fd_text, "    mov byte al, [", get_reg(load->src->reg, type_size(load->src->type)), "]\n");
                        _fputs3(fd_text, "    mov byte ", get_reg(value->reg, sz), ", al\n");
                    }
                    if (sz == 8) {
                        _fputs3(fd_text, "    mov qword rax, [", get_reg(load->src->reg, type_size(load->src->type)), "]\n");
                        _fputs3(fd_text, "    mov qword ", get_reg(value->reg, sz), ", rax\n");
                    }

                    from_reg(value, fd_text);
                    free_reg(load->src);
                }
                else if (type == IRNodeStore) {
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
                        }
                        else if (sz == 8) {
                            _fputs(fd_text, "    mov qword [");
                        }
                        _fputs2(fd_text, get_reg(store->dst->reg, type_size(store->dst->type)), "], ");
                        _fputs2(fd_text, get_reg(store->src->reg, type_size(store->src->type)), "\n");
                        free_reg(store->dst);
                        free_reg(store->src);
                    }
                }
                else if (type == IRNodeCall) {
                    struct IRCall *call = value->node_ptr;
                    int sz_args = vsize(&call->arguments);
                    for (int arg = 0; arg < sz_args; arg++) {
                        enum Register reg = call_reg(arg);
                        struct IRNode *value_arg = call->arguments.ptr[arg];
                        bininstr_rv("mov", reg, value_arg, fd_text);
                    }
                    
                    for (enum Register reg = R8; reg <= R11; reg++) {
                        uninstr_r("push", reg, 8, fd_text);
                    }

                    uninstr_v("call", call->function, fd_text);

                    for (enum Register reg = R11; reg >= R8; reg--) {
                        uninstr_r("pop", reg, 8, fd_text);
                    }

                    if (value->type->size != 0) {
                        bininstr_vr("mov", value, RAX, fd_text);
                    }
                }
                else if (type == IRNodeBr) {
                    struct IRBr *br = value->node_ptr;
                    ir_compile_phi_x86_64(block, br->block, fd_text);
                    _fputs(fd_text, "    jmp ");
                    ir_compile_block_x86_64(br->block, fd_text);
                    _fputs(fd_text, "\n");
                    break;
                }
                else if (type == IRNodeCondBr) {
                    struct IRCondBr *condbr = value->node_ptr;
                    if (condbr->condition->node_type == IRNodeConst) {
                        struct IRConst *_const = condbr->condition->node_ptr;
                        if (_const->value == 0) {
                            _fputs(fd_text, "    jmp ");
                            ir_compile_block_x86_64(condbr->block_else, fd_text);
                            _fputs(fd_text, "\n");
                        }
                        else {
                            _fputs(fd_text, "    jmp ");
                            ir_compile_block_x86_64(condbr->block_then, fd_text);
                            _fputs(fd_text, "\n");
                        }
                    }
                    else {
                        bininstr_vc("cmp", condbr->condition, "0", fd_text);
                        ir_compile_phi_x86_64(block, condbr->block_then, fd_text);
                        _fputs(fd_text, "    jne ");
                        ir_compile_block_x86_64(condbr->block_then, fd_text);
                        _fputs(fd_text, "\n");
                        ir_compile_phi_x86_64(block, condbr->block_else, fd_text);
                        _fputs(fd_text, "    jmp ");
                        ir_compile_block_x86_64(condbr->block_else, fd_text);
                        _fputs(fd_text, "\n");
                    }
                    break;
                }
                else if (type == IRNodeRet) {
                    struct IRRet *ret = value->node_ptr;
                    if (ret->value) {
                        bininstr_rv("mov", RAX, ret->value, fd_text);
                    }
                    _fputsi(fd_text, "    sub rsp, ", cur_stack_phase, "\n");
                    for (enum Register reg = R15; reg >= R12; reg--) {
                        uninstr_r("pop", reg, 8, fd_text);
                    }
                    _fputs(fd_text, "    pop rbp\n");
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
                            occupy_reg(value, RAX);
                            to_reg(binary_operator->left, RCX, fd_text);
                            to_reg(binary_operator->right, RDX, fd_text);
                            bininstr_rr("xor", RDI, RDI, 8, fd_text);
                            bininstr_rr("xor", RSI, RSI, 8, fd_text);
                            bininstr_vc("cmp", binary_operator->left, "0", fd_text);
                            uninstr_r("setne", RDI, 1, fd_text);
                            bininstr_vc("cmp", binary_operator->right, "0", fd_text);
                            uninstr_r("setne", RSI, 1, fd_text);
                            bininstr_rr("and", RDI, RSI, 8, fd_text);
                            bininstr_vr("mov", value, RDI, fd_text);
                            from_reg(value, fd_text);
                            free_reg(binary_operator->left);
                            free_reg(binary_operator->right);
                        } break;
                        case IRNodeOr: {
                            occupy_reg(value, RAX);
                            to_reg(binary_operator->left, RCX, fd_text);
                            to_reg(binary_operator->right, RDX, fd_text);
                            bininstr_rr("xor", RDI, RDI, 8, fd_text);
                            bininstr_rr("xor", RSI, RSI, 8, fd_text);
                            bininstr_vc("cmp", binary_operator->left, "0", fd_text);
                            uninstr_r("setne", RDI, 1, fd_text);
                            bininstr_vc("cmp", binary_operator->right, "0", fd_text);
                            uninstr_r("setne", RSI, 1, fd_text);
                            bininstr_rr("or", RDI, RSI, 8, fd_text);
                            bininstr_vr("mov", value, RDI, fd_text);
                            from_reg(value, fd_text);
                            free_reg(binary_operator->left);
                            free_reg(binary_operator->right);
                        } break;
                        case IRNodeNot: {
                            occupy_reg(value, RAX);
                            to_reg(binary_operator->right, RDX, fd_text);
                            bininstr_rr("xor", RSI, RSI, 8, fd_text);
                            bininstr_vc("cmp", binary_operator->right, "0", fd_text);
                            uninstr_r("sete", RSI, 1, fd_text);
                            bininstr_vr("mov", value, RSI, fd_text);
                            from_reg(value, fd_text);
                            free_reg(binary_operator->right);
                        } break;
                        case IRNodeBitwiseAnd: {
                            occupy_reg(value, RAX);
                            bininstr_vv("mov", value, binary_operator->left, fd_text);
                            bininstr_vv("and", value, binary_operator->right, fd_text);
                            from_reg(value, fd_text);
                        } break;
                        case IRNodeBitwiseOr: {
                            occupy_reg(value, RAX);
                            bininstr_vv("mov", value, binary_operator->left, fd_text);
                            bininstr_vv("or", value, binary_operator->right, fd_text);
                            from_reg(value, fd_text);
                        } break;
                        case IRNodeBitwiseXor: {
                            occupy_reg(value, RAX);
                            bininstr_vv("mov", value, binary_operator->left, fd_text);
                            bininstr_vv("xor", value, binary_operator->right, fd_text);
                            from_reg(value, fd_text);
                        } break;
                        case IRNodeBitwiseNot: {
                            occupy_reg(value, RAX);
                            bininstr_vv("mov", value, binary_operator->right, fd_text);
                            uninstr_v("not", value, fd_text);
                            from_reg(value, fd_text);
                        } break;
                        case IRNodeBitwiseShiftLeft: {
                            occupy_reg(value, RAX);
                            bininstr_vv("mov", value, binary_operator->left, fd_text);
                            bininstr_vv("shl", value, binary_operator->right, fd_text);
                            from_reg(value, fd_text);
                        } break;
                        case IRNodeBitwiseShiftRight: {
                            occupy_reg(value, RAX);
                            bininstr_vv("mov", value, binary_operator->left, fd_text);
                            bininstr_vv("shr", value, binary_operator->right, fd_text);
                            from_reg(value, fd_text);
                        } break;
                        case IRNodeAddition: {
                            occupy_reg(value, RAX);
                            bininstr_vv("mov", value, binary_operator->left, fd_text);
                            bininstr_vv("add", value, binary_operator->right, fd_text);
                            from_reg(value, fd_text);
                        } break;
                        case IRNodeSubtraction: {
                            if (binary_operator->left) {
                                occupy_reg(value, RAX);
                                bininstr_vv("mov", value, binary_operator->left, fd_text);
                                bininstr_vv("sub", value, binary_operator->right, fd_text);
                                from_reg(value, fd_text);
                            }
                            else {
                                occupy_reg(value, RAX);
                                bininstr_vv("mov", value, binary_operator->right, fd_text);
                                uninstr_v("neg", value, fd_text);
                                from_reg(value, fd_text);
                            }
                        } break;
                        case IRNodeMultiplication: {
                            occupy_reg(value, RCX);
                            to_reg(binary_operator->left, RAX, fd_text);
                            to_reg(binary_operator->right, RDX, fd_text);
                            if (binary_operator->left->reg != RAX) {
                                bininstr_rv("mov", RAX, binary_operator->left, fd_text);
                            }
                            uninstr_v("mul", binary_operator->right, fd_text);
                            bininstr_vr("mov", value, RAX, fd_text);
                            from_reg(value, fd_text);
                            free_reg(binary_operator->left);
                            free_reg(binary_operator->right);
                        } break;
                        case IRNodeDivision: {
                            occupy_reg(value, RCX);
                            to_reg(binary_operator->left, RAX, fd_text);
                            to_reg(binary_operator->right, RDI, fd_text);
                            if (binary_operator->left->reg != RAX) {
                                bininstr_rv("mov", RAX, binary_operator->left, fd_text);
                            }
                            bininstr_rr("xor", RDX, RDX, 8, fd_text);
                            uninstr_r("div", binary_operator->right->reg, type_size(binary_operator->right->type), fd_text);
                            bininstr_rr("mov", value->reg, RAX, type_size(value->type), fd_text);
                            from_reg(value, fd_text);
                            free_reg(binary_operator->left);
                            free_reg(binary_operator->right);
                        } break;
                        case IRNodeModulo: {
                            occupy_reg(value, RCX);
                            to_reg(binary_operator->left, RAX, fd_text);
                            to_reg(binary_operator->right, RDI, fd_text);
                            if (binary_operator->left->reg != RAX) {
                                bininstr_rv("mov", RAX, binary_operator->left, fd_text);
                            }
                            bininstr_rr("xor", RDX, RDX, 8, fd_text);
                            uninstr_r("div", binary_operator->right->reg, type_size(binary_operator->right->type), fd_text);
                            bininstr_rr("mov", value->reg, RDX, type_size(value->type), fd_text);
                            from_reg(value, fd_text);
                            free_reg(binary_operator->left);
                            free_reg(binary_operator->right);
                        } break;
                        case IRNodeLess: {
                            bininstr_rr("xor", RAX, RAX, 8, fd_text);
                            to_reg(binary_operator->left, RCX, fd_text);
                            bininstr_vv("cmp", binary_operator->left, binary_operator->right, fd_text);
                            free_reg(binary_operator->left);
                            uninstr_r("setl", RAX, 1, fd_text);
                            bininstr_vr("mov", value, RAX, fd_text);
                        } break;
                        case IRNodeGreater: {
                            bininstr_rr("xor", RAX, RAX, 8, fd_text);
                            to_reg(binary_operator->left, RCX, fd_text);
                            bininstr_vv("cmp", binary_operator->left, binary_operator->right, fd_text);
                            free_reg(binary_operator->left);
                            uninstr_r("setg", RAX, 1, fd_text);
                            bininstr_vr("mov", value, RAX, fd_text);
                        } break;
                        case IRNodeEqual: {
                            bininstr_rr("xor", RAX, RAX, 8, fd_text);
                            to_reg(binary_operator->left, RCX, fd_text);
                            bininstr_vv("cmp", binary_operator->left, binary_operator->right, fd_text);
                            free_reg(binary_operator->left);
                            uninstr_r("sete", RAX, 1, fd_text);
                            bininstr_vr("mov", value, RAX, fd_text);
                        } break;
                        case IRNodeLessEqual: {
                            bininstr_rr("xor", RAX, RAX, 8, fd_text);
                            to_reg(binary_operator->left, RCX, fd_text);
                            bininstr_vv("cmp", binary_operator->left, binary_operator->right, fd_text);
                            free_reg(binary_operator->left);
                            uninstr_r("setle", RAX, 1, fd_text);
                            bininstr_vr("mov", value, RAX, fd_text);
                        } break;
                        case IRNodeGreaterEqual: {
                            bininstr_rr("xor", RAX, RAX, 8, fd_text);
                            to_reg(binary_operator->left, RCX, fd_text);
                            bininstr_vv("cmp", binary_operator->left, binary_operator->right, fd_text);
                            free_reg(binary_operator->left);
                            uninstr_r("setge", RAX, 1, fd_text);
                            bininstr_vr("mov", value, RAX, fd_text);
                        } break;
                        case IRNodeNotEqual: {
                            bininstr_rr("xor", RAX, RAX, 8, fd_text);
                            to_reg(binary_operator->left, RCX, fd_text);
                            bininstr_vv("cmp", binary_operator->left, binary_operator->right, fd_text);
                            free_reg(binary_operator->left);
                            uninstr_r("setne", RAX, 1, fd_text);
                            bininstr_vr("mov", value, RAX, fd_text);
                        } break;
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
