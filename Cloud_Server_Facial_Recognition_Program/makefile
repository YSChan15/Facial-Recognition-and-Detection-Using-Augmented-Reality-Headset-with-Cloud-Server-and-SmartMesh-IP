all: CPEF CPFF CPLBPH CPSVM

CPEF: CPEF.cpp
	g++ CPEF.cpp -o CPEF -I/usr/local/include/opencv4 -L/usr/local/lib64 -lopencv_core -lopencv_face -lopencv_highgui -lopencv_imgproc -lopencv_imgcodecs

CPFF: CPFF.cpp
	g++ CPFF.cpp -o CPFF -I/usr/local/include/opencv4 -L/usr/local/lib64 -lopencv_core -lopencv_face -lopencv_highgui -lopencv_imgproc -lopencv_imgcodecs

CPLBPH: CPLBPH.cpp
	g++ CPLBPH.cpp -o CPLBPH -I/usr/local/include/opencv4 -L/usr/local/lib64 -lopencv_core -lopencv_face -lopencv_highgui -lopencv_imgproc -lopencv_imgcodecs

CPSVM: CPSVM.cpp
	g++ CPSVM.cpp -o CPSVM -I/usr/local/include/opencv4 -L/usr/local/lib64 -lopencv_core -lopencv_face -lopencv_imgproc -lopencv_imgcodecs -lopencv_highgui -lopencv_ml

clean:
	rm CPEF
	rm CPFF
	rm CPLBPH
	rm CPSVM

