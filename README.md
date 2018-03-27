# ABpC
Automatic Brassica-pics Capture

## compile instructions (quite possibly a lot of things I don't need...)

``g++ -std=c++11 abcp.cpp -I../librealsense/examples/ -lrealsense2 -lopencv_core -lopencv_highgui -lGL -lGLU -lglut -lopencv_imgproc `pkg-config --libs opencv glfw3` -o abcp``
