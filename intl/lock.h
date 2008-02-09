/* Locking in multithreaded situations.
   Copyright (C) 2005-2007 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published
   by the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
   USA.  */

/* Written by Bruno Haible <bruno@clisp.org>, 2005.
   Based on GCC's gthr-posix.h, gthr-posix95.h, gthr-solaris.h,
   gthr-win32.h.  */

/* This file contains locking primitives for use with a given thread library.
   It does not contain primitives for creating threads or for other
   synchronization primitives.

   Normal (non-recursive) locks:
     Type:                gl_lock_t
     Declaration:         gl_lock_define(extern, name)
     Initializer:         gl_lock_define_initialized(, name)
     Initialization:      gl_lock_init (name);
     Taking the lock:     gl_lock_lock (name);
     Releasing the lock:  gl_lock_unlock (name);
     De-initialization:   gl_lock_destroy (name);

   Read-Write (non-recursive) locks:
     Type:                gl_rwlock_t
     Declaration:         gl_rwlock_define(extern, name)
     Initializer:         gl_rwlock_define_initialized(, name)
     Initialization:      gl_rwlock_init (name);
     Taking the lock:     gl_rwlock_rdlock (name);
                          gl_rwlock_wrlock (name);
     Releasing the lock:  gl_rwlock_unlock (name);
     De-initialization:   gl_rwlock_destroy (name);

   Recursive locks:
     Type:                gl_recursive_lock_t
     Declaration:         gl_recursive_lock_define(extern, name)
     Initializer:         gl_recursive_lock_define_initialized(, name)
     Initialization:      gl_recursive_lock_init (name);
     Taking the lock:     gl_recursive_lock_lock (name);
     Releasing the lock:  gl_recursive_lock_unlock (name);
     De-initialization:   gl_recursive_lock_destroy (name);

  Once-only execution:
     Type:                gl_once_t
     Initializer:         gl_once_define(extern, name)
     Execution:           gl_once (name, initfunction);
*/


#ifndef _LOCK_H
#define _LOCK_H

/* ========================================================================= */

#if USE_POSIX_THREADS

/* Use the POSIX threads library.  */

# include <pthread.h>
# include <stdlib.h>

