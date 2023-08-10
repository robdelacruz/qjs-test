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
    int z;
    JSValueConst v, v2;
    JSValueConst p1;
    const char *name, *detail1;
    int detail2;

    if (argc == 0) {
        return JS_UNDEFINED;
    }
    p1 = argv[0];

    v = JS_GetPropertyStr(ctx, p1, "name");
    name = JS_ToCString(ctx, v);
    JS_FreeValue(ctx, v);

    v = JS_GetPropertyStr(ctx, p1, "details");
    v2 = JS_GetPropertyStr(ctx, v, "detail1");
    detail1 = JS_ToCString(ctx, v2);
    JS_FreeValue(ctx, v);

    v2 = JS_GetPropertyStr(ctx, v, "detail2");
    z = JS_ToInt32(ctx, &detail2, v2);
    if (z) return JS_EXCEPTION;

    int32_t array_len;
    v = JS_GetPropertyStr(ctx, p1, "list1");
    v2 = JS_GetPropertyStr(ctx, v, "length");
    z = JS_ToInt32(ctx, &array_len, v2);
    if (z) return JS_EXCEPTION;
    if (JS_IsArray(ctx, v)) {
        printf("array_len: %d\n", array_len);
        for (int i=0; i < array_len; i++) {
            v2 = JS_GetPropertyUint32(ctx, v, i);
            int n;
            z = JS_ToInt32(ctx, &n, v2);
            if (z) return JS_EXCEPTION;

            if (i == array_len-1) {
                printf("%d\n", n);
            } else {
                printf("%d, ", n);
            }
        }
    }

    printf("name: '%s'\n", name);
    printf("detail1: '%s'\n", detail1);
    printf("detail2: %d\n", detail2);

    JS_FreeCString(ctx, name);
    JS_FreeCString(ctx, detail1);
    JS_FreeValue(ctx, v);
    JS_FreeValue(ctx, v2);


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

