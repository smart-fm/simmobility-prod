//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*****************************************************************
 * WARNING: This file should ONLY contain a main function
 *          which dispatches to main_impl() and immediately
 *          returns. Please make all changes to main_impl.cpp.
 *          This file should NEVER change.
 *
 * Explanation: We separate main() into its own file so that we can
 *              build the short/medium/long term simulator as a library.
 *              A library that exports main is bad, so we need an easy
 *              way to exclude it. Hence this file.
 *****************************************************************/

//Prototype declaration: this function is found in main_imple.cpp.
int main_impl(int ARGC, char* ARGV[]);

//The actual main method simply dispatches to main_impl() in main_impl.cpp
int main(int ARGC, char* ARGV[])
{
	return main_impl(ARGC, ARGV);
}

