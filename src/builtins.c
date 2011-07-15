#ifndef BUILTINS
#define BUILTINS

// This file contains basic builtin functions like cons, car, cdr, quote, and so on.

#include "oyster.h"
#include "parsing.h"


// ------------------ The Apparatus -------------------//

oyster *make_builtin(oyster * (*func) (machine * m))
{
    oyster *ret = make_oyster(BUILT_IN_FUNCTION);
    ret->in->built_in = func;
    return ret;
}

oyster *arg(char *name)
{
    return make_symbol(sym_id_from_string(name));
}

oyster *unev(char *name)
{
    return list(2, make_symbol(CLEAR), arg(name));
}

oyster *quot(char *name)
{
    return list(2, arg("'"), arg(name));
}

void add_builtin(char *name,
                 oyster * lambda_list,
                 oyster * (*func) (machine * m), machine * m)
{
    oyster *ofunc = list(2, lambda_list, make_builtin(func));
    set(sym_id_from_string(name), ofunc, m->base_frame);
}



// ------------------ The Functions -------------------//
// TODO OMGS: in put val id ay shun

oyster *builtin_cons(machine * m)
{
    ARG(car);
    ARG(cdr);
    return cons(car, cdr);
}

oyster *builtin_car(machine * m)
{
    ARG(cons);
    if (cons->in->type != CONS) {
        oyster *signal = list(2, arg("car-of-wrong-type"), cons);
        toss_signal(make_signal(signal, m), m);
        return NULL;
    } 
    return car(cons);
}

oyster *builtin_cdr(machine * m)
{
    ARG(cons);
    if (cons->in->type != CONS) {
        oyster *signal = list(2, arg("cdr-of-wrong-type"), cons);
        toss_signal(make_signal(signal, m), m);
        return NULL;
    } 
    return cdr(cons);
}

oyster *builtin_set(machine * m)
{
    ARG(symbol);
    ARG(value);
    set(symbol->in->symbol_id, value, m->current_frame);
    return value;
}

oyster *builtin_leak(machine * m)
{
    // Gross.
    ARG(symbol);
    ARG(closure);
    int id = symbol->in->symbol_id;
    if (nilp(closure)){
        int i = 0;
        table_entry *a = table_get_entry(id, m->current_frame->scope_below, &i);
        if(i) {
            table_put_entry(id, a, m->current_frame->scope);
        } else {
            a = make_table_entry(NULL);
            table_put_entry(id, a, m->current_frame->scope);
            table_put_entry(id, a, m->current_frame->scope_below);
        }
    } else {
        if(!closure->bindings)
            closure->bindings = make_table();
        leak(id, closure->bindings);
    }

    return closure;
}

oyster *builtin_quote(machine * m)
{
    ARG(x);
    return oyster_copy(x, m->now->scope_below);
}

oyster *builtin_atom_p(machine * m)
{
    ARG(x);
    if (x->in->type == CONS)
        return nil();
    return x;
}

oyster *builtin_is(machine * m)
{
    ARG(a);
    ARG(b);
    if (a->in->type == b->in->type && a->in->value == b->in->value)
        return make_symbol(sym_id_from_string("t"));
    return nil();
}


oyster *builtin_oif(machine * m)
{
    ARG(test);
    ARG(then);
    sARG(el, "else");
    if (!nilp(test)) {
        push_new_instruction(m, then, EVALUATE);
    } else {
        push_new_instruction(m, el, EVALUATE);
    }
    return NULL;
}

oyster *builtin_print(machine * m)
{
    // This gets a little silly...
    ARG(xs);
    incref(xs);
    oyster *ret = car(xs);
    do {
        oyster_print(car(xs));
        printf(" ");
        oyster *nxs = cdr(xs);
        incref(nxs);
        decref(xs);
        xs = nxs;
    } while (!nilp(xs));
    decref(xs);
    printf("\n");
    return ret;
}

oyster *builtin_signal(machine * m)
{
    ARG(message);
    toss_signal(make_signal(message, m), m);
    return NULL;
}

oyster *builtin_with_signal_handler(machine * m)
{
    ARG(handler);
    ARG(code);
    push_new_instruction(m, handler, HANDLE_SIGNALS);
    push_new_instruction(m, code, EVALUATE);
    incref(handler);
    return handler;
}

oyster *builtin_current_scope(machine * m)
{
    oyster *ret = make_oyster(sym_id_from_string("table"));
    ret->in->value = m->current_frame->scope;
    incref(ret->in->value);
    return ret;
}

oyster *builtin_table_get(machine * m)
{
    sARG(tab, "table");
    ARG(symbol);
    int i = 0;
    oyster *ret = table_get(symbol->in->symbol_id, tab->in->value, &i);
    if (!i)
        return nil();
    return list(1, ret);
}

oyster *print_bindings(machine * m)
{
    ARG(x);
    table_print(x->bindings);
    return x;
}

void add_builtins(machine * m)
{
    add_builtin("cons", list(2, arg("car"), arg("cdr")), builtin_cons, m);
    add_builtin("car", list(1, arg("cons")), builtin_car, m);
    add_builtin("cdr", list(1, arg("cons")), builtin_cdr, m);

    add_builtin("set!", list(2, unev("symbol"), arg("value")), builtin_set,
                m);
    add_builtin("leak", list(2, unev("symbol"), arg("closure")),
                builtin_leak, m);

    add_builtin("'", list(1, unev("x")), builtin_quote, m);

    add_builtin("atom-p", list(1, arg("x")), builtin_atom_p, m);
    add_builtin("is", list(2, arg("a"), arg("b")), builtin_is, m);
    add_builtin("oif", list(3, arg("test"), quot("then"), quot("else")),
                builtin_oif, m);

    add_builtin("print", list(2, arg("..."), arg("xs")), builtin_print, m);

    add_builtin("signal", list(1, arg("message")), builtin_signal, m);
    add_builtin("with-signal-handler",
                list(3, quot("handler"),
                        arg("..."),
                        quot("code")),
                builtin_with_signal_handler, m);

    add_builtin("current-scope", nil(), builtin_current_scope, m);
    add_builtin("table-get", list(2, arg("table"), unev("symbol")),
                builtin_table_get, m);

    add_builtin("print-bindings", list(1, arg("x")),
                print_bindings, m);
}

#endif