# ifdef __cplusplus
extern "C" {
# endif

# if PTHREAD_IN_USE_DETECTION_HARD

/* The pthread_in_use() detection needs to be done at runtime.  */
#  define pthread_in_use() \
     glthread_in_use ()
extern int glthread_in_use (void);

# endif

# if USE_POSIX_THREADS_WEAK

/* Use weak references to the POSIX threads library.  */

/* Weak references avoid dragging in external libraries if the other parts
   of the program don't use them.  Here we use them, because we don't want
   every program that uses libintl to depend on libpthread.  This assumes
   that libpthread would not be loaded after libintl; i.e. if libintl is
   loaded first, by an executable that does not depend on libpthread, and
   then a module is dynamically loaded that depends on libpthread, libintl
   will not be multithread-safe.  */

/* The way to test at runtime whether libpthread is present is to test
   whether a function pointer's value, such as &pthread_mutex_init, is
   non-NULL.  However, some versions of GCC have a bug through which, in
   PIC mode, &foo != NULL always evaluates to true if there is a direct
   call to foo(...) in the same function.  To avoid this, we test the
   address of a function in libpthread that we don't use.  */

#  pragma weak pthread_mutex_init
#  pragma weak pthread_mutex_lock
#  pragma weak pthread_mutex_unlock
#  pragma weak pthread_mutex_destroy
#  pragma weak pthread_rwlock_init
#  pragma weak pthread_rwlock_rdlock
#  pragma weak pthread_rwlock_wrlock
#  pragma weak pthread_rwlock_unlock
#  pragma weak pthread_rwlock_destroy
#  pragma weak pthread_once
#  pragma weak pthread_cond_init
#  pragma weak pthread_cond_wait
#  pragma weak pthread_cond_signal
#  pragma weak pthread_cond_broadcast
#  pragma weak pthread_cond_destroy
#  pragma weak pthread_mutexattr_init
#  pragma weak pthread_mutexattr_settype
#  pragma weak pthread_mutexattr_destroy
#  ifndef pthread_self
#   pragma weak pthread_self
#  endif

#  if !PTHREAD_IN_USE_DETECTION_HARD
#   pragma weak pthread_cancel
#   define pthread_in_use() (pthread_cancel != NULL)
#  endif

# else

#  if !PTHREAD_IN_USE_DETECTION_HARD
#   define pthread_in_use() 1
#  endif

# endif

/* -------------------------- gl_lock_t datatype -------------------------- */

typedef pthread_mutex_t gl_lock_t;
# define gl_lock_define(STORAGECLASS, NAME) \
    STORAGECLASS pthread_mutex_t NAME;
# define gl_lock_define_initialized(STORAGECLASS, NAME) \
    STORAGECLASS pthread_mutex_t NAME = gl_lock_initializer;
# define gl_lock_initializer \
    PTHREAD_MUTEX_INITIALIZER
# define gl_lock_init(NAME) \
    do                                                                  \
      {                                                                 \
        if (pthread_in_use () && pthread_mutex_init (&NAME, NULL) != 0) \
          abort ();                                                     \
      }                                                                 \
    while (0)
# define gl_lock_lock(NAME) \
    do                                                            \
      {                                                           \
        if (pthread_in_use () && pthread_mutex_lock (&NAME) != 0) \
          abort ();                                               \
      }                                                           \
    while (0)
# define gl_lock_unlock(NAME) \
    do                                                              \
      {                                                             \
        if (pthread_in_use () && pthread_mutex_unlock (&NAME) != 0) \
          abort ();                                                 \
      }                                                             \
    while (0)
# define gl_lock_destroy(NAME) \
    do                                                               \
      {                                                              \
        if (pthread_in_use () && pthread_mutex_destroy (&NAME) != 0) \
          abort ();                                                  \
      }                                                              \
    while (0)

/* ------------------------- gl_rwlock_t datatype ------------------------- */

# if HAVE_PTHREAD_RWLOCK

#  ifdef PTHREAD_RWLOCK_INITIALIZER

typedef pthread_rwlock_t gl_rwlock_t;
#   define gl_rwlock_define(STORAGECLASS, NAME) \
      STORAGECLASS pthread_rwlock_t NAME;
#   define gl_rwlock_define_initialized(STORAGECLASS, NAME) \
      STORAGECLASS pthread_rwlock_t NAME = gl_rwlock_initializer;
#   define gl_rwlock_initializer \
      PTHREAD_RWLOCK_INITIALIZER
#   define gl_rwlock_init(NAME) \
      do                                                                   \
        {                                                                  \
          if (pthread_in_use () && pthread_rwlock_init (&NAME, NULL) != 0) \
            abort ();                                                      \
        }                                                                  \
      while (0)
#   define gl_rwlock_rdlock(NAME) \
      do                                                               \
        {                                                              \
          if (pthread_in_use () && pthread_rwlock_rdlock (&NAME) != 0) \
            abort ();                                                  \
        }                                                              \
      while (0)
#   define gl_rwlock_wrlock(NAME) \
      do                                                               \
        {                                                              \
          if (pthread_in_use () && pthread_rwlock_wrlock (&NAME) != 0) \
            abort ();                                                  \
        }                                                              \
      while (0)
#   define gl_rwlock_unlock(NAME) \
      do                                                               \
        {                                                              \
          if (pthread_in_use () && pthread_rwlock_unlock (&NAME) != 0) \
            abort ();                                                  \
        }                                                              \
      while (0)
#   define gl_rwlock_destroy(NAME) \
      do                                                                \
        {                                                               \
          if (pthread_in_use () && pthread_rwlock_destroy (&NAME) != 0) \
            abort ();                                                   \
        }                                                               \
      while (0)

#  else

typedef struct
        {
          int initialized;
          pthread_mutex_t guard;   /* protects the initialization */
          pthread_rwlock_t rwlock; /* read-write lock */
        }
        gl_rwlock_t;
#   define gl_rwlock_define(STORAGECLASS, NAME) \
      STORAGECLASS gl_rwlock_t NAME;
#   define gl_rwlock_define_initialized(STORAGECLASS, NAME) \
      STORAGECLASS gl_rwlock_t NAME = gl_rwlock_initializer;
#   define gl_rwlock_initializer \
      { 0, PTHREAD_MUTEX_INITIALIZER }
#   define gl_rwlock_init(NAME) \
      do                                  \
        {                                 \
          if (pthread_in_use ())          \
            glthread_rwlock_init (&NAME); \
        }                                 \
      while (0)
#   define gl_rwlock_rdlock(NAME) \
      do                                    \
        {                                   \
          if (pthread_in_use ())            \
            glthread_rwlock_rdlock (&NAME); \
        }                                   \
      while (0)
#   define gl_rwlock_wrlock(NAME) \
      do                                    \
        {                                   \
          if (pthread_in_use ())            \
            glthread_rwlock_wrlock (&NAME); \
        }                                   \
      while (0)
#   define gl_rwlock_unlock(NAME) \
      do                                    \
        {                                   \
          if (pthread_in_use ())            \
            glthread_rwlock_unlock (&NAME); \
        }                                   \
      while (0)
#   define gl_rwlock_destroy(NAME) \
      do                                     \
        {                                    \
          if (pthread_in_use ())             \
            glthread_rwlock_destroy (&NAME); \
        }                                    \
      while (0)
extern void glthread_rwlock_init (gl_rwlock_t *lock);
extern void glthread_rwlock_rdlock (gl_rwlock_t *lock);
extern void glthread_rwlock_wrlock (gl_rwlock_t *lock);
extern void glthread_rwlock_unlock (gl_rwlock_t *lock);
extern void glthread_rwlock_destroy (gl_rwlock_t *lock);

#  endif

# else

typedef struct
        {
          pthread_mutex_t lock; /* protects the remaining fields */
          pthread_cond_t waiting_readers; /* waiting readers */
          pthread_cond_t waiting_writers; /* waiting writers */
          unsigned int waiting_writers_count; /* number of waiting writers */
          int runcount; /* number of readers running, or -1 when a writer runs */
        }
        gl_rwlock_t;
# define gl_rwlock_define(STORAGECLASS, NAME) \
    STORAGECLASS gl_rwlock_t NAME;
# define gl_rwlock_define_initialized(STORAGECLASS, NAME) \
    STORAGECLASS gl_rwlock_t NAME = gl_rwlock_initializer;
# define gl_rwlock_initializer \
    { PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER, 0, 0 }
# define gl_rwlock_init(NAME) \
    do                                  \
      {                                 \
        if (pthread_in_use ())          \
          glthread_rwlock_init (&NAME); \
      }                                 \
    while (0)
# define gl_rwlock_rdlock(NAME) \
    do                                    \
      {                                   \
        if (pthread_in_use ())            \
          glthread_rwlock_rdlock (&NAME); \
      }                                   \
    while (0)
# define gl_rwlock_wrlock(NAME) \
    do                                    \
      {                                   \
        if (pthread_in_use ())            \
          glthread_rwlock_wrlock (&NAME); \
      }                                   \
    while (0)
# define gl_rwlock_unlock(NAME) \
    do                                    \
      {                                   \
        if (pthread_in_use ())            \
          glthread_rwlock_unlock (&NAME); \
      }                                   \
    while (0)
# define gl_rwlock_destroy(NAME) \
    do                                     \
      {                                    \
        if (pthread_in_use ())             \
          glthread_rwlock_destroy (&NAME); \
      }                                    \
    while (0)
extern void glthread_rwlock_init (gl_rwlock_t *lock);
extern void glthread_rwlock_rdlock (gl_rwlock_t *lock);
extern void glthread_rwlock_wrlock (gl_rwlock_t *lock);
extern void glthread_rwlock_unlock (gl_rwlock_t *lock);
extern void glthread_rwlock_destroy (gl_rwlock_t *lock);

# endif

/* --------------------- gl_recursive_lock_t datatype --------------------- */

# if HAVE_PTHREAD_MUTEX_RECURSIVE

#  if defined PTHREAD_RECURSIVE_MUTEX_INITIALIZER || defined PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP

typedef pthread_mutex_t gl_recursive_lock_t;
#   define gl_recursive_lock_define(STORAGECLASS, NAME) \
      STORAGECLASS pthread_mutex_t NAME;
#   define gl_recursive_lock_define_initialized(STORAGECLASS, NAME) \
      STORAGECLASS pthread_mutex_t NAME = gl_recursive_lock_initializer;
#   ifdef PTHREAD_RECURSIVE_MUTEX_INITIALIZER
#    define gl_recursive_lock_initializer \
       PTHREAD_RECURSIVE_MUTEX_INITIALIZER
#   else
#    define gl_recursive_lock_initializer \
       PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP
#   endif
#   define gl_recursive_lock_init(NAME) \
      do                                                                  \
        {                                                                 \
          if (pthread_in_use () && pthread_mutex_init (&NAME, NULL) != 0) \
            abort ();                                                     \
        }                                                                 \
      while (0)
#   define gl_recursive_lock_lock(NAME) \
      do                                                            \
        {                                                           \
          if (pthread_in_use () && pthread_mutex_lock (&NAME) != 0) \
            abort ();                                               \
        }                                                           \
      while (0)
#   define gl_recursive_lock_unlock(NAME) \
      do                                                              \
        {                                                             \
          if (pthread_in_use () && pthread_mutex_unlock (&NAME) != 0) \
            abort ();                                                 \
        }                                                             \
      while (0)
#   define gl_recursive_lock_destroy(NAME) \
      do                                                               \
        {                                                              \
          if (pthread_in_use () && pthread_mutex_destroy (&NAME) != 0) \
            abort ();                                                  \
        }                                                              \
      while (0)

#  else

typedef struct
        {
          pthread_mutex_t recmutex; /* recursive mutex */
          pthread_mutex_t guard;    /* protects the initialization */
          int initialized;
        }
        gl_recursive_lock_t;
#   define gl_recursive_lock_define(STORAGECLASS, NAME) \
      STORAGECLASS gl_recursive_lock_t NAME;
#   define gl_recursive_lock_define_initialized(STORAGECLASS, NAME) \
      STORAGECLASS gl_recursive_lock_t NAME = gl_recursive_lock_initializer;
#   define gl_recursive_lock_initializer \
      { PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER, 0 }
#   define gl_recursive_lock_init(NAME) \
      do                                          \
        {                                         \
          if (pthread_in_use ())                  \
            glthread_recursive_lock_init (&NAME); \
        }                                         \
      while (0)
#   define gl_recursive_lock_lock(NAME) \
      do                                          \
        {                                         \
          if (pthread_in_use ())                  \
            glthread_recursive_lock_lock (&NAME); \
        }                                         \
      while (0)
#   define gl_recursive_lock_unlock(NAME) \
      do                                            \
        {                                           \
          if (pthread_in_use ())                    \
            glthread_recursive_lock_unlock (&NAME); \
        }                                           \
      while (0)
#   define gl_recursive_lock_destroy(NAME) \
      do                                             \
        {                                            \
          if (pthread_in_use ())                     \
            glthread_recursive_lock_destroy (&NAME); \
        }                                            \
      while (0)
extern void glthread_recursive_lock_init (gl_recursive_lock_t *lock);
extern void glthread_recursive_lock_lock (gl_recursive_lock_t *lock);
extern void glthread_recursive_lock_unlock (gl_recursive_lock_t *lock);
extern void glthread_recursive_lock_destroy (gl_recursive_lock_t *lock);

#  endif

# else

/* Old versions of POSIX threads on Solaris did not have recursive locks.
   We have to implement them ourselves.  */

typedef struct
        {
          pthread_mutex_t mutex;
          pthread_t owner;
          unsigned long depth;
        }
        gl_recursive_lock_t;
#  define gl_recursive_lock_define(STORAGECLASS, NAME) \
     STORAGECLASS gl_recursive_lock_t NAME;
#  define gl_recursive_lock_define_initialized(STORAGECLASS, NAME) \
     STORAGECLASS gl_recursive_lock_t NAME = gl_recursive_lock_initializer;
#  define gl_recursive_lock_initializer \
     { PTHREAD_MUTEX_INITIALIZER, (pthread_t) 0, 0 }
#  define gl_recursive_lock_init(NAME) \
     do                                          \
       {                                         \
         if (pthread_in_use ())                  \
           glthread_recursive_lock_init (&NAME); \
       }                                         \
     while (0)
#  define gl_recursive_lock_lock(NAME) \
     do                                          \
       {                                         \
         if (pthread_in_use ())                  \
           glthread_recursive_lock_lock (&NAME); \
       }                                         \
     while (0)
#  define gl_recursive_lock_unlock(NAME) \
     do                                            \
       {                                           \
         if (pthread_in_use ())                    \
           glthread_recursive_lock_unlock (&NAME); \
       }                                           \
     while (0)
#  define gl_recursive_lock_destroy(NAME) \
     do                                             \
       {                                            \
         if (pthread_in_use ())                     \
           glthread_recursive_lock_destroy (&NAME); \
       }                                            \
     while (0)
extern void glthread_recursive_lock_init (gl_recursive_lock_t *lock);
extern void glthread_recursive_lock_lock (gl_recursive_lock_t *lock);
extern void glthread_recursive_lock_unlock (gl_recursive_lock_t *lock);
extern void glthread_recursive_lock_destroy (gl_recursive_lock_t *lock);

# endif

/* -------------------------- gl_once_t datatype -------------------------- */

typedef pthread_once_t gl_once_t;
# define gl_once_define(STORAGECLASS, NAME) \
    STORAGECLASS pthread_once_t NAME = PTHREAD_ONCE_INIT;
# define gl_once(NAME, INITFUNCTION) \
    do                                                   \
      {                                                  \
        if (pthread_in_use ())                           \
          {                                              \
            if (pthread_once (&NAME, INITFUNCTION) != 0) \
              abort ();                                  \
          }                                              \
        else                                             \
          {                                              \
            if (glthread_once_singlethreaded (&NAME))    \
              INITFUNCTION ();                           \
          }                                              \
      }                                                  \
    while (0)
extern int glthread_once_singlethreaded (pthread_once_t *once_control);

# ifdef __cplusplus
}
# endif

