#include <types.h>
#include <lib.h>
#include <synch.h>
#include <test.h>
#include <thread.h>
#include "bar.h"




/* instantiate the orders array */
struct order orders[NCUSTOMERS];

/*
 * Global variables and constants
 */

// variables
int ndrinkers;										// the number of drinkers left in the bar
int takeorder;										// tells bartenders to take order, and prevents them from being stuck in the bar
const int norders = NCUSTOMERS;						// unnecessary, but I thinks this makes thing nicer

// semaphores
static struct semaphore *emptyglasses;				// semaphore on how many empty glasses there are
static struct semaphore *ordersready;				// semaphore so we know when there are some ready orders for "take_order"
static struct semaphore *ordercomplete[NCUSTOMERS];	// we create one semaphore for each order, for waiting until the order is complete

// locks
static struct lock *orderslock;						// used for locking the order array (orders)
static struct lock *bottleslock;					// used for locking the bottle array (bottles)
static struct lock *glassarraylock;					// used for locking the array with glasses
static struct lock *ndrinkerslock;					// used for locking, when changing the "ndrinkers" variable




/*
 * SUBMIT_ORDER: 
 *
 * Takes three arguments specifying what drink is being ordered. It is
 * expected that the glass is filled in the order specified 1,2,3. The
 * function returns when (and only when) the drink is ready. It
 * returns a pointer to the glass containing the drink.
 */ 
struct glass *submit_order(unsigned int ingredient1, 
			   unsigned int ingredient2, 
			   unsigned int ingredient3)
{
	int i;
	struct glass *g;
 

	// Find an available slot in orders
	// Set the ingredients in this spesific order
	// Change status to 1, that means "order is given by customer"
	// Stores the ordernumber so we can release the ringt semaphore later
	lock_acquire(orderslock);
	for (i=0; i<norders; i++){
		if (orders[i].state == 0){
			orders[i].ing[0] = ingredient1;		
			orders[i].ing[1] = ingredient2;
			orders[i].ing[2] = ingredient3;
			orders[i].state = 1;		
			orders[i].ordernumber = i;
			break;
		}
	}
	lock_release(orderslock);

	// We are using a semaphore on orders, to tell the bartenders when they can 
	// search for available orders in the orders array. We do a V (up) on this
	// semaphore, telling a bartender to start searching...
	V(ordersready);

	// Then we have to wait until the order is ready to be served (the drink is mixed)
	P(ordercomplete[i]);

	// The glass that will be returned to the customer
	g = orders[i].glass;

	// stores the ordernumber for this glass
	g->ordernumber = i;
	
	// We change the state of the order, to tell that this slot in the order array still is in use (order given to customer)
	lock_acquire(orderslock);
	orders[i].state = 3;
	lock_release(orderslock);

	// Returns the drink
	return g;
}




/*
 * RETURN_GLASS:
 *
 * This function takes a pointer to the now-empty glass and makes the
 * glass available for reuse by the bartender.
 */
void return_glass(struct glass *g)
{
	int ordernumber;

	// Change the state of the glass, till empty
	// and get the ordernumber this glass was used for
	lock_acquire(glassarraylock);
	g->state=0;
	ordernumber = g->ordernumber;
	lock_release(glassarraylock);

	// Do a "up" on the "emptyglass" semaphore
	// We don't bother to wash the glass ((:
	V(emptyglasses);

	// resets the order, a new order can be given here...
	lock_acquire(orderslock);
	orders[ordernumber].state=0;
	lock_release(orderslock);

}
 



/*
 * TAKE_ORDER:
 *
 * This function waits for a new order to be submitted. When
 * submitted, it records the details, and returns a pointer to the
 * details.
 */
struct order * take_order()
{

	int i;

	// Wait until a order is ready to be taken, do a "down" on the semaphore
	P(ordersready);

	// Goes through all orders to find one that is ready
	lock_acquire(orderslock);

	for (i=0; i<norders; i++)
		if (orders[i].state == 1){
			orders[i].state = 2;
			lock_release(orderslock);
			return &orders[i];
		}

	panic("take_order: there should be a order ready to be taken");

	// return 0 to ignore compiler warning
	return 0;
}




