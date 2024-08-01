#ifndef __LOG_H
#define __LOG_H


typedef enum {
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_LOG,
    LOG_LEVEL_INFO
} LogLevel;

// 打开或重新打开日志文件
void openLogFile();
// 关闭日志文件
void closeLogFile();
// 记录日志
void logMessage(LogLevel level, const char *message);
// 错误日志函数
void logError(const char *message);
// 警告日志函数
void logWarning(const char *message);
// 普通日志函数
void logLog(const char *message);
// 信息日志函数
void logInfo(const char *message);

#endif
