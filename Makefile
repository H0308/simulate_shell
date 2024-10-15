TARGET=myshell
SRC=myshell.cpp test.cpp

$(TARGET):$(SRC)
	g++ -o $(TARGET) $(SRC) -std=c++11

.PHONY:clean
clean:
	rm -f $(TARGET)
