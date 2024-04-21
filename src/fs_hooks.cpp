#include <wups/function_patching.h>

#include <coreinit/filesystem.h>

//#include "utils/logger.h"
#include <whb/log.h>

//FS logging hooks

DECL_FUNCTION(int, FSOpenFile, void *pClient, void *pCmd, const char *path, const char *mode, int *fd, int errHandling){
	WHBLogPrintf("FSOpenFile(): %s", path);
	return real_FSOpenFile(pClient, pCmd, path, mode, fd, errHandling);
}

DECL_FUNCTION(int, FSOpenFileAsync, void *pClient, void *pCmd, const char *path, const char *mode, int *handle, int error, const void *asyncParams) {
	WHBLogPrintf("FSOpenFileAsync(): %s", path);
	return real_FSOpenFileAsync(pClient, pCmd, path, mode, handle, error, asyncParams);
}

DECL_FUNCTION(int, FSOpenDir, void *pClient, void *pCmd, const char *path, int32_t *dh, int errHandling) {
	WHBLogPrintf("FSOpenDir(): %s", path);
	return real_FSOpenDir(pClient, pCmd, path, dh, errHandling);
}

DECL_FUNCTION(int, FSOpenDirAsync, void *pClient, void *pCmd, const char *path, int *handle, int error, void *asyncParams) {
	WHBLogPrintf("FSOpenDirAsync(): %s", path);
	return real_FSOpenDirAsync(pClient, pCmd, path, handle, error, asyncParams);
}

DECL_FUNCTION(int, FSGetStat, void *pClient, void *pCmd, const char *path, FSStat *stats, int errHandling) {
	WHBLogPrintf("FSGetStat(): %s", path);
	return real_FSGetStat(pClient, pCmd, path, stats, errHandling);
}

DECL_FUNCTION(int, FSGetStatAsync, void *pClient, void *pCmd, const char *path, FSStat *stats, int errHandling, void *asyncParams) {
	WHBLogPrintf("FSGetStatAsync(): %s", path);
	return real_FSGetStatAsync(pClient, pCmd, path, stats, errHandling, asyncParams);
}

WUPS_MUST_REPLACE(FSOpenFile ,WUPS_LOADER_LIBRARY_COREINIT,FSOpenFile);
WUPS_MUST_REPLACE(FSOpenFileAsync,WUPS_LOADER_LIBRARY_COREINIT,FSOpenFileAsync);
WUPS_MUST_REPLACE(FSOpenDir,WUPS_LOADER_LIBRARY_COREINIT,FSOpenDir);
WUPS_MUST_REPLACE(FSOpenDirAsync,WUPS_LOADER_LIBRARY_COREINIT,FSOpenDirAsync);
WUPS_MUST_REPLACE(FSGetStat,WUPS_LOADER_LIBRARY_COREINIT,FSGetStat);
WUPS_MUST_REPLACE(FSGetStatAsync,WUPS_LOADER_LIBRARY_COREINIT,FSGetStatAsync);