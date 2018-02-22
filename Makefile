all:
	mpic++ -L/usr/local/lib findUniqueNumber.cpp -o findUniqueNumber
clean:
	rm findUniqueNumber
