/* Locking in multithreaded situations.
   Copyright (C) 2005-2006 Free Software Foundation, Inc.

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

#include <config.h>

#include "lock.h"

/* ========================================================================= */

#if USE_POSIX_THREADS

/* Use the POSIX threads library.  */

# if PTHREAD_IN_USE_DETECTION_HARD

/* The function to be executed by a dummy thread.  */
static void *
dummy_thread_func (void *arg)
{
  return arg;
}

int
glthread_in_use (void)
{
  static int tested;
  static int result; /* 1: linked with -lpthread, 0: only with libc */

  if (!tested)
    {
      pthread_t thread;

      if (pthread_create (&thread, NULL, dummy_thread_func, NULL) != 0)
	/* Thread creation failed.  */
	result = 0;
      else
	{
	  /* Thread creation works.  */
	  void *retval;
	  if (pthread_join (thread, &retval) != 0)
	    abort ();
	  result = 1;
	}
      tested = 1;
    }
  return result;
}

# endif

/* -------------------------- gl_lock_t datatype -------------------------- */

/* ------------------------- gl_rwlock_t datatype ------------------------- */

# if HAVE_PTHREAD_RWLOCK

#  if !defined PTHREAD_RWLOCK_INITIALIZER

void
glthread_rwlock_init (gl_rwlock_t *lock)
{
  if (pthread_rwlock_init (&lock->rwlock, NULL) != 0)
    abort ();
  lock->initialized = 1;
}

void
glthread_rwlock_rdlock (gl_rwlock_t *lock)
{
  if (!lock->initialized)
    {
      if (pthread_mutex_lock (&lock->guard) != 0)
	abort ();
      if (!lock->initialized)
	glthread_rwlock_init (lock);
      if (pthread_mutex_unlock (&lock->guard) != 0)
	abort ();
    }
  if (pthread_rwlock_rdlock (&lock->rwlock) != 0)
    abort ();
}

void
glthread_rwlock_wrlock (gl_rwlock_t *lock)
{
  if (!lock->initialized)
    {
      if (pthread_mutex_lock (&lock->guard) != 0)
	abort ();
      if (!lock->initialized)
	glthread_rwlock_init (lock);
      if (pthread_mutex_unlock (&lock->guard) != 0)
	abort ();
    }
  if (pthread_rwlock_wrlock (&lock->rwlock) != 0)
    abort ();
}

void
glthread_rwlock_unlock (gl_rwlock_t *lock)
{
  if (!lock->initialized)
    abort ();
  if (pthread_rwlock_unlock (&lock->rwlock) != 0)
    abort ();
}

void
glthread_rwlock_destroy (gl_rwlock_t *lock)
{
  if (!lock->initialized)
    abort ();
  if (pthread_rwlock_destroy (&lock->rwlock) != 0)
    abort ();
  lock->initialized = 0;
}

#  endif

# else

void
glthread_rwlock_init (gl_rwlock_t *lock)
{
  if (pthread_mutex_init (&lock->lock, NULL) != 0)
    abort ();
  if (pthread_cond_init (&lock->waiting_readers, NULL) != 0)
    abort ();
  if (pthread_cond_init (&lock->waiting_writers, NULL) != 0)
    abort ();
  lock->waiting_writers_count = 0;
  lock->runcount = 0;
}

void
glthread_rwlock_rdlock (gl_rwlock_t *lock)
{
  if (pthread_mutex_lock (&lock->lock) != 0)
    abort ();
  /* Test whether only readers are currently running, and whether the runcount
     field will not overflow.  */
  /* POSIX says: "It is implementation-defined whether the calling thread
     acquires the lock when a writer does not hold the lock and there are
     writers blocked on the lock."  Let's say, no: give the writers a higher
     priority.  */
  while (!(lock->runcount + 1 > 0 && lock->waiting_writers_count == 0))
    {
      /* This thread has to wait for a while.  Enqueue it among the
	 waiting_readers.  */
      if (pthread_cond_wait (&lock->waiting_readers, &lock->lock) != 0)
	abort ();
    }
  lock->runcount++;
  if (pthread_mutex_unlock (&lock->lock) != 0)
    abort ();
}

