#include <list>
#include <vector>
#include <set>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <thread>
#include <cstring>
#include <cstdio>
#include <jni.h>
#include <unistd.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <dlfcn.h>
#include <sstream>
#include <map>
#include <mutex>
#include <atomic>
#include <ctime>
#include <cstdarg>
#include <algorithm>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <android/log.h>
#include <link.h>

#include "KittyMemory/KittyInclude.hpp"
#include "Dobby/dobby.h"
#include "CrashDetector.h"

extern "C" {
#include "shadowhook/shadowhook.h"
#include "shadowhook/a.h"
}

#include "Includes/Logger.h"
#include "Includes/obfuscate.h"
#include "Includes/Utils.h"
#include "Includes/Macros.h"
#include "oxorany/oxorany.cpp"

// ═══════════════════════════════════════════════════════
//  🔥 MASTER CONFIG
// ═══════════════════════════════════════════════════════
#define MAX_BODY             4000
#define MAX_SSL_BUF          200000
#define USE_SHADOWHOOK       true
#define HOOK_DEPTH_LIMIT     5

#define TXT_SYSTEM_ON        true
#define ENABLE_URL_LOG       true
#define ENABLE_REPLACE       true
#define REPLACE_HOST         true
#define REPLACE_SNI          true
#define REPLACE_PATH         true
#define REPLACE_FULL_URL     true
#define REPLACE_RESPONSE     true

#define BYPASS_SSL_PINNING   true
#define BYPASS_OPENSSL       true
#define BYPASS_BORINGSSL     true
#define BYPASS_JAVA_TM       true

#define ENABLE_FLUTTER       true
#define ENABLE_UNITY         true
#define ENABLE_OKHTTP        true

const char* CRACK_SIG = "Cracked By @LEGALxCRACKING";

#define LOG_FILE_PATH   "/storage/emulated/0/Download/hook_log.txt"
#define URL_FILE_PATH   "/storage/emulated/0/Download/full_urls.txt"
#define SIG_FILE_PATH   "/storage/emulated/0/Download/crack_by_LEGALxCRACKING.txt"
#define RES_FILE_PATH   "/storage/emulated/0/Download/replaced_responses.txt"
#define HOOK_RESP_PATH  "/storage/emulated/0/Download/hook_responses.txt"

// ═══════════════════════════════════════════════════════
//  STRING HELPERS
// ═══════════════════════════════════════════════════════
static std::string S(const char* a, const char* b) { return std::string(a) + b; }
static std::string S(const char* a, const char* b, const char* c) { return std::string(a) + b + c; }

// ═══ NEW DOMAIN — legalxcracking.site ═══
static std::string strNewDomain() {
    return S(oxorany("legalxcrack"), oxorany("ing"), oxorany(".site"));
}
static std::string strNewFullURL() {
    return S(oxorany("https://"), oxorany("legalxcrack"), oxorany("ing.site/connect"));
}
static std::string strNewPath() {
    return S(oxorany("/con"), oxorany("nect"));
}
static std::string strHttps()     { return S(oxorany("https"), oxorany("://")); }
static std::string strHostHdr()   { return S(oxorany("Host"), oxorany(": ")); }
static std::string strHostHdrLc() { return S(oxorany("host"), oxorany(": ")); }
static std::string strMaps()      { return S(oxorany("/proc/self"), oxorany("/maps")); }

// ═══════════════════════════════════════════════════════
//  TARGET DOMAINS
// ═══════════════════════════════════════════════════════
static std::vector<std::string> getReplaceDomains() {
    return {
        S(oxorany("hackingclub"), oxorany(".tech")),
        S(oxorany("crake"), oxorany(".v6"), oxorany(".army")),
        S(oxorany("panel.modkey"), oxorany(".online")),
        S(oxorany("v6"), oxorany(".army")),
        S(oxorany("softpanel"), oxorany(".site")),
        S(oxorany("login-key"), oxorany(".xyz")),
        S(oxorany("log.authkey"), oxorany(".site")),
        S(oxorany("authkey"), oxorany(".site")),
        S(oxorany("api.vipkeys"), oxorany(".shop")),
        S(oxorany("vipkeys"), oxorany(".shop")),
        S(oxorany("api.x-shop"), oxorany(".online")),
        S(oxorany("x-shop"), oxorany(".online")),
        S(oxorany("best-cheat.bgmiplugin"), oxorany(".in")),
        S(oxorany("bgmiplugin"), oxorany(".in")),
        S(oxorany("newton-loader.dropkey"), oxorany(".online")),
        S(oxorany("dropkey"), oxorany(".online")),
        S(oxorany("oneplus.battleground-hub"), oxorany(".shop")),
        S(oxorany("battleground-hub"), oxorany(".shop")),
        S(oxorany("infinite.kuropanels"), oxorany(".in")),
        S(oxorany("api.xsilent"), oxorany(".shop")),
        S(oxorany("xsilent"), oxorany(".shop")),
        S(oxorany("ogpremium"), oxorany(".shop")),
    };
}

static std::vector<std::string> getSkipDomains() {
    return {
        S(oxorany("google"), oxorany(".com")),
        S(oxorany("googleapis"), oxorany(".com")),
        S(oxorany("gstatic"), oxorany(".com")),
        S(oxorany("play.google"), oxorany(".com")),
        S(oxorany("facebook"), oxorany(".com")),
        S(oxorany("facebook"), oxorany(".net")),
        S(oxorany("fbcdn"), oxorany(".net")),
        S(oxorany("cloudflare"), oxorany(".com")),
        S(oxorany("cloudflare-dns"), oxorany(".com")),
        S(oxorany("android"), oxorany(".com")),
        S(oxorany("android.clients.google"), oxorany(".com")),
        S(oxorany("firebase"), oxorany(".com")),
        S(oxorany("firebaseio"), oxorany(".com")),
        S(oxorany("crashlytics"), oxorany(".com")),
        S(oxorany("app-measurement"), oxorany(".com")),
        S(oxorany("legalxcrack"), oxorany("ing.site")),  // 🔥 khud ko skip
        S(oxorany("flutter"), oxorany(".io")),
        S(oxorany("storage.googleapis"), oxorany(".com")),
        S(oxorany("unity"), oxorany(".com")),
        S(oxorany("unity3d"), oxorany(".com")),
    };
}

