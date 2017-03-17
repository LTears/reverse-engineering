#include "Common.h"
using namespace std;

jclass cls_PosixException;//com/saurik/substrate/PosixException
jmethodID met_PosixException_init;//com/saurik/substrate/PosixException.<init>

#define dosafethrow(ret,x)\
	while((int)(ret = x) == -1)\
	{\
		if(errno != EINTR)\
		{\
			dothrow(env, errno, __FILE__, __LINE__);\
			break;\
		}\
	}

#define T_VALIST

jthrowable createnewobj(JNIEnv* env, jclass cls, jmethodID met, ...)
{
	va_list va;
	va_start(va, env);
	return (jthrowable)env->NewObjectV(cls, met, va);
}

void dothrow(JNIEnv* env, int code, const char* file, int line)
{
	jthrowable excobj = createnewobj(env, cls_PosixException, met_PosixException_init,
			code, env->NewStringUTF(file), line);
	env->Throw(excobj);
}

JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *vm, void *reserved)
{
	JNIEnv* env = 0;
	if(vm->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK)
		return -1;
	jclass l_cls_PosixException = env->FindClass("com/saurik/substrate/PosixException");
	met_PosixException_init = env->GetMethodID(l_cls_PosixException, "<init>", "(ILjava/lang/String;I)V");
	cls_PosixException = (jclass)env->NewGlobalRef(l_cls_PosixException);
	return JNI_VERSION_1_6;
}

/*
 * �޸�Ŀ¼�ļ��û�������
 */
JNIEXPORT void JNICALL
Java_com_saurik_substrate_Unix_chmod(JNIEnv *env, jclass type, jstring path, jint mode) {
    const char *path_cstr = env->GetStringUTFChars(path, 0);
    int status;
    dosafethrow(status, chmod(path_cstr, mode));
    env->ReleaseStringUTFChars(path, path_cstr);
}

/*
 * �޸�Ŀ¼�ļ�ִ�ж�д����
 */
JNIEXPORT void JNICALL
Java_com_saurik_substrate_Unix_chown(JNIEnv *env, jclass type, jstring path, jint uid, jint gid) {
    const char *path_cstr = env->GetStringUTFChars(path, 0);
    int status;
    dosafethrow(status, chown(path_cstr, uid, gid));
    env->ReleaseStringUTFChars(path, path_cstr);
}

/*
 * ��ȡ������id
 */
JNIEXPORT jint JNICALL
Java_com_saurik_substrate_Unix_getppid(JNIEnv *env, jclass type) {
	return getppid();
}

/*
 * �����ļ��ַ���
 */
JNIEXPORT jboolean JNICALL
Java_com_saurik_substrate_Unix_grep_1F(JNIEnv *env, jclass type, jstring filepath, jstring tosearch) {
    const char *filepath_cstr = env->GetStringUTFChars(filepath, 0);
    int tosearch_len = env->GetStringLength(tosearch);
    const char* tosearch_cstr = env->GetStringUTFChars(tosearch, 0);
    int fd;
    int status;
    bool find = false;
    dosafethrow(fd, open(filepath_cstr, O_RDONLY));
    if(fd != -1)
    {
    	struct stat fst;
    	dosafethrow(status, fstat(fd, &fst));
    	if(status != -1)
    	{
    		void* base;
    		dosafethrow(base, mmap(0, fst.st_blksize, PROT_READ, MAP_PRIVATE, fd, 0));
    		if(base != MAP_FAILED)
    		{
    			if(memmem(base, fst.st_blksize, tosearch_cstr, tosearch_len))
    				find = true;
    			dosafethrow(status, munmap(base, fst.st_blksize));
    		}
    	}
    	dosafethrow(status, close(fd));
    }
    env->ReleaseStringUTFChars(filepath, filepath_cstr);
    env->ReleaseStringUTFChars(tosearch, tosearch_cstr);
    return find;
}

/*
 * ɱ������
 */
JNIEXPORT void JNICALL
Java_com_saurik_substrate_Unix_kill(JNIEnv *env, jclass type, jint pid, jint sig) {
	int status;
	dosafethrow(status, kill(pid, sig));
}

/*
 * ����Ŀ¼
 */
JNIEXPORT void JNICALL
Java_com_saurik_substrate_Unix_mkdir(JNIEnv *env, jclass type, jstring path, jint mode) {
    const char *path_cstr = env->GetStringUTFChars(path, 0);
    int status;
    dosafethrow(status, mkdir(path_cstr, mode));
    env->ReleaseStringUTFChars(path, path_cstr);
}

/*
 * ��ȡ�����ӵ�ַ
 */
JNIEXPORT jstring JNICALL
Java_com_saurik_substrate_Unix_readlink(JNIEnv *env, jclass type, jstring path) {
    const char* path_cstr = env->GetStringUTFChars(path, 0);
    char linkedpath_cstr[1024] = {0};
    int readlen;
    dosafethrow(readlen, readlink(path_cstr, linkedpath_cstr, 1024));
    jstring linkedpath = env->NewStringUTF(linkedpath_cstr);
    env->ReleaseStringUTFChars(path, path_cstr);
    return linkedpath;
}

/*
 * ��ӳ�����
 */
JNIEXPORT void JNICALL
Java_com_saurik_substrate_Unix_remount(JNIEnv *env, jclass type, jstring path, jboolean readonly) {
    const char *path_cstr = env->GetStringUTFChars(path, 0);
    int status;
	int mountflags = MS_REMOUNT;
	if(readonly)
		mountflags |= MS_RDONLY;
    dosafethrow(status, mount("", path_cstr, "none", mountflags, 0));
    env->ReleaseStringUTFChars(path, path_cstr);
}

/*
 * ����������
 */
JNIEXPORT void JNICALL
Java_com_saurik_substrate_Unix_symlink(JNIEnv *env, jclass type, jstring srcpath, jstring dstpath) {
    const char *srcpath_cstr = env->GetStringUTFChars(srcpath, 0);
    const char *dstpath_cstr = env->GetStringUTFChars(dstpath, 0);
    int status;
    dosafethrow(status, symlink(srcpath_cstr, dstpath_cstr));
    // TODO

    env->ReleaseStringUTFChars(srcpath, srcpath_cstr);
    env->ReleaseStringUTFChars(dstpath, dstpath_cstr);
}

/*
 * �Ƴ�������
 */
JNIEXPORT void JNICALL
Java_com_saurik_substrate_Unix_unlink(JNIEnv *env, jclass type, jstring path) {
    const char *path_cstr = env->GetStringUTFChars(path, 0);
    int status;
    dosafethrow(status, unlink(path_cstr));
    env->ReleaseStringUTFChars(path, path_cstr);
}