void
glthread_rwlock_wrlock (gl_rwlock_t *lock)
{
  if (pthread_mutex_lock (&lock->lock) != 0)
    abort ();
  /* Test whether no readers or writers are currently running.  */
  while (!(lock->runcount == 0))
    {
      /* This thread has to wait for a while.  Enqueue it among the
	 waiting_writers.  */
      lock->waiting_writers_count++;
      if (pthread_cond_wait (&lock->waiting_writers, &lock->lock) != 0)
	abort ();
      lock->waiting_writers_count--;
    }
  lock->runcount--; /* runcount becomes -1 */
  if (pthread_mutex_unlock (&lock->lock) != 0)
    abort ();
}

void
glthread_rwlock_unlock (gl_rwlock_t *lock)
{
  if (pthread_mutex_lock (&lock->lock) != 0)
    abort ();
  if (lock->runcount < 0)
    {
      /* Drop a writer lock.  */
      if (!(lock->runcount == -1))
	abort ();
      lock->runcount = 0;
    }
  else
    {
      /* Drop a reader lock.  */
      if (!(lock->runcount > 0))
	abort ();
      lock->runcount--;
    }
  if (lock->runcount == 0)
    {
      /* POSIX recommends that "write locks shall take precedence over read
	 locks", to avoid "writer starvation".  */
      if (lock->waiting_writers_count > 0)
	{
	  /* Wake up one of the waiting writers.  */
	  if (pthread_cond_signal (&lock->waiting_writers) != 0)
	    abort ();
	}
      else
	{
	  /* Wake up all waiting readers.  */
	  if (pthread_cond_broadcast (&lock->waiting_readers) != 0)
	    abort ();
	}
    }
  if (pthread_mutex_unlock (&lock->lock) != 0)
    abort ();
}

void
glthread_rwlock_destroy (gl_rwlock_t *lock)
{
  if (pthread_mutex_destroy (&lock->lock) != 0)
    abort ();
  if (pthread_cond_destroy (&lock->waiting_readers) != 0)
    abort ();
  if (pthread_cond_destroy (&lock->waiting_writers) != 0)
    abort ();
}

# endif

/* --------------------- gl_recursive_lock_t datatype --------------------- */

# if HAVE_PTHREAD_MUTEX_RECURSIVE

#  if !(defined PTHREAD_RECURSIVE_MUTEX_INITIALIZER || defined PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP)

void
glthread_recursive_lock_init (gl_recursive_lock_t *lock)
{
  pthread_mutexattr_t attributes;

  if (pthread_mutexattr_init (&attributes) != 0)
    abort ();
  if (pthread_mutexattr_settype (&attributes, PTHREAD_MUTEX_RECURSIVE) != 0)
    abort ();
  if (pthread_mutex_init (&lock->recmutex, &attributes) != 0)
    abort ();
  if (pthread_mutexattr_destroy (&attributes) != 0)
    abort ();
  lock->initialized = 1;
}

void
glthread_recursive_lock_lock (gl_recursive_lock_t *lock)
{
  if (!lock->initialized)
    {
      if (pthread_mutex_lock (&lock->guard) != 0)
	abort ();
      if (!lock->initialized)
	glthread_recursive_lock_init (lock);
      if (pthread_mutex_unlock (&lock->guard) != 0)
	abort ();
    }
  if (pthread_mutex_lock (&lock->recmutex) != 0)
    abort ();
}

void
glthread_recursive_lock_unlock (gl_recursive_lock_t *lock)
{
  if (!lock->initialized)
    abort ();
  if (pthread_mutex_unlock (&lock->recmutex) != 0)
    abort ();
}

void
glthread_recursive_lock_destroy (gl_recursive_lock_t *lock)
{
  if (!lock->initialized)
    abort ();
  if (pthread_mutex_destroy (&lock->recmutex) != 0)
    abort ();
  lock->initialized = 0;
}

#  endif

# else

void
glthread_recursive_lock_init (gl_recursive_lock_t *lock)
{
  if (pthread_mutex_init (&lock->mutex, NULL) != 0)
    abort ();
  lock->owner = (pthread_t) 0;
  lock->depth = 0;
}

void
glthread_recursive_lock_lock (gl_recursive_lock_t *lock)
{
  pthread_t self = pthread_self ();
  if (lock->owner != self)
    {
      if (pthread_mutex_lock (&lock->mutex) != 0)
	abort ();
      lock->owner = self;
    }
  if (++(lock->depth) == 0) /* wraparound? */
    abort ();
}

