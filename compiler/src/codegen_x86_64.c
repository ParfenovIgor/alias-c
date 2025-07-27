#include <stdio.h>
#include <string.h>
#include <posix.h>
#include <ast.h>
#include <context.h>
#include <compile.h>
#include <codegen.h>

void x86_64_compile_memcpy(const char *dst, const char *src, int sz, struct CPContext *context) {
    if (sz == 1) {
        _fputs3(context->fd_text, "mov cl, ", src, "\n");
        _fputs3(context->fd_text, "mov ", dst, ", cl\n");
    }
    else if (sz == 8) {
        _fputs3(context->fd_text, "mov rcx, ", src, "\n");
        _fputs3(context->fd_text, "mov ", dst, ", rcx\n");
    }
    else {
        _fputs3(context->fd_text, "lea rdi, ", dst, "\n");
        _fputs3(context->fd_text, "lea rsi, ", src, "\n");
        _fputsi(context->fd_text, "mov rcx, ", sz, "\n");
        _fputs(context->fd_text, "rep movsb\n");
    }
}

void x86_64_compile_memzero(const char *dst, int sz, struct CPContext *context) {
    if (sz == 1) {
        _fputs3(context->fd_text, "mov byte", dst, ", 0\n");
    }
    else if (sz == 8) {
        _fputs3(context->fd_text, "mov qword", dst, ", 0\n");
    }
    else {
        _fputs(context->fd_text, "mov rsi, rax\n");
        _fputs3(context->fd_text, "lea rdi, ", dst, "\n");
        _fputs(context->fd_text, "mov rax, 0\n");
        _fputsi(context->fd_text, "mov rcx, ", sz, "\n");
        _fputs(context->fd_text, "rep stosb\n");
        _fputs(context->fd_text, "mov rax, rsi\n");
    }
}

const char *x86_64_codegen_register(enum Register reg) {
    switch (reg) {
        case REG_A: return "rax";
        case REG_B: return "rbx";
        case REG_C: return "rcx";
        case REG_D: return "rdx";
        default: return NULL;
    }
}

void x86_64_codegen_buffer_zero(const char *label, int size, struct CPContext *context) {
    _fputs2(context->fd_bss, label, ":\n");
    _fputsi(context->fd_bss, "resb ", size, "\n");
}

void x86_64_codegen_buffer_int(const char *label, int value, struct CPContext *context) {
    _fputs2(context->fd_data, label, ":\n");
    _fputsi(context->fd_data, "dq ", value, "\n");
}

void x86_64_codegen_buffer_string(const char *label, const char *str, struct CPContext *context) {
    _fputs2(context->fd_data, label, ":\n");
    _fputs(context->fd_data, "db ");
    for (int i = 0; i < _strlen(str); i++) {
        _fputi(context->fd_data, str[i]);
        _fputs(context->fd_data, ", ");
    }
    _fputs(context->fd_data, "0\n");
}

void x86_64_codegen_label(const char *label, struct CPContext *context) {
    _fputs2(context->fd_text, label, ":\n");
}

void x86_64_codegen_dereference(enum Register dst, enum Register src, struct CPContext *context) {
    _fputs3(context->fd_text, "mov ", x86_64_codegen_register(dst), ", [");
    _fputs2(context->fd_text, x86_64_codegen_register(src), "]\n");
}

void x86_64_codegen_move_regreg(enum Register dst, enum Register src, struct CPContext *context) {
    _fputs3(context->fd_text, "mov ", x86_64_codegen_register(dst), ", ");
    _fputs2(context->fd_text, x86_64_codegen_register(src), "\n");
}

void x86_64_codegen_move_labelreg(enum Register dst, const char *label, struct CPContext *context) {
    _fputs3(context->fd_text, "mov ", x86_64_codegen_register(dst), ", ");
    _fputs2(context->fd_text, label, "\n");
}

void x86_64_codegen_jump(const char *label, struct CPContext *context) {
    _fputs3(context->fd_text, "jmp ", label, "\n");
}

void x86_64_codegen_jump_ifzero(enum Register reg, const char *label, struct CPContext *context) {
    _fputs3(context->fd_text, "cmp ", x86_64_codegen_register(reg), ", 0\n");
    _fputs3(context->fd_text, "je ", label, "\n");
}

