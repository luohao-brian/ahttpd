#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "log.h"
#include <stdio.h>


typedef struct {
    FILE *file;
    int is_opened;
} LogHandler;

#define LOG_BUFFER_SIZE 1024
#define LOG_FILE "ahttpd.log"


LogHandler logHandler;

// 打开或重新打开日志文件
void openLogFile() {
    if (logHandler.is_opened) {
        return;
    }

    logHandler.file = fopen(LOG_FILE, "a");
    if (logHandler.file == NULL) {
        perror("Failed to open log file");
        return;
    }

    logHandler.is_opened = 1;
}

// 关闭日志文件
void closeLogFile() {
    if (logHandler.is_opened && logHandler.file!= NULL) {
        fclose(logHandler.file);
        logHandler.is_opened = 0;
    }
}

// 记录日志
void logMessage(LogLevel level, const char *message) {
    openLogFile();

    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char timeStr[20];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", tm_info);

    const char *levelStr;
    switch (level) {
        case LOG_LEVEL_ERROR:
            levelStr = "ERROR";
            break;
        case LOG_LEVEL_WARNING:
            levelStr = "WARNING";
            break;
        case LOG_LEVEL_LOG:
            levelStr = "LOG";
            break;
        case LOG_LEVEL_INFO:
            levelStr = "INFO";
            break;
        default:
            levelStr = "UNKNOWN";
            break;
    }

    char logBuffer[LOG_BUFFER_SIZE];
    snprintf(logBuffer, LOG_BUFFER_SIZE, "[%s] [%s] %s\n", timeStr, levelStr, message);

    fputs(logBuffer, logHandler.file);
    fflush(logHandler.file);
}

// 错误日志函数
void logError(const char *message) {
    logMessage(LOG_LEVEL_ERROR, message);
}

// 警告日志函数
void logWarning(const char *message) {
    logMessage(LOG_LEVEL_WARNING, message);
}

// 普通日志函数
void logLog(const char *message) {
    logMessage(LOG_LEVEL_LOG, message);
}

// 信息日志函数
void logInfo(const char *message) {
    logMessage(LOG_LEVEL_INFO, message);
}

/*
int main() {
    logError("This is an error message");
    logWarning("This is a warning message");
    logLog("This is a log message");
    logInfo("This is an info message");

    closeLogFile();

    return 0;
}
*/
