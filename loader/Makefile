COMPILER=gcc


project:
	@$(COMPILER) -c src/queue/*.c  -o bin/queue.o
	@echo "Built queue.o"
	@$(COMPILER) -c src/sockets/*.c  -o bin/sockets.o 
	@echo "Built sockets.o"
	$(COMPILER) -c src/process.c  -o bin/process.o
	@$(COMPILER) bin/sockets.o bin/process.o -o process
	@echo "Finished bulding process"
	$(COMPILER) -c src/main.c  -o bin/main.o 
	@$(COMPILER) bin/queue.o bin/sockets.o bin/main.o -o loader 
	@echo "Finished building loader"

testLinkedList:
	@$(COMPILER) -c src/linkedlist/*.c  -o bin/linkedlist.o 
	@echo "Built linkedlist.o"
	@$(COMPILER) -c tests/test_linked_list.c -o bin/test_linked_list.o
	@$(COMPILER) bin/linkedlist.o bin/test_linked_list.o -o test_linked_list 
	@echo "Finished building test_linked_list"
	@echo "Running test_linked_list"
	@./test_linked_list
testQueue:
	@$(COMPILER) -c src/queue/*.c  -o bin/queue.o 
	@echo "Built queue.o"
	@$(COMPILER) -c tests/test_queue.c -o bin/test_queue.o
	@$(COMPILER) bin/queue.o bin/test_queue.o -o test_queue
	@echo "Finished building test_queue"
	@echo "Running test_queue"
	@./test_queue
