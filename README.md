cd shewchuk && make trilibrary && cd ..

g++ -O -I./shewchuk -o my_project my_project.cpp shewchuk/triangle.o -lm && ./my_project
