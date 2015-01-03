CC=gcc -g
CFLAGS=-Wall -ansi -std=c99 -Iinclude/ -pthread
DEBUG=-DDEBUG
LIBS=-lm
EXEC_NAME=simulation

INC_DIR=./include
OBJ_DIR=./obj
SRC_DIR=./src

OBJS=main.o \
	 factory.o \
	 kanban.o \
	 workstation.o \
	 simulation.o

OBJS_DIR=$(addprefix $(OBJ_DIR)/,$(OBJS))


$(EXEC_NAME): $(OBJS_DIR)
	@echo "Compiling $(OBJS) :"
	$(CC) $(CFLAGS) $(DEBUG) -o $@ $(OBJS_DIR) $(LIBS)
	@echo

release: mrproper unsetDebug $(EXEC_NAME)

unsetDebug:
	$(eval CC=gcc)
	$(eval DEBUG=)

$(OBJ_DIR)/main.o: $(SRC_DIR)/main.c $(INC_DIR)/simulation.h
	@echo "Compiling $< :"
	$(CC) $(CFLAGS) $(DEBUG) -c $< $(LIBS) -o $@
	@echo 

$(OBJ_DIR)/factory.o: $(SRC_DIR)/factory.c $(INC_DIR)/factory.h $(INC_DIR)/structures.h $(INC_DIR)/workstation.h
	@echo "Compiling $< :"
	$(CC) $(CFLAGS) $(DEBUG) -c $< $(LIBS) -o $@
	@echo 

$(OBJ_DIR)/kanban.o: $(SRC_DIR)/kanban.c $(INC_DIR)/kanban.h $(INC_DIR)/structures.h
	@echo "Compiling $< :"
	$(CC) $(CFLAGS) $(DEBUG) -c $< $(LIBS) -o $@
	@echo 

$(OBJ_DIR)/workstation.o: $(SRC_DIR)/workstation.c $(INC_DIR)/workstation.h $(INC_DIR)/structures.h $(INC_DIR)/kanban.h $(INC_DIR)/config.h
	@echo "Compiling $< :"
	$(CC) $(CFLAGS) $(DEBUG) -c $< $(LIBS) -o $@
	@echo 

$(OBJ_DIR)/simulation.o: $(SRC_DIR)/simulation.c $(INC_DIR)/simulation.h $(INC_DIR)/workstation.h $(INC_DIR)/factory.h $(INC_DIR)/kanban.h $(INC_DIR)/structures.h $(INC_DIR)/config.h
	@echo "Compiling $< :"
	$(CC) $(CFLAGS) $(DEBUG) -c $< $(LIBS) -o $@
	@echo

check-leak:
	valgrind --leak-check=full --show-reachable=yes ./$(EXEC_NAME)

clean:
	${RM} $(OBJ_DIR)/*.o

mrproper: clean
	${RM} ./$(EXEC_NAME)


