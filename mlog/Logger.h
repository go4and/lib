/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#else
#define MLOG_DECLARE_LOGGER(name) 
#endif

#if !MLOG_NO_LOGGING
MLOG_DECL const std::string & levelName(LogLevel level);
#endif

}
