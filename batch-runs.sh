#! /bin/sh -f

# Running on more random instances
testcases="random layered unit-disk"
for t in $testcases
do
	echo "Running $t instances, STARTING TIME: `date`"
	echo "-----------------------------------------------------"
	timeout 500s ./mcp $t noILP 
	#timeout 2000s ./mcp $t noILP runLargeInstances
	echo "-----------------------------------------------------"
done

# Takes about 2 hours for all tests to finish.
#echo "Running roadnet instances, STARTING TIME: `date`" 
#echo "-----------------------------------------------------"
#timeout 10000s ./mcp roadnet noILP
