#include "../GSPy/GSPyInterface.h"

#include <stdio.h>

int main()
{
	double inargs[12] = {10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
	double outargs[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

	printf("Inputs:\n");
	printf("%4.1f, %4.1f, %4.1f\n", inargs[0], inargs[1],  inargs[2]);
	printf("%4.1f, %4.1f, %4.1f\n", inargs[3], inargs[4],  inargs[5]);
	printf("%4.1f, %4.1f, %4.1f\n", inargs[6], inargs[7],  inargs[8]);
	printf("%4.1f, %4.1f, %4.1f\n", inargs[9], inargs[10], inargs[11]);

	int status[1] = { 1 };
	GSPyInterface(XF_REP_VERSION, status, inargs, outargs);
	GSPyInterface(XF_INITIALIZE, status, inargs, outargs);
	GSPyInterface(XF_REP_ARGUMENTS, status, inargs, outargs);
	GSPyInterface(XF_CALCULATE, status, inargs, outargs);
	GSPyInterface(XF_CLEANUP, status, inargs, outargs);

	printf("Result:\n");
	printf("%4.1f, %4.1f, %4.1f\n", outargs[0], outargs[1], outargs[2]);
	printf("%4.1f, %4.1f, %4.1f\n", outargs[3], outargs[4], outargs[5]);

	return 0;
}