void x86_64_codegen_jump_ifnonzero(enum Register reg, const char *label, struct CPContext *context) {
    _fputs3(context->fd_text, "cmp ", x86_64_codegen_register(reg), ", 0\n");
    _fputs3(context->fd_text, "jne ", label, "\n");
}

void x86_64_codegen_def_extern(const char *label, struct CPContext *context) {
    _fputs3(context->fd_text, "extern ", label, "\n");
}

void x86_64_codegen_def_global(const char *label, struct CPContext *context) {
    _fputs3(context->fd_text, "global ", label, "\n");
}

void x86_64_codegen_add_reg(enum Register dst, enum Register src, struct CPContext *context) {
    _fputs3(context->fd_text, "add ", x86_64_codegen_register(dst), ", ");
    _fputs2(context->fd_text, x86_64_codegen_register(src), "\n");
}

void x86_64_codegen_add_const(enum Register dst, int x, struct CPContext *context) {
    _fputs3(context->fd_text, "add ", x86_64_codegen_register(dst), ", ");
    _fputs2(context->fd_text, _itoa(x), "\n");
}

void x86_64_codegen_stack_shift(int x, struct CPContext *context) {
    _fputsi(context->fd_text, "add rsp, ", x, "\n");
}

void x86_64_codegen_stack_pushword(enum Register reg, struct CPContext *context) {
    _fputs3(context->fd_text, "mov qword [rsp - 8], ", x86_64_codegen_register(reg), "\n");
}

void x86_64_codegen_stack_popword(enum Register reg, struct CPContext *context) {
    _fputs3(context->fd_text, "mov qword ", x86_64_codegen_register(reg), ", [rsp - 8]\n");
}

void x86_64_codegen_stack_pushint(int n, struct CPContext *context) {
    _fputs3(context->fd_text, "mov qword [rsp - 8], ", _itoa(n), "\n");
}

void x86_64_codegen_stack_pushlabel(const char *label, struct CPContext *context) {
    _fputs3(context->fd_text, "mov qword [rsp - 8], ", label, "\n");
}

void x86_64_codegen_stack_pushword_phase(enum Register reg, int phase, struct CPContext *context) {
    _fputsi(context->fd_text, "mov qword [rsp + ", phase, "], ");
    _fputs2(context->fd_text, x86_64_codegen_register(reg), "\n");
}

void x86_64_codegen_stack_popword_phase(enum Register reg, int phase, struct CPContext *context) {
    _fputs2(context->fd_text, "mov qword ", x86_64_codegen_register(reg));
    _fputsi(context->fd_text, ", [rsp + ", phase, "]\n");
}

void x86_64_codegen_stack_push_memcpy(enum Register src, int type_size, struct CPContext *context) {
    const char *dst = _concat3("[rsp - ", _itoa(align_to_word(type_size)), "]");
    x86_64_compile_memzero(dst, align_to_word(type_size), context);
    x86_64_compile_memcpy(dst, _concat3("[", x86_64_codegen_register(src), "]"), type_size, context);
}

void x86_64_codegen_stack_pop_memcpy(enum Register dst, int type_size, struct CPContext *context) {
    const char *src = _concat3("[rsp - ", _itoa(align_to_word(type_size)), "]");
    x86_64_compile_memcpy(_concat3("[", x86_64_codegen_register(dst), "]"), src, type_size, context);
}

void x86_64_codegen_stack_pop_memcpy_label(const char *label, int type_size, struct CPContext *context) {
    const char *src = _concat3("[rsp - ", _itoa(align_to_word(type_size)), "]");
    x86_64_compile_memcpy(_concat3("[", label, "]"), src, type_size, context);
}

void x86_64_codegen_stack_pop_memcpy_stackframe(int stack_phase, int type_size, struct CPContext *context) {
    const char *dst = _concat3("[rbp + ", _itoa(stack_phase), "]");
    const char *src = _concat3("[rsp - ", _itoa(align_to_word(type_size)), "]");
    x86_64_compile_memcpy(dst, src, type_size, context);
}

void x86_64_codegen_stackframe_memcpy(int dst_phase, int src_phase, int type_size, struct CPContext *context) {
    const char *dst = _concat3("[rbp + ", _itoa(dst_phase), "]");
    const char *src = _concat3("[rbp + ", _itoa(src_phase), "]");
    x86_64_compile_memcpy(dst, src, type_size, context);
}