static std::vector<std::string> getSSLWriteSymbols() {
    return {
        S(oxorany("SSL"), oxorany("_write")),
        S(oxorany("ssl"), oxorany("_write")),
        S(oxorany("BIO"), oxorany("_write")),
        S(oxorany("bio"), oxorany("_write")),
        S(oxorany("tls"), oxorany("_write")),
        S(oxorany("tls_write"), oxorany("_records")),
        S(oxorany("ssl_write"), oxorany("_impl")),
        S(oxorany("SSL_write"), oxorany("_ex")),
        S(oxorany("BIO_write"), oxorany("_ex")),
    };
}
static std::vector<std::string> getSSLReadSymbols() {
    return {
        S(oxorany("SSL"), oxorany("_read")),
        S(oxorany("ssl"), oxorany("_read")),
        S(oxorany("BIO"), oxorany("_read")),
        S(oxorany("bio"), oxorany("_read")),
        S(oxorany("tls"), oxorany("_read")),
        S(oxorany("tls_read"), oxorany("_records")),
        S(oxorany("ssl_read"), oxorany("_impl")),
        S(oxorany("SSL_read"), oxorany("_ex")),
        S(oxorany("BIO_read"), oxorany("_ex")),
    };
}
static std::vector<std::string> getSSLSniSymbols() {
    return {
        S(oxorany("SSL_set_tlsext"), oxorany("_host_name")),
        S(oxorany("ssl_set_tlsext"), oxorany("_host_name")),
        S(oxorany("SSL_set_tlsext"), oxorany("_host_name_ex")),
    };
}
static std::vector<std::string> getSSLPinningSymbols() {
    return {
        S(oxorany("SSL_get_verify"), oxorany("_result")),
        S(oxorany("SSL_CTX_set_verify"), oxorany("")),
        S(oxorany("X509_verify"), oxorany("_cert")),
        S(oxorany("X509_STORE_CTX"), oxorany("_get_error")),
        S(oxorany("SSL_get_verify"), oxorany("_result_ex")),
        S(oxorany("X509_verify"), oxorany("_cert_ex")),
        S(oxorany("X509_check"), oxorany("_host")),
    };
}

// ═══════════════════════════════════════════════════════
//  GLOBALS
// ═══════════════════════════════════════════════════════
std::mutex g_mutex;
std::mutex g_fileMutex;
static thread_local bool g_inLog = false;
static thread_local int  g_hookDepth = 0;
static std::atomic<int>  g_fullSeq{0};
static std::atomic<int>  g_respSeq{0};

static std::set<std::string> g_hookedLibs;
static std::mutex g_hookMutex;

static std::set<std::string> g_loggedFullURLs;
static std::mutex g_fullURLMutex;

static std::atomic<bool> g_targetFound{false};
static std::atomic<bool> g_pinningBypassed{false};
static std::atomic<bool> g_pinBypassLogged{false};   // 🔥 once log
static std::mutex g_sigMutex;

static std::map<std::string, std::string> g_responseCache;
static std::mutex g_responseMutex;

static JavaVM* g_javaVM = nullptr;
static std::atomic<bool> g_okhttpHooked{false};

// ═══════════════════════════════════════════════════════
//  TYPEDEFS
// ═══════════════════════════════════════════════════════
typedef int      (*getaddrinfo_t)(const char*, const char*, const struct addrinfo*, struct addrinfo**);
typedef struct hostent* (*gethostbyname_t)(const char*);
typedef int      (*connect_t)(int, const struct sockaddr*, socklen_t);
typedef ssize_t  (*send_t)(int, const void*, size_t, int);
typedef ssize_t  (*recv_t)(int, void*, size_t, int);
typedef int      (*SSL_write_t)(void*, const void*, int);
typedef int      (*SSL_read_t)(void*, void*, int);
typedef int      (*SSL_set_tlsext_host_name_t)(void*, const char*);
typedef int      (*BIO_write_t)(void*, const void*, int);
typedef int      (*BIO_read_t)(void*, void*, int);

typedef long     (*SSL_get_verify_result_t)(const void*);
typedef void     (*SSL_CTX_set_verify_t)(void*, int, void*);
typedef int      (*X509_verify_cert_t)(void*);
typedef int      (*X509_STORE_CTX_get_error_t)(void*);
typedef int      (*X509_check_host_t)(void*, const char*, size_t, unsigned int, char**);

getaddrinfo_t   orig_getaddrinfo   = nullptr;
gethostbyname_t orig_gethostbyname = nullptr;
connect_t       orig_connect       = nullptr;
send_t          orig_send          = nullptr;
recv_t          orig_recv          = nullptr;
SSL_write_t     orig_SSL_write     = nullptr;
SSL_read_t      orig_SSL_read      = nullptr;
SSL_set_tlsext_host_name_t orig_SSL_set_tlsext_host_name = nullptr;
BIO_write_t     orig_BIO_write     = nullptr;
BIO_read_t      orig_BIO_read      = nullptr;

SSL_get_verify_result_t       orig_SSL_get_verify_result = nullptr;
SSL_CTX_set_verify_t          orig_SSL_CTX_set_verify    = nullptr;
X509_verify_cert_t            orig_X509_verify_cert      = nullptr;
X509_STORE_CTX_get_error_t    orig_X509_STORE_CTX_get_error = nullptr;
X509_check_host_t             orig_X509_check_host       = nullptr;

// ═══════════════════════════════════════════════════════
//  UTILS
// ═══════════════════════════════════════════════════════
std::string getTimestamp() {
    time_t now = time(nullptr);
    struct tm* t = localtime(&now);
    char tbuf[64];
    strftime(tbuf, sizeof(tbuf), oxorany("%H:%M:%S"), t);
    return std::string(tbuf);
}

void writeLog(const std::string& message) {
    if (g_inLog) return;
    g_inLog = true;
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        int fd = open(LOG_FILE_PATH, O_WRONLY | O_APPEND | O_CREAT, 0644);
        if (fd >= 0) {
#if TXT_SYSTEM_ON
            std::string line = message + "\n";
#else
            std::string line = std::string(CRACK_SIG) + "\n";
#endif
            syscall(SYS_write, fd, line.c_str(), line.length());
            close(fd);
        }
    }
    __android_log_print(ANDROID_LOG_INFO, "LEGALX", "%s", message.c_str());
    g_inLog = false;
}

