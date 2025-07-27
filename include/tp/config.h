#ifndef INCLUDE_TP_CONFIG_H
#define INCLUDE_TP_CONFIG_H

// namespace tp
#define NAMESPACE_TP_BEGIN namespace tp {
#define NAMESPACE_TP_END   }

// log info
#define HAS_LOG
#ifdef HAS_LOG
#   define LOGINFO LogStream(__func__, __LINE__)
#   define LOGERROR LogStream(__func__, __LINE__, std::cerr)
#else
#   define LOGINFO
#   define LOGERROR
#endif

#endif //INCLUDE_TP_CONFIG_H
