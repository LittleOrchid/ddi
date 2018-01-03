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

static struct dalvik_hook_t threadSNT;

static struct dalvik_hook_t threadCRT;

static void* thread_hook_start(JNIEnv *env, jobject obj)
{

	LOGE("thread_hook_start()\n")
	LOGE("env = 0x%x\n", env)
	LOGE("obj = 0x%x\n", obj)
	dalvik_prepare(&d, &threadDHT, env);
	(*env)->CallVoidMethod(env, obj, threadDHT.mid); 
	LOGE("success calling : %s\n", threadDHT.method_name)
	dalvik_postcall(&d, &threadDHT);
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

static void* thread_hook_create(JNIEnv *env, 
								jobject obj,
								jobject threadGroup,
								jobject runnable,
								jobject threadName,
								jlong stackSize) {
	LOGE("in thread_hook_create: %%x%x\n", obj);
	LOGE("threadGroup: %%x%x\n", threadGroup);
	LOGE("runnable: %%x%x\n", runnable);
	LOGE("threadName: %%x%x\n", threadName);
	LOGE("stackSize: %%x%x\n", stackSize);
	dalvik_prepare(&d, &threadCRT, env);
	jvalue args[4];
	args[0].l = threadGroup;
	args[1].l = runnable;
	args[2].l = threadName;
	args[3].j = stackSize;
	(*env)->CallVoidMethodA(env, obj, threadCRT.mid, args); 
	LOGE("success calling : %s\n", threadCRT.method_name)
	dalvik_postcall(&d, &threadCRT);
	return 0;
}

void do_patch()
{
	LOGE("do_patch()\n")

	dalvik_hook_setup(&threadDHT, "Ljava/lang/Thread;",  "start",  "()V", 1, thread_hook_start);
	threadDHT.debug_me = 0;
	threadDHT.dump = 0;
	dalvik_hook(&d, &threadDHT);

	dalvik_hook_setup(&threadSNT, "Ljava/lang/Thread;",  "setName",  "(Ljava/lang/String;)V", 2, thread_hook_setName);
	threadSNT.debug_me = 0;
	threadSNT.dump = 0;
	dalvik_hook(&d, &threadSNT);

	dalvik_hook_setup(&threadCRT, 
					  "Ljava/lang/Thread;",  
					  "create",  
					  "(Ljava/lang/ThreadGroup;Ljava/lang/Runnable;Ljava/lang/String;J)V", 
					  5, 
					  thread_hook_create);
	threadCRT.debug_me = 1;		
	threadCRT.dump = 1;		  
	dalvik_hook(&d, &threadCRT);

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

    hook(&eph, getpid(), "libc.", "epoll_wait", my_epoll_wait, 0);
}
