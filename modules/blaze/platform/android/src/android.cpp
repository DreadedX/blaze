#include "android.h"

#include <android/log.h>

#include <iostream>

int main();

JNIEnv* _env;
jobject _obj;

jmethodID _appendToLog;
void appendToLog(std::string message) {
	_env->CallVoidMethod(_obj, _appendToLog, _env->NewStringUTF(message.c_str()));
}

// Entrypoint for android
void startNative(JNIEnv* env, jobject obj) {
	// Store pointers to env and obj
	// @todo Is this allowed?
	_env = env;
	_obj = obj;

	jclass clazz = _env->GetObjectClass(_obj);
	_appendToLog = _env->GetMethodID(clazz, "appendLog", "(Ljava/lang/String;)V");

	// Call the normal entrypoint
	main();
}

JNINativeMethod _methods[] = {
	{"startNative", "()V", (void*)startNative},
};

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserverd) {
	JNIEnv* env;
	if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
		return -1;
	}

	jclass clazz = env->FindClass("nl/mtgames/blaze/ui/bootstrap/BootstrapViewModel");
	if (clazz == nullptr) {
		return -1;
	}

	if (env->RegisterNatives(clazz, _methods, sizeof(_methods)/sizeof(_methods[0])) < 0) {
		return -1;
	}

	return JNI_VERSION_1_6;
}

namespace BLAZE_NAMESPACE::platform {

	const std::string Android::get_base_path() const {
		// @todo We need to just use the context to get the actual path
		// return "/data/user/0/nl.mtgames.blaze/files/";
		return "/storage/emulated/0/Android/data/nl.mtgames.blaze/files/";
	}

	bool Android::has_async_support() const {
		return true;
	}

	logger::LogHandler Android::get_logger() {
		return [](Level, std::string, int, std::string message){
			appendToLog(message);
		};
	}
}

