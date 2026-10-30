#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "xnor_network.h"

#define MAX_EPOCHS 100000
//training data
double inputs[4][NB_INPUT] = {
	{0.0, 0.0},
	{0.0, 1.0},
	{1.0, 0.0},
	{1.0, 1.0}
};

double targets[4] = {1.0, 0.0, 0.0, 1.0};

double sigmoid(double x){
	return 1.0 / (1.0 + exp(-x));
}
//a is the output of the sigmoid function
double sigmoid_prime(double a){
	return a * (1.0 - a);
}

double rand_uniform(void){
	return (double)rand() / (double)RAND_MAX;
}

double rand_range(double min, double max){
	return min + (max - min) * rand_uniform();
}

//initializes the arrays with random weights and random biases
void init_weights_biases(network *net){

	for (int i = 0; i < NB_INPUT; i++){
		for (int y = 0; y < NB_HIDDEN; y++){
			net->w1[i][y] = rand_range(-1.0, 1.0);
		}
	}
	for (int u = 0; u < NB_HIDDEN; u++){
		net->b1[u] = rand_range(-1.0, 1.0);
	}
	for (int j = 0; j < NB_OUTPUT; j++){
		for (int k = 0; k < NB_HIDDEN; k++){
			net->w2[j][k] = rand_range(-1.0, 1.0);
		}
	}
	net->b2[0] = rand_range(-1.0, 1.0);
}
//print the seed
void print_network(network *net){
	for (int j = 0; j < NB_HIDDEN; j++){
		printf("hidden %d: b=%+.3f w=", j, net->b1[j]);
		for (int i = 0; i < NB_INPUT; i++){
			printf("%+.3f ", net->w1[j][i]);
		}
		printf("\n");
	}
	for (int k = 0; k < NB_OUTPUT; k++){
		printf("output %d: b=%+.3f w=", k, net->b2[k]);
		for (int j = 0; j < NB_HIDDEN; j++){
			printf("%+.3f ", net->w2[k][j]);
		}
		printf("\n");
	}
}
//take one input and compute the network's output
void forward(network *net, double *x, double *h, double *o){
	for (int j = 0; j < NB_HIDDEN; j++){
		double z = net->b1[j] + net->w1[0][j] * x[0] + net->w1[1][j] * x[1];
		double a = sigmoid(z);
		h[j] = a;
	}
	double h_out = net->b2[0] + h[0] * (net->w2[0][0]) + h[1] * (net->w2[0][1]) + h[2] * (net->w2[0][2]);
	double o_res = sigmoid(h_out);
	o[0] = o_res;
}
			
//compute the error
double error(double *o, double target){
	return pow((o[0] - target), 2.0) / 2.0;
}

//calculate the gradient for each weight and updates accordingly
void backward(network *net, double *x, double *h,
              double *o, double t, double lr)
{
    /* output error signal */
    double d2 = (o[0] - t) * o[0] * (1.0 - o[0]);

    /* hidden error signal*/
    double d1[NB_HIDDEN];
    for (int j = 0; j < NB_HIDDEN; j++)
        d1[j] = d2 * net->w2[0][j] * h[j] * (1.0 - h[j]);

    /* update output layer */
    for (int j = 0; j < NB_HIDDEN; j++)
        net->w2[0][j] -= lr * d2 * h[j];
    net->b2[0] -= lr * d2;

    /* update hidden layer */
    for (int j = 0; j < NB_HIDDEN; j++)
    {
        for (int i = 0; i < NB_INPUT; i++)
            net->w1[j][i] -= lr * d1[j] * x[i];
        net->b1[j] -= lr * d1[j];
    }
}

void update(network net,double *h, double *o){
	FILE *fp = fopen("results", "w");
	if (!fp){
		printf("error opening result file\n");
		return;
	}
	for (int epoch = 0; epoch < MAX_EPOCHS; epoch++){
		double loss = 0.0;
		for (int s = 0; s < 4; s++){
			forward(&net, inputs[s], h, o);
			fprintf(fp, "%.0f %.0f -> %.4f (expected %.0f)\n",
				inputs[0][s], inputs[1][s], o[0], targets[s]);
			loss += 0.5 * (o[0] - targets[s]) * (o[0] - targets[s]);
			backward(&net, inputs[s], h, o, targets[s], 0.5);
		}
		if (epoch % 1000 == 0){
			printf("epoch %d, loss %f\n", epoch, loss);
		}
		if (loss < 0.001){
			break;
		}
	}
}

int main(){
	network net;
	init_weights_biases(&net);
	print_network(&net);
	double h[NB_HIDDEN];
	double o[NB_OUTPUT];
	update(net, h, o);
}
