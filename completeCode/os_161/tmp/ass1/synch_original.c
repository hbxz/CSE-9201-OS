/*
 * Synchronization primitives.
 * See synch.h for specifications of the functions.
 */

#include <types.h>
#include <lib.h>
#include <synch.h>
#include <thread.h>
#include <curthread.h>
#include <machine/spl.h>

////////////////////////////////////////////////////////////
//
// Semaphore.

struct semaphore *
sem_create(const char *namearg, int initial_count)
{
	struct semaphore *sem;

	sem = kmalloc(sizeof(struct semaphore));
	if (sem == NULL) {
		return NULL;
	}

	sem->name = kstrdup(namearg);
	if (sem->name == NULL) {
		kfree(sem);
		return NULL;
	}

	sem->count = initial_count;
	return sem;
}

void
sem_destroy(struct semaphore *sem)
{
	int spl;
	assert(sem != NULL);

	spl = splhigh();
	assert(thread_hassleepers(sem)==0);
	splx(spl);

	/*
	 * Note: while someone could theoretically start sleeping on
	 * the semaphore after the above test but before we free it,
	 * if they're going to do that, they can just as easily wait
	 * a bit and start sleeping on the semaphore after it's been
	 * freed. Consequently, there's not a whole lot of point in 
	 * including the kfrees in the splhigh block, so we don't.
	 */

	kfree(sem->name);
	kfree(sem);
}

void 
P(struct semaphore *sem)
{
	int spl;
	assert(sem != NULL);

	/*
	 * May not block in an interrupt handler.
	 *
	 * For robustness, always check, even if we can actually
	 * complete the P without blocking.
	 */
	assert(in_interrupt==0);

	spl = splhigh();
	while (sem->count==0) {
		thread_sleep(sem);
	}
	assert(sem->count>0);
	sem->count--;
	splx(spl);
}

void
V(struct semaphore *sem)
{
	int spl;
	assert(sem != NULL);
	spl = splhigh();
	sem->count++;
	assert(sem->count>0);
	thread_wakeup(sem);
	splx(spl);
}

////////////////////////////////////////////////////////////
//
// Lock.

struct lock *
lock_create(const char *name)
{
	struct lock *lock;

	lock = kmalloc(sizeof(struct lock));
	if (lock == NULL) {
		return NULL;
	}

	lock->name = kstrdup(name);
	if (lock->name == NULL) {
		kfree(lock);
		return NULL;
	}
	/*
	 * ADD STUFF HERE AS REQUIRED
	 */
	
	return lock;
}

void
lock_destroy(struct lock *lock)
{
	assert(lock != NULL);

	/*
	 * ADD STUFF HERE AS REQUIRED
	 */

	kfree(lock->name);
	kfree(lock);
}

void
lock_acquire(struct lock *lock)
{
	/*
	 * ADD STUFF HERE AS REQUIRED
	 */
  (void) lock; // avoid compiler warning until code is here
}

void
lock_release(struct lock *lock)
{
	/*
	 * ADD STUFF HERE AS REQUIRED
	 */
  (void) lock; // avoid compiler warning until code is here
}

int
lock_do_i_hold(struct lock *lock)
{
	/*
	 * ADD STUFF HERE AS REQUIRED
	 */

  (void) lock; // avoid compiler warning until code is here
  return 1; // dummy until real code is added
}

////////////////////////////////////////////////////////////
//
// CV


struct cv *
cv_create(const char *name)
{
	struct cv *cv;

	cv = kmalloc(sizeof(struct cv));
	if (cv == NULL) {
		return NULL;
	}

	cv->name = kstrdup(name);
	if (cv->name==NULL) {
		kfree(cv);
		return NULL;
	}
	
	return cv;
}

void
cv_destroy(struct cv *cv)
{
	assert(cv != NULL);

	kfree(cv->name);
	kfree(cv);
}

void
cv_wait(struct cv *cv, struct lock *lock)
{

	int spl;

	assert(cv != NULL);
	assert(lock != NULL);
	
	// Turn off interrupts.
	spl = splhigh();

	// Must hold before waiting
	// Don't need to check this, because lock_release does.
	//assert(lock_do_i_hold(lock));

	// Let go of the lock.
	lock_release(lock);

	// Sleep until someone signals or broadcasts on the CV.
	thread_sleep(cv);
	
	// Get the lock back.
	lock_acquire(lock);
	
	// Return interrupts to previous level.
	splx(spl);


}

void
cv_signal(struct cv *cv, struct lock *lock)
{
	int spl;

	assert(cv != NULL);
	assert(lock != NULL);

	// Turn off interrupts.
	spl = splhigh();

	// Must hold the lock to signal.
	if (!lock_do_i_hold(lock)) {
		panic("cv_signal: cv %s at %p, lock %s at %p: Not holder.\n",
		      cv->name, cv, lock->name, lock);
	}

	// Just wake one thread up.
	thread_wakeone(cv);

	// Return interrupts to previous level.
	splx(spl);

}

void
cv_broadcast(struct cv *cv, struct lock *lock)
{
	int spl;

	assert(cv != NULL);
	assert(lock != NULL);

	// Turn off interrupts.
	spl = splhigh();

	// Must hold the lock to broadcast.
	if (!lock_do_i_hold(lock)) {
		panic("cv_broadcast: cv %s at %p, lock %s at %p: "
		      "Not holder.\n",
		      cv->name, cv, lock->name, lock);
	}

	// Wake 'em all.
	thread_wakeup(cv);

	// Return interrupts to previous level.
	splx(spl);

}
