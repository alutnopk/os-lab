producer: producer.cpp
	g++ producer.cpp -o producer
main: main.cpp producer
	g++ main.cpp -o main
run: main
	./main
clean:
	rm producer main
.PHONY: run