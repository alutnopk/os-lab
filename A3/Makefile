producer: producer.cpp
	g++ producer.cpp -o producer
consumer: consumer.cpp
	g++ consumer.cpp -o consumer
main: main.cpp producer consumer
	g++ main.cpp -o main
run: main
	./main
clean:
	rm producer main
.PHONY: run