void
glthread_recursive_lock_unlock (gl_recursive_lock_t *lock)
{
  if (lock->owner != pthread_self ())
    abort ();
  if (lock->depth == 0)
    abort ();
  if (--(lock->depth) == 0)
    {
      lock->owner = (pthread_t) 0;
      if (pthread_mutex_unlock (&lock->mutex) != 0)
	abort ();
    }
}

void
glthread_recursive_lock_destroy (gl_recursive_lock_t *lock)
{
  if (lock->owner != (pthread_t) 0)
    abort ();
  if (pthread_mutex_destroy (&lock->mutex) != 0)
    abort ();
}

# endif

/* -------------------------- gl_once_t datatype -------------------------- */

static const pthread_once_t fresh_once = PTHREAD_ONCE_INIT;

int
glthread_once_singlethreaded (pthread_once_t *once_control)
{
  /* We don't know whether pthread_once_t is an integer type, a floating-point
     type, a pointer type, or a structure type.  */
  char *firstbyte = (char *)once_control;
  if (*firstbyte == *(const char *)&fresh_once)
    {
      /* First time use of once_control.  Invert the first byte.  */
      *firstbyte = ~ *(const char *)&fresh_once;
      return 1;
    }
  else
    return 0;
}

#endif

/* ========================================================================= */

#if USE_PTH_THREADS

/* Use the GNU Pth threads library.  */

/* -------------------------- gl_lock_t datatype -------------------------- */

/* ------------------------- gl_rwlock_t datatype ------------------------- */

/* --------------------- gl_recursive_lock_t datatype --------------------- */

/* -------------------------- gl_once_t datatype -------------------------- */

void
glthread_once_call (void *arg)
{
  void (**gl_once_temp_addr) (void) = (void (**) (void)) arg;
  void (*initfunction) (void) = *gl_once_temp_addr;
  initfunction ();
}

int
glthread_once_singlethreaded (pth_once_t *once_control)
{
  /* We know that pth_once_t is an integer type.  */
  if (*once_control == PTH_ONCE_INIT)
    {
      /* First time use of once_control.  Invert the marker.  */
      *once_control = ~ PTH_ONCE_INIT;
      return 1;
    }
  else
    return 0;
}

#endif

/* ========================================================================= */

#if USE_SOLARIS_THREADS

/* Use the old Solaris threads library.  */

/* -------------------------- gl_lock_t datatype -------------------------- */

/* ------------------------- gl_rwlock_t datatype ------------------------- */

/* --------------------- gl_recursive_lock_t datatype --------------------- */

void
glthread_recursive_lock_init (gl_recursive_lock_t *lock)
{
  if (mutex_init (&lock->mutex, USYNC_THREAD, NULL) != 0)
    abort ();
  lock->owner = (thread_t) 0;
  lock->depth = 0;
}

void
glthread_recursive_lock_lock (gl_recursive_lock_t *lock)
{
  thread_t self = thr_self ();
  if (lock->owner != self)
    {
      if (mutex_lock (&lock->mutex) != 0)
	abort ();
      lock->owner = self;
    }
  if (++(lock->depth) == 0) /* wraparound? */
    abort ();
}

void
glthread_recursive_lock_unlock (gl_recursive_lock_t *lock)
{
  if (lock->owner != thr_self ())
    abort ();
  if (lock->depth == 0)
    abort ();
  if (--(lock->depth) == 0)
    {
      lock->owner = (thread_t) 0;
      if (mutex_unlock (&lock->mutex) != 0)
	abort ();
    }
}

void
glthread_recursive_lock_destroy (gl_recursive_lock_t *lock)
{
  if (lock->owner != (thread_t) 0)
    abort ();
  if (mutex_destroy (&lock->mutex) != 0)
    abort ();
}

/* -------------------------- gl_once_t datatype -------------------------- */

void
glthread_once (gl_once_t *once_control, void (*initfunction) (void))
{
  if (!once_control->inited)
    {
      /* Use the mutex to guarantee that if another thread is already calling
	 the initfunction, this thread waits until it's finished.  */
      if (mutex_lock (&once_control->mutex) != 0)
	abort ();
      if (!once_control->inited)
	{
	  once_control->inited = 1;
	  initfunction ();
	}
      if (mutex_unlock (&once_control->mutex) != 0)
	abort ();
    }
}