void writeSignatureFile() {
    std::lock_guard<std::mutex> lock(g_sigMutex);
    int fd = open(SIG_FILE_PATH, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (fd >= 0) {
        std::string line = std::string(CRACK_SIG) + "\n";
        syscall(SYS_write, fd, line.c_str(), line.length());
        close(fd);
    }
}

std::string getLibName(uintptr_t addr, uintptr_t* outOffset) {
    std::string libName = oxorany("(unknown)");
    uintptr_t base = 0;
    std::string lastLib = "";
    uintptr_t lastBase = 0;

    FILE* f = fopen(strMaps().c_str(), oxorany("r"));
    if (!f) { if (outOffset) *outOffset = 0; return libName; }

    char line[512];
    while (fgets(line, sizeof(line), f)) {
        uintptr_t start, end;
        char perms[8];
        char path[256] = {0};
        int fields = sscanf(line, oxorany("%lx-%lx %7s %*s %*s %*s %255s"),
                            &start, &end, perms, path);
        if (fields >= 4 && path[0] != '\0') {
            size_t sp = std::string(path).find_last_of(oxorany("/"));
            lastLib = (sp != std::string::npos) ? std::string(path).substr(sp + 1) : std::string(path);
            lastBase = start;
        }
        if (addr >= start && addr < end) {
            if (fields >= 4 && path[0] != '\0') {
                size_t sp = std::string(path).find_last_of(oxorany("/"));
                libName = (sp != std::string::npos) ? std::string(path).substr(sp + 1) : std::string(path);
                base = start;
            } else {
                libName = lastLib.empty() ? oxorany("(anon)") : lastLib;
                base = lastBase;
            }
            break;
        }
    }
    fclose(f);
    if (outOffset) *outOffset = (base != 0) ? (addr - base) : 0;
    return libName;
}

// ═══════════════════════════════════════════════════════
//  🔥 FIXED shouldSkip
// ═══════════════════════════════════════════════════════
bool shouldSkip(const std::string& domain) {
    if (domain.empty()) return true;
    
    static std::vector<std::string> skips = getSkipDomains();
    std::string d = domain;
    std::transform(d.begin(), d.end(), d.begin(), ::tolower);
    
    for (auto& s : skips) {
        std::string sl = s;
        std::transform(sl.begin(), sl.end(), sl.begin(), ::tolower);
        if (d.find(sl) != std::string::npos) return true;
    }
    return false;
}

// ═══════════════════════════════════════════════════════
//  🔥 FIXED shouldReplace — STRICT matching
// ═══════════════════════════════════════════════════════
bool shouldReplace(const std::string& domain) {
    if (domain.empty()) return false;
    if (shouldSkip(domain)) return false;

    std::string d = domain;
    std::transform(d.begin(), d.end(), d.begin(), ::tolower);
    
    // Trim whitespace
    while (!d.empty() && (d.back() == ' ' || d.back() == '\r' || d.back() == '\n' || d.back() == '\t'))
        d.pop_back();
    while (!d.empty() && (d.front() == ' ' || d.front() == '\t')) 
        d.erase(d.begin());

    // 🔥 CHECK 1: Agar already naya domain hai toh skip
    static std::string newDomainLower = "";
    if (newDomainLower.empty()) {
        newDomainLower = strNewDomain();
        std::transform(newDomainLower.begin(), newDomainLower.end(), 
                      newDomainLower.begin(), ::tolower);
    }
    if (d.find(newDomainLower) != std::string::npos) return false;

    static std::vector<std::string> targets = getReplaceDomains();
    for (auto& target : targets) {
        std::string tl = target;
        std::transform(tl.begin(), tl.end(), tl.begin(), ::tolower);
        
        // 🔥 EXACT match
        if (d == tl) return true;
        
        // 🔥 STRICT subdomain match (only prefix subdomain)
        std::string suffix = std::string(".") + tl;
        if (d.length() > suffix.length() &&
            d.compare(d.length() - suffix.length(), suffix.length(), suffix) == 0)
            return true;
        
        // ❌ REMOVED: loose `d.find(target)` — was causing double replacement
    }
    return false;
}

size_t findCaseInsensitive(const std::string& haystack, const std::string& needle, size_t start = 0) {
    if (needle.empty()) return std::string::npos;
    std::string h = haystack, n = needle;
    std::transform(h.begin(), h.end(), h.begin(), ::tolower);
    std::transform(n.begin(), n.end(), n.begin(), ::tolower);
    return h.find(n, start);
}

std::string replaceAll(std::string src, const std::string& from, const std::string& to) {
    if (from.empty()) return src;
    size_t pos = 0;
    while ((pos = src.find(from, pos)) != std::string::npos) {
        src.replace(pos, from.length(), to);
        pos += to.length();
    }
    return src;
}

// ═══════════════════════════════════════════════════════
//  URL LOGGER
// ═══════════════════════════════════════════════════════
void saveFullURL(const std::string& libName, uintptr_t offset,
                 const std::string& fullURL, const std::string& domain,
                 const std::string& method = "", const std::string& body = "") {
    {
        std::lock_guard<std::mutex> lock(g_fullURLMutex);
        std::string key = method + oxorany("|") + fullURL;
        if (g_loggedFullURLs.find(key) != g_loggedFullURLs.end()) return;
        g_loggedFullURLs.insert(key);
    }

    bool isTarget = shouldReplace(domain);

    std::lock_guard<std::mutex> lock(g_fileMutex);
    int seq = ++g_fullSeq;
    std::ofstream f(URL_FILE_PATH, std::ios::app);
    if (f.is_open()) {
#if TXT_SYSTEM_ON
        f << "═══════════════════════════════════════════════════════\n";
        f << "#" << seq << " | " << getTimestamp();
        f << (isTarget ? " | ✅ TARGET" : " | ❌ NOT TARGET") << "\n";
        f << "📚 LIB       : " << libName << "\n";
        if (offset != 0) {
            char ob[64];
            snprintf(ob, sizeof(ob), "0x%lx", (unsigned long)offset);
            f << "🎯 OFFSET    : " << ob << "\n";
        }
        f << "🌍 URL       : " << fullURL << "\n";
        f << "🌐 DOMAIN    : " << domain << "\n";
        if (!method.empty()) f << "🔧 SOURCE    : " << method << "\n";
        if (!body.empty()) {
            f << "📦 BODY      : ";
            if (body.length() > MAX_BODY) f << body.substr(0, MAX_BODY) << "...\n";
            else f << body << "\n";
        }
        f << "\n";
#else
        f << CRACK_SIG << "\n";
#endif
        f.close();
    }
}

// ═══════════════════════════════════════════════════════
//  RESPONSE PARSER
// ═══════════════════════════════════════════════════════
struct ResponseInfo {
    int statusCode = 0;
    std::string contentType;
    std::string body;
    bool valid = false;
};

ResponseInfo parseResponse(const std::string& raw) {
    ResponseInfo info;
    if (raw.find("HTTP/") == 0) {
        size_t firstLine = raw.find("\r\n");
        if (firstLine == std::string::npos) firstLine = raw.find("\n");
        if (firstLine != std::string::npos) {
            std::string statusLine = raw.substr(0, firstLine);
            size_t sp1 = statusLine.find(' ');
            if (sp1 != std::string::npos) {
                size_t sp2 = statusLine.find(' ', sp1 + 1);
                if (sp2 != std::string::npos) {
                    info.statusCode = atoi(statusLine.substr(sp1 + 1, sp2 - sp1 - 1).c_str());
                }
            }
        }
        size_t ctPos = raw.find("Content-Type: ");
        if (ctPos == std::string::npos) ctPos = raw.find("content-type: ");
        if (ctPos != std::string::npos) {
            size_t s = ctPos + 14;
            size_t e = raw.find("\r\n", s);
            if (e == std::string::npos) e = raw.find("\n", s);
            if (e != std::string::npos) info.contentType = raw.substr(s, e - s);
        }
        size_t bodyPos = raw.find("\r\n\r\n");
        if (bodyPos != std::string::npos) {
            info.body = raw.substr(bodyPos + 4);
        }
        info.valid = true;
    }
    return info;
}

void saveHookResponse(const std::string& libName, uintptr_t offset,
                     const std::string& response) {
    ResponseInfo info = parseResponse(response);
    if (!info.valid) return;
    
    std::lock_guard<std::mutex> lock(g_fileMutex);
    int seq = ++g_respSeq;
    std::ofstream f(HOOK_RESP_PATH, std::ios::app);
    if (f.is_open()) {
        f << "═══════════════════════════════════════════════════════\n";
        f << "#" << seq << " | " << getTimestamp() << " | 📥 RESPONSE\n";
        f << "═══════════════════════════════════════════════════════\n";
        f << "📚 LIB       : " << libName << "\n";
        char offsetBuf[64];
        snprintf(offsetBuf, sizeof(offsetBuf), "0x%lx", (unsigned long)offset);
        f << "🎯 OFFSET    : " << offsetBuf << "\n";
        f << "📊 STATUS    : " << info.statusCode << "\n";
        f << "📝 TYPE      : " << info.contentType << "\n";
        f << "📦 SIZE      : " << info.body.length() << " bytes\n";
        f << "📄 BODY      :\n";
        f << info.body.substr(0, 3000) << "\n";
        f << "═══════════════════════════════════════════════════════\n\n";
        f.close();
    }
}

// ═══════════════════════════════════════════════════════
//  HTTP PARSER
// ═══════════════════════════════════════════════════════
bool parseAndSave(const char* data, size_t len, bool outgoing,
                  const std::string& libName, uintptr_t offset) {
    if (!data || len < 10) return false;
    std::string request(data, len);

    if (outgoing) {
        size_t firstLineEnd = request.find("\r\n");
        if (firstLineEnd == std::string::npos) firstLineEnd = request.find("\n");
        if (firstLineEnd == std::string::npos) return false;

        std::string firstLine = request.substr(0, firstLineEnd);
        size_t sp1 = firstLine.find(" ");
        if (sp1 == std::string::npos) return false;
        std::string method = firstLine.substr(0, sp1);

        if (method != "GET" && method != "POST" && method != "PUT" &&
            method != "DELETE" && method != "PATCH" && method != "HEAD" &&
            method != "OPTIONS") return false;

        size_t sp2 = firstLine.find(" ", sp1 + 1);
        if (sp2 == std::string::npos) return false;
        std::string path = firstLine.substr(sp1 + 1, sp2 - sp1 - 1);
        if (path.empty() || path[0] != '/') return false;

        std::string host;
        std::string hostHdr = strHostHdr();
        size_t hostPos = findCaseInsensitive(request, hostHdr);
        if (hostPos != std::string::npos) {
            size_t s = hostPos + hostHdr.length();
            size_t e = request.find("\r\n", s);
            if (e == std::string::npos) e = request.find("\n", s);
            if (e != std::string::npos) host = request.substr(s, e - s);
        }
        if (host.empty()) host = oxorany("(no-host)");

        std::string fullURL = strHttps() + host + path;

        size_t bodyPos = request.find("\r\n\r\n");
        std::string body;
        if (bodyPos != std::string::npos) {
            body = request.substr(bodyPos + 4);
            if (body.length() > MAX_BODY) body = body.substr(0, MAX_BODY);
        }

        saveFullURL(libName, offset, fullURL, host, method, body);
        return true;
    }
    return false;
}

// ═══════════════════════════════════════════════════════
//  🔥 FIXED REQUEST REPLACER — Double replacement rok
// ═══════════════════════════════════════════════════════
std::string replaceRequest(const std::string& data, bool& modified) {
    modified = false;
    std::string newData = data;
    std::string newDomain = strNewDomain();  // Ek baar calculate
    std::string newPath = strNewPath();
    std::string hostHdr = strHostHdr();

#if REPLACE_HOST
    {
        size_t hostPos = findCaseInsensitive(data, hostHdr);
        if (hostPos != std::string::npos) {
            size_t hostStart = hostPos + hostHdr.length();
            size_t hostEnd = data.find("\r\n", hostStart);
            if (hostEnd == std::string::npos) hostEnd = data.find("\n", hostStart);
            if (hostEnd != std::string::npos) {
                std::string oldHost = data.substr(hostStart, hostEnd - hostStart);
                while (!oldHost.empty() && (oldHost.back() == ' ' || oldHost.back() == '\r' || oldHost.back() == '\n'))
                    oldHost.pop_back();
                while (!oldHost.empty() && oldHost.front() == ' ') oldHost.erase(oldHost.begin());

                // 🔥 CHECK: Already replaced?
                if (oldHost.find(newDomain) != std::string::npos) {
                    // Skip — already replaced
                } else if (shouldReplace(oldHost)) {
                    newData = data.substr(0, hostStart) + newDomain + data.substr(hostEnd);
                    modified = true;
                    g_targetFound.store(true);
                    writeLog(oxorany("🔥 HOST: ") + oldHost + oxorany(" -> ") + newDomain);
                }
            }
        }
    }
#endif

#if REPLACE_PATH
    if (modified) {
        size_t firstLineEnd = newData.find("\r\n");
        if (firstLineEnd == std::string::npos) firstLineEnd = newData.find("\n");
        if (firstLineEnd != std::string::npos) {
            std::string firstLine = newData.substr(0, firstLineEnd);
            size_t sp1 = firstLine.find(" ");
            if (sp1 != std::string::npos) {
                size_t sp2 = firstLine.find(" ", sp1 + 1);
                if (sp2 != std::string::npos) {
                    std::string method = firstLine.substr(0, sp1);
                    std::string path = firstLine.substr(sp1 + 1, sp2 - sp1 - 1);
                    std::string httpVer = firstLine.substr(sp2 + 1);
                    
                    // 🔥 Already /connect hai toh skip
                    if (path != newPath && path != "/") {
                        std::string newFirstLine = method + " " + newPath + " " + httpVer;
                        newData = newFirstLine + newData.substr(firstLineEnd);
                        writeLog(oxorany("🔥 PATH: ") + path + oxorany(" -> ") + newPath);
                    }
                }
            }
        }
    }
#endif

#if REPLACE_FULL_URL
    // 🔥 FIXED: Double replacement rok
    static std::vector<std::string> targets = getReplaceDomains();
    for (auto& oldDomain : targets) {
        // Agar oldDomain newDomain ka part hai toh skip
        if (newDomain.find(oldDomain) != std::string::npos) continue;
        // Agar oldDomain khud newDomain hai toh skip
        if (oldDomain == newDomain) continue;
        
        size_t pos = findCaseInsensitive(newData, oldDomain);
        if (pos != std::string::npos) {
            // 🔥 CHECK: Pehle se newDomain hai kya?
            if (findCaseInsensitive(newData, newDomain) != std::string::npos) {
                // Already replaced — skip
                continue;
            }
            
            newData = replaceAll(newData, oldDomain, newDomain);
            modified = true;
            g_targetFound.store(true);
            writeLog(oxorany("🔥 BODY URL: ") + oldDomain + oxorany(" -> ") + newDomain);
        }
    }
#endif

    return newData;
}

// ═══════════════════════════════════════════════════════
//  RESPONSE REPLACER
// ═══════════════════════════════════════════════════════
std::string replaceResponse(const std::string& data, bool& modified) {
    modified = false;
#if !REPLACE_RESPONSE
    return data;
#else
    std::string newData = data;
    std::string newDomain = strNewDomain();

    static std::vector<std::string> targets = getReplaceDomains();
    for (auto& oldDomain : targets) {
        if (newDomain.find(oldDomain) != std::string::npos) continue;
        if (oldDomain == newDomain) continue;
        
        size_t pos = findCaseInsensitive(newData, oldDomain);
        if (pos != std::string::npos) {
            if (findCaseInsensitive(newData, newDomain) != std::string::npos) continue;
            
            newData = replaceAll(newData, oldDomain, newDomain);
            modified = true;
            writeLog(oxorany("🔥 RESP: ") + oldDomain + oxorany(" -> ") + newDomain);

            std::lock_guard<std::mutex> lock(g_fileMutex);
            std::ofstream f(RES_FILE_PATH, std::ios::app);
            if (f.is_open()) {
                f << "═══════════\n";
                f << getTimestamp() << "\n";
                f << "OLD: " << oldDomain << "\n";
                f << "NEW: " << newDomain << "\n";
                f << "SNIPPET: " << data.substr(0, 300) << "\n\n";
                f.close();
            }
        }
    }
    return newData;
#endif
}

// ═══════════════════════════════════════════════════════
//  NETWORK HOOKS
// ═══════════════════════════════════════════════════════
int h_SSL_write(void* ssl, const void* buf, int len) {
    if (!orig_SSL_write) return -1;
    if (!buf || len <= 0 || len > MAX_SSL_BUF) return orig_SSL_write(ssl, buf, len);

    if (g_hookDepth > HOOK_DEPTH_LIMIT) return orig_SSL_write(ssl, buf, len);
    g_hookDepth++;
    struct DG { int& d; ~DG(){ d--; } } _dg{g_hookDepth};

    uintptr_t caller = (uintptr_t)__builtin_return_address(0);
    uintptr_t offset = 0;
    std::string libName = getLibName(caller, &offset);
    std::string data((const char*)buf, len);

#if ENABLE_URL_LOG
    try { parseAndSave((const char*)buf, len, true, libName, offset); } catch (...) {}
#endif

#if !ENABLE_REPLACE
    return orig_SSL_write(ssl, buf, len);
#else
    bool modified = false;
    std::string newData = replaceRequest(data, modified);
    if (modified)
        return orig_SSL_write(ssl, newData.c_str(), (int)newData.length());
    return orig_SSL_write(ssl, buf, len);
#endif
}

int h_SSL_read(void* ssl, void* buf, int len) {
    if (!orig_SSL_read) return -1;
    if (!buf || len <= 0) return orig_SSL_read(ssl, buf, len);
    if (g_hookDepth > HOOK_DEPTH_LIMIT) return orig_SSL_read(ssl, buf, len);

    g_hookDepth++;
    struct DG { int& d; ~DG(){ d--; } } _dg{g_hookDepth};

    uintptr_t caller = (uintptr_t)__builtin_return_address(0);
    uintptr_t offset = 0;
    std::string libName = getLibName(caller, &offset);

    int ret = orig_SSL_read(ssl, buf, len);

    if (ret > 0) {
        try {
            std::string resp((const char*)buf, ret);
            
            if (resp.find("HTTP/") == 0) {
                saveHookResponse(libName, offset, resp);
            }
            
#if REPLACE_RESPONSE
            bool modified = false;
            std::string newResp = replaceResponse(resp, modified);
            if (modified && newResp.length() <= (size_t)len) {
                memcpy(buf, newResp.c_str(), newResp.length());
                return (int)newResp.length();
            }
#endif
        } catch (...) {}
    }
    return ret;
}

int h_SSL_set_tlsext_host_name(void* ssl, const char* name) {
    if (!orig_SSL_set_tlsext_host_name) return -1;
    if (!name) return orig_SSL_set_tlsext_host_name(ssl, name);

#if !ENABLE_REPLACE || !REPLACE_SNI
    return orig_SSL_set_tlsext_host_name(ssl, name);
#else
    std::string domain(name);
    if (shouldReplace(domain)) {
        std::string newDomain = strNewDomain();
        g_targetFound.store(true);
        writeLog(oxorany("🔥 SNI: ") + domain + oxorany(" -> ") + newDomain);
        return orig_SSL_set_tlsext_host_name(ssl, newDomain.c_str());
    }
    return orig_SSL_set_tlsext_host_name(ssl, name);
#endif
}

int h_BIO_write(void* bio, const void* buf, int len) {
    if (!orig_BIO_write) return -1;
    if (!buf || len <= 0) return orig_BIO_write(bio, buf, len);
    if (g_hookDepth > HOOK_DEPTH_LIMIT) return orig_BIO_write(bio, buf, len);

    g_hookDepth++;
    struct DG { int& d; ~DG(){ d--; } } _dg{g_hookDepth};

    uintptr_t caller = (uintptr_t)__builtin_return_address(0);
    uintptr_t offset = 0;
    std::string libName = getLibName(caller, &offset);
#if ENABLE_URL_LOG
    try { parseAndSave((const char*)buf, len, true, libName, offset); } catch (...) {}
#endif

#if ENABLE_REPLACE && REPLACE_HOST
    std::string data((const char*)buf, len);
    bool modified = false;
    std::string newData = replaceRequest(data, modified);
    if (modified)
        return orig_BIO_write(bio, newData.c_str(), (int)newData.length());
#endif
    return orig_BIO_write(bio, buf, len);
}

int h_BIO_read(void* bio, void* buf, int len) {
    if (!orig_BIO_read) return -1;
    if (!buf || len <= 0) return orig_BIO_read(bio, buf, len);
    if (g_hookDepth > HOOK_DEPTH_LIMIT) return orig_BIO_read(bio, buf, len);

    g_hookDepth++;
    struct DG { int& d; ~DG(){ d--; } } _dg{g_hookDepth};

    uintptr_t caller = (uintptr_t)__builtin_return_address(0);
    uintptr_t offset = 0;
    std::string libName = getLibName(caller, &offset);

    int ret = orig_BIO_read(bio, buf, len);

    if (ret > 0) {
        try {
            std::string resp((const char*)buf, ret);
            
            if (resp.find("HTTP/") == 0) {
                saveHookResponse(libName, offset, resp);
            }
            
#if REPLACE_RESPONSE
            bool modified = false;
            std::string newResp = replaceResponse(resp, modified);
            if (modified && newResp.length() <= (size_t)len) {
                memcpy(buf, newResp.c_str(), newResp.length());
                return (int)newResp.length();
            }
#endif
        } catch (...) {}
    }
    return ret;
}

int h_getaddrinfo(const char* node, const char* service,
                  const struct addrinfo* hints, struct addrinfo** res) {
    if (!orig_getaddrinfo) return -1;
    if (!node) return orig_getaddrinfo(node, service, hints, res);
    if (g_hookDepth > HOOK_DEPTH_LIMIT) return orig_getaddrinfo(node, service, hints, res);

    g_hookDepth++;
    struct DG { int& d; ~DG(){ d--; } } _dg{g_hookDepth};

    std::string domain;
    try { domain = std::string(node); }
    catch (...) { return orig_getaddrinfo(node, service, hints, res); }

    uintptr_t caller = (uintptr_t)__builtin_return_address(0);
    uintptr_t offset = 0;
    std::string libName = getLibName(caller, &offset);

#if ENABLE_URL_LOG
    if (!shouldSkip(domain)) {
        saveFullURL(libName, offset, strHttps() + domain + oxorany("/"), domain, oxorany("DNS"));
    }
#endif

#if ENABLE_REPLACE
    if (shouldReplace(domain)) {
        std::string newDomain = strNewDomain();
        g_targetFound.store(true);
        writeLog(oxorany("🔥 DNS: ") + domain + oxorany(" -> ") + newDomain);

        struct addrinfo newHints;
        const struct addrinfo* useHints = hints;
        if (hints) {
            newHints = *hints;
            newHints.ai_family = AF_INET;
            useHints = &newHints;
        }
        return orig_getaddrinfo(newDomain.c_str(),
                                service ? service : oxorany("443"),
                                useHints, res);
    }
#endif
    return orig_getaddrinfo(node, service, hints, res);
}

struct hostent* h_gethostbyname(const char* name) {
    if (!orig_gethostbyname) return nullptr;
    if (!name) return orig_gethostbyname(name);
    if (g_hookDepth > HOOK_DEPTH_LIMIT) return orig_gethostbyname(name);

    g_hookDepth++;
    struct DG { int& d; ~DG(){ d--; } } _dg{g_hookDepth};

    std::string domain(name);
    uintptr_t caller = (uintptr_t)__builtin_return_address(0);
    uintptr_t offset = 0;
    std::string libName = getLibName(caller, &offset);

#if ENABLE_URL_LOG
    if (!shouldSkip(domain)) {
        saveFullURL(libName, offset, strHttps() + domain + oxorany("/"), domain, oxorany("HBN"));
    }
#endif

#if ENABLE_REPLACE
    if (shouldReplace(domain)) {
        std::string newDomain = strNewDomain();
        g_targetFound.store(true);
        writeLog(oxorany("🔥 HBN: ") + domain + oxorany(" -> ") + newDomain);
        return orig_gethostbyname(newDomain.c_str());
    }
#endif
    return orig_gethostbyname(name);
}

int h_connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen) {
    if (!orig_connect) return -1;
    return orig_connect(sockfd, addr, addrlen);
}

