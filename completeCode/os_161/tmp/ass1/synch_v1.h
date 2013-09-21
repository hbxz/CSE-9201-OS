/*
 * Header file for synchronization primitives.
 */

#ifndef _SYNCH_H_
#define _SYNCH_H_

/*
 * Dijkstra-style semaphore.
 * Operations:
 *     P (proberen): decrement count. If the count is 0, block until
 *                   the count is 1 again before decrementing.
 *     V (verhogen): increment count.
 * 
 * Both operations are atomic.
 *
 * The name field is for easier debugging. A copy of the name is made
 * internally.
 */

struct semaphore {
	char *name;
	volatile int count;
};

struct semaphore *sem_create(const char *name, int initial_count);
void              P(struct semaphore *);
void              V(struct semaphore *);
void              sem_destroy(struct semaphore *);


/*
 * Simple lock for mutual exclusion.
 * Operations:
 *    lock_acquire - Get the lock. Only one thread can hold the lock at the
 *                   same time.
 *    lock_release - Free the lock. Only the thread holding the lock may do
 *                   this.
 *    lock_do_i_hold - Return true if the current thread holds the lock; 
 *                   false otherwise.
 *
 * These operations must be atomic. You get to write them.
 *
 * When the lock is created, no thread should be holding it.
 *
 * The name field is for easier debugging. A copy of the name is made
 * internally.
 */

struct lock {
	char *name;
 	/*
	 * ADD STUFF HERE AS REQUIRED
	 *
	 * DON'T FORGET TO MARK THINGS VOLATILE AS NEEDED
	 */

	// -- start of changes (SoC) --
	// The lock operations are done by a thread, a thread "is the owner"
	// of the lock. So in the lock we need to have a relation to the 
	// thread that actually get's the lock. Let's call this thread the "lock_owner".

	// [http://www.programmersheaven.com/articles/pathak/article1.htm]
	// "A variable should be declared volatile whenever its value can be changed 
	// by something beyond the control of the program in which it appears, 
	// such as a concurrently executing thread."
	// That is the reason why I make this variable volatile.
	// (And because it is mentioned in this function)

	// this is a pointer to the "owner" of the lock, which is a thread.
//	struct thread volatile *lock_owner;
	struct thread *volatile lock_owner;

	// -- end of changes (EoC) --


};

struct lock *lock_create(const char *name);
void         lock_acquire(struct lock *);
void         lock_release(struct lock *);
int          lock_do_i_hold(struct lock *);
void         lock_destroy(struct lock *);


/*
 * Condition variable.
 *
 * Note that the "variable" is a bit of a misnomer: a CV is normally used
 * to wait until a variable meets a particular condition, but there's no
 * actual variable, as such, in the CV.
 *
 * Operations:
 *    cv_wait      - Release the supplied lock, go to sleep, and, after
 *                   waking up again, re-acquire the lock.
 *    cv_signal    - Wake up one thread that's sleeping on this CV.
 *    cv_broadcast - Wake up all threads sleeping on this CV.
 *
 * For all three operations, the current thread must hold the lock passed 
 * in. Note that under normal circumstances the same lock should be used
 * on all operations with any particular CV.
 *
 * These operations must be atomic. You get to write them.
 *
 * The name field is for easier debugging. A copy of the name is made
 * internally.
 */

struct cv {
	char *name;
};

struct cv *cv_create(const char *name);
void       cv_wait(struct cv *cv, struct lock *lock);
void       cv_signal(struct cv *cv, struct lock *lock);
void       cv_broadcast(struct cv *cv, struct lock *lock);
void       cv_destroy(struct cv *);

#endif /* _SYNCH_H_ */
