#include <types.h>
#include <lib.h>
#include <synch.h>
#include <test.h>
#include <thread.h>

#include "bar.h"

/*
 * DEFINE THIS MACRO TO SWITCH ON MORE PRINTING
 *
 * Note: Your solution should work whether printing is on or off
 *  
 */
//#undef PRINT_ON 
#define PRINT_ON

/* this semaphore is for cleaning up at the end. */
static struct semaphore *alldone;

/* the array of glasses */
struct glass glasses[NGLASSES];

/* The array of bottles.
 * Note: bottle 1 is GIN, 2 is TONIC, 3 is BEER, and so forth as outlined
 * in bar.h
 */
struct bottle bottles[NBOTTLES];

/* An array of orders. Note that customers only can have one outstanding 
 * order. So maximum orders is equal to maximum customers.
 */
struct order orders[NCUSTOMERS];


/* CUSTOMERS
 * Customers are rather simple, they order, drink, 
 * return their glass, and repeat until they go home 
 */

void customer(void *unusedpointer, unsigned long customernum)
{
  struct glass * glass;
  int i,j;
  unsigned int ing1, ing2, ing3;

  (void) unusedpointer; /* avoid compiler warning */


  i = 0;
  do {

    /*
     * Our current clientele are fairly simple, they only drink B52s
     * or upside-down B52s.  However, your bar synchronisation must
     * handle more adventurous customers that order other mixed
     * drinks, and customers that choose a drink randomly.
     */
    if (customernum & 1) {
      ing1 = KAHLUA;    /* you may change these assignments for your testing */
      ing2 = GRAND_MARNIER;
      ing3 = BAILEYS;
    } else {
      ing3 = KAHLUA;   
      ing2 = GRAND_MARNIER;
      ing1 = BAILEYS;
    }

#ifdef PRINT_ON
    kprintf("C %ld ordering %d, %d, %d\n", customernum, ing1, ing2, ing3);
#endif

    glass = submit_order(ing1, ing2, ing3);
    

    /* Bottoms up!!! */

#ifdef PRINT_ON
    kprintf("C %ld drinking %d, %d, %d\n", customernum,
	    glass->contents[0],
	    glass->contents[1],
	    glass->contents[2]);
#endif

    for (j = 0; j < 3; j++) {
      glass->contents[j] = 0;
    }
    
    
    /* Ahhh, I needed that. 
       Short pause */
    thread_yield();

    /* be nice to bar staff,  return the glass */
    
#ifdef PRINT_ON
    kprintf("C %ld returning glass\n", customernum); 
#endif

    return_glass(glass);

    i++;
  } while (/* still standing */ i < 5); /* Note: You must be able to cope
				     with very irresponsible drinkers */
  
  kprintf("C %ld going home\n", customernum);
  crawl_home();
  
  V(alldone);
}


/* BARTENDERS
 * bartenders are only slightly more complicated than the customers.
 * They take_orders, and if valid, they fill them and serve them.
 * When all the customers have left, the bartenders go home.
 */
void bartender(void *unusedpointer, unsigned long bartendernum)
{
  
  struct order *o;
  
  (void)unusedpointer; /* avoid compiler warning */


  while (drinkers()) {
    
#ifdef PRINT_ON
    kprintf("B %ld taking order\n", bartendernum);
#endif
  
    o = take_order();
    
    if (o != NULL) {
      
#ifdef PRINT_ON
      kprintf("B %ld filling\n", bartendernum);
#endif
      
      fill_order(o);
      
#ifdef PRINT_ON
      kprintf("B %ld serving\n", bartendernum);
#endif
      
      serve_order(o);
    }
  };
  
  kprintf("B %ld going home\n", bartendernum);
  V(alldone);
}


/* CREATEBAR
 * This routine set up the bar
 */

int createbar(int nargs, char **args)
{
  int i, result;
  
  (void) nargs; /* avoid compiler warnings */
  (void) args;

  /* this semaphore indicates everybody has gone home */

  alldone = sem_create("alldone", 0);
  if (alldone==NULL) {
    panic("createbar: out of memory\n");
  }
  
  /* initialise all the bottles as full (number of doses = 0) */
  for (i=0; i < NBOTTLES; i++) {
    bottles[i].doses = 0;
  }
  
  /***********************************************************************
   * call your routine that initialises the rest of the bar 
   */
  bar_open();

  /* Start the bartenders */
  for (i=0; i<NBARTENDERS; i++) {
    result = thread_fork("bartender thread", NULL, i,
			 bartender, NULL);
    if (result) {
      panic("createbar: thread_fork failed: %s\n",
	    strerror(result));
    }
  }
  
  /* Start the customers */
  for (i=0; i<NCUSTOMERS; i++) {
    result = thread_fork("customer thread", NULL, i,
			 customer, NULL);
    if (result) {
      panic("createbar: thread_fork failed: %s\n",
	    strerror(result));
    }
  }
  
  /* Wait for everybody to finish. */
  for (i=0; i< NCUSTOMERS+NBARTENDERS; i++) {
    P(alldone);
  }
  
  /* Print out the bar stats for the evening */
  for (i =0 ; i < NBOTTLES; i++) {
    kprintf("Bottle %d served %d doses\n", i+1, bottles[i].doses);
  }
  
  kprintf("The bar is closed, goodnight!!!\n");

  /***********************************************************************
   * Call you bar clean up routine
   */
  bar_close();
  
  sem_destroy(alldone);
  return 0;
}



/*
 * POUR
 * pour takes a glass and an order (the potentially three ingredients) and fills
 * the glass.
 * POUR NEEDS THE ROUTINE THAT CALLS IT TO ENSURE THAT IT HAS EXCLUSIVE 
 * ACCESS TO THE BOTTLES IT NEEDS.
 */

void pour(struct glass *g, 
	  unsigned int ing1,
	  unsigned int ing2,
	  unsigned int ing3)
{
  int i;
  i = 0;

  /* add ingredients into glass in order given and increment number of 
     doses from particular bottle */

  if (ing1 != NOTHING) {
    g->contents[i] = ing1;
    i++;
    bottles[ing1].doses += 1;
  }
  if (ing2 != NOTHING) {
    g->contents[i] = ing2;
    i++;
    bottles[ing2].doses += 1;
  }
  if (ing3 != NOTHING) {
    g->contents[i] = ing3;
    bottles[ing3].doses += 1;
  }
}


  
  
