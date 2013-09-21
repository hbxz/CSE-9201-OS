#include <types.h>
#include <lib.h>
#include <synch.h>
#include <test.h>
#include <thread.h>

#include "bar.h"

/* instantiate the orders array */

struct order orders[NCUSTOMERS];


/*
 * You can define an other variables or constants you need here
 */


/*
 * SUBMIT_ORDER: 
 *
 * Takes three arguments specifying what drink is being ordered. It is
 * expected that the glass in filled in the order specified 1,2,3. The
 * function returns when (and only when) the drink is ready. It
 * returns a pointer to the glass containing the drink.
 */ 

struct glass *submit_order(unsigned int ingredient1, 
			   unsigned int ingredient2, 
			   unsigned int ingredient3)
{
  (void) ingredient1; /* avoid compiler warnings */
  (void) ingredient2;
  (void) ingredient3;
  
  panic("You need to write some code\n");
  return 0;
}

/*
 * RETURN_GLASS:
 *
 * This function takes a pointer to the now-empty glass and makes the
 * glass available for reuse by the bartender.
 */

void return_glass(struct glass *g)
{

  (void) g;
  
  panic("You need to write some code\n");
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
  panic("You need to write some code\n");
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
  (void) o;
  
  panic("You need to write some code\n");
}

/*
 * SERVE_ORDER:
 *
 * Takes a filled order and makes it available to the waiting customer.
 */

void serve_order(struct order *o)
{
  (void) o;
  
  panic("You need to write some code\n");
}


/*
 * DRINKERS:
 *
 * This function returns the number of drinkers remaining in the bar.
 *
 */

int drinkers()
{
   
  panic("You need to write some code\n");
  return 0;
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
  panic("You need to write some code\n"); 
}


/*
 * BAR_OPEN:
 *
 * Perform any initialisation you need prior to opening the bar to
 * staff and customers
 */
void bar_open()
{
  panic("You need to write some code\n"); 
}

/*
 * BAR_CLOSE:
 *
 * Perform any cleanup after the bar has closed and everybody has
 * stumbled home.
 */

void bar_close()
{
  panic("You need to write some code\n"); 
}