int
glthread_once_singlethreaded (gl_once_t *once_control)
{
  /* We know that gl_once_t contains an integer type.  */
  if (!once_control->inited)
    {
      /* First time use of once_control.  Invert the marker.  */
      once_control->inited = ~ 0;
      return 1;
    }
  else
    return 0;
}

#endif

/* ========================================================================= */

#if USE_WIN32_THREADS

/* -------------------------- gl_lock_t datatype -------------------------- */

void
glthread_lock_init (gl_lock_t *lock)
{
  InitializeCriticalSection (&lock->lock);
  lock->guard.done = 1;
}

void
glthread_lock_lock (gl_lock_t *lock)
{
  if (!lock->guard.done)
    {
      if (InterlockedIncrement (&lock->guard.started) == 0)
	/* This thread is the first one to need this lock.  Initialize it.  */
	glthread_lock_init (lock);
      else
	/* Yield the CPU while waiting for another thread to finish
	   initializing this lock.  */
	while (!lock->guard.done)
	  Sleep (0);
    }
  EnterCriticalSection (&lock->lock);
}

void
glthread_lock_unlock (gl_lock_t *lock)
{
  if (!lock->guard.done)
    abort ();
  LeaveCriticalSection (&lock->lock);
}

void
glthread_lock_destroy (gl_lock_t *lock)
{
  if (!lock->guard.done)
    abort ();
  DeleteCriticalSection (&lock->lock);
  lock->guard.done = 0;
}

/* ------------------------- gl_rwlock_t datatype ------------------------- */

static inline void
gl_waitqueue_init (gl_waitqueue_t *wq)
{
  wq->array = NULL;
  wq->count = 0;
  wq->alloc = 0;
  wq->offset = 0;
}

/* Enqueues the current thread, represented by an event, in a wait queue.
   Returns INVALID_HANDLE_VALUE if an allocation failure occurs.  */
static HANDLE
gl_waitqueue_add (gl_waitqueue_t *wq)
{
  HANDLE event;
  unsigned int index;

  if (wq->count == wq->alloc)
    {
      unsigned int new_alloc = 2 * wq->alloc + 1;
      HANDLE *new_array =
	(HANDLE *) realloc (wq->array, new_alloc * sizeof (HANDLE));
      if (new_array == NULL)
	/* No more memory.  */
	return INVALID_HANDLE_VALUE;
      /* Now is a good opportunity to rotate the array so that its contents
	 starts at offset 0.  */
      if (wq->offset > 0)
	{
	  unsigned int old_count = wq->count;
	  unsigned int old_alloc = wq->alloc;
	  unsigned int old_offset = wq->offset;
	  unsigned int i;
	  if (old_offset + old_count > old_alloc)
	    {
	      unsigned int limit = old_offset + old_count - old_alloc;
	      for (i = 0; i < limit; i++)
		new_array[old_alloc + i] = new_array[i];
	    }
	  for (i = 0; i < old_count; i++)
	    new_array[i] = new_array[old_offset + i];
	  wq->offset = 0;
	}
      wq->array = new_array;
      wq->alloc = new_alloc;
    }
  event = CreateEvent (NULL, TRUE, FALSE, NULL);
  if (event == INVALID_HANDLE_VALUE)
    /* No way to allocate an event.  */
    return INVALID_HANDLE_VALUE;
  index = wq->offset + wq->count;
  if (index >= wq->alloc)
    index -= wq->alloc;
  wq->array[index] = event;
  wq->count++;
  return event;
}

/* Notifies the first thread from a wait queue and dequeues it.  */
static inline void
gl_waitqueue_notify_first (gl_waitqueue_t *wq)
{
  SetEvent (wq->array[wq->offset + 0]);
  wq->offset++;
  wq->count--;
  if (wq->count == 0 || wq->offset == wq->alloc)
    wq->offset = 0;
}