void x86_64_codegen_function_prologue(struct CPContext *context) {
    _fputs(context->fd_text, "push rbp\n");
    _fputs(context->fd_text, "mov rbp, rsp\n");
}

void x86_64_codegen_function_epilogue(struct CPContext *context) {
    _fputs(context->fd_text, "leave\n");
    _fputs(context->fd_text, "ret\n");
}

void x86_64_codegen_syscall(const char *func, const char *arg, struct CPContext *context) {
    _fputs3(context->fd_text, "mov rax, ", func, "\n");
    _fputs3(context->fd_text, "mov rdi, ", arg, "\n");
    _fputs(context->fd_text, "syscall\n");
}

void x86_64_codegen_from_stackframe_to_stack(int stackframe_phase, int size, struct CPContext *context) {    
    const char *src = _concat3("[rbp + ", _itoa(stackframe_phase), "]");
    const char *dst = _concat3("[rsp - ", _itoa(size), "]");
    x86_64_compile_memcpy(dst, src, size, context);
}

void x86_64_codegen_from_global_to_stack(const char *identifier, int size, struct CPContext *context) {    
    const char *src = _concat3("[", identifier, "]");
    const char *dst = _concat3("[rsp - ", _itoa(size), "]");
    x86_64_compile_memcpy(dst, src, size, context);
}

void x86_64_codegen_get_stackframe_position(enum Register reg, int stackframe_phase, struct CPContext *context) {
    _fputs3(context->fd_text, "lea ", x86_64_codegen_register(reg), ", ");
    _fputsi(context->fd_text, "[rbp + ", stackframe_phase, "]\n");
}

const char *x86_64_codegen_call_arguments_registers[] = {
    "rdi",
    "rsi",
    "rdx",
    "rcx",
    "r8",
    "r9"
};

void x86_64_codegen_call_arguments_push(int n, struct CPContext *context) {
    if (n > sizeof(x86_64_codegen_call_arguments_registers) / sizeof(x86_64_codegen_call_arguments_registers[0])) {
        posix_exit(3);
    }
    for (int i = 0; i < n; i++) {
        _fputs3(context->fd_text, "push ", x86_64_codegen_call_arguments_registers[i], "\n");
    }
}

void x86_64_codegen_call_arguments_restore(int n, struct CPContext *context) {
    if (n > sizeof(x86_64_codegen_call_arguments_registers) / sizeof(x86_64_codegen_call_arguments_registers[0])) {
        posix_exit(3);
    }
    for (int i = 0; i < n; i++) {
        _fputs3(context->fd_text, "mov ", x86_64_codegen_call_arguments_registers[i], ", ");
        _fputsi(context->fd_text, "[rsp - ", WORD * (i + 2), "]\n");
    }
}

void x86_64_codegen_call_reg(enum Register reg, struct CPContext *context) {
    _fputs3(context->fd_text, "call ", x86_64_codegen_register(reg), "\n");
}

void x86_64_codegen_call_label(const char *label, struct CPContext *context) {
    _fputs3(context->fd_text, "call ", label, "\n");
}

void x86_64_codegen_index(int multiplier, struct CPContext *context) {
    _fputs(context->fd_text, "mov rax, [rsp - 16]\n");
    _fputsi(context->fd_text, "mov rbx, ", multiplier, "\n");
    _fputs(context->fd_text, "mul rbx\n");
    _fputs(context->fd_text, "add rax, [rsp - 8]\n");
}

void x86_64_codegen_pack_struct(struct Vector *type_sizes, struct CPContext *context) {
    int sz = vsize(type_sizes);
    int orig_struct_size = 0;
    int packed_struct_size = 0;
    for (int i = 0; i < sz; i++) {
        int field_size = (long)type_sizes->ptr[i];
        orig_struct_size += align_to_word(field_size);
        packed_struct_size += field_size;
    }
    int ptr1 = 0;
    int ptr2 = 0;
    for (int i = 0; i < sz; i++) {
        const char *dst = _concat3("[rsp - ", _itoa(orig_struct_size + packed_struct_size - ptr1), "]");
        const char *src = _concat3("[rsp - ", _itoa(orig_struct_size - ptr2), "]");
        int tsize = (long)type_sizes->ptr[i];
        x86_64_compile_memcpy(dst, src, tsize, context);
        ptr1 += tsize;
        ptr2 += align_to_word(tsize);
    }

    const char *dst = _concat3("[rsp - ", _itoa(align_to_word(packed_struct_size)), "]");
    const char *src = _concat3("[rsp - ", _itoa(orig_struct_size + packed_struct_size), "]");
    x86_64_compile_memcpy(dst, src, packed_struct_size, context);
}

