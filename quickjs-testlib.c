#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

static JSClassID js_testlib_win_class_id;

typedef struct {
    char *title;
    int width, height;
} JSTESTLIBWin;

static JSValue js_testlib_new_win(JSContext *ctx, JSValueConst this_val,
                                  int argc, JSValueConst *argv) {
    printf("js_testlib_new_win()\n");
    JSTESTLIBWin *w;
    JSValue obj;
    obj = JS_NewObjectClass(ctx, js_testlib_win_class_id);
    if (JS_IsException(obj))
        return obj;
    w = js_mallocz(ctx, sizeof(*w)); 
    if (!w) {
        JS_FreeValue(ctx, obj);
        return JS_EXCEPTION;
    }
    w->title = NULL;
    w->width = 100;
    w->height = 100;
    JS_SetOpaque(obj, w);
    return obj;
}

static void js_testlib_win_finalizer(JSRuntime *rt, JSValue val) {
    printf("js_testlib_win_finalizer()\n");
    JSTESTLIBWin *s = JS_GetOpaque(val, js_testlib_win_class_id);
    if (s) {
        js_free_rt(rt, s);
    }
}

static JSValue js_testlib_win_set_title(JSContext *ctx, JSValueConst this_val,
                                        int argc, JSValueConst *argv) {
    const char *str;
    JSTESTLIBWin *w = JS_GetOpaque2(ctx, this_val, js_testlib_win_class_id);
    if (!w)
        return JS_EXCEPTION;
    if (argc == 0)
        return JS_EXCEPTION;
    str = JS_ToCString(ctx, argv[0]);
    if (!str)
        return JS_EXCEPTION;

    if (w->title)
        free(w->title);
    w->title = strdup(str);
    JS_SetPropertyStr(ctx, this_val, "title", JS_NewString(ctx, str));

    printf("js_testlib_win_set_title title: '%s'\n", str);
    JS_FreeCString(ctx, str);
    return JS_UNDEFINED;
}

static JSValue js_testlib_win_set_size(JSContext *ctx, JSValueConst this_val,
                                        int argc, JSValueConst *argv) {
    int32_t width, height;
    JSTESTLIBWin *w = JS_GetOpaque2(ctx, this_val, js_testlib_win_class_id);
    if (!w)
        return JS_EXCEPTION;
    if (argc < 2)
        return JS_EXCEPTION;

    if (JS_ToInt32(ctx, &width, argv[0]))
        return JS_EXCEPTION;
    if (JS_ToInt32(ctx, &height, argv[1]))
        return JS_EXCEPTION;
    printf("js_testlib_win_set_size width/height: %d,%d\n", width, height);
    w->width = width;
    w->height = height;
    JS_SetPropertyStr(ctx, this_val, "width", JS_NewInt32(ctx, width));
    JS_SetPropertyStr(ctx, this_val, "height", JS_NewInt32(ctx, height));
    return JS_UNDEFINED;
}

static const JSCFunctionListEntry js_testlib_funcs[] = {
    JS_CFUNC_DEF("sum_all", 1, js_testlib_sum_all),
    JS_CFUNC_DEF("printobj", 1, js_testlib_printobj),
    JS_CFUNC_DEF("new_win", 0, js_testlib_new_win),
};

static const JSCFunctionListEntry js_testlib_win_proto_funcs[] = {
    JS_CFUNC_DEF("set_title", 1, js_testlib_win_set_title),
    JS_CFUNC_DEF("set_size", 2, js_testlib_win_set_size),
};

static JSClassDef js_testlib_win_class = {
    "WIN",
    .finalizer = js_testlib_win_finalizer,
};

static int js_testlib_init(JSContext *ctx, JSModuleDef *m) {
    JSValue proto;

    // Win class
    JS_NewClassID(&js_testlib_win_class_id);
    JS_NewClass(JS_GetRuntime(ctx), js_testlib_win_class_id, &js_testlib_win_class);
    proto = JS_NewObject(ctx);
    JS_SetPropertyFunctionList(ctx, proto, js_testlib_win_proto_funcs,
                               countof(js_testlib_win_proto_funcs));
    JS_SetClassProto(ctx, js_testlib_win_class_id, proto);

    JS_SetModuleExportList(ctx, m, js_testlib_funcs,
                           countof(js_testlib_funcs));
    return 0;
}

JSModuleDef *js_init_module_testlib(JSContext *ctx, const char *module_name) {
    JSModuleDef *m;
    m = JS_NewCModule(ctx, module_name, js_testlib_init);
    if (!m)
        return NULL;
    JS_AddModuleExportList(ctx, m, js_testlib_funcs, countof(js_testlib_funcs));
    return m;
}