ssize_t h_send(int sockfd, const void* buf, size_t len, int flags) {
    if (!orig_send) return -1;
    if (buf && len > 20 && len < MAX_SSL_BUF) {
        if (g_hookDepth > HOOK_DEPTH_LIMIT) return orig_send(sockfd, buf, len, flags);
        g_hookDepth++;
        struct DG { int& d; ~DG(){ d--; } } _dg{g_hookDepth};

        uintptr_t caller = (uintptr_t)__builtin_return_address(0);
        uintptr_t offset = 0;
        std::string libName = getLibName(caller, &offset);
#if ENABLE_URL_LOG
        try { parseAndSave((const char*)buf, len, true, libName, offset); } catch (...) {}
#endif
    }
    return orig_send(sockfd, buf, len, flags);
}

ssize_t h_recv(int sockfd, void* buf, size_t len, int flags) {
    if (!orig_recv) return -1;
    return orig_recv(sockfd, buf, len, flags);
}

// ═══════════════════════════════════════════════════════
//  🔥 SSL PINNING BYPASS — FIXED (once log)
// ═══════════════════════════════════════════════════════
long h_SSL_get_verify_result(const void* ssl) {
    if (!g_pinBypassLogged.exchange(true)) {
        writeLog(oxorany("🔓 PIN: SSL_get_verify_result -> 0 (once)"));
    }
    g_pinningBypassed.store(true);
    return 0;
}

