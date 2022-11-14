#ifndef __SYNTHESIS__
#include <iostream>
#endif

extern "C" {
	void vadd(
	        const unsigned int *in1, // Read-Only Vector 1
	        const unsigned int *in2, // Read-Only Vector 2
	        unsigned int *out,       // Output Result
	        int size                 // Size in integer
	        )
	{

#ifndef __SYNTHESIS__
std::cout<<"TEST OUTPUT!!!!!"<<std::endl;
#endif

	    for(int i = 0; i < size; ++i)
	    {
	        out[i] = in1[i] + in2[i];
	    }
	}
}
