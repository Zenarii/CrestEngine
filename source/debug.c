#if ZERAVIA_INTERNAL
    #define Assert(expression) InvalidCode(__LINE__, __FILE__, #expression, expression, 1);
    #define SoftAssert(expression) InvalidCode(__LINE__, __FILE__, #expression, expression, 0);
#else
    #define Assert(...)
    #define SoftAssert(...)
#endif

#define MAX_LOG_OUT 1024

#ifdef _WIN32
internal void
CrestLog(const char * Out) {
    OutputDebugStringA(Out);
    //TODO(Zen): Output in a console?
}
#endif


internal void
CrestLogF(const char * Format, ...) {
    va_list args;
    va_start(args, Format);

    char InfoLog[MAX_LOG_OUT];
    vsprintf(InfoLog, Format, args);
    CrestLog(InfoLog);
    va_end(args);
}



internal void
InvalidCode(const int Line, const char * File, const char * Error, b32 result, b32 Crash) {
    if(result) return;


    CrestLogF("Assert Failed: %s\nLine: %i File:%s\n", Error, Line, File);

    if(Crash) {
        *((int *)0) = 0;
    }
}
