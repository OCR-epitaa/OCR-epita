#include <stdio.h>
#include <stdlib.h>
#include "xnor_network.h"

//code for a nand gate, the xnor gate is just multiple nand gates
//for the weights, w1 and w2 are equal to -2 and the bias is 3 
int nand_gate(int A, int B){
	struct perceptron nand_gate;
	nand_gate.weight = -2;
	nand_gate.bias = 3;
	if (A * nand_gate.weight + B * nand_gate.weight + nand_gate.bias <= 0){
		return 0;
	}
	else {
		return 1;
	}
}

int main(int argc, char *argv[]){
	if (argc != 3){
		printf("only 3 arguments\n");
		return -1;
	}
	int A = atoi(argv[1]);
	int B = atoi(argv[2]);
	int res = 
		nand_gate(
			nand_gate(nand_gate(A, A), nand_gate(B, B)),
				nand_gate(A, B));
	printf("%d", res);
	return res;
}