/* Notifies all threads from a wait queue and dequeues them all.  */
static inline void
gl_waitqueue_notify_all (gl_waitqueue_t *wq)
{
  unsigned int i;

  for (i = 0; i < wq->count; i++)
    {
      unsigned int index = wq->offset + i;
      if (index >= wq->alloc)
	index -= wq->alloc;
      SetEvent (wq->array[index]);
    }
  wq->count = 0;
  wq->offset = 0;
}

void
glthread_rwlock_init (gl_rwlock_t *lock)
{
  InitializeCriticalSection (&lock->lock);
  gl_waitqueue_init (&lock->waiting_readers);
  gl_waitqueue_init (&lock->waiting_writers);
  lock->runcount = 0;
  lock->guard.done = 1;
}

void
glthread_rwlock_rdlock (gl_rwlock_t *lock)
{
  if (!lock->guard.done)
    {
      if (InterlockedIncrement (&lock->guard.started) == 0)
	/* This thread is the first one to need this lock.  Initialize it.  */
	glthread_rwlock_init (lock);
      else
	/* Yield the CPU while waiting for another thread to finish
	   initializing this lock.  */
	while (!lock->guard.done)
	  Sleep (0);
    }
  EnterCriticalSection (&lock->lock);
  /* Test whether only readers are currently running, and whether the runcount
     field will not overflow.  */
  if (!(lock->runcount + 1 > 0))
    {
      /* This thread has to wait for a while.  Enqueue it among the
	 waiting_readers.  */
      HANDLE event = gl_waitqueue_add (&lock->waiting_readers);
      if (event != INVALID_HANDLE_VALUE)
	{
	  DWORD result;
	  LeaveCriticalSection (&lock->lock);
	  /* Wait until another thread signals this event.  */
	  result = WaitForSingleObject (event, INFINITE);
	  if (result == WAIT_FAILED || result == WAIT_TIMEOUT)
	    abort ();
	  CloseHandle (event);
	  /* The thread which signalled the event already did the bookkeeping:
	     removed us from the waiting_readers, incremented lock->runcount.  */
	  if (!(lock->runcount > 0))
	    abort ();
	  return;
	}
      else
	{
	  /* Allocation failure.  Weird.  */
	  do
	    {
	      LeaveCriticalSection (&lock->lock);
	      Sleep (1);
	      EnterCriticalSection (&lock->lock);
	    }
	  while (!(lock->runcount + 1 > 0));
	}
    }
  lock->runcount++;
  LeaveCriticalSection (&lock->lock);
}

void
glthread_rwlock_wrlock (gl_rwlock_t *lock)
{
  if (!lock->guard.done)
    {
      if (InterlockedIncrement (&lock->guard.started) == 0)
	/* This thread is the first one to need this lock.  Initialize it.  */
	glthread_rwlock_init (lock);
      else
	/* Yield the CPU while waiting for another thread to finish
	   initializing this lock.  */
	while (!lock->guard.done)
	  Sleep (0);
    }
  EnterCriticalSection (&lock->lock);
  /* Test whether no readers or writers are currently running.  */
  if (!(lock->runcount == 0))
    {
      /* This thread has to wait for a while.  Enqueue it among the
	 waiting_writers.  */
      HANDLE event = gl_waitqueue_add (&lock->waiting_writers);
      if (event != INVALID_HANDLE_VALUE)
	{
	  DWORD result;
	  LeaveCriticalSection (&lock->lock);
	  /* Wait until another thread signals this event.  */
	  result = WaitForSingleObject (event, INFINITE);
	  if (result == WAIT_FAILED || result == WAIT_TIMEOUT)
	    abort ();
	  CloseHandle (event);
	  /* The thread which signalled the event already did the bookkeeping:
	     removed us from the waiting_writers, set lock->runcount = -1.  */
	  if (!(lock->runcount == -1))
	    abort ();
	  return;
	}
      else
	{
	  /* Allocation failure.  Weird.  */
	  do
	    {
	      LeaveCriticalSection (&lock->lock);
	      Sleep (1);
	      EnterCriticalSection (&lock->lock);
	    }
	  while (!(lock->runcount == 0));
	}
    }
  lock->runcount--; /* runcount becomes -1 */
  LeaveCriticalSection (&lock->lock);
}