#endif

/* ========================================================================= */

#if USE_PTH_THREADS

/* Use the GNU Pth threads library.  */

# include <pth.h>
# include <stdlib.h>

# ifdef __cplusplus
extern "C" {
# endif

# if USE_PTH_THREADS_WEAK

/* Use weak references to the GNU Pth threads library.  */

#  pragma weak pth_mutex_init
#  pragma weak pth_mutex_acquire
#  pragma weak pth_mutex_release
#  pragma weak pth_rwlock_init
#  pragma weak pth_rwlock_acquire
#  pragma weak pth_rwlock_release
#  pragma weak pth_once

#  pragma weak pth_cancel
#  define pth_in_use() (pth_cancel != NULL)

# else

#  define pth_in_use() 1

# endif

/* -------------------------- gl_lock_t datatype -------------------------- */

typedef pth_mutex_t gl_lock_t;
# define gl_lock_define(STORAGECLASS, NAME) \
    STORAGECLASS pth_mutex_t NAME;
# define gl_lock_define_initialized(STORAGECLASS, NAME) \
    STORAGECLASS pth_mutex_t NAME = gl_lock_initializer;
# define gl_lock_initializer \
    PTH_MUTEX_INIT
# define gl_lock_init(NAME) \
    do                                               \
      {                                              \
        if (pth_in_use() && !pth_mutex_init (&NAME)) \
          abort ();                                  \
      }                                              \
    while (0)
# define gl_lock_lock(NAME) \
    do                                                           \
      {                                                          \
        if (pth_in_use() && !pth_mutex_acquire (&NAME, 0, NULL)) \
          abort ();                                              \
      }                                                          \
    while (0)
# define gl_lock_unlock(NAME) \
    do                                                  \
      {                                                 \
        if (pth_in_use() && !pth_mutex_release (&NAME)) \
          abort ();                                     \
      }                                                 \
    while (0)
# define gl_lock_destroy(NAME) \
    (void)(&NAME)

/* ------------------------- gl_rwlock_t datatype ------------------------- */

typedef pth_rwlock_t gl_rwlock_t;
#  define gl_rwlock_define(STORAGECLASS, NAME) \
     STORAGECLASS pth_rwlock_t NAME;
#  define gl_rwlock_define_initialized(STORAGECLASS, NAME) \
     STORAGECLASS pth_rwlock_t NAME = gl_rwlock_initializer;
#  define gl_rwlock_initializer \
     PTH_RWLOCK_INIT
#  define gl_rwlock_init(NAME) \
     do                                                \
       {                                               \
         if (pth_in_use() && !pth_rwlock_init (&NAME)) \
           abort ();                                   \
       }                                               \
     while (0)
#  define gl_rwlock_rdlock(NAME) \
     do                                                              \
       {                                                             \
         if (pth_in_use()                                            \
             && !pth_rwlock_acquire (&NAME, PTH_RWLOCK_RD, 0, NULL)) \
           abort ();                                                 \
       }                                                             \
     while (0)
#  define gl_rwlock_wrlock(NAME) \
     do                                                              \
       {                                                             \
         if (pth_in_use()                                            \
             && !pth_rwlock_acquire (&NAME, PTH_RWLOCK_RW, 0, NULL)) \
           abort ();                                                 \
       }                                                             \
     while (0)
#  define gl_rwlock_unlock(NAME) \
     do                                                   \
       {                                                  \
         if (pth_in_use() && !pth_rwlock_release (&NAME)) \
           abort ();                                      \
       }                                                  \
     while (0)
#  define gl_rwlock_destroy(NAME) \
     (void)(&NAME)

/* --------------------- gl_recursive_lock_t datatype --------------------- */

/* In Pth, mutexes are recursive by default.  */
typedef pth_mutex_t gl_recursive_lock_t;
#  define gl_recursive_lock_define(STORAGECLASS, NAME) \
     STORAGECLASS pth_mutex_t NAME;
#  define gl_recursive_lock_define_initialized(STORAGECLASS, NAME) \
     STORAGECLASS pth_mutex_t NAME = gl_recursive_lock_initializer;
#  define gl_recursive_lock_initializer \
     PTH_MUTEX_INIT
#  define gl_recursive_lock_init(NAME) \
     do                                               \
       {                                              \
         if (pth_in_use() && !pth_mutex_init (&NAME)) \
           abort ();                                  \
       }                                              \
     while (0)
#  define gl_recursive_lock_lock(NAME) \
     do                                                           \
       {                                                          \
         if (pth_in_use() && !pth_mutex_acquire (&NAME, 0, NULL)) \
           abort ();                                              \
       }                                                          \
     while (0)
#  define gl_recursive_lock_unlock(NAME) \
     do                                                  \
       {                                                 \
         if (pth_in_use() && !pth_mutex_release (&NAME)) \
           abort ();                                     \
       }                                                 \
     while (0)
#  define gl_recursive_lock_destroy(NAME) \
     (void)(&NAME)

/* -------------------------- gl_once_t datatype -------------------------- */

typedef pth_once_t gl_once_t;
# define gl_once_define(STORAGECLASS, NAME) \
    STORAGECLASS pth_once_t NAME = PTH_ONCE_INIT;
# define gl_once(NAME, INITFUNCTION) \
    do                                                                \
      {                                                               \
        if (pth_in_use ())                                            \
          {                                                           \
            void (*gl_once_temp) (void) = INITFUNCTION;               \
            if (!pth_once (&NAME, glthread_once_call, &gl_once_temp)) \
              abort ();                                               \
          }                                                           \
        else                                                          \
          {                                                           \
            if (glthread_once_singlethreaded (&NAME))                 \
              INITFUNCTION ();                                        \
          }                                                           \
      }                                                               \
    while (0)
extern void glthread_once_call (void *arg);
extern int glthread_once_singlethreaded (pth_once_t *once_control);

# ifdef __cplusplus
}
# endif

