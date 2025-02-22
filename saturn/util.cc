#include "util.h"

#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <execinfo.h>
#include <sstream>
#include <string>
#include <type_traits>

#include "fiber.h"
#include "log.h"
#include "macro.h"

namespace saturn {
    static saturn::Logger::ptr g_logger = LOGGER();


    std::string timestampToString(uint64_t timestamp, std::string_view fmt) {
        // 将时间戳转换为 std::chrono::system_clock::time_point
        std::chrono::system_clock::time_point tp = std::chrono::system_clock::from_time_t(timestamp);

        // 转换为 std::tm 结构
        std::time_t time_t_val = std::chrono::system_clock::to_time_t(tp);
        std::tm tm_val = *std::gmtime(&time_t_val); // 转换为 UTC 时间

        std::ostringstream oss;
        oss << std::put_time(&tm_val, fmt.data());
        return oss.str();
    }


 
    pid_t getThreadId() {
        return syscall(SYS_gettid);
    }
    uint32_t getFiberId() {
        return Fiber::GetFiberId();
    }

    void backtrace(std::vector<std::string>& vec, int size, int skip) {
        int nptrs;
        void** buffer = static_cast<void**>(malloc(size * sizeof(void*)));
        char** strings;
        nptrs = ::backtrace(buffer, size);
        strings = ::backtrace_symbols(buffer, nptrs);
        if (strings == nullptr) {
            SATURN_LOG_ERROR(g_logger) << "backtrace error.";
            free(buffer);
            free(strings);
            return;
        }
        for (size_t i = skip; i < nptrs; ++i) {
            vec.emplace_back(strings[i]);
        }
        free(buffer);
        free(strings);
    }

    std::string backtraceStr(int size , int skip , std::string_view prefix) {
        std::vector<std::string> strings;
        backtrace(strings, size, skip);
        std::stringstream ss;

        for (size_t i = 0; i < strings.size(); ++i) {
            ss << prefix << strings[i] << std::endl;
        }
        return ss.str();
    }

    std::string StringUtil::Format(const char* fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        auto rt = Formatv(fmt, ap);
        va_end(ap);
        return rt;
    }
    std::string StringUtil::Formatv(const char* fmt, va_list ap) {
        char *res = nullptr;
        int rt = vasprintf(&res, fmt, ap);
        if (rt == -1)
            return "";
        return std::string(res);
    }

    static const char uri_chars[256] = {
        /* 0 */
        0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 1, 1, 0,
        1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 0, 0, 0, 1, 0, 0,
        /* 64 */
        0, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 0, 0, 0, 0, 1,
        0, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 0, 0, 0, 1, 0,
        /* 128 */
        0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
        /* 192 */
        0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    };
    
    static const char xdigit_chars[256] = {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,1,2,3,4,5,6,7,8,9,0,0,0,0,0,0,
        0,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    };
    
    #define CHAR_IS_UNRESERVED(c)           \
        (uri_chars[(unsigned char)(c)])
    
    std::string StringUtil::UrlEncode(const std::string& str, bool space_as_plus) {
        static const char *hexdigits = "0123456789ABCDEF";
        std::string* ss = nullptr;
        const char* end = str.c_str() + str.length();
        for(const char* c = str.c_str() ; c < end; ++c) {
            if(!CHAR_IS_UNRESERVED(*c)) {
                if(!ss) {
                    ss = new std::string;
                    ss->reserve(str.size() * 1.2);
                    ss->append(str.c_str(), c - str.c_str());
                }
                if(*c == ' ' && space_as_plus) {
                    ss->append(1, '+');
                } else {
                    ss->append(1, '%');
                    ss->append(1, hexdigits[(uint8_t)*c >> 4]);
                    ss->append(1, hexdigits[*c & 0xf]);
                }
            } else if(ss) {
                ss->append(1, *c);
            }
        }
        if(!ss) {
            return str;
        } else {
            std::string rt = *ss;
            delete ss;
            return rt;
        }
    }
    
    std::string StringUtil::UrlDecode(const std::string& str, bool space_as_plus) {
        std::string* ss = nullptr;
        const char* end = str.c_str() + str.length();
        for(const char* c = str.c_str(); c < end; ++c) {
            if(*c == '+' && space_as_plus) {
                if(!ss) {
                    ss = new std::string;
                    ss->append(str.c_str(), c - str.c_str());
                }
                ss->append(1, ' ');
            } else if(*c == '%' && (c + 2) < end
                        && isxdigit(*(c + 1)) && isxdigit(*(c + 2))){
                if(!ss) {
                    ss = new std::string;
                    ss->append(str.c_str(), c - str.c_str());
                }
                ss->append(1, (char)(xdigit_chars[(int)*(c + 1)] << 4 | xdigit_chars[(int)*(c + 2)]));
                c += 2;
            } else if(ss) {
                ss->append(1, *c);
            }
        }
        if(!ss) {
            return str;
        } else {
            std::string rt = *ss;
            delete ss;
            return rt;
        }
    }

    std::string StringUtil::Trim(const std::string& str, const std::string& delimit) {
        size_t start = str.find_first_not_of(delimit);
        if (start == std::string::npos) return "";
    
        size_t end = str.find_last_not_of(delimit);
        return str.substr(start, end - start + 1);
    }
    std::string StringUtil::TrimLeft(const std::string& str, const std::string& delimit) {
        size_t start = str.find_first_not_of(delimit);
        if (start == std::string::npos) return "";
        return str.substr(start);
    }
    std::string StringUtil::TrimRight(const std::string& str, const std::string& delimit) {
        auto end = str.find_last_not_of(delimit);
        if(end == std::string::npos) {
            return "";
        }
        return str.substr(0, end + 1);
    }


    std::string StringUtil::WStringToString(const std::wstring& ws) {
        std::string str_locale = setlocale(LC_ALL, "");
        const wchar_t* wch_src = ws.c_str();
        size_t n_dest_size = wcstombs(NULL, wch_src, 0) + 1;
        char *ch_dest = new char[n_dest_size];
        memset(ch_dest,0,n_dest_size);
        wcstombs(ch_dest,wch_src,n_dest_size);
        std::string str_result = ch_dest;
        delete []ch_dest;
        setlocale(LC_ALL, str_locale.c_str());
        return str_result;
    }
    
    std::wstring StringUtil::StringToWString(const std::string& s) {
        std::string str_locale = setlocale(LC_ALL, "");
        const char* chSrc = s.c_str();
        size_t n_dest_size = mbstowcs(NULL, chSrc, 0) + 1;
        wchar_t* wch_dest = new wchar_t[n_dest_size];
        wmemset(wch_dest, 0, n_dest_size);
        mbstowcs(wch_dest,chSrc,n_dest_size);
        std::wstring wstr_result = wch_dest;
        delete []wch_dest;
        setlocale(LC_ALL, str_locale.c_str());
        return wstr_result;
    }    


}