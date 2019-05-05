
#include <jni.h>
#include <dlfcn.h>
#include <android/log.h>
#include <string.h>
#include <malloc.h>
#include <stdbool.h>

#include <stdlib.h>

#include <stdio.h>
#include "include/inlineHook.h"
#include "include/dlfcn_compat.h"


//#include "mono/metadata/image.h"
//#include "mono/eglib/glib.h"


#define TAG "Q296488320"
//#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__);
//#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__);
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__);

jboolean isU3d = false;



struct _MonoImage{
    int ref_count;
    void *raw_data_handle;
    char *raw_data;
    int raw_data_len;

}typedef MonoImage;






//在 map里面对应的 内容

// art的 so
#define Load_SO      "/data/app/com.avalon.caveonline.cn.leiting-1/lib/arm/libcocos2dlua.so"
// dvm 的 so路径
#define DvmLoad_SO   "/data/data/com.lyh.q296488320/lib/libcocos2dlua.so"



MonoImage *  (*my_mono_image_init_mod_t)(char *data,
                                 size_t data_len,
                                 int  need_copy,
                                 void *status,
                                 int  refonly, char *name)=NULL;


int (*origin_luaL_loadbuffer)(void *lua_state, char *buff, size_t size, char *name) = NULL;

void free(void *__ptr);


int my_luaL_loadbuffer(void *lua_state, char *buff, size_t size, char *name) {
    LOGE("lua size: %d, name: %s", (uint32_t) size, name);
    if (name != NULL) {
        char *name_t = strdup(name);
        if (name_t != " " && name_t[0] != ' ') {
            FILE *file;
            char full_name[256];
            int name_len = strlen(name);
            if (8 < name_len <= 100) {
                char *base_dir = (char *) "/sdcard/hookLua/";
                int i = 0;
                while (i < name_len) {
                    if (name_t[i] == '/') {
                        name_t[i] = '.';
                    }
                    i++;
                }
                if (strstr(name_t, ".lua")) {
                    sprintf(full_name, "%s%s", base_dir, name_t);
                    file = fopen(full_name, "wb");
                    if (file != NULL) {
                        fwrite(buff, 1, size, file);
                        fclose(file);
                        free(name_t);
                    }



                    //lua脚本hook加载
                    /*file = fopen(full_name, "r");
                    if (file != NULL) {
                        LOGD("[Tencent]-------path-----%s", full_name);
                        fseek(file, 0, SEEK_END);
                        size_t new_size = ftell(file);
                        fseek(file, 0, SEEK_SET);
                        char *new_buff = (char *) alloca(new_size + 1);
                        fread(new_buff, new_size, 1, file);
                        fclose(file);
                        return origin_luaL_loadbuffer(lua_state, buff, size, name);
                    }*/




                }
            }
        }
    }

    return origin_luaL_loadbuffer(lua_state, buff, size, name);
}


//mono_image_open_from_data_with_name
//        (char *data,
//         guint32 data_len,
//         gboolean need_copy,
//         MonoImageOpenStatus *status,
//         gboolean refonly,
//         const char *name)
//data: 脚本内容
//
//data_len:脚本长度
//
//name:脚本名称

MonoImage * my_mono_image_init_mod(char *data,
                                   size_t data_len,
                                   int need_copy,
                                   void *status,
                                   int refonly,
                                   char *name) {


    if (name != NULL) {
        //strdup()在内部调用了malloc()为变量分配内存
        char *name_t = strdup(name);
        if (name_t != " " && name_t[0] != ' ') {
            FILE *file;
            char full_name[256];
            int name_len = strlen(name);
            //if (8 < name_len <= 100) {
            char *base_dir = (char *) "/sdcard/hookDll/";
            int i = 0;
            while (i < name_len) {
                //将 / 换成 .
                if (name_t[i] == '/') {
                    name_t[i] = '.';
                }
                i++;
            }
            //lua脚本保存
            file = fopen(full_name, "wb");
            if (file != NULL) {
                fwrite(data, 1, data_len, file);
                fclose(file);
                free(name_t);
            }

        }
    }
    return my_mono_image_init_mod_t(data,data_len,need_copy,status,refonly,name);

}


JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {


    LOGE("JNI_OnLoad 开始加载");
    //在 onload 改变 指定函数 函数地址 替换成自己的
    JNIEnv *env = NULL;
    if ((*vm)->GetEnv(vm, (void **) &env, JNI_VERSION_1_6) == JNI_OK) {
        LOGE("GetEnv OK");
        void *handle=NULL;


        if(get_sdk_level() <= 19){
            // 4.4 直接 dlopen打开 就行 没有
            // dlopen限制 可以加载 任意路径的 so
             handle = dlopen(DvmLoad_SO, RTLD_NOW);
            LOGE("在 DVM 拿句柄 ");
        } else{
            handle = dlopen_compat(Load_SO, RTLD_NOW);
            LOGE("在 ART 拿句柄  ");
        }
        if (handle) {
            LOGE("拿到 so 地址 ");
            if (isU3d) {
                //从句柄里拿到 luaL_loadbuffer 函数
                //根据 动态链接库 操作句柄(handle)与符号(symbol)，
                // 返回符号对应的地址。使用这个函数不但可以获取函数地址，
                // 也可以获取变量地址。
                //是否是 dump  u3d的
                void *mono_image_open = dlsym_compat(handle, "mono_image_open_from_data_with_name");

                if (mono_image_open) {
                    if (ELE7EN_OK == registerInlineHook((uint32_t) mono_image_open,
                                                        (uint32_t) my_mono_image_init_mod,
                                                        (uint32_t **) &my_mono_image_init_mod_t)) {

                        if (ELE7EN_OK == inlineHook((uint32_t) mono_image_open)) {
                            LOGE("inlineHook mono_image_open_from_data_with_name success");
                        }
                    }
                }
///         dump lua的 Hook的 函数
            } else {
                LOGE(TAG, "开始 Hook Lua");
                void *luabuffer = dlsym_compat(handle, "luaL_loadbuffer");

                if (luabuffer) {
                    LOGE(TAG, "拿到了 luaL_loadbuffer 地址 ");
                    if (ELE7EN_OK == registerInlineHook((uint32_t) luabuffer,
                                                        (uint32_t) my_luaL_loadbuffer,
                                                        (uint32_t **) &origin_luaL_loadbuffer)) {

                        if (ELE7EN_OK == inlineHook((uint32_t) luabuffer)) {
                            LOGE("挂钩成功 luaL_loadbuffer ");
                        }
                    }
                }
            }
        }
        LOGE("JNI_OnLoad leave");
        return JNI_VERSION_1_6;
    }
}