#endif

/* ========================================================================= */

#if USE_SOLARIS_THREADS

/* Use the old Solaris threads library.  */

# include <thread.h>
# include <synch.h>
# include <stdlib.h>

# ifdef __cplusplus
extern "C" {
# endif

# if USE_SOLARIS_THREADS_WEAK

/* Use weak references to the old Solaris threads library.  */

#  pragma weak mutex_init
#  pragma weak mutex_lock
#  pragma weak mutex_unlock
#  pragma weak mutex_destroy
#  pragma weak rwlock_init
#  pragma weak rw_rdlock
#  pragma weak rw_wrlock
#  pragma weak rw_unlock
#  pragma weak rwlock_destroy
#  pragma weak thr_self

#  pragma weak thr_suspend
#  define thread_in_use() (thr_suspend != NULL)

# else

#  define thread_in_use() 1

# endif

/* -------------------------- gl_lock_t datatype -------------------------- */

typedef mutex_t gl_lock_t;
# define gl_lock_define(STORAGECLASS, NAME) \
    STORAGECLASS mutex_t NAME;
# define gl_lock_define_initialized(STORAGECLASS, NAME) \
    STORAGECLASS mutex_t NAME = gl_lock_initializer;
# define gl_lock_initializer \
    DEFAULTMUTEX
# define gl_lock_init(NAME) \
    do                                                                       \
      {                                                                      \
        if (thread_in_use () && mutex_init (&NAME, USYNC_THREAD, NULL) != 0) \
          abort ();                                                          \
      }                                                                      \
    while (0)
# define gl_lock_lock(NAME) \
    do                                                   \
      {                                                  \
        if (thread_in_use () && mutex_lock (&NAME) != 0) \
          abort ();                                      \
      }                                                  \
    while (0)
# define gl_lock_unlock(NAME) \
    do                                                     \
      {                                                    \
        if (thread_in_use () && mutex_unlock (&NAME) != 0) \
          abort ();                                        \
      }                                                    \
    while (0)
# define gl_lock_destroy(NAME) \
    do                                                      \
      {                                                     \
        if (thread_in_use () && mutex_destroy (&NAME) != 0) \
          abort ();                                         \
      }                                                     \
    while (0)

/* ------------------------- gl_rwlock_t datatype ------------------------- */

typedef rwlock_t gl_rwlock_t;
# define gl_rwlock_define(STORAGECLASS, NAME) \
    STORAGECLASS rwlock_t NAME;
# define gl_rwlock_define_initialized(STORAGECLASS, NAME) \
    STORAGECLASS rwlock_t NAME = gl_rwlock_initializer;
# define gl_rwlock_initializer \
    DEFAULTRWLOCK
# define gl_rwlock_init(NAME) \
    do                                                                        \
      {                                                                       \
        if (thread_in_use () && rwlock_init (&NAME, USYNC_THREAD, NULL) != 0) \
          abort ();                                                           \
      }                                                                       \
    while (0)
# define gl_rwlock_rdlock(NAME) \
    do                                                  \
      {                                                 \
        if (thread_in_use () && rw_rdlock (&NAME) != 0) \
          abort ();                                     \
      }                                                 \
    while (0)
# define gl_rwlock_wrlock(NAME) \
    do                                                  \
      {                                                 \
        if (thread_in_use () && rw_wrlock (&NAME) != 0) \
          abort ();                                     \
      }                                                 \
    while (0)
# define gl_rwlock_unlock(NAME) \
    do                                                  \
      {                                                 \
        if (thread_in_use () && rw_unlock (&NAME) != 0) \
          abort ();                                     \
      }                                                 \
    while (0)
# define gl_rwlock_destroy(NAME) \
    do                                                       \
      {                                                      \
        if (thread_in_use () && rwlock_destroy (&NAME) != 0) \
          abort ();                                          \
      }                                                      \
    while (0)

/* --------------------- gl_recursive_lock_t datatype --------------------- */

/* Old Solaris threads did not have recursive locks.
   We have to implement them ourselves.  */

typedef struct
        {
          mutex_t mutex;
          thread_t owner;
          unsigned long depth;
        }
        gl_recursive_lock_t;
# define gl_recursive_lock_define(STORAGECLASS, NAME) \
    STORAGECLASS gl_recursive_lock_t NAME;
# define gl_recursive_lock_define_initialized(STORAGECLASS, NAME) \
    STORAGECLASS gl_recursive_lock_t NAME = gl_recursive_lock_initializer;
# define gl_recursive_lock_initializer \
    { DEFAULTMUTEX, (thread_t) 0, 0 }
# define gl_recursive_lock_init(NAME) \
    do                                          \
      {                                         \
        if (thread_in_use ())                   \
          glthread_recursive_lock_init (&NAME); \
      }                                         \
    while (0)
# define gl_recursive_lock_lock(NAME) \
    do                                          \
      {                                         \
        if (thread_in_use ())                   \
          glthread_recursive_lock_lock (&NAME); \
      }                                         \
    while (0)
# define gl_recursive_lock_unlock(NAME) \
    do                                            \
      {                                           \
        if (thread_in_use ())                     \
          glthread_recursive_lock_unlock (&NAME); \
      }                                           \
    while (0)
# define gl_recursive_lock_destroy(NAME) \
    do                                             \
      {                                            \
        if (thread_in_use ())                      \
          glthread_recursive_lock_destroy (&NAME); \
      }                                            \
    while (0)
extern void glthread_recursive_lock_init (gl_recursive_lock_t *lock);
extern void glthread_recursive_lock_lock (gl_recursive_lock_t *lock);
extern void glthread_recursive_lock_unlock (gl_recursive_lock_t *lock);
extern void glthread_recursive_lock_destroy (gl_recursive_lock_t *lock);

/* -------------------------- gl_once_t datatype -------------------------- */

typedef struct
        {
          volatile int inited;
          mutex_t mutex;
        }
        gl_once_t;
# define gl_once_define(STORAGECLASS, NAME) \
    STORAGECLASS gl_once_t NAME = { 0, DEFAULTMUTEX };
# define gl_once(NAME, INITFUNCTION) \
    do                                                \
      {                                               \
        if (thread_in_use ())                         \
          {                                           \
            glthread_once (&NAME, INITFUNCTION);      \
          }                                           \
        else                                          \
          {                                           \
            if (glthread_once_singlethreaded (&NAME)) \
              INITFUNCTION ();                        \
          }                                           \
      }                                               \
    while (0)
extern void glthread_once (gl_once_t *once_control, void (*initfunction) (void));
extern int glthread_once_singlethreaded (gl_once_t *once_control);

# ifdef __cplusplus
}
# endif

