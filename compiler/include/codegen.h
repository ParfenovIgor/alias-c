#include <ast.h>
#include <context.h>

enum Register {
    REG_A,
    REG_B,
    REG_C,
    REG_D
};

struct Codegen {
    void (*buffer_zero)                 (const char *label, int size, struct CPContext *context);
    void (*buffer_int)                  (const char *label, int value, struct CPContext *context);
    void (*buffer_string)               (const char *label, const char *str, struct CPContext *context);
    void (*label)                       (const char *label, struct CPContext *context);
    void (*dereference)                 (enum Register dst, enum Register src, struct CPContext *context);
    void (*move_regreg)                 (enum Register dst, enum Register src, struct CPContext *context);
    void (*move_labelreg)               (enum Register dst, const char *label, struct CPContext *context);
    void (*jump)                        (const char *label, struct CPContext *context);
    void (*jump_ifzero)                 (enum Register reg, const char *label, struct CPContext *context);
    void (*jump_ifnonzero)              (enum Register reg, const char *label, struct CPContext *context);
    void (*def_extern)                  (const char *label, struct CPContext *context);
    void (*def_global)                  (const char *label, struct CPContext *context);
    void (*add_reg)                     (enum Register dst, enum Register src, struct CPContext *context);
    void (*add_const)                   (enum Register dst, int x, struct CPContext *context);
    void (*stack_shift)                 (int x, struct CPContext *context);
    void (*stack_pushword)              (enum Register reg, struct CPContext *context);
    void (*stack_popword)               (enum Register reg, struct CPContext *context);
    void (*stack_pushint)               (int n, struct CPContext *context);
    void (*stack_pushlabel)             (const char *label, struct CPContext *context);
    void (*stack_pushword_phase)        (enum Register reg, int phase, struct CPContext *context);
    void (*stack_popword_phase)         (enum Register reg, int phase, struct CPContext *context);
    void (*stack_push_memcpy)           (enum Register src, int type_size, struct CPContext *context);
    void (*stack_pop_memcpy)            (enum Register dst, int type_size, struct CPContext *context);
    void (*stack_pop_memcpy_label)      (const char *label, int type_size, struct CPContext *context);
    void (*stack_pop_memcpy_stackframe) (int stack_phase, int type_size, struct CPContext *context);
    void (*stackframe_memcpy)           (int dst_phase, int src_phase, int type_size, struct CPContext *context);
    void (*function_prologue)           (struct CPContext *context);
    void (*function_epilogue)           (struct CPContext *context);
    void (*syscall)                     (const char *func, const char *arg, struct CPContext *context);
    void (*from_stackframe_to_stack)    (int stackframe_phase, int size, struct CPContext *context);
    void (*from_global_to_stack)        (const char *identifier, int size, struct CPContext *context);
    void (*get_stackframe_position)     (enum Register reg, int stackframe_phase, struct CPContext *context);
    void (*call_arguments_push)         (int n, struct CPContext *context);
    void (*call_arguments_restore)      (int n, struct CPContext *context);
    void (*call_reg)                    (enum Register reg, struct CPContext *context);
    void (*call_label)                  (const char *label, struct CPContext *context);
    void (*index)                       (int multiplier, struct CPContext *context);
    void (*pack_struct)                 (struct Vector *type_sizes, struct CPContext *context);
    void (*arithmetic)                  (enum NodeType node_type, bool unary, struct CPContext *context);
};

struct Codegen *x86_64_codegen_init();
// struct Codegen *c_codegen_init();

