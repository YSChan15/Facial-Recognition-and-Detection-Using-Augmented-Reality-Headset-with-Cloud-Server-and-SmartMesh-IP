### Setting up the cloud server
-  Ensure that all the cloud programs are installed to a directory on the server. In each of the programs, modify the IP Address and Ports to your needs. Modify the path to the database and database file also.

-  Ensure that a database directory is located inside of this directory. Each image in this file must be a cropped face named by the person and the number of the image (i.e. the 19th image of Brycen is Brycen19.jpg).

-  Create a database text file named "Database.txt" to be used by the cloud programs. This file must contain the complete paths to each file in the database along with the label associated with them. In this project, Brycen is 0, Yu Sheng is 1, Brandon is 2, and Hayden is 3. An example line in the file is "<path_to_file>/Brycen19.jpg;0".

-  To compile all the programs, run the "make" command.

-  To run the Support Vector Machine facial recognition cloud server program, run "./CPSVM".

-  To run the Local Binary Patterns Histograms facial recognition cloud server program, run "./CPLBPH".

-  To run the Eigenface facial recognition cloud server program, run "./CPEF".

-  To run the Fisherface facial recognition cloud server program, run "./CPFF".

-  Ensure that this program is running on the cloud server before running the program on the AR headset and the Wireless Sensor Network.