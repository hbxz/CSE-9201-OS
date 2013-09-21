
/* 
 * bar stock, each number represents a type of drink 
 */

#define NOTHING 0 /* bottle 0 used to represent no ingredient */
#define GIN    1
#define TONIC  2
#define BEER   3
#define VODKA  4
#define LIME_JUICE   5
#define COINTREAU    6
#define ORANGE_JUICE 7
#define LEMONADE     8
#define KAHLUA       9
#define BAILEYS      10
#define GRAND_MARNIER 11
#define CRANBERRY_JUICE 12

#define NBOTTLES 13

/*
 * drink orders:
 *
 * Can be any combination of the above.  Some standard recipes are
 * below for those who lack daring or imagination.
 *
 *
 * GIN_N_TONIC = GIN + TONIC
 * B_52 = KAHLUA +  GRAND_MARNIER + BAILEYS
 * VODKA_N_ORANGE = VOKDA + ORANGE_JUICE
 */




/*
 * The data type representing a glass 
 */ 
struct glass {
	unsigned int contents[3]; 	// representing the content of the glass
	int state;					// 0 = empty, 1 = in use
	int ordernumber;			// the order a glass is used for
};




/*
 * The data type representing a bottle 
 */
struct bottle {
	int doses; 					// The number of doses a particular bottle has served.
};




/*
 * The data type representing an order.
 */
struct order {
	unsigned int ing[3];		// the three ingredients
	struct glass *glass;		// the glass to be used for this order
	int ordernumber;			// the ordernumber, the number in the orders array
	int state;					// 0 = no order, 1 = order given by customer, 2 = order taken by bartender, 3 = order "given to" customer
	
};




/*
 * FUNCTION PROTOTYPES FOR THE FUNCTIONS YOU MUST WRITE
 *
 * YOU CANNOT MODIFY THESE PROTOTYPES
 *  
 */

/* Customer functions */
extern struct glass *submit_order(unsigned int, unsigned int, unsigned int);
extern void return_glass(struct glass *g);
extern void crawl_home();

/* Bartender functions */ 
extern struct order * take_order();
extern void fill_order(struct order *o);
extern void serve_order(struct order *o);
extern int drinkers();

/* Bar opening and closing functions */
extern void bar_open();
extern void bar_close();


/*
 * Function prototype for the supplied routine that pours the various
 * bottle contents into glasses
 */
extern void pour(struct glass *g, 
		 unsigned int,
		 unsigned int,
		 unsigned int);

/* variables accessible to both bar_driver.c and bar.c */
extern struct order orders[];
extern struct glass glasses[];
extern struct bottle bottles[];



/* 
 * WARNING: LIMITATION:
 * The NBARTENDERS must never exceed NCUSTOMERS x no of drinks pr customer
 * If that happens the bartenders will get stuck in the bar, waiting for orders.
 *
 */

/*
 * THESE PARAMETERS CAN BE CHANGED BY US, so you should test various
 * combinations. NOTE: We will only ever set these to something
 * greater than zero.
 */ 

#define NGLASSES 10        /* The number of glasses at the bar */
#define NCUSTOMERS 10    /* The number of customers drinking today*/
#define NBARTENDERS 3     /* The number of bartenders working today */

#include "test.h" /* DO NOT REMOVE THIS LINE. IT MUST BE LAST IN THIS FILE */




