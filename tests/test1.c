#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>


struct test{
  int oflag;
  int cflag;
  int vflag;
  char *ivalue;
  char *ovalue;
  int c;
};

int main (int argc, char **argv)
{
  struct test B1={0,0,0,NULL,NULL,0};

  opterr = 0;

  while ((B1.c = getopt (argc, argv, "ho:i:cv")) != -1)
    switch (B1.c)
      {
      case 'h':
        printf("Need Help?\n");
        return EXIT_SUCCESS;
      break;
      case 'o':
      B1.oflag =1;
      B1.ovalue=optarg;
        break;
      case 'i':
      B1.ivalue=optarg;
        break;
      case 'c':
      B1.cflag = 1;
        break;
      case 'v':
      B1.vflag = 1;
        break;
      case '?':
        if (optopt == 'c')
          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        else if (isprint (optopt))
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr,
                   "Unknown option character `\\x%x'.\n",
                   optopt);
        return 1;
      default:
        abort ();
      }

if(B1.oflag==0) //optional
{
   B1.ovalue=B1.ivalue;
}

printf ("oflag = %d, cflag = %d, vflag = %d, ivalue = %s, ovalue = %s\n",B1.oflag,B1.cflag,B1.vflag,B1.ivalue,B1.ovalue);
  
  return 0;
}


















