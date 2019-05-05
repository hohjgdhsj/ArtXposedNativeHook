package com.lyh.q296488320;

import android.util.Log;

import de.robv.android.xposed.IXposedHookLoadPackage;
import de.robv.android.xposed.XC_MethodHook;
import de.robv.android.xposed.XposedHelpers;
import de.robv.android.xposed.callbacks.XC_LoadPackage;

import static de.robv.android.xposed.XposedHelpers.findAndHookMethod;


public class main implements IXposedHookLoadPackage {
    public static final String TAG = "Q296488320";
    public static final String PackageName = "com.avalon.caveonline.cn.leiting";
    //libcocos2dlua
    public static final String SoName = "libcocos2dlua.so";
    public static final String DUMP_LUA_SO = "/data/data/com.lyh.q296488320/lib/libdumpLua.so";


    @Override
    public void handleLoadPackage(XC_LoadPackage.LoadPackageParam param) throws Throwable {

        Log.e(TAG, param.packageName);
        if (!param.packageName.equals(PackageName))
            return;
        Log.e(TAG, "找到了 要Hook的 包名 " + param.packageName);
        findAndHookMethod(Runtime.class, "doLoad", String.class, ClassLoader.class,
                new XC_MethodHook() {
                    @Override
                    protected void afterHookedMethod(MethodHookParam param) throws Throwable {
                        String name = (String) param.args[0];
                        Log.e(TAG, "Load so file: " + name);

                        //Hook 要加载的 so
                        //当 加载 的 so 是 要 加载的 就加载自己的  libcocos2dlua
                        if (param.hasThrowable() || name == null || !name.endsWith(SoName)) {
                            return;
                        }
                        try {
                            if (android.os.Build.VERSION.SDK_INT <= 19) {
                                Log.e(TAG, "走了 4.4的  load方法 ");
                                System.load(DUMP_LUA_SO);
                            }else {
                                //如果 包含 加载的 so 就 加载自己的 在 自己so里面 再打开 目标 so
                                if (((String) param.args[0]).contains(SoName)) {
                                    Log.e(TAG, "开始调用");
                                    //注入 自己的 so 吧 classloader进行 传入
                                    initMySo(param.args[1]);
                                }
                            }
                        } catch (Exception e) {
                            Log.e(TAG, "异常" + e.getMessage());
                        }
                        Log.e(TAG, "java层执行完毕");
                    }
                });

    }

    /**
     * 在这里 把自己的 so进行 注入
     *
     * @param arg
     */
    private void initMySo(Object arg) {
        String path = "/data/app/" + "com.lyh.q296488320" + "-1/lib/arm/libdumpLua.so";
        XposedHelpers.callMethod(Runtime.getRuntime(), "doLoad", path, arg);
        Log.e(TAG, "注入成功");

    }


}
