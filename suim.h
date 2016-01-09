//----------------------------------------------------------------
// Copyright (c) 2015 Takashi Yoshihara
// Released under the MIT license
// https://github.com/Yoshisan0/suim/blob/master/MIT-LICENSE.txt
//----------------------------------------------------------------

#if defined(WINDOWS)
  #define SUIMLIB_API __declspec(dllexport)
  #define SUIMLIB_CALL __stdcall
#else /* defined (WINDOWS) */
  #define SUIMLIB_API
  #define SUIMLIB_CALL
#endif

#define VERSION 1

#define TRUE 1
#define FALSE 0

#define SUCCESS 0

#define ERR_FUNC -1			//error in exe side function.
#define ERR_NONE -2			//error.
#define ERR_S_NULL -3		//suim pointer is null.
#define ERR_MEMORY -4		//malloc error.
#define ERR_NAME -5			//name is null or empty.
#define ERR_SET_NAME -6		//can't create name.
#define ERR_BUFFLER -7		//buffer is null or empty.
#define ERR_SET_BUFFER -8	//can't create buffer.
#define ERR_OVERFLOW -9		//buffer over flow.
#define ERR_TYPE -10		//type error.
#define ERR_SYNTAX -11		//syntax error.
#define ERR_FACTOR -12		//unsupported factor.
#define ERR_VARIABLE -13	//variable type error.
#define ERR_CALCULATE -14	//calculate error.
#define ERR_STACK -15		//stack error.
#define ERR_TOKEN -16		//token error.
#define ERR_KEY -17			//key error.
#define ERR_FUNC_NAME -18	//can't find function name.
#define ERR_NO -19			//no error.
#define ERR_VERSION -20		//version error.

#define WAR_NULL -1001

#define ORDER_SEPARATOR -2001
#define ORDER_RETURN -2002
#define ORDER_END -2003

#define MAX_KEY_LENGTH 40
#define MAX_STACK 100
#define MAX_WORD 32
#define MAX_LINENO 16
#define MAX_LINE 1024
#define MAX_ERRMSG 256
#define MAX_VARIABLE_COUNT 2704
#define MAX_STACK_FUNCTION 1024

typedef const int (SUIMLIB_CALL *CallBack)(const int *);
