void print_string(char* file) {
    asm("li $v0, 4");
    asm("la $a0,($4)");							
    asm("syscall");
}

void read_string(char* file, int size) {
    asm("li $v0, 8");
    asm("syscall");
}

void create_process(char* asm_file) {
    asm("li $v0, 18"); // 18 defined as CREATE_PROCESS_SYSCALL in syscall.h
    asm("syscall");
}

// return file to be run if given string is run, 0 otherwise
char* is_run(char* str) {
    const char* run = "run ";
    char*s = str;
    while(*run) {
        if (*s != *run) return "\0";
        run++;
        s++;
    }
    // remove \n and return
    char*end = s;
    while(*end != '\n') ++end;
    *end = '\0';
    return s;
}

// return 1 if given string is exit, 0 otherwise
int is_exit(char* str) {
    const char* exit = "exit\n";
    char*s = str;
    while(*s) {
        if (*s != *exit) return 0;
        exit++;
        s++;
    }
    return 1;
}

int main(int argc, char *argv[]) {
	char input[256];
    char* file;

	while(1) {
		print_string("[shell]> ");
		read_string(input, 256);
        if (is_exit(input))
            break;
        file = is_run(input);
        if (*file != '\0' ) {
            create_process(file);
        } else {
            print_string("ERROR: command not found\n");
        }
	}
	return 0;
}
