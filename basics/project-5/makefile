# declaring the compiler and complier flags

CXX = g++
CFLAG = -Wall -Wextra -std=c++17

# after this comes the directory which contians the source and build files

SRC_DIR= ./src
BUILD_DIR= ./build
INCLUDE_DIR= ./include 

# Next comes the Source files and Objects files 

SRC = connection.cpp file_server.cpp main.cpp utils.cpp
OBJ = $(SRC:.cpp=.o)

# after we are going for main logic 

TARGET = file_transfer_server

# when we call the make from the cmd this will execute (starting point)

all: $BUILD_DIR $TARGET




