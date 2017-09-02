/*
 * newlib_devopttab.h
 *
 *  Created on: 28.08.2017
 *      Author: Kamil Cukrowski
 */

#ifndef NEWLIB_DEVOPTAB_H_
#define NEWLIB_DEVOPTAB_H_

#include <sys/reent.h>
#include <sys/types.h>

// extern int _close_r _PARAMS ((struct _reent *, int));
// extern int _execve_r _PARAMS ((struct _reent *, const char *, char *const *, char *const *));
// extern int _fcntl_r _PARAMS ((struct _reent *, int, int, int));
// extern int _fork_r _PARAMS ((struct _reent *));
// extern int _fstat_r _PARAMS ((struct _reent *, int, struct stat *));
// extern int _getpid_r _PARAMS ((struct _reent *));
// extern int _isatty_r _PARAMS ((struct _reent *, int));
// extern int _kill_r _PARAMS ((struct _reent *, int, int));
// extern int _link_r _PARAMS ((struct _reent *, const char *, const char *));
// extern _off_t _lseek_r _PARAMS ((struct _reent *, int, _off_t, int));
// extern int _mkdir_r _PARAMS ((struct _reent *, const char *, int));
// extern int _open_r _PARAMS ((struct _reent *, const char *, int, int));
// extern _ssize_t _read_r _PARAMS ((struct _reent *, int, void *, size_t));
// extern int _rename_r _PARAMS ((struct _reent *, const char *, const char *));
// extern void *_sbrk_r _PARAMS ((struct _reent *, ptrdiff_t));
// extern int _stat_r _PARAMS ((struct _reent *, const char *, struct stat *));
// extern _CLOCK_T_ _times_r _PARAMS ((struct _reent *, struct tms *));
// extern int _unlink_r _PARAMS ((struct _reent *, const char *));
// extern int _wait_r _PARAMS ((struct _reent *, int *));
// extern _ssize_t _write_r _PARAMS ((struct _reent *, int, const void *, size_t));

typedef struct {
    const char *name;
    _ssize_t (*read_r  )(struct _reent *ptr, int fd, void *buf, size_t cnt);
    _ssize_t (*write_r )(struct _reent *ptr, int fd, const void *buf, size_t cnt);
    int  (*open_r  )(struct _reent *ptr, const char *path, int flags, int mode);
    int  (*close_r )(struct _reent *ptr, int fd);
    _off_t (*lseek_r )(struct _reent *ptr, int, _off_t, int);
    int (*fcntl_r)(struct _reent *ptr, int fd, int cmd, int arg);
} devoptab_t;

extern const devoptab_t devoptab_list[]; // defined by user application

#endif /* NEWLIB_DEVOPTAB_H_ */
