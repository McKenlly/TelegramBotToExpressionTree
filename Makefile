FLAGS=-std=c++11 -pedantic -Wall -Werror -Wno-sign-compare -Wno-long-long -O2
CC=g++
LINK=-lm
BOT_FLAGS=-I/usr/local/include -lTgBot -lboost_system -lboost_iostreams -lssl -lcrypto -lpthread
#g++ telegram_bot.cpp -o telegram_bot --std=c++11 -I/usr/local/include -lTgBot -lboost_system -lboost_iostreams -lssl -lcrypto -lpthread

all: mainBot clear

mainBot:
	$(CC) $(FLAGS) $(BOT_FLAGS) parser.cpp TreeExpression.cpp -o telegram_bot --std=c++11 -I/usr/local/include -lTgBot -lboost_system -lboost_iostreams -lssl -lcrypto -lpthread

main:  parser.cpp
	$(CC) $(FLAGS)  -c parser.cpp
	$(CC) parser.cpp TreeExpression.o  -o parser $(FLAGS) $(BOT_FLAGS) parser.o TreeExpression.o 

expressionTree: TreeExpression.cpp
	$(CC) $(FLAGS) -c TreeExpression.cpp

clear:
	rm -f *.o
	rm -fr *.dSYM

delete_main:
	rm parser
