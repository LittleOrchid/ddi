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
static long threadObjArray;
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

static struct dalvik_hook_t threadSNT;

static char startFuncEnterLog[50] = "thread_hook_start(enter)";
static char startFuncExitLog[50] = "thread_hook_start(exit)";

static pthread_mutex_t mutex;

static void* thread_hook_start(JNIEnv *env, jobject obj)
{
	pthread_mutex_lock(&mutex);
	LOGE("%s", startFuncEnterLog);
	// LOGE("env = 0x%x\n", env)
	// LOGE("obj = 0x%x\n", obj)
	dalvik_prepare(&d, &threadDHT, env);
	(*env)->CallVoidMethod(env, obj, threadDHT.mid); 
	dalvik_postcall(&d, &threadDHT);
	LOGE("%s\n", startFuncExitLog);	
	pthread_mutex_unlock(&mutex);
	jclass threadClass = (*env)->GetObjectClass(env, obj);
	jfieldID threadNameId = (*env)->GetFieldID(env, threadClass, "name", "Ljava/lang/String;");
	jstring threadNameJs = (jstring)(*env)->GetObjectField(env, obj, threadNameId);
	if(threadNameJs) {
		const char* threadNameChars = (*env)->GetStringUTFChars(env, threadNameJs, 0);
		if(threadNameChars) {
			LOGE("Thread Name : %s", threadNameChars);
			(*env)->ReleaseStringUTFChars(env, threadNameJs, threadNameChars); 
		}
	}

	jclass  throwable_class = (*env)->FindClass(env, "java/lang/Throwable");
	jmethodID  throwable_init = (*env)->GetMethodID(env, throwable_class, "<init>", "()V");
	jobject throwable_obj = (*env)->NewObject(env, throwable_class, throwable_init);
	jmethodID throwable_mid = (*env)->GetMethodID(env, throwable_class, "printStackTrace", "()V");
	(*env)->CallVoidMethod(env, throwable_obj, throwable_mid);
	return 0;
}

static void* thread_hook_setName(JNIEnv *env, jobject obj, jobject threadName)
{
	LOGE("thread_hook_setName()\n")
	// LOGE("env = 0x%x\n", env)
	// LOGE("obj = 0x%x\n", obj)
	dalvik_prepare(&d, &threadSNT, env);
	jvalue args[1];
	args[0].l = threadName;
	(*env)->CallVoidMethodA(env, obj, threadSNT.mid, args); 
	LOGE("success calling : %s\n", threadSNT.method_name)
	dalvik_postcall(&d, &threadSNT);
	return 0;
}

void do_patch()
{
	LOGE("do_patch()\n")

	dalvik_hook_setup(&threadDHT, "Ljava/lang/Thread;",  "start",  "()V", 1, thread_hook_start);
	threadDHT.debug_me = 0;
	threadDHT.dump = 0;
	dalvik_hook(&d, &threadDHT);

	// dalvik_hook_setup(&threadSNT, "Ljava/lang/Thread;",  "setName",  "(Ljava/lang/String;)V", 2, thread_hook_setName);
	// threadSNT.debug_me = 0;
	// threadSNT.dump = 0;
	// dalvik_hook(&d, &threadSNT);

	LOGE("do_patch: after dalvik_hook");
}

static int my_epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout)
{
	LOGE("in my_epoll_wait");
	int (*orig_epoll_wait)(int epfd, struct epoll_event *events, int maxevents, int timeout);
	orig_epoll_wait = (void*)eph.orig;
	// remove hook for epoll_wait
	hook_precall(&eph);
	LOGE("leave the native hook");
	// resolve symbols from DVM
	dexstuff_resolv_dvm(&d);
	// insert hooks
	do_patch();
	
	// call dump class (demo)
	dalvik_dump_class(&d, "Ljava/lang/Thread;");
        
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

	pthread_mutex_init(&mutex, NULL);

    hook(&eph, getpid(), "libc.", "epoll_wait", my_epoll_wait, 0);
}
