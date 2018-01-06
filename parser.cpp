//
// Created by Oleg Morozenkov on 25.01.17.
//

#include <signal.h>
#include <stdio.h>
#include <exception>

#include <tgbot/tgbot.h>
#include "TreeExpression.h"
using namespace std;
using namespace TgBot;

bool sigintGot = false;
string introduce = "Hello from expression!\n" \
					"Now you must enter an aripthmetic expression with variables, numbers, round brackets and set of operations ('+', '-', '/' and '*')\n" \
					"Bot can format your expression on treeExpression" \
					"Please, enter expression. Run command /expression";
string uncorrectExpr = "Your expression is incorrect.";
void CompileToDot();
int main() {
	printf("Starting\n");
	Bot bot("347206610:AAHXMoWj0QTpvyWWXOPIhdzUbIi2TwZ0zdI");
	bot.getEvents().onCommand("start", [&bot](Message::Ptr message) {
		bot.getApi().sendMessage(message->chat->id, introduce);
	});

	bot.getEvents().onCommand("expression", [&bot](Message::Ptr message) {
		bot.getApi().sendMessage(message->chat->id, "Enter your expression:");
		bot.getEvents().onAnyMessage([&bot](Message::Ptr message) {
			Expression expression(message->text);
			if (!expression.correct()) {
				bot.getApi().sendMessage(message->chat->id, uncorrectExpr);
			} else {
				const string photoFilePath = "./Picture/default.png";
				const string photoMimeType = "image/png";
				Tree* tree = expression.createTree();
				expression.ToDot(tree);
				CompileToDot();
				bot.getApi().sendPhoto(message->chat->id, InputFile::fromFile(photoFilePath, photoMimeType));
				expression.deleteTree(&tree);
			}
		});
	});

	signal(SIGINT, [](int s) {
		printf("SIGINT got");
		sigintGot = true;
	});
	try {
		printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());

		TgLongPoll longPoll(bot);
		while (!sigintGot) {
			printf("Long poll started\n");
			longPoll.start();
		}
	} catch (exception& e) {
		printf("error: %s\n", e.what());
	}

	return 0;
}

void CompileToDot() {
    pid_t p;
    p = fork();
    if (p == 0) {
        execl("/bin/sh", "sh",  "./toDot.sh", (char *) 0);
        perror("execl failed");
    } else {
        return;
    }
}