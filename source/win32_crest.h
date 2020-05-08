internal char *
CrestLoadFileAsString(const char* Path);

internal void
CrestWriteFile(const char * Path, const char * Data, i32 DataLength);

internal void
CrestMakeDirectory(const char * Path);

internal char *
CrestLoadLastModifiedFile(const char * Path);

internal r64
CrestCurrentTime();
