/*
 *  Collin's Dynamic Dalvik Instrumentation Toolkit for Android
 *  Collin Mulliner <collin[at]mulliner.org>
 *
 *  (c) 2012,2013
 *
 *  License: LGPL v2.1
 *
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <string.h>
#include <termios.h>
#include <pthread.h>
#include <sys/epoll.h>

#include <jni.h>
#include <stdlib.h>

#include "hook.h"
#include "dexstuff.h"
#include "dalvik_hook.h"
#include "base.h"

#undef log

#include <android/log.h>

#define LOG_TAG_HOOK "dalvikThreadHook"
#define LOGE(fmt, args...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG_HOOK, fmt, ##args);

static struct hook_t eph;
static struct dexstuff_t d;

static int debug;

static void my_log(char *msg)
{
	LOGE("%s", msg)
}
static void my_log2(char *msg)
{
	if (debug)
		LOGE("%s", msg)
}

static struct dalvik_hook_t threadDHT;

static struct dalvik_hook_t threadCHT;

/*
// helper function
void printString(JNIEnv *env, jobject str, char *l)
{
	const char *s = (*env)->GetStringUTFChars(env, str, 0);
	if (s) {
		LOGE("%s%s\n", l, s)
		(*env)->ReleaseStringUTFChars(env, str, s); 
	}
}

// patches
static void* sb1_tostring(JNIEnv *env, jobject obj)
{
	dalvik_prepare(&d, &sb1, env);
	void *res = (*env)->CallObjectMethod(env, obj, sb1.mid); 
	LOGE("success calling : %s\n", sb1.method_name)
	dalvik_postcall(&d, &sb1);

	const char *s = (*env)->GetStringUTFChars(env, res, 0);
	if (s) {
		LOGE("sb1.toString() = %s\n", s)
		(*env)->ReleaseStringUTFChars(env, res, s); 
	}

	return res;
}

static void* sb20_tostring(JNIEnv *env, jobject obj)
{
	LOGE("tostring\n")
	LOGE("env = 0x%x\n", env)
	LOGE("obj = 0x%x\n", obj)

	dalvik_prepare(&d, &sb20, env);
	void *res = (*env)->CallObjectMethod(env, obj, sb20.mid); 
	LOGE("success calling : %s\n", sb20.method_name)
	dalvik_postcall(&d, &sb20);

	const char *s = (*env)->GetStringUTFChars(env, res, 0);
	if (s) {
		LOGE("sb20.toString() = %s\n", s)
		(*env)->ReleaseStringUTFChars(env, res, s); 
	}

	return res;
}

static void* sb2_compareto(JNIEnv *env, jobject obj, jobject str)
{

	LOGE("compareto\n")
	LOGE("env = 0x%x\n", env)
	LOGE("obj = 0x%x\n", obj)
	LOGE("str = 0x%x\n", str)


	jvalue args[1];
	args[0].l = str;
	dalvik_prepare(&d, &sb2, env);
	int res = (*env)->CallIntMethodA(env, obj, sb2.mid, args); 
	LOGE("success calling : %s\n", sb2.method_name)
	dalvik_postcall(&d, &sb2);

	printString(env, obj, "sb2 = "); 

	const char *s = (*env)->GetStringUTFChars(env, str, 0);
	if (s) {
		LOGE("sb2.comapreTo() = %d s> %s\n", res, s)
		(*env)->ReleaseStringUTFChars(env, str, s);
	}
	
	return (void *)res;
}

static void* sb3_comparetocase(JNIEnv *env, jobject obj, jobject str)
{
	LOGE("comparetocase\n")
	LOGE("env = 0x%x\n", env)
	LOGE("obj = 0x%x\n", obj)
	LOGE("str = 0x%x\n", str)


	jvalue args[1];
	args[0].l = str;
	dalvik_prepare(&d, &sb3, env);
	int res = (*env)->CallIntMethodA(env, obj, sb3.mid, args); 
	LOGE("success calling : %s\n", sb3.method_name)
	dalvik_postcall(&d, &sb3);

	printString(env, obj, "sb3 = "); 

	const char *s = (*env)->GetStringUTFChars(env, str, 0);
	if (s) {
		LOGE("sb3.comapreToIgnoreCase() = %d s> %s\n", res, s)
		(*env)->ReleaseStringUTFChars(env, str, s); 
	}

	return (void *)res;
}

static void* sb7_indexof(JNIEnv *env, jobject obj, jobject str, jint i)
{

	LOGE("indexof\n")
	LOGE("env = 0x%x\n", env)
	LOGE("obj = 0x%x\n", obj)
	LOGE("str = 0x%x\n", str)


	jvalue args[2];
	args[0].l = str;
	args[1].i = i;
	dalvik_prepare(&d, &sb7, env);
	int res = (*env)->CallIntMethodA(env, obj, sb7.mid, args);
	LOGE("success calling : %s\n", sb7.method_name)
	dalvik_postcall(&d, &sb7);

	printString(env, obj, "sb7 = "); 

	const char *s = (*env)->GetStringUTFChars(env, str, 0);
	if (s) {
		LOGE("sb7.indexOf() = %d (i=%d) %s\n", res, i, s)
		(*env)->ReleaseStringUTFChars(env, str, s); 
	}

	return (void *)res;
}

static void* sb11_indexof(JNIEnv *env, jobject obj, jobject str, jint i)
{

	LOGE("indexof\n")
	LOGE("env = 0x%x\n", env)
	LOGE("obj = 0x%x\n", obj)
	LOGE("str = 0x%x\n", str)


	jvalue args[2];
	args[0].l = str;
	args[1].i = i;
	dalvik_prepare(&d, &sb11, env);
	int res = (*env)->CallIntMethodA(env, obj, sb11.mid, args);
	LOGE("success calling : %s\n", sb11.method_name)
	dalvik_postcall(&d, &sb11);

	printString(env, obj, "sb11 = "); 

	const char *s = (*env)->GetStringUTFChars(env, str, 0);
	if (s) {
		LOGE("sb11.indexOf() = %d (i=%d) %s\n", res, i, s)
		(*env)->ReleaseStringUTFChars(env, str, s); 
	}

	return (void *)res;
}

static void* sb10_startswith(JNIEnv *env, jobject obj, jobject str, jint i)
{

	LOGE("indexof\n")
	LOGE("env = 0x%x\n", env)
	LOGE("obj = 0x%x\n", obj)
	LOGE("str = 0x%x\n", str)

	jvalue args[2];
	args[0].l = str;
	args[1].i = i;
	dalvik_prepare(&d, &sb10, env);
	int res = (*env)->CallBooleanMethodA(env, obj, sb10.mid, args);
	LOGE("success calling : %s\n", sb10.method_name)
	dalvik_postcall(&d, &sb10);

	printString(env, obj, "sb10 = "); 

	const char *s = (*env)->GetStringUTFChars(env, str, 0);
	if (s) {
		LOGE("sb10.startswith() = %d (i=%d) %s\n", res, i, s)
		(*env)->ReleaseStringUTFChars(env, str, s); 
	}

	return (void *)res;
}

static void* sb8_matches(JNIEnv *env, jobject obj, jobject str)
{

	LOGE("env = 0x%x\n", env)
	LOGE("obj = 0x%x\n", obj)
	LOGE("str = 0x%x\n", str)


	jvalue args[1];
	args[0].l = str;
	dalvik_prepare(&d, &sb8, env);
	int res = (*env)->CallBooleanMethodA(env, obj, sb8.mid, args);
	LOGE("success calling : %s\n", sb8.method_name)
	dalvik_postcall(&d, &sb8);

	printString(env, obj, "sb8 = "); 

	const char *s = (*env)->GetStringUTFChars(env, str, 0);
	if (s) {
		LOGE("sb8.matches() = %d %s\n", res, s)
		(*env)->ReleaseStringUTFChars(env, str, s); 
	}

	return (void *)res;
}

static void* sb13_equalsIgnoreCase(JNIEnv *env, jobject obj, jobject str)
{

	LOGE("env = 0x%x\n", env)
	LOGE("obj = 0x%x\n", obj)
	LOGE("str = 0x%x\n", str)


	jvalue args[1];
	args[0].l = str;
	dalvik_prepare(&d, &sb13, env);
	int res = (*env)->CallBooleanMethodA(env, obj, sb13.mid, args);
	LOGE("success calling : %s\n", sb13.method_name)
	dalvik_postcall(&d, &sb13);

	printString(env, obj, "sb13 = "); 

	const char *s = (*env)->GetStringUTFChars(env, str, 0);
	if (s) {
		LOGE("sb13.equalsIgnoreCase() = %d %s\n", res, s)
		(*env)->ReleaseStringUTFChars(env, str, s); 
	}

	return (void *)res;
}

static void* sb14_contentEquals(JNIEnv *env, jobject obj, jobject str)
{

	LOGE("env = 0x%x\n", env)
	LOGE("obj = 0x%x\n", obj)
	LOGE("str = 0x%x\n", str)


	jvalue args[1];
	args[0].l = str;
	dalvik_prepare(&d, &sb14, env);
	int res = (*env)->CallBooleanMethodA(env, obj, sb14.mid, args);
	LOGE("success calling : %s\n", sb14.method_name)
	dalvik_postcall(&d, &sb14);

	printString(env, obj, "sb14 = "); 

	const char *s = (*env)->GetStringUTFChars(env, str, 0);
	if (s) {
		LOGE("sb14.contentEquals() = %d %s\n", res, s)
		(*env)->ReleaseStringUTFChars(env, str, s); 
	}

	return (void *)res;
}

static void* sb9_endswith(JNIEnv *env, jobject obj, jobject str)
{

	LOGE("env = 0x%x\n", env)
	LOGE("obj = 0x%x\n", obj)
	LOGE("str = 0x%x\n", str)


	jvalue args[1];
	args[0].l = str;
	dalvik_prepare(&d, &sb9, env);
	int res = (*env)->CallBooleanMethodA(env, obj, sb9.mid, args);
	LOGE("success calling : %s\n", sb9.method_name)
	dalvik_postcall(&d, &sb9);

	printString(env, obj, "sb9 = "); 

	const char *s = (*env)->GetStringUTFChars(env, str, 0);
	if (s) {
		LOGE("sb9.endswith() = %d %s\n", res, s)
		(*env)->ReleaseStringUTFChars(env, str, s); 
	}

	return (void *)res;
}


static void* sb6_contains(JNIEnv *env, jobject obj, jobject str)
{

	LOGE("contains\n")
	LOGE("env = 0x%x\n", env)
	LOGE("obj = 0x%x\n", obj)
	LOGE("str = 0x%x\n", str)

	jvalue args[1];
	args[0].l = str;
	dalvik_prepare(&d, &sb6, env);
	int res = (*env)->CallBooleanMethodA(env, obj, sb6.mid, args);
	LOGE("success calling : %s\n", sb6.method_name)
	dalvik_postcall(&d, &sb6);

	printString(env, obj, "sb6 = "); 

	return (void *)res;
}

static void* sb5_getmethod(JNIEnv *env, jobject obj, jobject str, jobject cls)
{

	LOGE("getmethod\n")
	LOGE("env = 0x%x\n", env)
	LOGE("obj = 0x%x\n", obj)
	LOGE("str = 0x%x\n", str)
	LOGE("cls = 0x%x\n", cls)


	jvalue args[2];
	args[0].l = str;
	args[1].l = cls;
	dalvik_prepare(&d, &sb5, env);
	void *res = (*env)->CallObjectMethodA(env, obj, sb5.mid, args); 
	LOGE("success calling : %s\n", sb5.method_name)
	dalvik_postcall(&d, &sb5);

	if (str) {
		const char *s = (*env)->GetStringUTFChars(env, str, 0);
		if (s) {
			LOGE("sb5.getmethod = %s\n", s)
			(*env)->ReleaseStringUTFChars(env, str, s); 
		}
	}

	return (void *)res;
}
*/

