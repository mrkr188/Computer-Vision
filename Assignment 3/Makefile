all: CImg.h a3.cpp Classifier.h NearestNeighbor.h
	g++ a3.cpp -o a3 -lX11 -lpthread -I. -Isiftpp -O3 siftpp/sift.cpp
	g++ `pkg-config --cflags opencv` a2_3.cpp -o a2_3 -lX11 -lpthread -I. -Isiftpp -O3 siftpp/sift.cpp `pkg-config --libs opencv`

clean:
	rm a3
