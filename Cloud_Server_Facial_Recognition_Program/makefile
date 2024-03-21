all: CloudProgram EigenFace FisherFace LBPH CPEF CPFF CPLBPH CPSVM SVM CPSVMCOPY

CloudProgram: CloudProgram.cpp
	g++ CloudProgram.cpp -o CloudProgram

EigenFace: EigenFace.cpp
	g++ EigenFace.cpp -o EigenFace -L/home/ec2-user/opencv/build/lib -I/usr/local/include/opencv4 -lopencv_core -lopencv_face -lopencv_highgui -lopencv_imgproc -lopencv_imgcodecs

FisherFace: FisherFace.cpp
	g++ FisherFace.cpp -o FisherFace -I/usr/local/include/opencv4 -L/usr/local/lib64 -lopencv_core -lopencv_face -lopencv_highgui -lopencv_imgproc -lopencv_imgcodecs

LBPH: LBPH.cpp
	g++ LBPH.cpp -o LBPH -I/usr/local/include/opencv4 -L/usr/local/lib64 -lopencv_core -lopencv_face -lopencv_highgui -lopencv_imgproc -lopencv_imgcodecs

CPEF: CPEF.cpp
	g++ CPEF.cpp -o CPEF -I/usr/local/include/opencv4 -L/usr/local/lib64 -lopencv_core -lopencv_face -lopencv_highgui -lopencv_imgproc -lopencv_imgcodecs

CPFF: CPFF.cpp
	g++ CPFF.cpp -o CPFF -I/usr/local/include/opencv4 -L/usr/local/lib64 -lopencv_core -lopencv_face -lopencv_highgui -lopencv_imgproc -lopencv_imgcodecs

CPLBPH: CPLBPH.cpp
	g++ CPLBPH.cpp -o CPLBPH -I/usr/local/include/opencv4 -L/usr/local/lib64 -lopencv_core -lopencv_face -lopencv_highgui -lopencv_imgproc -lopencv_imgcodecs

CPSVM: CPSVM.cpp
	g++ CPSVM.cpp -o CPSVM -I/usr/local/include/opencv4 -L/usr/local/lib64 -lopencv_core -lopencv_face -lopencv_imgproc -lopencv_imgcodecs -lopencv_highgui -lopencv_ml

SVM: SVM.cpp
	g++ SVM.cpp -o SVM -I/usr/local/include/opencv4 -L/usr/local/lib64 -lopencv_core -lopencv_face -lopencv_imgproc -lopencv_imgcodecs -lopencv_highgui -lopencv_ml 

CPSVMCOPY: CPSVMCOPY.cpp
	g++ CPSVMCOPY.cpp -o CPSVMCOPY -I/usr/local/include/opencv4 -L/usr/local/lib64 -lopencv_core -lopencv_face -lopencv_imgproc -lopencv_imgcodecs -lopencv_highgui -lopencv_ml

clean:
	rm CloudProgram
	rm EigenFace
	rm FisherFace
	rm LBPH
	rm CPEF
	rm CPFF
	rm CPLBPH
	rm SVM
	rm CPSVM
	rm CPSVMCOPY

