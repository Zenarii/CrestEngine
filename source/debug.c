#if ZERAVIA_INTERNAL
    #define Assert(expression) InvalidCode(__LINE__, __FILE__, #expression, expression, 1)
    #define SoftAssert(expression) InvalidCode(__LINE__, __FILE__, #expression, expression, 0)
#else
    #define Assert(...)
    #define SoftAssert(...)
#endif

#define MAX_LOG_OUT 1024

typedef enum crest_log_type crest_log_type;
enum crest_log_type {
    CREST_LOG_TRACE,
    CREST_LOG_INFO,
    CREST_LOG_WARNING,
    CREST_LOG_ERROR,
    CREST_LOG_IMPORTANT
};

#define CrestTrace(Out) CrestLog(CREST_LOG_TRACE, Out)
#define CrestTraceF(...) CrestLogF(CREST_LOG_TRACE, __VA_ARGS__)

#define CrestInfo(Out) CrestLog(CREST_LOG_INFO, Out)
#define CrestInfoF(...) CrestLogF(CREST_LOG_INFO, __VA_ARGS__)

#define CrestWarn(Out) CrestLog(CREST_LOG_WARN, Out)
#define CrestWarnF(...) CrestLogF(CREST_LOG_WARN, __VA_ARGS__)

#define CrestError(Out) CrestLog(CREST_LOG_ERROR, Out)
#define CrestErrorF(...) CrestLogF(CREST_LOG_ERROR, __VA_ARGS__)

#define CrestImportant(Out) CrestLog(CREST_LOG_IMPORTANT, Out)
#define CrestImportantF(...) CrestLogF(CREST_LOG_IMPORTANT, __VA_ARGS__)

global crest_log_type MinimumLogLevel = CREST_LOG_INFO;

#ifdef _WIN32
internal void
CrestLog(crest_log_type Level, const char * Out) {
    if(Level >= MinimumLogLevel) {
        OutputDebugStringA(Out);
    }
    //TODO(Zen): Output in a dev console?
}
#endif


internal void
CrestLogF(crest_log_type Level, const char * Format, ...) {
    va_list args;
    va_start(args, Format);

    char InfoLog[MAX_LOG_OUT];
    vsprintf(InfoLog, Format, args);
    CrestLog(Level, InfoLog);
    va_end(args);
}



internal void
InvalidCode(const int Line, const char * File, const char * Error, b32 result, b32 Crash) {
    if(result) return;


    CrestImportantF("Assert Failed: %s\nLine: %i File:%s\n", Error, Line, File);

    if(Crash) {
        *((int *)0) = 0;
    }
}