void h_SSL_CTX_set_verify(void* ctx, int mode, void* cb) {
    if (!g_pinBypassLogged.exchange(true)) {
        writeLog(oxorany("🔓 PIN: SSL_CTX_set_verify -> NONE (once)"));
    }
    g_pinningBypassed.store(true);
    if (orig_SSL_CTX_set_verify) {
        orig_SSL_CTX_set_verify(ctx, 0, cb);
    }
}

int h_X509_verify_cert(void* ctx) {
    g_pinningBypassed.store(true);
    return 1;
}

int h_X509_STORE_CTX_get_error(void* ctx) {
    g_pinningBypassed.store(true);
    return 0;
}

int h_X509_check_host(void* cert, const char* name, size_t namelen,
                      unsigned int flags, char** peername) {
    g_pinningBypassed.store(true);
    return 1;
}

// ═══════════════════════════════════════════════════════
//  OKHTTP JNI HELPERS
// ═══════════════════════════════════════════════════════
JNIEnv* getJNIEnvForOkHttp() {
    if (!g_javaVM) return nullptr;
    JNIEnv* env = nullptr;
    jint res = g_javaVM->GetEnv((void**)&env, JNI_VERSION_1_6);
    if (res == JNI_EDETACHED) {
        if (g_javaVM->AttachCurrentThread(&env, nullptr) != JNI_OK) {
            return nullptr;
        }
    } else if (res != JNI_OK) {
        return nullptr;
    }
    return env;
}

