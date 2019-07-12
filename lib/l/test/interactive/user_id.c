
/* $Id: user_id.c 4723 2009-11-16 18:57:09Z kobus $ */


#    include <stdio.h> 
#    include <unistd.h> 
#    include <pwd.h> 
#    include <sys/types.h> 


/* UNIX */ int main(void)
/* UNIX */ {
/* UNIX */     uid_t user_id_num;
/* UNIX */     struct passwd *user_passwd_ptr; 
/* UNIX */ 
/* UNIX */ 
/* UNIX */     user_id_num = getuid();
/* UNIX */ 
/* UNIX */     user_passwd_ptr = getpwuid(user_id_num); 
/* UNIX */ 
/* UNIX */     if (user_passwd_ptr == NULL) 
/* UNIX */     {
/* UNIX */         printf("Unable to determine user id.\n");
/* UNIX */         return 1;
/* UNIX */     }
               else
               {
/* UNIX */         printf("User id %s.\n", user_passwd_ptr->pw_name);
               }
               
               return 0;
/* UNIX */  } 


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

