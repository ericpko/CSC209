#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "hcq.h"
#define INPUT_BUFFER_SIZE 256

/*
 * Return a pointer to the struct curr_stu with name stu_name
 * or NULL if no curr_stu with this name exists in the stu_list
 */
Student *find_student(Student *stu_list, char *student_name) {
    for (Student *stu = stu_list; stu != NULL; stu = stu->next_overall) {
        if (strcmp(stu->name, student_name) == 0) {
            return stu;
        }
    }
    return NULL;
}



/*   Return a pointer to the ta with name ta_name or NULL
 *   if no such TA exists in ta_list.
 */
Ta *find_ta(Ta *ta_list, char *ta_name) {
    Ta *curr = ta_list;
    while (curr != NULL) {
        if (strcmp(curr->name, ta_name) == 0) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}


/*  Return a pointer to the course with this code in the course list
 *  or NULL if there is no course in the list with this code.
 */
Course *find_course(Course *courses, int num_courses, char *course_code) {
    for (int i = 0; i < num_courses; i++) {
        if (strcmp(courses[i].code, course_code) == 0) {
            return &courses[i];
        }
    }
    return NULL;
}


/* Initialize a new curr_stu and set their attributes.
 * Note: <last_stu_overall> could be NULL.
 */
Student *_init_student(Student *last_stu_overall, char *student_name, Course *course) {

    // create a new curr_stu
    Student *new_stu = malloc(sizeof(Student));
    if (new_stu == NULL) {
        perror("malloc for new student");
        exit(EXIT_FAILURE);
    }

    // allocate space for the students name
    new_stu->name = malloc(strlen(student_name) + 1);
    if (new_stu->name == NULL) {
        perror("malloc for student name");
        exit(EXIT_FAILURE);
    }
    strcpy(new_stu->name, student_name);
    new_stu->name[strlen(student_name)] = '\0';
    // strncpy(new_stu->name, student_name, strlen(student_name)); DOES NOT WORK!!!

    // set the students course they want help with
    new_stu->course = course;

    // set this students time of arrival
    new_stu->arrival_time = malloc(sizeof(time_t));
    if (new_stu->arrival_time == NULL) {
        perror("malloc for students arrival time");
        exit(EXIT_FAILURE);
    }
    *new_stu->arrival_time = time(NULL);

    // find the curr_stu at the tail waiting for this <course>
    Student *last_stu_course = course->tail; // could be NULL

    // update the last curr_stu overall and last curr_stu course
    if (last_stu_overall != NULL) {
        last_stu_overall->next_overall = new_stu;
    }
    if (last_stu_course != NULL) {
        last_stu_course->next_course = new_stu;
    }

    // update the courses tail and new_stu
    course->tail = new_stu;
    new_stu->next_course = NULL;
    new_stu->next_overall = NULL;

    return new_stu;
}


/* Add a curr_stu to the queue with student_name and a question about course_code.
 * if a curr_stu with this name already has a question in the queue (for any
   course), return 1 and do not create the curr_stu.
 * If course_code does not exist in the list, return 2 and do not create
 * the curr_stu struct.
 * For the purposes of this assignment, don't check anything about the
 * uniqueness of the name.
 */
int add_student(Student **stu_list_ptr, char *student_name, char *course_code,
                Course *course_array, int num_courses) {

    // check if a curr_stu with <student_name> is already in queue
    Student *curr_stu = *stu_list_ptr;          // curr
    Student *last_stu_overall = *stu_list_ptr;  // prev
    while (curr_stu != NULL) {
        if (strcmp(curr_stu->name, student_name) == 0) {
            return 1;
        }
        last_stu_overall = curr_stu;
        curr_stu = curr_stu->next_overall;
    }

    // check if <course_code> exists
    Course *course = find_course(course_array, num_courses, course_code);
    if (course == NULL) {
        return 2;
    }

    // create a new curr_stu and set attributes
    // <last_stu_overall> COULD be NULL
    Student *new_stu = _init_student(last_stu_overall, student_name, course);

    // case: no students are in the list yet
    if (*stu_list_ptr == NULL) {
        *stu_list_ptr = new_stu;
    }
    if (course->head == NULL) {
        course->head = new_stu;
    }

    return 0;
}


/* Student student_name has given up waiting and left the help centre
 * before being called by a Ta. Record the appropriate statistics, remove
 * the curr_stu from the queues and clean up any no-longer-needed memory.
 *
 * If there is no curr_stu by this name in the stu_list, return 1.
 */
int give_up_waiting(Student **stu_list_ptr, char *student_name) {

    // check if the curr_stu with <student_name> exists
    Student *prev_stu_overall = *stu_list_ptr;
    Student *curr_stu = *stu_list_ptr;
    while (curr_stu != NULL) {
        if (strcmp(curr_stu->name, student_name) == 0) {
            break;
        }
        prev_stu_overall = curr_stu;
        curr_stu = curr_stu->next_overall;
    }
    if (curr_stu == NULL) {
        return 1;
    }

    // find the course the student was waiting for
    Course *course = curr_stu->course;

    // find the previous student waiting for the course
    Student *prev_stu_course = NULL;
    curr_stu = course->head;
    while (curr_stu != NULL) {
        if (strcmp(curr_stu->name, student_name) == 0) {
            break;
        }
        prev_stu_course = curr_stu;
        curr_stu = curr_stu->next_course;
    }

    // update the course info
    course->bailed += 1;
    course->wait_time += difftime(time(NULL), (*curr_stu->arrival_time));


    // update the overall curr_stu list
    // cases: front, middle, back
    // case: front
    if (curr_stu == *stu_list_ptr) {
        // then curr_stu is at the head of the overall list
        *stu_list_ptr = curr_stu->next_overall;
        // there is no previous

    // case: middle, tail
    } else {
        // then curr_stu is either in the middle or the tail
        prev_stu_overall->next_overall = curr_stu->next_overall;
    }

    // update course student list
    // case: front
    if (course->head == curr_stu) {
        course->head = curr_stu->next_course;
        if (course->tail == curr_stu) {
            // then <curr_stu> is the head and tail
            course->tail = NULL;
        }
    } else if (course->tail == curr_stu) { // case: tail
        course->tail = prev_stu_course;
        prev_stu_course->next_course = NULL;
    } else { // case: middle
        prev_stu_course->next_course = curr_stu->next_course;
    }

    // deallocate heap memory
    free(curr_stu->name);
    free(curr_stu->arrival_time);
    free(curr_stu);
    return 0;
}


/* Create and prepend Ta with ta_name to the head of ta_list.
 * For the purposes of this assignment, assume that ta_name is unique
 * to the help centre and don't check it.
 */
void add_ta(Ta **ta_list_ptr, char *ta_name) {
    // first create the new Ta struct and populate
    Ta *new_ta = malloc(sizeof(Ta));
    if (new_ta == NULL) {
       perror("malloc for TA");
       exit(1);
    }
    new_ta->name = malloc(strlen(ta_name)+1);
    if (new_ta->name == NULL) {
       perror("malloc for TA name");
       exit(1);
    }
    strcpy(new_ta->name, ta_name);
    new_ta->current_student = NULL;

    // insert into front of list
    new_ta->next = *ta_list_ptr;
    *ta_list_ptr = new_ta;
}


/* The TA ta is done with their current curr_stu.
 * Calculate the stats (the times etc.) and then
 * free the memory for the curr_stu.
 * If the TA has no current curr_stu, do nothing.
 */
void release_current_student(Ta *ta) {

    if (ta->current_student != NULL) {
        ta->current_student->course->helped += 1;
        ta->current_student->course->help_time += difftime(time(NULL), *ta->current_student->arrival_time);
        free(ta->current_student->name);
        free(ta->current_student->arrival_time);
        free(ta->current_student);
    }
    ta->current_student = NULL;
}


/* Remove this Ta from the ta_list and free the associated memory with
 * both the Ta we are removing and the current curr_stu (if any).
 * Return 0 on success or 1 if this ta_name is not found in the list
 */
int remove_ta(Ta **ta_list_ptr, char *ta_name) {
    Ta *head = *ta_list_ptr;
    if (head == NULL) {
        return 1;
    } else if (strcmp(head->name, ta_name) == 0) {
        // TA is at the head so special case
        *ta_list_ptr = head->next;
        release_current_student(head);
        // memory for the curr_stu has been freed. Now free memory for the TA.
        free(head->name);
        free(head);
        return 0;
    }
    while (head->next != NULL) {
        if (strcmp(head->next->name, ta_name) == 0) {
            Ta *ta_tofree = head->next;
            //  We have found the ta to remove, but before we do that
            //  we need to finish with the curr_stu and free the curr_stu.
            //  You need to complete this helper function
            release_current_student(ta_tofree);

            head->next = head->next->next;
            // memory for the curr_stu has been freed. Now free memory for the TA.
            free(ta_tofree->name);
            free(ta_tofree);
            return 0;
        }
        head = head->next;
    }
    // if we reach here, the ta_name was not in the list
    return 1;
}


/* TA ta_name is finished with the curr_stu they are currently helping (if any)
 * and are assigned to the next curr_stu in the full queue.
 * If the queue is empty, then TA ta_name simply finishes with the curr_stu
 * they are currently helping, records appropriate statistics,
 * and sets current_student for this TA to NULL.
 * If ta_name is not in ta_list, return 1 and do nothing.
 */
int take_next_overall(char *ta_name, Ta *ta_list, Student **stu_list_ptr) {

    // check if ta_name is in the list
    Ta *ta = find_ta(ta_list, ta_name);
    if (ta == NULL) {
        return 1;
    }
    // check if the queue is empty
    if (*stu_list_ptr == NULL) {
        release_current_student(ta);
        return 0;
    }

    // find the next student in the queue overall
    Student *next_stu_overall = *stu_list_ptr;

    // update the front pointer
    *stu_list_ptr = next_stu_overall->next_overall; // could be NULL

    // release curr_stu if there is one and update stats
    if (ta->current_student != NULL) {
        release_current_student(ta);
    }

    // update the ta's current student
    ta->current_student = next_stu_overall;

    // update the courses wait time
    Course *course = next_stu_overall->course;
    course->wait_time += difftime(time(NULL), *next_stu_overall->arrival_time);

    // reset student's time with ta
    *next_stu_overall->arrival_time = time(NULL);

    // update the course
    course->head = next_stu_overall->next_course;
    if (course->tail == next_stu_overall) {
        course->tail = NULL;
    }

    // update students pointers
    next_stu_overall->next_course = NULL;
    next_stu_overall->next_overall = NULL;

    return 0;
}



/* TA ta_name is finished with the curr_stu they are currently helping (if any)
 * and are assigned to the next curr_stu in the course with this course_code.
 * If no curr_stu is waiting for this course, then TA ta_name simply finishes
 * with the curr_stu they are currently helping, records appropriate statistics,
 * and sets current_student for this TA to NULL.
 * If ta_name is not in ta_list, return 1 and do nothing.
 * If course is invalid return 2, but finish with any current curr_stu.
 */
int take_next_course(char *ta_name, Ta *ta_list, Student **stu_list_ptr, char *course_code, Course *courses, int num_courses) {

    // find the ta
    Ta *ta = find_ta(ta_list, ta_name);
    if (ta == NULL) {
        return 1;
    }

    // find the course
    Course *course = find_course(courses, num_courses, course_code);
    if (course == NULL) {
        release_current_student(ta);
        return 2;
    }

    // check if any student is waiting for this course
    if (course->head == NULL) { // no students are waiting
        release_current_student(ta);
        return 0;
    }

    // check if this ta currently has a student
    if (ta->current_student != NULL) {
        release_current_student(ta);
    }
    // else...
    // find the next student waiting for the course queue
    Student *next_stu_course = course->head;

    // update overall links
    Student *prev_student_overall = NULL;
    Student *curr_overall = *stu_list_ptr;
    while (curr_overall != NULL) {
        if (strcmp(curr_overall->name, course->head->name) == 0) {
            break;
        }
        prev_student_overall = curr_overall;
        curr_overall = curr_overall->next_overall;
    }
    if (prev_student_overall != NULL) {
        prev_student_overall->next_overall = next_stu_course->next_overall;
    } else {
        // update the front of the overall list
        *stu_list_ptr = course->head->next_overall;
    }

    // update the ta's current student
    ta->current_student = course->head;

    // update the courses wait time
    course->wait_time += difftime(time(NULL), *next_stu_course->arrival_time);

    // reset student's time with ta
    *next_stu_course->arrival_time = time(NULL);

    // update the course head and tail
    course->head = course->head->next_course; // might be NULL
    if (course->tail == next_stu_course) {
        course->tail = NULL;
    }

    // update students pointers
    next_stu_course->next_course = NULL;
    next_stu_course->next_overall = NULL;


    return 0;
}


/* Return the number of students in the queue for a certain course.
 *
 */
int get_queue_len(Course *course) {
    int queue_len = 0;
    if (course == NULL) {
        return queue_len;
    }
    for (Student *curr = course->head; curr != NULL; curr = curr->next_course) {
        queue_len++;
    }
    return queue_len;
}


/* For each course (in the same order as in the config file), print
 * the <course code>: <number of students waiting> "in queue\n" followed by
 * one course_des per curr_stu waiting with the format "\t%s\n" (tab name newline)
 * Uncomment and use the printf statements below. Only change the variable
 * names.
 */
void print_all_queues(Student *stu_list, Course *courses, int num_courses) {

    int num_stu_waiting = 0;
    for (int i = 0; i < num_courses; i++) {
        num_stu_waiting = get_queue_len(&courses[i]);
        printf("%s: %d in queue\n", courses[i].code, num_stu_waiting);

        for (Student *ta = courses[i].head; ta != NULL; ta = ta->next_course) {
            printf("\t%s\n", ta->name);
        }
    }
}


/*
 * Print to stdout, a list of each TA, who they are serving at from what course
 * Uncomment and use the printf statements
 */
void print_currently_serving(Ta *ta_list) {
    if (ta_list == NULL) {
        printf("No TAs are in the help centre.\n");
    }

    Student *curr_stu = NULL;
    Course *course = NULL;
    for (Ta *curr_ta = ta_list; curr_ta != NULL; curr_ta = curr_ta->next) {
        if (curr_ta->current_student != NULL) {
            curr_stu = curr_ta->current_student;
            course = curr_stu->course;
            printf("TA: %s is serving %s from %s\n", curr_ta->name, curr_stu->name, course->code);
        } else {
            printf("TA: %s has no curr_stu\n", curr_ta->name);
        }
    }
}


/*  list all students in queue (for testing and debugging)
 *   maybe suggest it is useful for debugging but not included in marking?
 */
void print_full_queue(Student *stu_list) {

    printf("All students in the queue:\n");
    for (Student *curr = stu_list; curr != NULL; curr = curr->next_overall) {
        if (curr->next_overall != NULL) {
            printf("%s -> ", curr->name);
        } else {
            printf("%s\n", curr->name);
        }
    }

    // printf("All students in each course queue:\n");
    // Student *next_stu_course = NULL;
    // for (Student *curr = stu_list; curr != NULL; curr = curr->next_overall) {
    //     next_stu_course = curr;
    //     while (next_stu_course != NULL) {
    //         if (next_stu_course->next_course != NULL) {
    //             printf("%s -> ", next_stu_course->name);
    //         } else {
    //             printf("%s\n", next_stu_course->name);
    //         }
    //         next_stu_course = next_stu_course->next_course;
    //     }
    // }
}


/* Prints statistics to stdout for course with this course_code
 * See example output from assignment handout for formatting.
 *
 */
int stats_by_course(Student *stu_list, char *course_code, Course *courses, int num_courses, Ta *ta_list) {

    // TODO: students will complete these next pieces but not all of this
    //       function since we want to provide the formatting
    Course *found = find_course(courses, num_courses, course_code);

    int students_waiting = 0;
    for (Student *stu = found->head; stu != NULL; stu = stu->next_course) {
        students_waiting++;
    }

    int students_being_helped = 0;
    for (Ta *ta = ta_list; ta != NULL; ta = ta->next) {
        if (ta->current_student != NULL) {
            if (strcmp(ta->current_student->course->code, course_code) == 0) {
                students_being_helped++;
            }
        }
    }

    // You MUST not change the following statements or your code
    //  will fail the testing.

    printf("%s:%s \n", found->code, found->description);
    printf("\t%d: waiting\n", students_waiting);
    printf("\t%d: being helped currently\n", students_being_helped);
    printf("\t%d: already helped\n", found->helped);
    printf("\t%d: gave_up\n", found->bailed);
    printf("\t%f: total time waiting\n", found->wait_time);
    printf("\t%f: total time helping\n", found->help_time);

    return 0;
}


/* Dynamically allocate space for the array course list and populate it
 * according to information in the configuration file config_filename
 * Return the number of courses in the array.
 * If the configuration file can not be opened, call perror() and exit.
 */
int config_course_list(Course **courselist_ptr, char *config_filename) {
    // try to open the file
    FILE *input_file = fopen(config_filename, "r");
    if (input_file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    // initialize variable for parsing file
    char course_des[INPUT_BUFFER_SIZE - 5];

    // read the first course_des and initialize array
    fgets(course_des, INPUT_BUFFER_SIZE - 6, input_file);
    int num_courses = strtol(course_des, NULL, 10);
    *courselist_ptr = malloc(num_courses * sizeof(Course));

    // read the rest of the file
    for (int i = 0; i < num_courses; i++) {
        if (fscanf(input_file, "%6s %[^\n]s", (*courselist_ptr)[i].code, course_des) != 2) {
            perror("error adding courses!");
            exit(EXIT_FAILURE);
        }

        (*courselist_ptr)[i].description = malloc(sizeof(char) * (strlen(course_des) + 1));
        strncpy((*courselist_ptr)[i].description, course_des, strlen(course_des));
        (*courselist_ptr)[i].description[strlen(course_des)] = '\0';    // for safety!

        // set up course defaults
        (*courselist_ptr)[i].head = NULL;
        (*courselist_ptr)[i].tail = NULL;
        (*courselist_ptr)[i].helped = 0;
        (*courselist_ptr)[i].bailed = 0;
        (*courselist_ptr)[i].wait_time = 0.0f;
        (*courselist_ptr)[i].help_time = 0.0f;
    }


    // close file and return
    fclose(input_file);
    return num_courses;
}