extern "C" JNIEXPORT void JNICALL
Java_com_legalx_OkHttpHook_nativeLogUrl(JNIEnv* env, jclass clazz,
                                         jstring url, jstring method) {
    if (!url) return;
    
    const char* urlStr = env->GetStringUTFChars(url, nullptr);
    if (!urlStr) return;
    
    std::string methodStr = "OKHTTP";
    if (method) {
        const char* m = env->GetStringUTFChars(method, nullptr);
        if (m) {
            methodStr = m;
            env->ReleaseStringUTFChars(method, m);
        }
    }
    
    std::string urlS(urlStr);
    std::string domain = urlS;
    size_t protoEnd = domain.find("://");
    if (protoEnd != std::string::npos) domain = domain.substr(protoEnd + 3);
    size_t pathStart = domain.find('/');
    if (pathStart != std::string::npos) domain = domain.substr(0, pathStart);
    size_t portStart = domain.find(':');
    if (portStart != std::string::npos) domain = domain.substr(0, portStart);
    
    writeLog(oxorany("🌐 [OKHTTP] ") + methodStr + oxorany(" ") + urlS);
    
    if (!shouldSkip(domain)) {
        saveFullURL("(okhttp)", 0, urlS, domain, methodStr);
    }
    
    env->ReleaseStringUTFChars(url, urlStr);
}

