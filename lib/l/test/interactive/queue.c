
/* $Id: queue.c 21653 2017-08-04 13:42:33Z kobus $ */



#include "l/l_incl.h" 

/* -------------------------------------------------------------------------- */

Queue_element* string_stack_head = NULL;
Queue_element* string_stack_tail = NULL;
Queue_element* string_list_head = NULL;
Queue_element* string_list_tail = NULL;
Queue_element* ordered_string_queue_head = NULL;
Queue_element* ordered_string_queue_tail = NULL;

/* -------------------------------------------------------------------------- */

static int build_ordered_string_queue  (char* input_string);
static int print_string_queue          (Queue_element* head);
static int print_string_queue_backwards(Queue_element* tail);
static int compare_queue_strings       (const void* str1, const void* str2);

/* -------------------------------------------------------------------------- */

/*ARGSUSED*/
int main(void)
{
    char           line[ 100 ];
    Queue_element* removed_queue_head;


    kjb_set_debug_level(5);

    set_io_atn_trap(); 
    dont_restart_on_atn(); 

    while ((BUFF_STDIN_GET_LINE(">", line)) != EOF) 
    {
        insert_into_queue(&string_stack_head, &string_stack_tail, 
                          kjb_strdup(line)); 

        insert_at_end_of_queue(&string_list_head, &string_list_tail, 
                               kjb_strdup(line)); 

        build_ordered_string_queue(line); 
    }

    pso("\nSTACK FORWARD\n"); 
    print_string_queue(string_stack_head);  

    pso("\nThis stack %s ordered.\n", is_queue_ordered(string_stack_head, compare_queue_strings) ? "is" : "is not");

    pso("\nSTACK BACKWARDS\n"); 
    print_string_queue_backwards(string_stack_tail);  




    pso("\nLIST FORWARD\n"); 
    print_string_queue(string_list_head);  

    pso("\nThis list %s ordered.\n", is_queue_ordered(string_list_head, compare_queue_strings) ? "is" : "is not");

    pso("\nLIST BACKWARDS\n"); 
    print_string_queue_backwards(string_list_tail);  



    pso("\nORDERED\n"); 
    print_string_queue(ordered_string_queue_head);  

    pso("\nThis queue %s ordered.\n", is_queue_ordered(ordered_string_queue_head, compare_queue_strings) ? "is" : "is not");

    pso("\nORDERED BACKWARDS\n"); 
    print_string_queue_backwards(ordered_string_queue_tail);  

    removed_queue_head = 
        remove_elements_less_than_key(&ordered_string_queue_head, &ordered_string_queue_tail,
                                      compare_queue_strings, "m"); 

    pso("\nORDERED REMOVED\n"); 
    print_string_queue(removed_queue_head);  

    pso("\nORDERED REST\n"); 
    print_string_queue(ordered_string_queue_head);  

    pso("\nORDERED REST BACKWARDS\n"); 
    print_string_queue_backwards(ordered_string_queue_tail);  


    free_queue(&removed_queue_head, (Queue_element**)NULL, kjb_free); 

    free_queue(&ordered_string_queue_head, &ordered_string_queue_tail, 
               kjb_free);
    free_queue(&string_stack_head, &string_stack_tail, kjb_free);
    free_queue(&string_list_head,  &string_list_tail, kjb_free); 

    return EXIT_SUCCESS; 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int build_ordered_string_queue(char *input_string)
{
    extern Queue_element *ordered_string_queue_head;
    extern Queue_element *ordered_string_queue_tail;
    char *input_string_copy; 

    input_string_copy = kjb_strdup(input_string); 

    insert_into_ordered_queue(&ordered_string_queue_head, 
                              &ordered_string_queue_tail, 
                              (void *)input_string_copy,
                              compare_queue_strings,
                              FALSE); 

    return NO_ERROR; 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int print_string_queue(Queue_element *head)
{
    Queue_element *cur_elem; 

    cur_elem = head; 

    /*
    dbx(cur_elem); 
    if (cur_elem) dbx(cur_elem->next); 
    if (cur_elem) dbx(cur_elem->previous); 
    */

    while (cur_elem != NULL) 
    {
        pso("String in queue is: %s\n", (char*)cur_elem->contents); 

        cur_elem = cur_elem->next; 

        /*
        if (cur_elem) dbx(cur_elem->next); 
        if (cur_elem) dbx(cur_elem->previous); 
        */
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int print_string_queue_backwards(Queue_element *tail)
{
    Queue_element *cur_elem; 

    cur_elem = tail; 

    /*
    dbx(cur_elem); 
    if (cur_elem) dbx(cur_elem->next); 
    if (cur_elem) dbx(cur_elem->previous); 
    */

    while (cur_elem != NULL) 
    {
        pso("String in queue is: %s\n", (char*)cur_elem->contents); 

        cur_elem = cur_elem->previous; 

        /*
        dbx(cur_elem); 
        if (cur_elem) dbx(cur_elem->next); 
        if (cur_elem) dbx(cur_elem->previous); 
        */
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int compare_queue_strings(const void *str1, const void *str2)
{

    return kjb_strcmp((const char*)str1, (const char*)str2);
}

