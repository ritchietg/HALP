#ifndef SD_UTIL
#define SD_UTIL

class SdUtil {
  private:
    bool isMounted = false;
    bool isAttached = false;
    uint64_t cardSize = 0;
    uint64_t cardSpaceTotal = 0;
    uint64_t cardSpaceUsed = 0;
    
  public:
    SdUtil();
    String configsPath = "/configs";
    String logsPath = "/logs";
    String issuesPath = "/issues";
    uint64_t cardFreeSpace = 0;
    bool initSdCard();
    bool sDAttached();
    void getSpace();
    void logThis(String root, String fileName, String msg);
};
#endif