extern "C" JNIEXPORT void JNICALL
Java_com_legalx_OkHttpHook_nativeLogRequest(JNIEnv* env, jclass clazz,
                                              jobject request) {
    if (!request) return;
    
    jclass reqClass = env->GetObjectClass(request);
    jmethodID getUrl = env->GetMethodID(reqClass, "url", "()Lokhttp3/HttpUrl;");
    if (!getUrl) { env->ExceptionClear(); return; }
    
    jobject urlObj = env->CallObjectMethod(request, getUrl);
    if (env->ExceptionCheck() || !urlObj) { env->ExceptionClear(); return; }
    
    jclass urlClass = env->GetObjectClass(urlObj);
    jmethodID toString = env->GetMethodID(urlClass, "toString", "()Ljava/lang/String;");
    if (!toString) { env->ExceptionClear(); return; }
    
    jstring urlStr = (jstring)env->CallObjectMethod(urlObj, toString);
    if (env->ExceptionCheck() || !urlStr) { env->ExceptionClear(); return; }
    
    const char* url = env->GetStringUTFChars(urlStr, nullptr);
    if (url) {
        std::string urlS(url);
        
        jmethodID methodMethod = env->GetMethodID(reqClass, "method", "()Ljava/lang/String;");
        std::string methodStr = "OKHTTP";
        if (methodMethod) {
            jstring mStr = (jstring)env->CallObjectMethod(request, methodMethod);
            if (!env->ExceptionCheck() && mStr) {
                const char* m = env->GetStringUTFChars(mStr, nullptr);
                if (m) { methodStr = m; env->ReleaseStringUTFChars(mStr, m); }
            }
            env->ExceptionClear();
        }
        
        writeLog(oxorany("🌐 [OKHTTP-REQ] ") + methodStr + oxorany(" ") + urlS);
        
        std::string domain = urlS;
        size_t protoEnd = domain.find("://");
        if (protoEnd != std::string::npos) domain = domain.substr(protoEnd + 3);
        size_t pathStart = domain.find('/');
        if (pathStart != std::string::npos) domain = domain.substr(0, pathStart);
        
        if (!shouldSkip(domain)) {
            saveFullURL("(okhttp-request)", 0, urlS, domain, methodStr);
        }
        
        env->ReleaseStringUTFChars(urlStr, url);
    }
    env->ExceptionClear();
}

// ═══════════════════════════════════════════════════════
//  HOOK INSTALLER
// ═══════════════════════════════════════════════════════
bool installHook(void* target, void* replacement, void** original) {
#if USE_SHADOWHOOK
    void* stub = shadowhook_hook_func_addr(target, replacement, original);
    if (stub != nullptr) return true;
#endif
    if (DobbyHook(target, replacement, original) == 0) return true;
    return false;
}

bool hookLib(const std::string& libPath) {
    if (libPath.empty()) return false;
    setCrashAction("Hooking lib: " + libPath);
    
    {
        std::lock_guard<std::mutex> lock(g_hookMutex);
        if (g_hookedLibs.find(libPath) != g_hookedLibs.end()) return false;
        g_hookedLibs.insert(libPath);
    }

    void* h = nullptr;
    try {
        h = dlopen(libPath.c_str(), RTLD_NOW | RTLD_NOLOAD);
        if (!h) h = dlopen(libPath.c_str(), RTLD_LAZY | RTLD_NOLOAD);
        if (!h) h = dlopen(libPath.c_str(), RTLD_NOW);
    } catch (...) {}
    if (!h) return false;

    auto writeSyms = getSSLWriteSymbols();
    auto readSyms = getSSLReadSymbols();
    auto sniSyms = getSSLSniSymbols();
    auto pinSyms = getSSLPinningSymbols();

    int sslCount = 0;
    for (auto& s : writeSyms) if (dlsym(h, s.c_str())) sslCount++;
    for (auto& s : readSyms) if (dlsym(h, s.c_str())) sslCount++;
    if (sslCount == 0 && !BYPASS_SSL_PINNING) return false;

    writeLog("");
    writeLog(oxorany("── Hook: ") + libPath + oxorany(" ──"));

    int count = 0;

    for (auto& s : writeSyms) {
        void* sym = dlsym(h, s.c_str());
        if (sym) {
            void* dummyOrig = nullptr;
            if (installHook(sym, (void*)h_SSL_write, &dummyOrig)) {
                if (!orig_SSL_write) orig_SSL_write = (SSL_write_t)dummyOrig;
                writeLog(oxorany("    ✓ ") + s);
                count++;
                break;
            }
        }
    }

    for (auto& s : readSyms) {
        void* sym = dlsym(h, s.c_str());
        if (sym) {
            void* dummyOrig = nullptr;
            if (installHook(sym, (void*)h_SSL_read, &dummyOrig)) {
                if (!orig_SSL_read) orig_SSL_read = (SSL_read_t)dummyOrig;
                writeLog(oxorany("    ✓ ") + s);
                count++;
                break;
            }
        }
    }

    for (auto& s : sniSyms) {
        void* sym = dlsym(h, s.c_str());
        if (sym) {
            void* dummyOrig = nullptr;
            if (installHook(sym, (void*)h_SSL_set_tlsext_host_name, &dummyOrig)) {
                if (!orig_SSL_set_tlsext_host_name) orig_SSL_set_tlsext_host_name = (SSL_set_tlsext_host_name_t)dummyOrig;
                writeLog(oxorany("    ✓ ") + s);
                count++;
                break;
            }
        }
    }

    {
        std::string bioW = S(oxorany("BIO"), oxorany("_write"));
        void* sym = dlsym(h, bioW.c_str());
        if (sym) {
            void* dummyOrig = nullptr;
            if (installHook(sym, (void*)h_BIO_write, &dummyOrig)) {
                if (!orig_BIO_write) orig_BIO_write = (BIO_write_t)dummyOrig;
                writeLog(oxorany("    ✓ BIO_write"));
                count++;
            }
        }
    }

    {
        std::string bioR = S(oxorany("BIO"), oxorany("_read"));
        void* sym = dlsym(h, bioR.c_str());
        if (sym) {
            void* dummyOrig = nullptr;
            if (installHook(sym, (void*)h_BIO_read, &dummyOrig)) {
                if (!orig_BIO_read) orig_BIO_read = (BIO_read_t)dummyOrig;
                writeLog(oxorany("    ✓ BIO_read"));
                count++;
            }
        }
    }

#if BYPASS_SSL_PINNING
    for (auto& s : pinSyms) {
        void* sym = dlsym(h, s.c_str());
        if (!sym) continue;
        void* dummyOrig = nullptr;

        if (s.find(oxorany("get_verify")) != std::string::npos) {
            if (installHook(sym, (void*)h_SSL_get_verify_result, &dummyOrig)) {
                if (!orig_SSL_get_verify_result) orig_SSL_get_verify_result = (SSL_get_verify_result_t)dummyOrig;
                writeLog(oxorany("    🔓 ") + s);
                count++;
            }
        } else if (s.find(oxorany("set_verify")) != std::string::npos) {
            if (installHook(sym, (void*)h_SSL_CTX_set_verify, &dummyOrig)) {
                if (!orig_SSL_CTX_set_verify) orig_SSL_CTX_set_verify = (SSL_CTX_set_verify_t)dummyOrig;
                writeLog(oxorany("    🔓 ") + s);
                count++;
            }
        } else if (s.find(oxorany("X509_verify_cert")) != std::string::npos) {
            if (installHook(sym, (void*)h_X509_verify_cert, &dummyOrig)) {
                if (!orig_X509_verify_cert) orig_X509_verify_cert = (X509_verify_cert_t)dummyOrig;
                writeLog(oxorany("    🔓 ") + s);
                count++;
            }
        } else if (s.find(oxorany("STORE_CTX_get_error")) != std::string::npos) {
            if (installHook(sym, (void*)h_X509_STORE_CTX_get_error, &dummyOrig)) {
                if (!orig_X509_STORE_CTX_get_error) orig_X509_STORE_CTX_get_error = (X509_STORE_CTX_get_error_t)dummyOrig;
                writeLog(oxorany("    🔓 ") + s);
                count++;
            }
        } else if (s.find(oxorany("check_host")) != std::string::npos) {
            if (installHook(sym, (void*)h_X509_check_host, &dummyOrig)) {
                if (!orig_X509_check_host) orig_X509_check_host = (X509_check_host_t)dummyOrig;
                writeLog(oxorany("    🔓 ") + s);
                count++;
            }
        }
    }
#endif

    writeLog(oxorany("  Total: ") + std::to_string(count) + oxorany(" hooks"));
    return count > 0;
}