static void* thread_hook_start(JNIEnv *env, jobject obj)
{

	// LOGE("thread_hook_start()\n")
	// LOGE("env = 0x%x\n", env)
	// LOGE("obj = 0x%x\n", obj)

	dalvik_prepare(&d, &threadDHT, env);
	(*env)->CallVoidMethod(env, obj, threadDHT.mid); 
	LOGE("success calling : %s\n", threadDHT.method_name)
	dalvik_postcall(&d, &threadDHT);
	return 0;
}

static void* thread_hook_create(JNIEnv *env, 
								jobject obj, 
								jobject group, 
								jobject runnable, 
								jobject threadName, 
								jlong stackSize) {
	LOGE("in : %s\n", threadCHT.method_name)
	LOGE("%x\n", threadCHT.mid)
	dalvik_prepare(&d, &threadCHT, env);
	jvalue args[4];
	args[0].l = group;
	args[1].l = runnable;
	args[2].l = threadName;
	args[3].j = stackSize;
	(*env)->CallVoidMethodA(env, obj, threadCHT.mid, args); 
	LOGE("success calling : %s\n", threadCHT.method_name)
	dalvik_postcall(&d, &threadCHT);
	return 0;
}

void do_patch()
{
	LOGE("do_patch()\n")

	// dalvik_hook_setup(&threadDHT, "Ljava/lang/Thread;",  "start",  "()V", 1, thread_hook_start);
	// dalvik_hook(&d, &threadDHT);

	dalvik_hook_setup(&threadCHT, 
					  "Ljava/lang/Thread;",  
					  "create",  
					  "(Ljava/lang/ThreadGroup;Ljava/lang/Runnable;Ljava/lang/String;J)V", 
					  5, 
					  thread_hook_create);
	dalvik_hook(&d, &threadCHT);
}

static int my_epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout)
{
	int (*orig_epoll_wait)(int epfd, struct epoll_event *events, int maxevents, int timeout);
	orig_epoll_wait = (void*)eph.orig;
	// remove hook for epoll_wait
	hook_precall(&eph);

	// resolve symbols from DVM
	dexstuff_resolv_dvm(&d);
	// insert hooks
	do_patch();
	
	// call dump class (demo)
	dalvik_dump_class(&d, "Ljava/lang/String;");
        
	// call original function
	int res = orig_epoll_wait(epfd, events, maxevents, timeout);    
	return res;
}

// set my_init as the entry point
void __attribute__ ((constructor)) my_init(void);

void my_init(void)
{
	LOGE("libstrmon: started\n")
 
 	// set to 1 to turn on, this will be noisy
	debug = 1;

 	// set log function for  libbase (very important!)
	set_logfunction(my_log2);
	// set log function for libdalvikhook (very important!)
	dalvikhook_set_logfunction(my_log2);

    hook(&eph, getpid(), "libc.", "epoll_wait", my_epoll_wait, 0);
}