void
glthread_rwlock_unlock (gl_rwlock_t *lock)
{
  if (!lock->guard.done)
    abort ();
  EnterCriticalSection (&lock->lock);
  if (lock->runcount < 0)
    {
      /* Drop a writer lock.  */
      if (!(lock->runcount == -1))
	abort ();
      lock->runcount = 0;
    }
  else
    {
      /* Drop a reader lock.  */
      if (!(lock->runcount > 0))
	abort ();
      lock->runcount--;
    }
  if (lock->runcount == 0)
    {
      /* POSIX recommends that "write locks shall take precedence over read
	 locks", to avoid "writer starvation".  */
      if (lock->waiting_writers.count > 0)
	{
	  /* Wake up one of the waiting writers.  */
	  lock->runcount--;
	  gl_waitqueue_notify_first (&lock->waiting_writers);
	}
      else
	{
	  /* Wake up all waiting readers.  */
	  lock->runcount += lock->waiting_readers.count;
	  gl_waitqueue_notify_all (&lock->waiting_readers);
	}
    }
  LeaveCriticalSection (&lock->lock);
}

void
glthread_rwlock_destroy (gl_rwlock_t *lock)
{
  if (!lock->guard.done)
    abort ();
  if (lock->runcount != 0)
    abort ();
  DeleteCriticalSection (&lock->lock);
  if (lock->waiting_readers.array != NULL)
    free (lock->waiting_readers.array);
  if (lock->waiting_writers.array != NULL)
    free (lock->waiting_writers.array);
  lock->guard.done = 0;
}

/* --------------------- gl_recursive_lock_t datatype --------------------- */

void
glthread_recursive_lock_init (gl_recursive_lock_t *lock)
{
  lock->owner = 0;
  lock->depth = 0;
  InitializeCriticalSection (&lock->lock);
  lock->guard.done = 1;
}

void
glthread_recursive_lock_lock (gl_recursive_lock_t *lock)
{
  if (!lock->guard.done)
    {
      if (InterlockedIncrement (&lock->guard.started) == 0)
	/* This thread is the first one to need this lock.  Initialize it.  */
	glthread_recursive_lock_init (lock);
      else
	/* Yield the CPU while waiting for another thread to finish
	   initializing this lock.  */
	while (!lock->guard.done)
	  Sleep (0);
    }
  {
    DWORD self = GetCurrentThreadId ();
    if (lock->owner != self)
      {
	EnterCriticalSection (&lock->lock);
	lock->owner = self;
      }
    if (++(lock->depth) == 0) /* wraparound? */
      abort ();
  }
}

void
glthread_recursive_lock_unlock (gl_recursive_lock_t *lock)
{
  if (lock->owner != GetCurrentThreadId ())
    abort ();
  if (lock->depth == 0)
    abort ();
  if (--(lock->depth) == 0)
    {
      lock->owner = 0;
      LeaveCriticalSection (&lock->lock);
    }
}

void
glthread_recursive_lock_destroy (gl_recursive_lock_t *lock)
{
  if (lock->owner != 0)
    abort ();
  DeleteCriticalSection (&lock->lock);
  lock->guard.done = 0;
}

/* -------------------------- gl_once_t datatype -------------------------- */

void
glthread_once (gl_once_t *once_control, void (*initfunction) (void))
{
  if (once_control->inited <= 0)
    {
      if (InterlockedIncrement (&once_control->started) == 0)
	{
	  /* This thread is the first one to come to this once_control.  */
	  InitializeCriticalSection (&once_control->lock);
	  EnterCriticalSection (&once_control->lock);
	  once_control->inited = 0;
	  initfunction ();
	  once_control->inited = 1;
	  LeaveCriticalSection (&once_control->lock);
	}
      else
	{
	  /* Undo last operation.  */
	  InterlockedDecrement (&once_control->started);
	  /* Some other thread has already started the initialization.
	     Yield the CPU while waiting for the other thread to finish
	     initializing and taking the lock.  */
	  while (once_control->inited < 0)
	    Sleep (0);
	  if (once_control->inited <= 0)
	    {
	      /* Take the lock.  This blocks until the other thread has
		 finished calling the initfunction.  */
	      EnterCriticalSection (&once_control->lock);
	      LeaveCriticalSection (&once_control->lock);
	      if (!(once_control->inited > 0))
		abort ();
	    }
	}
    }
}

#endif

/* ========================================================================= */
