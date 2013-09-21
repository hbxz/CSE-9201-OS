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

	// -- start of changes (SoC) --
	// We have to set the values of the struct.
	// As mentioned in the header file, there is no
	// owner to a new lock. So we set lock_owner to nothing

	lock->lock_owner=NULL;
	// -- end of changes (EoC) --
	
	return lock;
}

void
lock_destroy(struct lock *lock)
{
	assert(lock != NULL);

	/*
	 * ADD STUFF HERE AS REQUIRED
	 */

	// -- start of changes (SoC) --
	// We have to free the memory assigned when setting lock owner.

	kfree(lock->lock_owner);
	// -- end of changes (EoC) --

	kfree(lock->name);
	kfree(lock);
}

void
lock_acquire(struct lock *lock)
{
	/*
	 * ADD STUFF HERE AS REQUIRED
	 */

	// -- start of changes (SoC) --

	// Have to disable interrupts, otherwise I got the error msg:
	// OS/161 kernel [? for menu]: sy2
	// Starting lock test...
	// scheduler: Dropping thread synchtest.
	// scheduler: Dropping thread synchtest.
	// scheduler: Dropping thread synchtest
	int spl;

	// turn off interrups
	spl = splhigh(); 

	// First check the lock parameter that is passed as an argument
	if (lock == NULL){
		// <Q>: Should I send some more arguments to panic?
		// <A>: panic() acts like printf(), so if you have no escape sequences in
		//		the format string, it's fine not to give any additional arguments.
		panic("lock == NULL");
	} 

	// Second, check that you don't already have the lock.
	// If this happens you would aquire a lock you already have, 
	// and wait forever (deadlock)
	else if (lock_do_i_hold(lock)){
		panic("I am already the lockowner, we have a deadlock!");
	}

	// Third, check if someone else holds the lock
	// We do the same as in semaphores. Put the thread to sleep,
	// but this time thread sleeps on lock, not on semaphore.
	// <Q>: Why do we have to use while here?
	// <A>: This depends under what circumstances the thread is woken up. If
	//		you are sure that:
	//		1. only lock_release will wake up threads blocked in lock_acquire
	//		2. lock_release will only wake one thread when the lock is released
	//		then you could do without the loop (which deals with the case where
	//		the thread is awakened but the lock isn't available). However, I
	//		doubt you can give these guarantees.
 
	while (lock->lock_owner != NULL){
		thread_sleep(lock);
	}

	// <Q>: What if to threads does this exactly at the same time? Semaphores? This is a critical region?!
	// <A>:	You are correct in noticing the critical region here, but semaphores
	//		(a much more complex synchronisation mechanism than simple locks) are
	//		the wrong way to protect it. Instead, look at the underlying mechanism
	//		that the semaphore code uses to avoid race conditions.

	// Finaly we assigns the lock to the current thread that is running
	lock->lock_owner = curthread;	

	// turn on interrups
	splx(spl);

	// -- end of changes (EoC) --

}

void
lock_release(struct lock *lock)
{
	/*
	 * ADD STUFF HERE AS REQUIRED
	 */

	// -- start of changes (SoC) --
	int spl;

	// check that lock is not NULL
	if (lock == NULL){
		panic("lock == NULL");
	} 

	// only release lock if curthread holds it
	if (lock->lock_owner == curthread){
		// disable interrupts
		spl = splhigh();
		// removes the owner from this lock
		lock->lock_owner = NULL;
		// wake up one or more threads sleeping on this lock
		thread_wakeup(lock);
		// enable interrupts
		splx(spl);
	} else {
		panic("curthread is not the owner of the lock");
	}
	// -- end of changes (EoC) --

}

int
lock_do_i_hold(struct lock *lock)
{
	/*
	 * ADD STUFF HERE AS REQUIRED
	 */

	// -- start of changes (SoC) --
	int return_value;
	int spl;

	if (lock == NULL){
		panic("Lock == NULL");
	}

	spl = splhigh();

	if (curthread == lock->lock_owner)
		return_value = 1;
	else
		return_value = 0;

	splx(spl);

	return return_value;

	// -- end of changes (EoC) --


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
