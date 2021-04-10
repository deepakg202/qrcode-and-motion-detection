# QR Code And Motion Detection
## Usage (For Debian/Ubuntu): 
- Open terminal and execute `sudo apt-get install libopencv-dev libzbar-dev`
- Clone the repo
- Cd into the repo 
- Execute ``g++ main.cpp -o main -lzbar `pkg-config --cflags --libs opencv4` ``
- Execute `./main`