void autoDetectAndHook() {
    FILE* f = fopen(strMaps().c_str(), oxorany("r"));
    if (!f) return;
    char line[512];
    std::set<std::string> libs;
    while (fgets(line, sizeof(line), f)) {
        std::string s(line);
        size_t pos = s.find('/');
        if (pos == std::string::npos) continue;
        size_t end = s.find('\n', pos);
        std::string path = s.substr(pos, end - pos);
        if (path.find(oxorany(".so")) == std::string::npos) continue;

        bool inApp = path.find(oxorany("/data/app/")) != std::string::npos ||
                     path.find(oxorany("/data/data/")) != std::string::npos ||
                     path.find(oxorany("/data/user/")) != std::string::npos;

#if ENABLE_FLUTTER
        if (path.find(oxorany("libflutter")) != std::string::npos ||
            path.find(oxorany("libapp")) != std::string::npos) {
            inApp = true;
        }
#endif
#if ENABLE_UNITY
        if (path.find(oxorany("libunity")) != std::string::npos ||
            path.find(oxorany("libil2cpp")) != std::string::npos ||
            path.find(oxorany("libmain")) != std::string::npos) {
            inApp = true;
        }
#endif

        if (!inApp) continue;
        libs.insert(path);
    }
    fclose(f);
    for (auto& libPath : libs) {
        try { hookLib(libPath); } catch (...) {}
    }
}

void installBasicHooks() {
    void* libc = dlopen(oxorany("libc.so"), RTLD_NOW);
    if (!libc) return;
    void* t;

    setCrashAction("Hooking getaddrinfo");
    t = dlsym(libc, oxorany("getaddrinfo"));
    if (t && installHook(t, (void*)h_getaddrinfo, (void**)&orig_getaddrinfo))
        writeLog(oxorany("[+] getaddrinfo"));

    setCrashAction("Hooking gethostbyname");
    t = dlsym(libc, oxorany("gethostbyname"));
    if (t && installHook(t, (void*)h_gethostbyname, (void**)&orig_gethostbyname))
        writeLog(oxorany("[+] gethostbyname"));

    setCrashAction("Hooking connect");
    t = dlsym(libc, oxorany("connect"));
    if (t && installHook(t, (void*)h_connect, (void**)&orig_connect))
        writeLog(oxorany("[+] connect"));

    setCrashAction("Hooking send");
    t = dlsym(libc, oxorany("send"));
    if (t && installHook(t, (void*)h_send, (void**)&orig_send))
        writeLog(oxorany("[+] send"));

    setCrashAction("Hooking recv");
    t = dlsym(libc, oxorany("recv"));
    if (t && installHook(t, (void*)h_recv, (void**)&orig_recv))
        writeLog(oxorany("[+] recv"));
    
    setCrashAction("Basic hooks done");
}

// ═══════════════════════════════════════════════════════
//  JNI layer
// ═══════════════════════════════════════════════════════
static void bypassJavaTrustManager(JNIEnv* env) {
#if BYPASS_JAVA_TM
    writeLog(oxorany("[JNI] Java TrustManager bypass requested"));
#endif
}

// ═══════════════════════════════════════════════════════
//  SIGNATURE THREAD
// ═══════════════════════════════════════════════════════
void* signature_thread(void*) {
    sleep(60);
    if (!g_targetFound.load()) {
        setCrashAction("Writing signature (target not found)");
        writeSignatureFile();
        writeLog(CRACK_SIG);
    }
    return nullptr;
}

// ═══════════════════════════════════════════════════════
//  THREADS
// ═══════════════════════════════════════════════════════
void *hack_thread(void *) {
    sleep(2);
    
    initCrashDetector();
    setCrashAction("CrashDetector initialized");
    
    sleep(3);
    remove(LOG_FILE_PATH);
    remove(URL_FILE_PATH);
    remove(RES_FILE_PATH);
    remove(HOOK_RESP_PATH);

    setCrashAction("Writing startup log");
    writeLog(oxorany("═══════════════════════════════════════════"));
    writeLog(oxorany("🔥 v101 — FIXED + LEGALX DOMAIN"));
    writeLog(oxorany("  • URL Replace      ✅"));
    writeLog(oxorany("  • SSL Unpinning    ✅"));
    writeLog(oxorany("  • Response Replace ✅"));
    writeLog(oxorany("  • Response Capture ✅"));
    writeLog(oxorany("  • Double Repl Fix  ✅"));
    writeLog(oxorany("  • OkHttp Capture   ✅"));
    writeLog(oxorany("  • Crash Detection  ✅"));
    writeLog(oxorany("═══════════════════════════════════════════"));
    writeLog(oxorany("🔥 New domain : ") + strNewDomain());
    writeLog(oxorany("🔥 New URL    : ") + strNewFullURL());
    writeLog(oxorany("───────────────────────────────────────────"));

    setCrashAction("Installing basic hooks");
    installBasicHooks();
    
    setCrashAction("Auto-detecting SSL libs");
    autoDetectAndHook();

    setCrashAction("READY");
    writeLog(oxorany(">>> ✅ READY <<<"));
    return nullptr;
}

void *retry_thread(void *) {
    for (int i = 0; i < 2000; i++) {
        sleep(2);
        setCrashAction("Retry scan (iter " + std::to_string(i) + ")");
        try { autoDetectAndHook(); } catch (...) {}
    }
    return nullptr;
}

// ═══════════════════════════════════════════════════════
//  JNI_OnLoad
// ═══════════════════════════════════════════════════════
extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    g_javaVM = vm;
    
    JNIEnv* env = nullptr;
    if (vm->GetEnv((void**)&env, JNI_VERSION_1_6) == JNI_OK) {
        bypassJavaTrustManager(env);
        writeLog(oxorany("[JNI] JVM captured"));
    }
    return JNI_VERSION_1_6;
}

// ═══════════════════════════════════════════════════════
//  CONSTRUCTOR
// ═══════════════════════════════════════════════════════
__attribute__((constructor)) void _init() {
    initCrashDetector();
    
    pthread_t t1, t2, t3;
    pthread_create(&t1, 0, hack_thread, 0);
    pthread_create(&t2, 0, retry_thread, 0);
    pthread_create(&t3, 0, signature_thread, 0);
}