#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void count_passing_students(const char *file_path, int num_tas, int pass_grade) {
    FILE *file = fopen(file_path, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    
    int num_students;
    fscanf(file, "%d", &num_students);
    int grades[num_students][2];
    
    for (int i = 0; i < num_students; i++) {
        fscanf(file, "%d %d", &grades[i][0], &grades[i][1]);
    }
    
    fclose(file);
    
    int students_per_ta = num_students / num_tas;
    int remaining_students = num_students % num_tas;
    
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        perror("Pipe failed");
        exit(EXIT_FAILURE);
    }
    
    for (int i = 0; i < num_tas; i++) {
        pid_t pid = fork();
        
        if (pid < 0) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            close(pipe_fd[0]); // Close read end in child
            
            int start = i * students_per_ta;
            int end = start + students_per_ta;
            if (i == num_tas - 1) {
                end += remaining_students;
            }
            
            int pass_count = 0;
            for (int j = start; j < end; j++) {
                if (grades[j][0] + grades[j][1] >= pass_grade) {
                    pass_count++;
                }
            }
            
            write(pipe_fd[1], &pass_count, sizeof(int));
            close(pipe_fd[1]);
            exit(EXIT_SUCCESS);
        }
    }
    
    close(pipe_fd[1]); // Close write end in parent
    
    int pass_results[num_tas];
    for (int i = 0; i < num_tas; i++) {
        wait(NULL);
        read(pipe_fd[0], &pass_results[i], sizeof(int));
    }
    close(pipe_fd[0]);
    
    for (int i = 0; i < num_tas; i++) {
        printf("%d%s", pass_results[i], (i == num_tas - 1) ? "\n" : " ");
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <file_path> <num_TAs> <pass_grade>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    const char *file_path = argv[1];
    int num_tas = atoi(argv[2]);
    int pass_grade = atoi(argv[3]);
    
    if (num_tas <= 0 || pass_grade <= 0) {
        fprintf(stderr, "Invalid input: num_TAs and pass_grade must be greater than 0\n");
        exit(EXIT_FAILURE);
    }
    
    count_passing_students(file_path, num_tas, pass_grade);
    return 0;
}
