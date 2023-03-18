
producer: producer.cpp headers.h
	g++ -I. producer.cpp -o producer 
consumer: consumer.cpp headers.h
	g++ -I. consumer.cpp -o consumer
main: main.cpp producer consumer headers.h
	g++ -I. main.cpp -o main
run: main
	./main
clean:
	rm -f main producer consumer consumer*.txt
.PHONY: run