#endif

/* ========================================================================= */

#if USE_WIN32_THREADS

# include <windows.h>

# ifdef __cplusplus
extern "C" {
# endif

/* We can use CRITICAL_SECTION directly, rather than the Win32 Event, Mutex,
   Semaphore types, because
     - we need only to synchronize inside a single process (address space),
       not inter-process locking,
     - we don't need to support trylock operations.  (TryEnterCriticalSection
       does not work on Windows 95/98/ME.  Packages that need trylock usually
       define their own mutex type.)  */

/* There is no way to statically initialize a CRITICAL_SECTION.  It needs
   to be done lazily, once only.  For this we need spinlocks.  */

typedef struct { volatile int done; volatile long started; } gl_spinlock_t;

/* -------------------------- gl_lock_t datatype -------------------------- */

typedef struct
        {
          gl_spinlock_t guard; /* protects the initialization */
          CRITICAL_SECTION lock;
        }
        gl_lock_t;
# define gl_lock_define(STORAGECLASS, NAME) \
    STORAGECLASS gl_lock_t NAME;
# define gl_lock_define_initialized(STORAGECLASS, NAME) \
    STORAGECLASS gl_lock_t NAME = gl_lock_initializer;
# define gl_lock_initializer \
    { { 0, -1 } }
# define gl_lock_init(NAME) \
    glthread_lock_init (&NAME)
# define gl_lock_lock(NAME) \
    glthread_lock_lock (&NAME)
# define gl_lock_unlock(NAME) \
    glthread_lock_unlock (&NAME)
# define gl_lock_destroy(NAME) \
    glthread_lock_destroy (&NAME)
extern void glthread_lock_init (gl_lock_t *lock);
extern void glthread_lock_lock (gl_lock_t *lock);
extern void glthread_lock_unlock (gl_lock_t *lock);
extern void glthread_lock_destroy (gl_lock_t *lock);

/* ------------------------- gl_rwlock_t datatype ------------------------- */

/* It is impossible to implement read-write locks using plain locks, without
   introducing an extra thread dedicated to managing read-write locks.
   Therefore here we need to use the low-level Event type.  */

typedef struct
        {
          HANDLE *array; /* array of waiting threads, each represented by an event */
          unsigned int count; /* number of waiting threads */
          unsigned int alloc; /* length of allocated array */
          unsigned int offset; /* index of first waiting thread in array */
        }
        gl_waitqueue_t;
typedef struct
        {
          gl_spinlock_t guard; /* protects the initialization */
          CRITICAL_SECTION lock; /* protects the remaining fields */
          gl_waitqueue_t waiting_readers; /* waiting readers */
          gl_waitqueue_t waiting_writers; /* waiting writers */
          int runcount; /* number of readers running, or -1 when a writer runs */
        }
        gl_rwlock_t;
# define gl_rwlock_define(STORAGECLASS, NAME) \
    STORAGECLASS gl_rwlock_t NAME;
# define gl_rwlock_define_initialized(STORAGECLASS, NAME) \
    STORAGECLASS gl_rwlock_t NAME = gl_rwlock_initializer;
# define gl_rwlock_initializer \
    { { 0, -1 } }
# define gl_rwlock_init(NAME) \
    glthread_rwlock_init (&NAME)
# define gl_rwlock_rdlock(NAME) \
    glthread_rwlock_rdlock (&NAME)
# define gl_rwlock_wrlock(NAME) \
    glthread_rwlock_wrlock (&NAME)
# define gl_rwlock_unlock(NAME) \
    glthread_rwlock_unlock (&NAME)
# define gl_rwlock_destroy(NAME) \
    glthread_rwlock_destroy (&NAME)
extern void glthread_rwlock_init (gl_rwlock_t *lock);
extern void glthread_rwlock_rdlock (gl_rwlock_t *lock);
extern void glthread_rwlock_wrlock (gl_rwlock_t *lock);
extern void glthread_rwlock_unlock (gl_rwlock_t *lock);
extern void glthread_rwlock_destroy (gl_rwlock_t *lock);

/* --------------------- gl_recursive_lock_t datatype --------------------- */

/* The Win32 documentation says that CRITICAL_SECTION already implements a
   recursive lock.  But we need not rely on it: It's easy to implement a
   recursive lock without this assumption.  */

typedef struct
        {
          gl_spinlock_t guard; /* protects the initialization */
          DWORD owner;
          unsigned long depth;
          CRITICAL_SECTION lock;
        }
        gl_recursive_lock_t;
# define gl_recursive_lock_define(STORAGECLASS, NAME) \
    STORAGECLASS gl_recursive_lock_t NAME;
# define gl_recursive_lock_define_initialized(STORAGECLASS, NAME) \
    STORAGECLASS gl_recursive_lock_t NAME = gl_recursive_lock_initializer;
# define gl_recursive_lock_initializer \
    { { 0, -1 }, 0, 0 }
# define gl_recursive_lock_init(NAME) \
    glthread_recursive_lock_init (&NAME)
# define gl_recursive_lock_lock(NAME) \
    glthread_recursive_lock_lock (&NAME)
# define gl_recursive_lock_unlock(NAME) \
    glthread_recursive_lock_unlock (&NAME)
# define gl_recursive_lock_destroy(NAME) \
    glthread_recursive_lock_destroy (&NAME)
extern void glthread_recursive_lock_init (gl_recursive_lock_t *lock);
extern void glthread_recursive_lock_lock (gl_recursive_lock_t *lock);
extern void glthread_recursive_lock_unlock (gl_recursive_lock_t *lock);
extern void glthread_recursive_lock_destroy (gl_recursive_lock_t *lock);

/* -------------------------- gl_once_t datatype -------------------------- */

typedef struct
        {
          volatile int inited;
          volatile long started;
          CRITICAL_SECTION lock;
        }
        gl_once_t;
# define gl_once_define(STORAGECLASS, NAME) \
    STORAGECLASS gl_once_t NAME = { -1, -1 };
# define gl_once(NAME, INITFUNCTION) \
    glthread_once (&NAME, INITFUNCTION)
extern void glthread_once (gl_once_t *once_control, void (*initfunction) (void));

# ifdef __cplusplus
}
# endif

