/* 
 * Dynamic sync control driver definitions (V2)
 * 
 * by andip71 (alias Lord Boeffla)
 * 
 * sdm845 changes by pappschlumpf (Erik Müller)
 */

#ifdef DYNAMIC_FSYNC_ENABLED
#define DYN_FSYNC_ACTIVE_DEFAULT true
#else
#define DYN_FSYNC_ACTIVE_DEFAULT false
#endif
#define DYN_FSYNC_VERSION_MAJOR 2
#define DYN_FSYNC_VERSION_MINOR 2

extern bool suspend_active;
extern bool dyn_fsync_active;