/*
 * FILL_ORDER:
 *
 * This function takes an order and 
 *     + associates a glass with it (when one becomes available)
 *     + it obtains exclusive access to the appropriate bottles to
 *     pour the drink 
 *     + it "pours" the drink into the glass associated with the
 *     order(PRESERVING THE POUR ORDER IN THE ORIGINAL ORDER)
 */
void fill_order(struct order *o)
{

	int i;
	struct glass *g;

	// Waits until there is an available glass
	P(emptyglasses);

	// We find the empty glass
	// and change the state of the glass till "in use"
	lock_acquire(glassarraylock);
	for (i=0; i<NGLASSES; i++){
		if (glasses[i].state == 0){
			glasses[i].state = 1;
			g = &glasses[i];
			break;
		}
	}
	lock_release(glassarraylock);

	// Associates a glass to the order
	o->glass = g;


	// The bartenders in this bar can not pour in parallell.
	// All the bottles has to be available for a bartender to pour.
	// This could have been optimized.
	lock_acquire(bottleslock);
	pour(g, o->ing[0], o->ing[1], o->ing[2]);
	lock_release(bottleslock);
}




/*
 * SERVE_ORDER:
 *
 * Takes a filled order and makes it available to the waiting customer.
 */
void serve_order(struct order *o)
{
	// Do a "up" on this spesific order, it's ready to be served...
	V(ordercomplete[o->ordernumber]);
}




/*
 * DRINKERS:
 *
 * This function returns the number of drinkers remaining in the bar.
 *
 */
int drinkers()
{
	// Returning the number of drinkers left in the bar
	return ndrinkers;
}




/*
 * CRAWL_HOME:
 *
 * This function takes our intrepid customers home. It must reduce the
 * number of remaining customers and ensure that the bartenders know
 * when they can go home.
 */
void crawl_home()
{
	struct glass *g;
	int i;
	
	// Decrease ndrinkers by one
	lock_acquire(ndrinkerslock);
	ndrinkers--;

	// If this is the last customer, he orders a lot of empty drinks to
	// let all the bartenders go home (otherwise they stay in the bar forever)
	if (ndrinkers == 0){
		for (i=0; i<NBARTENDERS; i++){
			g = submit_order(0,0,0);
			return_glass(g);
		}
	}

	lock_release(ndrinkerslock);
}




/*
 * BAR_OPEN:
 *
 * Perform any initialisation you need prior to opening the bar to
 * staff and customers
 */
void bar_open()
{
	int i;

	// Initialize the total number of drinkers in the bar
	ndrinkers = NCUSTOMERS;

	// Initialize all the glasses to be empty
	for (i=0; i<NGLASSES; i++)
		glasses[i].state = 0;

	// Initialize all the orders to be empty
	for (i=0; i<norders; i++)
		orders[i].state = 0;

	// Initialize semaphores
	emptyglasses = sem_create("emptyglasses", NGLASSES);
	if (emptyglasses==NULL) {
		panic("bar_open: sem_create failed\n");
	}

	ordersready = sem_create("ordersready", 0);
	if (ordersready==NULL) {
		panic("bar_open: sem_create failed\n");
	}

	for (i=0; i< NCUSTOMERS; i++) {
		ordercomplete[i] = sem_create("ordercomplete",0);
		if (ordercomplete[i]==NULL) {
			panic("bar_open: sem_create failed\n");
		}
	}

	// Initialize locks
	orderslock = lock_create("orderslock");
	if (orderslock == NULL) {
		panic("bar_open: lock_create failed\n");
	}

	bottleslock = lock_create("bottleslock");
	if (bottleslock == NULL) {
		panic("bar_open: lock_create failed\n");
	}

	glassarraylock = lock_create("glassarraylock");
	if (glassarraylock == NULL) {
		panic("bar_open: lock_create failed\n");
	}

	ndrinkerslock = lock_create("ndrinkerslock");
	if (ndrinkerslock == NULL){
		panic("bar_open: lock_create failed\n");
	}
}




/*
 * BAR_CLOSE:
 *
 * Perform any cleanup after the bar has closed and everybody has
 * stumbled home.
 */
void bar_close()
{
	int i;

	sem_destroy(emptyglasses);
	sem_destroy(ordersready);
	for (i=0; i< NCUSTOMERS; i++)
		sem_destroy(ordercomplete[i]);
	lock_destroy(orderslock);
	lock_destroy(bottleslock);
	lock_destroy(glassarraylock);
	lock_destroy(ndrinkerslock);
}
