#include <malloc.h>
#include <string.h>
#include <stdlib.h>
char* heap[8];
int sizes[8];

void print_menu() {
	puts("1. add note");
	puts("2. delete note");
	puts("3. edit note");
	puts("4. show note");
	puts("5. exit");
	puts("enter choice:");
}

void win() {
	system("/bin/sh");
}

int get_int() {
	char buffer[10];
	fgets(buffer, 10, stdin);
	return atoi(buffer);
}

void add_note() {
	int index = 0;
	int size = 0;
	puts("index:");
	index = get_int();
	if (index >= 0 && index <= 7) {
		puts("size:");
		size = get_int();
		heap[index] = (char*)malloc(size);
		sizes[index] = size;
		puts("content:");
		fgets(heap[index], size, stdin);
	}
	else {
		puts("Invalid index");
	}
}

void delete_note() {
	int index = 0;
	puts("index:");
	index = get_int();
	if (index >= 0 && index <= 7) {
		free(heap[index]);
		sizes[index] = 0;
	}
	else {
		puts("Invalid index");
	}
}

void edit_note() {
	int index = 0;
	puts("index:");
	index = get_int();
	if (index >= 0 && index <= 7) {
		fgets(heap[index], sizes[index], stdin);
	}
	else {
		puts("Invalid index");
	}
}

void show_note() {
	int index = 0;
	puts("index:");
	index = get_int();
	if (index >= 0 || index <= 7) {
		printf("%s", heap[index]);
	}
	else {
		puts("Invalid index");
	}
}

int main() {
	setbuf(stdout, NULL);
	setbuf(stdin, NULL);
	setbuf(stderr, NULL);
	while(1) {
		print_menu();
		int choice = get_int();
		if (choice == 1) {
			add_note();
		}
		else if (choice == 2) {
			delete_note();
		}
		else if (choice == 3) {
			edit_note();
		}
		else if (choice == 4) {
			show_note();
		}
		else if (choice == 5) {
			exit(0);
		}
		else {
			puts("Invalid choice");
		}
	}
}
