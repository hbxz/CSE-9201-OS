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

int ndrinkers;									// the number of drinkers left in the bar
int takeorder;									// a counter that prevents bartenders getting stuck in the bar
const int norders = NCUSTOMERS;					// unnecessary, but I thinks this makes thing nicer

static struct semaphore *emptyglasses;			// semaphore on how many empty glasses there are
static struct semaphore *ordersready;			// semaphore so we know when there are some ready orders for "take_order"
static struct semaphore *ordersem[NCUSTOMERS];	// we create one semaphore for each order
static struct semaphore *ndrinkerssem;			// a binary semaphore for accesing the "ndrinkers" variable
static struct semaphore *cregionsem;			// used around the critical region in serve, where bartenders wait for customers
static struct semaphore *bottlessem[NBOTTLES];	// semaphore array used to block different bottles
static struct semaphore *takingordersem;		// used for locking the takeorder integer
static struct semaphore *glassem;				// semaphore used for locking the glass array
static struct semaphore *aglassem[NGLASSES];	// semaphore for blocking one and one glass

static struct lock *orderslock;					// used for locking the order array (orders)
static struct lock *bottleslock;				// used for locking the bottle array (bottles)




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
 
 	// Increase the "takeorder" by 1. This is a way of telling the bartenders that 
 	// they might stop waiting (depending on NCUSTOMERS and NBARTENDERS) and take a new order 
	P(takingordersem);
	takeorder++;
	V(takingordersem);
 
	// We assumes that the no of orders = no of customers, so that each customer has to
	// drink up and return his glass before he can order a new drink. There will
	// always be a avaliable slot in the orders array

	// Lock the orders to change the state of this customers order
	// we do this outside the for loop, since the values might change
	// while we traverse the array

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
	P(ordersem[i]);

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
	P(glassem);
	g->state=0;
	ordernumber = g->ordernumber;

	V(aglassem[g->glassnumber]);		// testing

	V(glassem);

	// resets the order, a new order can be given here...
	lock_acquire(orderslock);
	orders[ordernumber].state=0;
	lock_release(orderslock);

	// Do a "up" on the "emptyglass" semaphore
	// We don't bother to wash the glass ((:
	V(emptyglasses);
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

	// Waits until there is a available glass
	P(emptyglasses);

	// We find the empty glass
	// and change the state of the glass till "empty and in use"
	P(glassem);
	for (i=0; i<NGLASSES; i++){
		if (glasses[i].state == 0){
			V(aglassem[i]);				// testing, this might help?
			glasses[i].glassnumber = i;	// testing
			glasses[i].state = 1;
			g = &glasses[i];
		}
	}
	V(glassem);

	// Associates a glass to the order
	o->glass = g;

	// If the "nothing" ingredient (0) is specified, we don't need a bottle
	lock_acquire(bottleslock);
	for (i=0; i<3; i++){
		if ( o->ing[i] != 0 ){
			P(bottlessem[o->ing[i]]);
		}
	}
	lock_release(bottleslock);

	// Pour ingredients into glass
	pour(g, o->ing[0], o->ing[1], o->ing[2]);

	// Finished using the bottles, release them
	// by doing an "up" on the bottle semaphore
	lock_acquire(bottleslock);
	for (i=0; i<3; i++)
		if ( o->ing[i] != 0 )
			V(bottlessem[o->ing[i]]);
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
	V(ordersem[o->ordernumber]);

	// This is done to prevent the race condition that can leed to bartenders 
	// getting stuck in the bar after all the customers has left. Only one
	// bartender at the time can be inside this region waiting for a
	// new order by the customer, or waiting for the customer to go home.
	P(cregionsem);

	// Busy waiting, do nothing...
	while (takeorder < 0){
	}

	// We have to check if there are more drinkers in the bar.
	// If all the customers has left we don't want to decrease
	// "takeorder" by one. Otherwise the bartenders that might
	// be waiting to step into this region would never come
	// out of the while loop (Because all the customers has left
	// and "takeorder" would never increase to 0 again, "takorder"
	// is increased in either "crawl_home()" or "submit_order()" )
	P(ndrinkerssem);
	if (ndrinkers != 0){
		P(takingordersem);
		takeorder--;
		V(takingordersem);
	}
	V(ndrinkerssem);

	V(cregionsem);
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
	// Decrease ndrinkers by one
	P(ndrinkerssem);
	ndrinkers--;
	V(ndrinkerssem);

	// Increase "takeorder" to tell a bartender to continue
	// (Check number of drinkers that is left in the bar).
	P(takingordersem);
	takeorder++;
	V(takingordersem);
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

	// "takeorder" has to be the negative sum of customers and bartenders. The reason for 
	// this is that every customer always rush to the bar and order one drink. 
	// And we have to prevent bartenders to take a new order if there are to many
	// bartenders compared to customers.
	// An axample helps clarify things:
	// We have a bar with 3 bartenders and 1 customer. The "takeorder" is then initialized
	// to -4. All three bartenders is already waiting to take orders. The customer asks 
	// for his first drink. The "takeorder" is then increased by one, to -3. Bartender 1
	// takes this drink and serves it. After the drink is served to customer, the bartender
	// waits asking for new orders, since two of his buddies are already are waiting. 
	// This is done by the while loop in "serve_order". When customer has finished the
	// drink, he ask for a new one (his second one), "takeorder" is increased by 1 to -2, 
	// bartender 2 serves the new drink and waits together with bartender 1, since bartender 3 
	// still is waiting for a drink to be ordered. The customer finish his second drink, and 
	// orders a third one. The "takeorder" is increased to -1. The third (last standing) 
	// bartender serves the customer this drink. Then all the bartenders wait, since "takeorder"
	// is below zero. The problem with this solution is if the number of drinks requested by
	// this single customer was below the number of bartenders. Then some bartenders would
	// be waiting forever, after the customer had left the bar. Thats why I do the assumption
	// that NBARTENDERS <= NCUSTOMERS x DRINKS EACH CUSTOMER.
	// Now, when the customer is finished with the third drink he might go home. If he do that
	// "takeorder" is increased by one. One bartender then "stands up" and count all the remaining
	// customers. If there are non, he goes home and lets bartender 2 checks the same. So they
	// will all go home. "takeorder" is also increased if the customers orders a new drink instead
	// of going home. Then one bartender will "go to the bar", take the order, serve the order, and 
	// wait again untill the customer goes home or order someting else. Etc. etc.
	takeorder = -(NCUSTOMERS+NBARTENDERS);

	// Initialize the total number of drinkers in the bar
	ndrinkers = NCUSTOMERS;

	// Initialize all the glasses to be empty
	for (i=0; i<NGLASSES; i++)
		glasses[i].state = 0;

	// Initialize all the orders to be empty
	for (i=0; i<norders; i++)
		orders[i].state = 0;

	// semaphores
	emptyglasses = sem_create("emptyglasses", NGLASSES);
	if (emptyglasses==NULL) {
		panic("bar_open: sem_create failed\n");
	}

	ordersready = sem_create("ordersready", 0);
	if (ordersready==NULL) {
		panic("bar_open: sem_create failed\n");
	}

	for (i=0; i< NCUSTOMERS; i++) {
		ordersem[i] = sem_create("ordersem",0);
		if (ordersem[i]==NULL) {
			panic("bar_open: sem_create failed\n");
		}
	}

	ndrinkerssem = sem_create("ndrinkerssem", 1);
	if (ndrinkerssem==NULL) {
		panic("bar_open: sem_create failed\n");
	}

	cregionsem = sem_create("cregionsem", 1);
	if (cregionsem==NULL) {
		panic("bar_open: sem_create failed\n");
	}

	for (i=0; i< NBOTTLES; i++) {
		bottlessem[i] = sem_create("bottlessem", 1);
		if (bottlessem[i]==NULL) {
			panic("bar_open: sem_create failed\n");
		}
	}

	takingordersem = sem_create("takingordersem", 1);
	if (takingordersem==NULL) {
		panic("bar_open: sem_create failed\n");
	}

	glassem = sem_create("glassem", 1);
	if (glassem==NULL) {
		panic("bar_open: sem_create failed\n");
	}

	for (i=0; i< NBOTTLES; i++) {
		aglassem[i] = sem_create("aglassem", 1);
		if (aglassem[i]==NULL) {
			panic("bar_open: sem_create failed\n");
		}
	}

	// locks
	orderslock = lock_create("orderslock");
	if (orderslock == NULL) {
		panic("bar_open: lock_create failed\n");
	}

	bottleslock = lock_create("bottleslock");
	if (bottleslock == NULL) {
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
		sem_destroy(ordersem[i]);
	sem_destroy(ndrinkerssem);
	sem_destroy(cregionsem);
	for (i=0; i< NBOTTLES; i++)
		sem_destroy(bottlessem[i]);
	for (i=0; i< NBOTTLES; i++)
		sem_destroy(aglassem[i]);
	sem_destroy(takingordersem);
	sem_destroy(glassem);
	lock_destroy(orderslock);
	lock_destroy(bottleslock);
}
