#include <stdlib.h>
#include <stdio.h>
#ifndef XNOR_NETWORK_H
#define XNOR_NETWORK_H

#define NB_INPUT 2
#define NB_HIDDEN 3
#define NB_OUTPUT 1

//one hidden layer with 3 neurons
typedef struct network{
	double w1[NB_HIDDEN][NB_INPUT];
	double b1[NB_HIDDEN];
	double w2[NB_OUTPUT][NB_HIDDEN];
	double b2[NB_OUTPUT];
}network;
#endif
