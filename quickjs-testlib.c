#include "quickjs.h"
#include "cutils.h"

static JSValue js_testlib_sum_all(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv) {
    int sum = 0;
    for (int i=0; i < argc; i++) {
        int n;
        if (JS_ToInt32(ctx, &n, argv[i])) {
            continue;
        }
        sum += n;
    }
    return JS_NewInt32(ctx, sum);
}

static JSValue js_testlib_printobj(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv) {
    if (argc == 0) {
        return JS_UNDEFINED;
    }
    JSValue v = JS_GetPropertyStr(ctx, argv[0], "name");
    const char *name_val = JS_ToCString(ctx, v);
    printf("name: '%s'\n", name_val);
    JS_FreeCString(ctx, name_val);

    return JS_UNDEFINED;
}

static const JSCFunctionListEntry js_testlib_funcs[] = {
    JS_CFUNC_DEF("sum_all", 1, js_testlib_sum_all),
    JS_CFUNC_DEF("printobj", 1, js_testlib_printobj),
};

static int js_testlib_init(JSContext *ctx, JSModuleDef *m) {
    return JS_SetModuleExportList(ctx, m, js_testlib_funcs, countof(js_testlib_funcs));
}

JSModuleDef *js_init_module_testlib(JSContext *ctx, const char *module_name) {
    JSModuleDef *m;
    m = JS_NewCModule(ctx, module_name, js_testlib_init);
    if (!m)
        return NULL;
    JS_AddModuleExportList(ctx, m, js_testlib_funcs, countof(js_testlib_funcs));
    return m;
}