#endif

/* ========================================================================= */

#if !(USE_POSIX_THREADS || USE_PTH_THREADS || USE_SOLARIS_THREADS || USE_WIN32_THREADS)

/* Provide dummy implementation if threads are not supported.  */

/* -------------------------- gl_lock_t datatype -------------------------- */

typedef int gl_lock_t;
# define gl_lock_define(STORAGECLASS, NAME)
# define gl_lock_define_initialized(STORAGECLASS, NAME)
# define gl_lock_init(NAME)
# define gl_lock_lock(NAME)
# define gl_lock_unlock(NAME)

/* ------------------------- gl_rwlock_t datatype ------------------------- */

typedef int gl_rwlock_t;
# define gl_rwlock_define(STORAGECLASS, NAME)
# define gl_rwlock_define_initialized(STORAGECLASS, NAME)
# define gl_rwlock_init(NAME)
# define gl_rwlock_rdlock(NAME)
# define gl_rwlock_wrlock(NAME)
# define gl_rwlock_unlock(NAME)

/* --------------------- gl_recursive_lock_t datatype --------------------- */

typedef int gl_recursive_lock_t;
# define gl_recursive_lock_define(STORAGECLASS, NAME)
# define gl_recursive_lock_define_initialized(STORAGECLASS, NAME)
# define gl_recursive_lock_init(NAME)
# define gl_recursive_lock_lock(NAME)
# define gl_recursive_lock_unlock(NAME)

/* -------------------------- gl_once_t datatype -------------------------- */

typedef int gl_once_t;
# define gl_once_define(STORAGECLASS, NAME) \
    STORAGECLASS gl_once_t NAME = 0;
# define gl_once(NAME, INITFUNCTION) \
    do                       \
      {                      \
        if (NAME == 0)       \
          {                  \
            NAME = ~ 0;      \
            INITFUNCTION (); \
          }                  \
      }                      \
    while (0)

#endif

/* ========================================================================= */

#endif /* _LOCK_H */
