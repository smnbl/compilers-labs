// based on wikipedia version of gaussian elimination algorithm in pseudo-code:
// https://en.wikipedia.org/wiki/Gaussian_elimination

int abs(int x) {
	if (x < 0)
		return -x;
	return x;
}

int main()
{
    int iterations = 10;

    int n = 512; // width == height
    int A[262144];


    int seed = 1;

    int iteration;

    int i; int j;
    int rand;

	int h; int k;
	int temp;

    for (iteration = 0; iteration < iterations; iteration = iteration + 1) {
        // pseudo-randon number generation
        for (j = 0; j < n*n; j = j+1) {
            // Get random number
            seed = seed * 1103515245 + 12345;
            rand = (seed/65536) % 32768;

            A[j] = rand;
        }

        h = 0;
        k = 0;

        int found_row;
   		int f;

		// gaussian elimination
		while ((h < n)*(k < n)) {
			// search for 0 zero column, below h 
			found_row = 0;
			for (i = h; (found_row == 0)*(i < n); i = i + 1) {
				if (A[i * n + k] != 0) {
					// swap with non-zero row
					for (j = 0; j < n; j = j + 1) {
						temp = A[i * n + j];
						A[i * n + j] = A[h * n + j];
						A[h * n + j] = temp;
					}
					found_row = 1;
				}
			}
			if (found_row == 0) { // no pivot in this row, pass to next column
				k = k + 1;
			} else {
				for (i = h + 1; i < n; i = i + 1) {
					f = A[i*n + k] / A[h*n + k];
					A[i*n + k] = 0;
					for (j = k + 1; j < n; j = j + 1)
						A[i*n + j] = A[i*n + j] - f*A[h*n + j];
				}
				// increase pivot row and column
				h = h + 1;
				k = k + 1;
			}
		}
    }

    // print out result
    //for (i = 0; i < n; i = i + 1) {
	//    for (j = 0; j < n; j = j + 1)
	//    {
	//		print(A[i*n + j]);
	//    }
	//    print_s("ROW");
	//}
}
