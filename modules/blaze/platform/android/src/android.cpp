#include "android.h"

#include <jni.h>
#include <android/log.h>

#include <iostream>

int main();

// @todo This is propably incorrect
JNIEnv* _env;
jobject _obj;

void java_print(std::string text) {
	jclass clazz = _env->GetObjectClass(_obj);
	jmethodID appendToLog = _env->GetMethodID(clazz, "appendToLog", "(Ljava/lang/String;)V");
	_env->CallVoidMethod(_obj, appendToLog, _env->NewStringUTF(text.c_str()));
}

extern "C" {

	JNIEXPORT void JNICALL Java_nl_mtgames_blazebootstrap_BootstrapActivity_start(JNIEnv* env, jobject obj) {

		_env = env;
		_obj = obj;

		java_print("Hello from C++\n");

		main();
	}
}

namespace BLAZE_NAMESPACE::platform {

	const std::string Android::get_base_path() const {
		return "/storage/emulated/0/Android/data/nl.mtgames.blazebootstrap/files/";
	}

	bool Android::has_async_support() const {
		return true;
	}
}