void x86_64_codegen_arithmetic(enum NodeType node_type, bool unary, struct CPContext *context) {
    switch (node_type) {
        case NodeBitwiseAnd: {
            _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
            _fputs(context->fd_text, "and rax, [rsp - 16]\n");
        } break;
        case NodeBitwiseOr: {
            _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
            _fputs(context->fd_text, "or rax, [rsp - 16]\n");
        } break;
        case NodeBitwiseXor: {
            _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
            _fputs(context->fd_text, "xor rax, [rsp - 16]\n");
        } break;
        case NodeBitwiseNot: {
            _fputs(context->fd_text, "mov rax, [rsp - 16]\n");
            _fputs(context->fd_text, "not rax\n");
        } break;
        case NodeBitwiseShiftLeft: {
            _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
            _fputs(context->fd_text, "mov rcx, [rsp - 16]\n");
            _fputs(context->fd_text, "shl rax, cl\n");
        } break;
        case NodeBitwiseShiftRight: {
            _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
            _fputs(context->fd_text, "mov rcx, [rsp - 16]\n");
            _fputs(context->fd_text, "shr rax, cl\n");
        } break;
        case NodeAddition: {
            _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
            _fputs(context->fd_text, "add rax, [rsp - 16]\n");
        } break;
        case NodeSubtraction: {
            if (!unary) _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
            else _fputs(context->fd_text, "mov rax, 0\n");
            _fputs(context->fd_text, "sub rax, [rsp - 16]\n");
        } break;
        case NodeMultiplication: {
            _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
            _fputs(context->fd_text, "mov rdx, [rsp - 16]\n");
            _fputs(context->fd_text, "mul rdx\n");
        } break;
        case NodeDivision: {
            _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
            _fputs(context->fd_text, "mov rdx, 0\n");
            _fputs(context->fd_text, "div qword [rsp - 16]\n");
        } break;
        case NodeModulo: {
            _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
            _fputs(context->fd_text, "mov rdx, 0\n");
            _fputs(context->fd_text, "div qword [rsp - 16]\n");
            _fputs(context->fd_text, "mov rax, rdx\n");
        } break;
        case NodeAnd: {
            _fputs(context->fd_text, "xor rax, rax\n");
            _fputs(context->fd_text, "sub qword [rsp - 8], 0\n");
            _fputs(context->fd_text, "setne al\n");
            _fputs(context->fd_text, "xor rbx, rbx\n");
            _fputs(context->fd_text, "sub qword [rsp - 16], 0\n");
            _fputs(context->fd_text, "setne bl\n");
            _fputs(context->fd_text, "and rax, rbx\n");
        }; break;
        case NodeOr: {
            _fputs(context->fd_text, "xor rax, rax\n");
            _fputs(context->fd_text, "sub qword [rsp - 8], 0\n");
            _fputs(context->fd_text, "setne al\n");
            _fputs(context->fd_text, "xor rbx, rbx\n");
            _fputs(context->fd_text, "sub qword [rsp - 16], 0\n");
            _fputs(context->fd_text, "setne bl\n");
            _fputs(context->fd_text, "or rax, rbx\n");
        } break;
        case NodeNot: {
            _fputs(context->fd_text, "xor rax, rax\n");
            _fputs(context->fd_text, "sub qword [rsp - 16], 0\n");
            _fputs(context->fd_text, "sete al\n");
        } break;
        case NodeLess: {
            _fputs(context->fd_text, "xor rax, rax\n");
            _fputs(context->fd_text, "mov rbx, [rsp - 8]\n");
            _fputs(context->fd_text, "sub rbx, [rsp - 16]\n");
            _fputs(context->fd_text, "setl al\n");
        } break;
        case NodeGreater: {
            _fputs(context->fd_text, "xor rax, rax\n");
            _fputs(context->fd_text, "mov rbx, [rsp - 8]\n");
            _fputs(context->fd_text, "sub rbx, [rsp - 16]\n");
            _fputs(context->fd_text, "setg al\n");
        } break;
        case NodeEqual: {
            _fputs(context->fd_text, "xor rax, rax\n");
            _fputs(context->fd_text, "mov rbx, [rsp - 8]\n");
            _fputs(context->fd_text, "sub rbx, [rsp - 16]\n");
            _fputs(context->fd_text, "sete al\n");
        } break;
        case NodeLessEqual: {
            _fputs(context->fd_text, "xor rax, rax\n");
            _fputs(context->fd_text, "mov rbx, [rsp - 8]\n");
            _fputs(context->fd_text, "sub rbx, [rsp - 16]\n");
            _fputs(context->fd_text, "setle al\n");
        } break;
        case NodeGreaterEqual: {
            _fputs(context->fd_text, "xor rax, rax\n");
            _fputs(context->fd_text, "mov rbx, [rsp - 8]\n");
            _fputs(context->fd_text, "sub rbx, [rsp - 16]\n");
            _fputs(context->fd_text, "setge al\n");
        } break;
        case NodeNotEqual: {
            _fputs(context->fd_text, "xor rax, rax\n");
            _fputs(context->fd_text, "mov rbx, [rsp - 8]\n");
            _fputs(context->fd_text, "sub rbx, [rsp - 16]\n");
            _fputs(context->fd_text, "setne al\n");
        } break;
        default: {
            posix_exit(3);
        }
    }
}

