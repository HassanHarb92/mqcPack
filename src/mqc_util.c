/* C Example */
#include <math.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <sys/wait.h>
/*
#define DEBUG
*/
typedef struct Input_Link_List {
  char *next;
  char *last;
  int Blank;
  int Keyword;
  int Link1;
  char *Name;
  char *Lower_Name;
} Input_Link_List;

void mqc_LOWER( char * );
void mqc_Remove_Init_WhiteSpace( char * );
int mqc_FINDstrINline( char *, char * );
void mqc_File_Name_list( char *, char *, int );
#ifdef MATRIXFILE
void mqc_get_MatrixFile_Name(char *, int);
#endif
void mqc_FormChk( char *, char *, int );
void mqc_get_FormChk_Name(char *, int);
void mqc_Global_init( void );
char *mqc_DupString( char *, char * );
Input_Link_List *mqc_Alloc_Input_Link_List( char *, Input_Link_List *, int );
void mqc_Write_Input_Link_List( char *, Input_Link_List * );
void mqc_Free_Input_Link_List( Input_Link_List * );
void mqc_Remove_Return( char * );
char *mqc_Replace_String( char *, char *, char * );
void mqc_create_File_Name_lists(char *, char *, int);
void mqc_error_i_c2f_0( char *, int * ); 
void print_line_c2f ( char *, int * ); 
void flush_c2f( int * );
void mqc_File_Exists( char *, int * );
void mqc_File_Is_Executable( char *, int *, int );
void mqc_get_scratch_dir( int );
int mqc_ASCII_or_Binary( char *, char *, int );
Input_Link_List *mqc_Read_to_Link_List( char *, char *, int, int *, int*, int * );

char CarriageReturn[8];
char EndOfString[8];
char Tab[8];
int int_tolower;
Input_Link_List *Last_Keyword = (Input_Link_List *)NULL;
Input_Link_List *Last_NonBlank = (Input_Link_List *)NULL;
#ifdef MATRIXFILE
Input_Link_List *Start_Mat = (Input_Link_List *)NULL;
Input_Link_List *Current_Mat = (Input_Link_List *)NULL;
#endif
Input_Link_List *Start_FChk = (Input_Link_List *)NULL;
Input_Link_List *Current_FChk = (Input_Link_List *)NULL;
 char mqc_scratch_dir[2048];

#ifdef MATRIXFILE
void mqc_get_MatrixFile_Name(char *FileName, int iout)
{
  static Input_Link_List *Current_Link_List = (Input_Link_List *)NULL;
  Input_Link_List *Next_Link_List;
  int i;
  char charBuf[2048];

  if( Current_Link_List == (Input_Link_List *)NULL ) {
    if( Start_FChk == (Input_Link_List *)NULL && Start_Mat == (Input_Link_List *)NULL ) {
      sprintf( charBuf,"Input_Link_List: Trying to extract Filenames, but empty file list.  Check that mqc_create_File_Name_lists_F2C has been previously called.%s", EndOfString);
      mqc_error_i_c2f_0(charBuf, &iout);
    }
    Current_Link_List = Start_Mat;
  } else { 
    Next_Link_List = (Input_Link_List *)Current_Link_List->next;
    Current_Link_List = Next_Link_List;
  }
  if( Current_Link_List != (Input_Link_List *)NULL ) {
    mqc_Remove_Return( Current_Link_List->Name );
    i = strlen( Current_Link_List->Name );
    strcpy( FileName, Current_Link_List->Name );
  } else {
    strcpy( FileName, "Done with MatrixFiles" );
    i = strlen( FileName );
  }
  FileName[i] = (char) 0;
}
#endif

