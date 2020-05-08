#if ZERAVIA_INTERNAL
    #define Assert(expression) InvalidCode(__LINE__, __FILE__, #expression, expression, 1);
    #define SoftAssert(expression) InvalidCode(__LINE__, __FILE__, #expression, expression, 0);
#else
    #define Assert(...)
    #define SoftAssert(...)
#endif

internal void
InvalidCode(const int Line, const char * File, const char * Error, b32 result, b32 Crash) {
    if(result) return;

    char InfoLog[256];

    #ifdef _WIN32
        wsprintf(InfoLog, "Assert Failed: %s\nLine: %i File:%s\n", Error, Line, File);
        OutputDebugStringA(InfoLog);
    #endif

    if(Crash) {
        *((int *)0) = 0;
    }
}

#ifdef _WIN32
internal void
CrestDebugLog(const char *Out) {

}
#endif