struct Codegen *x86_64_codegen_init() {
    struct Codegen *codegen = (struct Codegen*)_malloc(sizeof(struct Codegen));
    codegen->buffer_zero = x86_64_codegen_buffer_zero;
    codegen->buffer_int = x86_64_codegen_buffer_int;
    codegen->buffer_string = x86_64_codegen_buffer_string;
    codegen->label = x86_64_codegen_label;
    codegen->dereference = x86_64_codegen_dereference;
    codegen->move_regreg = x86_64_codegen_move_regreg;
    codegen->move_labelreg = x86_64_codegen_move_labelreg;
    codegen->jump = x86_64_codegen_jump;
    codegen->jump_ifzero = x86_64_codegen_jump_ifzero;
    codegen->jump_ifnonzero = x86_64_codegen_jump_ifnonzero;
    codegen->def_extern = x86_64_codegen_def_extern;
    codegen->def_global = x86_64_codegen_def_global;
    codegen->add_reg = x86_64_codegen_add_reg;
    codegen->add_const = x86_64_codegen_add_const;
    codegen->stack_shift = x86_64_codegen_stack_shift;
    codegen->stack_pushword = x86_64_codegen_stack_pushword;
    codegen->stack_popword = x86_64_codegen_stack_popword;
    codegen->stack_pushint = x86_64_codegen_stack_pushint;
    codegen->stack_pushlabel = x86_64_codegen_stack_pushlabel;
    codegen->stack_pushword_phase = x86_64_codegen_stack_pushword_phase;
    codegen->stack_popword_phase = x86_64_codegen_stack_popword_phase;
    codegen->stack_push_memcpy = x86_64_codegen_stack_push_memcpy;
    codegen->stack_pop_memcpy = x86_64_codegen_stack_pop_memcpy;
    codegen->stack_pop_memcpy_label = x86_64_codegen_stack_pop_memcpy_label;
    codegen->stack_pop_memcpy_stackframe = x86_64_codegen_stack_pop_memcpy_stackframe;
    codegen->stackframe_memcpy = x86_64_codegen_stackframe_memcpy;
    codegen->function_prologue = x86_64_codegen_function_prologue;
    codegen->function_epilogue = x86_64_codegen_function_epilogue;
    codegen->syscall = x86_64_codegen_syscall;
    codegen->from_stackframe_to_stack = x86_64_codegen_from_stackframe_to_stack;
    codegen->from_global_to_stack = x86_64_codegen_from_global_to_stack;
    codegen->get_stackframe_position = x86_64_codegen_get_stackframe_position;
    codegen->call_arguments_push = x86_64_codegen_call_arguments_push;
    codegen->call_arguments_restore = x86_64_codegen_call_arguments_restore;
    codegen->call_reg = x86_64_codegen_call_reg;
    codegen->call_label = x86_64_codegen_call_label;
    codegen->index = x86_64_codegen_index;
    codegen->pack_struct = x86_64_codegen_pack_struct;
    codegen->arithmetic = x86_64_codegen_arithmetic;
    return codegen;
}