void mqc_get_FormChk_Name(char *FileName, int iout)
{
  static Input_Link_List *Current_Link_List = (Input_Link_List *)NULL;
  Input_Link_List *Next_Link_List;
  int i;
  char charBuf[2048];

  if( Current_Link_List == (Input_Link_List *)NULL ) {
#ifdef MATRIXFILE
    if( Start_FChk == (Input_Link_List *)NULL && Start_Mat == (Input_Link_List *)NULL ) {
#else
    if( Start_FChk == (Input_Link_List *)NULL ) {
#endif
      sprintf( charBuf,"Input_Link_List: Trying to extract Filenames, but empty file list.  Check that mqc_create_File_Name_lists_F2C has been previously called.%s", EndOfString);
      mqc_error_i_c2f_0(charBuf, &iout);
    }
    Current_Link_List = Start_FChk;
  } else { 
    Next_Link_List = (Input_Link_List *)Current_Link_List->next;
    Current_Link_List = Next_Link_List;
  }
  if( Current_Link_List != (Input_Link_List *)NULL ) {
    mqc_Remove_Return( Current_Link_List->Name );
    i = strlen( Current_Link_List->Name );
    strcpy( FileName, Current_Link_List->Name );
  } else {
    strcpy( FileName, "Done with FormChk" );
    i = strlen( FileName );
  }
  FileName[i] = (char) 0;
}

void mqc_create_File_Name_lists(char *FileName, char *Program, int iout)
{
  static int mqc_first_call=0;

  if ( mqc_first_call == 0 ){
    mqc_Global_init( );
    mqc_first_call=1;
  }
  mqc_File_Name_list( FileName, Program, iout);
}

void mqc_File_Name_list( char *FileName, char *Program, int iout )
{
  char *MatFileName;
  char charBuf[4096];
  char JobName[1024];
  char InputFile[1024];
  char *rtn_fgets;
  char prnt_str[2048];
  int fclose_return;
  int rtn_fputs;
  int Restart;
  int last_job;
  int Current_job;
  int system_failure;
  int i, j, ilen;
  int NoModNeeded;
  int Status;
  Input_Link_List *Start_Link_List;
  Input_Link_List *Current_Link_List;
  Input_Link_List *Last_Link_List;
  Input_Link_List *Next_Link_List;

  rtn_fgets = (char *)NULL;
  last_job=-1;

  /*                              */
  /* Check that input file exists */
  /*                              */
  mqc_File_Exists( FileName, &i );
  if ( i == 0 ) {
    sprintf( charBuf, "The file %s either does not exist, or cannot be read%s", FileName, EndOfString);
    mqc_error_i_c2f_0( charBuf, &iout);
  }
  /*                   */
  /* Input file exists */
  /*                   */

  /* */
  /* Is the Input file ASCII or binary? */
  /* */
  i = mqc_ASCII_or_Binary( FileName, charBuf, iout );

  if ( i == 1 ) {
    /* */
    /* The input file is Binary, not ACSII. It is a MatrixFile */
    /* */
#ifdef DEBUG
    sprintf( prnt_str, "%s is binary, so a MatrixFile%s",FileName, EndOfString );
    print_line_c2f ( prnt_str, &iout ); 
    flush_c2f( &iout );
#endif
    MatFileName = mqc_DupString(FileName, (char *)NULL);
    /* Add name of Martix file to link list */
    /* Need check for Fchk file here */
#ifdef MATRIXFILE
    Current_Mat= mqc_Alloc_Input_Link_List( MatFileName, Current_Mat, iout );
    if ( Start_Mat == (Input_Link_List *)NULL ) {
      Start_Mat = Current_Mat;
    }
#else
    Current_FChk= mqc_Alloc_Input_Link_List( MatFileName, Current_FChk, iout );
    if ( Start_FChk == (Input_Link_List *)NULL ) {
      Start_FChk = Current_FChk;
    }
#endif
    return;	     
  }
  /* */
  /* The input file is ACSII. It is a not MatrixFile */
  /* */

  /*                                             */
  /* Check that Program is in path and executable*/
  /*                                             */
  mqc_File_Is_Executable( Program, &Status, iout );

  if ( Status == 0 ) {
    sprintf( charBuf, "Did not find %s in the path%s", Program, EndOfString);
    mqc_error_i_c2f_0( charBuf, &iout); 
  }

  /*                                  */
  /* Program is in path and executable*/
  /*                                  */
#ifdef DEBUG
  sprintf( prnt_str, "Open Gaussian input file%s", EndOfString );
  print_line_c2f ( prnt_str, &iout ); 
  flush_c2f( &iout );
#endif
  /* */
  /* Read the Input file */
  /* */
  Start_Link_List = mqc_Read_to_Link_List( FileName, charBuf, iout,
					&Current_job, &last_job, &Restart );

  /* */
  /* Done Reading in the input file */
  /* */

  /* Get the JobName, this will be used, if we need to insert changes to */
  /* save FormChk and Matrix Files */
  strcpy( JobName, FileName );
  ilen = strlen( JobName );
  if ( strncmp( &JobName[ilen-4], ".com", 4 ) == 0 ) {
    strcpy( &JobName[ilen-4], EndOfString );
  }
  /* We have the JobName */

#ifdef MATRIXFILE
  NoModNeeded = 1;
  if ( Current_job == 0 && last_job == -1 ) {
    if ( Restart != 1 ) {
#ifdef DEBUG
      sprintf( prnt_str, "WARNING: This input does not write a MartrixFile.%s", EndOfString );
      print_line_c2f ( prnt_str, &iout );
#endif
      NoModNeeded = 0;
    }
  } else if ( last_job == 0 ) {
#ifdef DEBUG
    sprintf( prnt_str, "WARNING: This input writes a MartrixFile, but the last job does not write the MatrixFile.%s", EndOfString );
    print_line_c2f ( prnt_str, &iout );
#endif
    NoModNeeded = 0;
  }
#ifdef DEBUG
  sprintf( prnt_str, " NoModNeeded pass 1 %d%s", NoModNeeded, EndOfString );
  print_line_c2f ( prnt_str, &iout ); 
  flush_c2f( &iout );
#endif
  /* */
  if ( NoModNeeded == 0 ) {
    mqc_Remove_Return( Last_Keyword->Name );
    sprintf( charBuf, "%s output=mat\n", Last_Keyword->Name );
    Last_Keyword->Name = mqc_DupString(charBuf, Last_Keyword->Name);
  }
#endif

  Last_Link_List = (Input_Link_List *)Last_NonBlank->last;
  if ( Last_Link_List->Blank == 0
       || mqc_FINDstrINline( " ", Last_Link_List->Name ) != -1 ){
    NoModNeeded = 0;
  }
#ifdef DEBUG
  sprintf( prnt_str, "before nonblank \"%s\"%s", Last_Link_List->Name, EndOfString);
  print_line_c2f ( prnt_str, &iout ); 
  sprintf( prnt_str, "Blank %d%s", Last_Link_List->Blank, EndOfString );
  print_line_c2f ( prnt_str, &iout ); 
  sprintf( prnt_str, "blank \"%s\"%s", Last_NonBlank->Name, EndOfString );
  print_line_c2f ( prnt_str, &iout ); 
  sprintf( prnt_str, " NoModNeeded pass 2 %d%s", NoModNeeded, EndOfString );
  print_line_c2f ( prnt_str, &iout ); 
  flush_c2f( &iout );
#endif
#ifdef MATRIXFILE
  /* */
  /* This just puts the MatrixFile name at the end of the input.  Or if */
  /* there is a Matrix file there already, it appends the name to the list */
  /* of Matrix files.  */
  /* This needs to be extended to do this to the end of every job. */
  /* also this is not needed if we are not working with MatrixFiles */
  /* */
  if ( NoModNeeded == 0 ) {
    /* Append a blank line, MatrixFile Name, blank line behind the */
    /* last non-blank line */
    sprintf( charBuf, "\n%s.mat\n\n", JobName );
    Current_Link_List = mqc_Alloc_Input_Link_List( charBuf, Last_NonBlank, iout );
    sprintf( charBuf, "%s.mat", JobName );
    MatFileName = mqc_DupString( charBuf, (char *)NULL);
    sprintf( InputFile, "%s.mod", JobName );
    mqc_Write_Input_Link_List( InputFile, Start_Link_List );
    /* Add name of Martix file to link list */
    Current_Mat = mqc_Alloc_Input_Link_List( MatFileName, Current_Mat, iout );
  } else {
    strcpy( InputFile, FileName );
    MatFileName = mqc_DupString( Last_NonBlank->Name, (char *)NULL);
    /* Add name of Martix file to link list */
    Current_Mat = mqc_Alloc_Input_Link_List( MatFileName, Current_Mat, iout );
  }
  if ( Start_Mat == (Input_Link_List *)NULL ) {
    Start_Mat = Current_Mat;
  }
#endif
  /* */
  /* Done appending MatrixFile name to the end of the last job */
  /* */
  mqc_Free_Input_Link_List( Start_Link_List );
  sprintf( charBuf, "%s < %s >%s.log", Program, InputFile, JobName );

#ifdef DEBUG
  sprintf( prnt_str, "execute %s%s", charBuf, EndOfString );
  print_line_c2f ( prnt_str, &iout ); 
  flush_c2f( &iout );
#endif
  /* MatrixFiles are not readable just after the job finishes. */
  /* The fork is an attempt to distance this process from the jobs. */
  system_failure = system(charBuf);
  if ( system_failure == -1 ) {
    sprintf( prnt_str, " Job failed. Command was:\n%s%s", charBuf, EndOfString );
    print_line_c2f ( prnt_str, &iout ); 
  }
  mqc_Remove_Return (MatFileName);
  /* Check that the job produced the MatrixFile */
  /* Does the MatrixFile exist? */
  mqc_File_Exists( MatFileName, &Status );
  if ( Status == 0 ) {
    sprintf( charBuf, "MatrixFile %s does not exist%s%s", MatFileName, EndOfString);
    mqc_error_i_c2f_0( charBuf, &iout);
  }
  /* */
  /* Is the Input file ASCII or binary? */
  /* */
  i = mqc_ASCII_or_Binary( MatFileName, charBuf, iout );

  if ( i != 1 ) {
    /* */
    /* The input file is ACSII. It is a MatrixFile */
    /* */
    sprintf( charBuf, "MatrixFile %s does not ASCII %s%s", MatFileName, EndOfString);
    mqc_error_i_c2f_0( charBuf, &iout);
  }

  return;
}

void mqc_Remove_Init_WhiteSpace( char *charBuf )
{
  while( strncmp( &charBuf[0], " ", 1) == 0 ||
	 strncmp( &charBuf[0], Tab, 1) == 0 ) 
	 strcpy( charBuf, &charBuf[1] );
  while( strncmp( &charBuf[0], "\n", 1) == 0 ) 
    strcpy( charBuf, &charBuf[1] );
  while( strncmp( charBuf, CarriageReturn, 1) == 0 ) 
    strcpy( charBuf, &charBuf[1] );

  return;
}

void mqc_Remove_Return( char *charBuf )
{
  int ilen;

  ilen = strlen(charBuf);
  while( strncmp( &charBuf[ilen-1], "\n", 1) == 0 ||
	 strncmp( &charBuf[ilen-1], CarriageReturn, 1) == 0 ) {
    strcpy( &charBuf[ilen-1], EndOfString );
    ilen = strlen(charBuf);
  }

  return;
}

void mqc_LOWER( char *charBuf )
{
  int i, len;
  int ucase;

  len = strlen( charBuf );
  for ( i=0; i < len; i++ ) {
    if( isupper( (int)charBuf[i] ) ) {
      ucase = (int)charBuf[i];
      charBuf[i] = (char) (ucase+int_tolower);
    }
  }
  return;
}

int mqc_FINDstrINline( char *string, char *line )
{
  int i,imax;
  int stringlen;

  stringlen = (int) strlen( string );
  if ( stringlen == 0 ) return(-1);
  imax = (int) strlen( line ) - stringlen + 1;
  for( i=0 ;  i < imax ; i++ ) {
    if ( 0==strncmp ( &line[i], string, (size_t)stringlen ) ) {
      return(i);
    }
  }
  return(-1);
}

void mqc_Global_init(void)
{
  char charBuf[1024];

  CarriageReturn[0]=(char)13;
  CarriageReturn[1]=(char)0; 
  /* Append EndOfString to end of all strings that go to FORTRAN */
  /* FORTRAN needs this.  It should be always there, but sometimes it's not */
  EndOfString[0]=(char)0; 
  strcpy( charBuf, "aA");
  int_tolower = (int)charBuf[0] - (int)charBuf[1];
}

char *mqc_DupString( char *old_str, char *free_this )
{
  char *new_str=NULL;
  int ilen;

  ilen = strlen( old_str );
  new_str = malloc( ilen+8 );
  strcpy( new_str, old_str );
  if (free_this != (char *)NULL) {
    free(free_this);
  }
  return( new_str );
}

Input_Link_List *mqc_Alloc_Input_Link_List( char *Input_Link_List_name, Input_Link_List *LAST_Input_Link_List_struct, int iout )
{
  static int Blank_count = 0;
  char charBuf[2048];

  Input_Link_List *new_struct = (Input_Link_List *)NULL;

  new_struct = (Input_Link_List *)malloc ( (size_t)sizeof( Input_Link_List )+128);
  if ( new_struct == (Input_Link_List *)NULL ) {
    sprintf( charBuf,"Error: Not able to allocate memory in mqc_Alloc_Input_Link_List.%s", EndOfString);
    mqc_error_i_c2f_0(charBuf, &iout);
  }
  if ( LAST_Input_Link_List_struct == (Input_Link_List *)NULL ) {
    new_struct->last = (char *)NULL;
    new_struct->next = (char *)NULL;
  } else {
    new_struct->next = LAST_Input_Link_List_struct->next;
    LAST_Input_Link_List_struct->next = (char *)new_struct;
    new_struct->last = (char *)LAST_Input_Link_List_struct;
  }
  new_struct->Name = mqc_DupString(Input_Link_List_name, (char *)NULL) ;
  mqc_Remove_Init_WhiteSpace( Input_Link_List_name );
  new_struct->Lower_Name = mqc_DupString(Input_Link_List_name, (char *)NULL) ;
  mqc_LOWER( new_struct->Lower_Name );

  new_struct->Keyword = 0;
  if ( strlen( new_struct->Lower_Name ) == 0 ) {
    /* when this is really smart, we will know what the input is by the */
    /* location between blank lines and the keywords. */
    Blank_count++;
    new_struct->Blank = Blank_count;
  } else {
    new_struct->Blank = 0;
    new_struct->Link1 = 0;
    Last_NonBlank = new_struct;
    /* The very last line in the input is the name of the MartixFile, */
    /* if we are writing one.  If we aren't writing one, need to append a */
    /* blank line and then the MartixFile name afterwards. */
    if ( strncmp( new_struct->Lower_Name, "#", 1 ) == 0 ) {
      new_struct->Keyword = 1;
      Last_Keyword = new_struct;
      /* We may need to append a keyword to cause a write of a Martrix file */
    } else if ( mqc_FINDstrINline( "--link1--", new_struct->Lower_Name ) != -1 ) {
      /* New job */
      Blank_count=0;
      new_struct->Link1 = 1;
    }
  }

  return(new_struct);
}

void mqc_Free_Input_Link_List( Input_Link_List *Start_Input_Link_List )
{
  Input_Link_List *Current_Input_Link_List = (Input_Link_List *)NULL;
  Input_Link_List *Next_Input_Link_List = (Input_Link_List *)NULL;

  Current_Input_Link_List = Start_Input_Link_List;
  while (Current_Input_Link_List != (Input_Link_List *)NULL ) {
    Next_Input_Link_List = (Input_Link_List *)Current_Input_Link_List->next;
    /* Free the strings */
    free(Current_Input_Link_List->Name);
    free(Current_Input_Link_List->Lower_Name);
    /* Set values to NULL, just in case we accidently to use a */
    /* freed structure */
    Current_Input_Link_List->Name = (char *)NULL;
    Current_Input_Link_List->Lower_Name = (char *)NULL;
    Current_Input_Link_List->next=NULL;
    Current_Input_Link_List->last=NULL;
    /* Free the structure */
    free(Current_Input_Link_List);
    Current_Input_Link_List = Next_Input_Link_List;
  }
}

void mqc_Write_Input_Link_List( char *File_Name, Input_Link_List *Start_Input_Link_List )
{
  FILE *MY_Write_File=NULL;
  int fclose_return;
  int rtn_fputs;
  Input_Link_List *Current_Input_Link_List = (Input_Link_List *)NULL;
  Input_Link_List *Next_Input_Link_List = (Input_Link_List *)NULL;

  MY_Write_File = fopen( File_Name,"w");
  Current_Input_Link_List = Start_Input_Link_List;
  while (Current_Input_Link_List != (Input_Link_List *)NULL ) {
    Next_Input_Link_List = (Input_Link_List *)Current_Input_Link_List->next;
    rtn_fputs=fputs( Current_Input_Link_List->Name,MY_Write_File);
    Current_Input_Link_List = Next_Input_Link_List;
  }
  fclose_return = fclose(MY_Write_File);
}

char *mqc_Replace_String( char *Find, char *Insert, char *Line )
{
  int Flen, Ilen;
  int i;
  char *Return_Line;

  i = mqc_FINDstrINline( Find, Line );
  if ( i != -1 ) {
    Ilen = strlen( Insert );
    Return_Line = malloc( strlen( Line ) + Ilen + 80 );
    strncpy( Return_Line, Line, i );
    strncpy( &Return_Line[i], Insert, Ilen );
    strcpy( &Return_Line[i+Ilen], &Line[i+strlen(Find)] );
    free( Line );
    return( mqc_Replace_String( Find, Insert, Return_Line ) );
  }
  return( Line );
}

/* */
/* routine that calls abort. This is used when an error occurs. */
/* Abort is useful, because is causes a stack trace. */
/* */
void mqc_abort_(void)
{
  fflush(stdout);
  fflush(stderr);
  abort();
}
/* */
/* routine that checks if a file exists */
/* */
void mqc_File_Exists( char *FileName, int *Status )
{
  if( access( FileName, F_OK ) != -1 ) {
     Status[0] = 1;
   } else {
     Status[0] = 0;
   }
}

/* */
/* routine that checks if a file is executable */
/* */
void mqc_File_Is_Executable( char *Program, int *Status, int iout )
{
  char charBuf[2048];
  char prnt_str[2048];
  FILE *Command_File=NULL;
  FILE *Results_File=NULL;
  char Command_File_Name[2048];
  char Command_Results[2048];
  char *rtn_fgets;
  int PID;
  int fclose_return;
  int rtn_fputs;
  int system_failure;

#ifdef DEBUG
  sprintf( prnt_str, "Check if the Program is the name of an executable which is in the path%s", EndOfString );
  print_line_c2f ( prnt_str, &iout ); 
  flush_c2f( &iout );
#endif
  PID = (int)getpid();
  mqc_get_scratch_dir( iout );
  sprintf( Command_File_Name, "%s/mqc_Command_%d", mqc_scratch_dir, PID);

  sprintf( Command_Results, "%s/mqc_Results_%d", mqc_scratch_dir, PID);
  sprintf( charBuf, "#! /bin/bash -f\nif command -v %s > mqc_tmp_file 2>&1; then\n echo 1 > %s\nelse\n echo 2 > %s\nfi\nexit\n", Program, Command_Results, Command_Results );

  Command_File = fopen( Command_File_Name,"w");
  if ( Command_File == (FILE *)NULL ) {
    sprintf(charBuf, "Failured to open %s%s", Command_File_Name, EndOfString );
    mqc_error_i_c2f_0( charBuf, &iout); 
  }
  rtn_fputs=fputs(charBuf,Command_File);
  fclose_return = fclose(Command_File);
  Command_File = NULL;
  sprintf( charBuf, "chmod 755 %s;%s%s", Command_File_Name, Command_File_Name, EndOfString );

#ifdef DEBUG
  sprintf( prnt_str, "execute %s%s", charBuf, EndOfString );
  print_line_c2f ( prnt_str, &iout ); 
  flush_c2f( &iout );
#endif
  /* Make the command File executable */
  system_failure = system(charBuf);
  unlink("mqc_tmp_file");
  unlink(Command_File_Name);
  if ( system_failure == -1 ) {
    sprintf(charBuf, "Failure in Script to determine if \"%s\" is an executable%s", Program, EndOfString );
    mqc_error_i_c2f_0( charBuf, &iout); 
  }

  Results_File = fopen( Command_Results,"r");
  if ( Results_File == (FILE *)NULL ) {
    sprintf(charBuf, "Failured to open %s%s", Command_Results, EndOfString );
    mqc_error_i_c2f_0( charBuf, &iout); 
  }
  rtn_fgets=fgets(charBuf,1024,Results_File);
  fclose_return = fclose(Results_File);
  Results_File = NULL;
  unlink(Command_Results);

  if ( strncmp( charBuf, "1", 1) != 0 ) {
    Status[0] = 0;
  } else {
    Status[0] = 1;
  }

}

void mqc_get_scratch_dir( int iout )
{
  char *scrdir;
  int first_call=1;
  if ( first_call != 1 ) return;
  first_call=0;
  scrdir=getenv( "GAUSS_SCRDIR" );
  if ( scrdir == NULL ) {
    strcpy( mqc_scratch_dir, ".");
  } else {
    strcpy( mqc_scratch_dir, scrdir);
  }
  return;
}

int mqc_ASCII_or_Binary( char *FileName, char *charBuf, int iout )
{
  char AorB_Command_File_Name[1024];
  char *rtn_fgets;
  char prnt_str[2048];
  FILE *AorB_Command_File=NULL;
  FILE *Input_File=NULL;
  int fclose_return;
  int PID;
  int rtn_fputs;
  int system_failure;

  rtn_fgets = (char *)NULL;

#ifdef DEBUG
  sprintf( prnt_str, "Open file to store temporaries.  This should be deleted when leave%s", EndOfString );
  print_line_c2f ( prnt_str, &iout ); 
  flush_c2f( &iout );
#endif
  PID = (int)getpid();
  mqc_get_scratch_dir( iout );
  sprintf( AorB_Command_File_Name, "%s/mqc_AorB_Command_%d", mqc_scratch_dir, PID);
#ifdef DEBUG
  sprintf( prnt_str, "Check if %s is the name of a binary MatrixFile or if it is an ASCII Gaussian input file%s", FileName, EndOfString );
  print_line_c2f ( prnt_str, &iout ); 
  flush_c2f( &iout );
#endif
  sprintf( charBuf, "file %s > %s\n", FileName, AorB_Command_File_Name );
#ifdef DEBUG
  sprintf( prnt_str, "execute %s%s", charBuf, EndOfString );
  print_line_c2f ( prnt_str, &iout ); 
  flush_c2f( &iout );
#endif
  system_failure = system(charBuf);

  if ( system_failure == -1 ) {
    /* the file on the First input failed.  Check if the file exists. */
    Input_File = fopen( FileName,"r");
    if ( Input_File == NULL ) {
      sprintf( charBuf, "Could not open input file: %s%s", FileName, EndOfString );
    } else {
      sprintf( charBuf, "file command on %s failed.  Tried to do this in the Bash shell%s", FileName, EndOfString );
    }
    fclose_return = fclose(Input_File);
    unlink(AorB_Command_File_Name);
    mqc_error_i_c2f_0( charBuf, &iout); 
  }

  AorB_Command_File = fopen( AorB_Command_File_Name,"r");
  if ( AorB_Command_File == NULL ) {
    sprintf( charBuf, "Could not open AorB_Command_File_Name file %s, should contain the output of a file command%s", AorB_Command_File_Name, EndOfString );
    mqc_error_i_c2f_0( charBuf, &iout); 
  }
  rtn_fgets=fgets(charBuf,1024,AorB_Command_File);
  fclose_return = fclose(AorB_Command_File);
  AorB_Command_File = NULL;
  unlink(AorB_Command_File_Name);
  if ( -1 == mqc_FINDstrINline( " ASCII ", charBuf ) ) {
    return( 1 );
  } else {
    return( 0 );
  }
}

Input_Link_List *mqc_Read_to_Link_List(char *FileName, char *charBuf, int iout,
					int *Current_job, int *last_job, 
					int *Restart )
{
  FILE *Input_File;
  int fclose_return;
  int Job_Number;
  int Total_Jobs;
  int i;
  int external;
  int MatrixFile_Jobs;
  int found;
  char *rtn_fgets;
  char *external_proc;
  char prnt_str[2048];
  char lowerBuf[1024];
  Input_Link_List *Start_Link_List;
  Input_Link_List *Current_Link_List;
  Input_Link_List *Last_Link_List;
  Input_Link_List *Next_Link_List;

  Total_Jobs=0;
  MatrixFile_Jobs=0;
  last_job[0]=-1;
  Job_Number=0;

  Input_File = fopen( FileName,"r");
  if ( Input_File == NULL ) {
    sprintf( charBuf, "Could not open Gaussian Input File %s%s", FileName, EndOfString);
    mqc_error_i_c2f_0( charBuf, &iout);
  }
  last_job[0]=-1;
  rtn_fgets=fgets(charBuf,1024,Input_File);
  if ( rtn_fgets!= NULL ) {
#ifdef DEBUG
    sprintf( prnt_str, " Read from input: %s%s", charBuf, EndOfString );
    print_line_c2f ( prnt_str, &iout ); 
    flush_c2f( &iout );
#endif
    Current_Link_List = mqc_Alloc_Input_Link_List( charBuf, (Input_Link_List *)NULL, iout );
    Start_Link_List = Current_Link_List;
  }

  while( rtn_fgets!= NULL ) {
    if ( Current_Link_List->Keyword == 1 ) {
      Restart[0]=0;
      Job_Number++;
      Current_job[0]=0;
      Total_Jobs++;
      while ( Current_Link_List->Blank == 0 && rtn_fgets!= NULL ) {
	if (mqc_FINDstrINline( "restart", Current_Link_List->Lower_Name ) != -1 ) {
	  Restart[0]=1;
	}
	found = 0;
	i = mqc_FINDstrINline( "output=", Current_Link_List->Lower_Name );
#ifdef DEBUG
	if ( i != -1 ) {
	  sprintf( prnt_str, "output= found%s", EndOfString ); 
	  print_line_c2f ( prnt_str, &iout ); 
	} else {
	  sprintf( prnt_str, "output= not found%s", EndOfString ); 
	  print_line_c2f ( prnt_str, &iout ); 
	}
	flush_c2f( &iout );
#endif
	if ( i != -1 ) {
	  strcpy( lowerBuf, Current_Link_List->Lower_Name );
	  strcpy( charBuf, &lowerBuf[i] );
	  i = mqc_FINDstrINline( " ", charBuf );
	  if ( i != -1 ) {
	    strcpy( &charBuf[i], EndOfString );
	  }
#ifdef DEBUG
	  sprintf( prnt_str, "output instructions - \"%s\"\n Mat detection %d\n Raw detection %d%s", charBuf, mqc_FINDstrINline( "mat", charBuf ), mqc_FINDstrINline( "raw", charBuf ),EndOfString);
	  print_line_c2f ( prnt_str, &iout ); 
	  flush_c2f( &iout );
#endif
	  Current_Link_List->Keyword = 1;
	  if ( mqc_FINDstrINline( "mat", Current_Link_List->Lower_Name ) != -1 || 
	       mqc_FINDstrINline( "raw", Current_Link_List->Lower_Name ) != -1 ) {
	    found = 1;
	  }
	}

	external_proc = mqc_DupString( Current_Link_List->Lower_Name, (char *)NULL );
	external_proc = mqc_Replace_String( ",external", ",", external_proc );
	external_proc = mqc_Replace_String( "(external", "(", external_proc );
	if ( mqc_FINDstrINline( "external", external_proc ) != -1 ){
	  external = 1;
	} else {
	  external = 0;
	}
#ifdef DEBUG
	sprintf( prnt_str, "found is - %d%s", found, EndOfString );
	print_line_c2f ( prnt_str, &iout ); 
	sprintf( prnt_str, "external is - %d %s%s", external, external_proc, EndOfString );
	print_line_c2f ( prnt_str, &iout ); 
	flush_c2f( &iout );
#endif
	free( external_proc );

	if ( found == 1 || external == 1 ){
	  if ( Current_job[0] == 0 ) {
	    MatrixFile_Jobs++;
	  } else {
	    sprintf( prnt_str, "WARNING: Detected multiple triggers to write MatrixFile in job %d%s", Job_Number, EndOfString );
	    print_line_c2f ( prnt_str, &iout ); 
	  }
	  Current_job[0]=1;
	}
	/* Keep reading keywords until reach a blank line */
	rtn_fgets=fgets(charBuf,1024,Input_File);
	if ( rtn_fgets!= NULL ) {
#ifdef DEBUG
	  sprintf( prnt_str, " Read from input: %s%s", charBuf, EndOfString );
	  print_line_c2f ( prnt_str, &iout ); 
	  flush_c2f( &iout );
#endif
	  Last_Link_List = Current_Link_List;
	  Current_Link_List = mqc_Alloc_Input_Link_List( charBuf, Last_Link_List, iout );
	}
	/* Find the name of the MartixFile in the keywords */
      }
      if ( Current_job[0] == 0 && Restart[0] == 1 ) {
	if ( last_job[0] == -1 ) {
	  /* This is a restart job, and the keywords for the job that has */
	  /* been restarted my trigger a write of a MatrixFile.  It's not */
	  /* possible to tell. */

	  sprintf( prnt_str, "WARNING: Whether to write the MatrixFile in job %d is determined by the keywords in the job that has been restarted.%s", Job_Number, EndOfString);
	  print_line_c2f ( prnt_str, &iout ); 

	} else {
	  /* This is a restart job, and the keywords from the last job in */
	  /* the iinput will determine whether we write the MatrixFile */
	  /* or not. */
	  if ( last_job[0] != 0 ) {
	    MatrixFile_Jobs++;
	  }
	  Current_job[0]=last_job[0];
	}
      } else {
	if ( last_job[0] != -1 || Current_job[0] != 0 ) {
	  last_job[0]=Current_job[0];
	}
      }
    }
    if ( rtn_fgets!= NULL ) {
      rtn_fgets=fgets(charBuf,1024,Input_File);
      if ( rtn_fgets!= NULL ) {
#ifdef DEBUG
	sprintf( prnt_str, " Read from input: %s%s", charBuf, EndOfString );
	print_line_c2f ( prnt_str, &iout ); 
	flush_c2f( &iout );
#endif
	Last_Link_List = Current_Link_List;
	Current_Link_List = mqc_Alloc_Input_Link_List( charBuf, Last_Link_List, iout );
      }
    }
  }
  fclose_return = fclose(Input_File);
  Input_File = NULL;
  if ( fclose_return != 0 ) {
    sprintf( charBuf, "Could not close Input File %s%s", FileName, EndOfString);
    mqc_error_i_c2f_0( charBuf, &iout);
  }
#ifdef DEBUG
  sprintf( prnt_str, "%d inputs of which %d generate Matrix files%s", 
	   Total_Jobs, MatrixFile_Jobs, EndOfString );
  print_line_c2f ( prnt_str, &iout ); 
  sprintf( prnt_str, " Current_job %d, last_job %d%s", Current_job[0], last_job[0], EndOfString );
  print_line_c2f ( prnt_str, &iout ); 
#endif

  return( Start_Link_List);
}

