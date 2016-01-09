//----------------------------------------------------------------
// Include
//----------------------------------------------------------------
#include <time.h>
#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "suim.h"

//----------------------------------------------------------------
// Struct
//----------------------------------------------------------------
/*
	Value struct
*/
struct ST_VALUE_FUNC {
	char pchKey[MAX_KEY_LENGTH];
	int inLineNo;
} ST_VALUE_FUNC;

struct ST_VALUE_CALLBACK {
	char pchKey[MAX_KEY_LENGTH];
	CallBack pclCallBack;
} ST_VALUE_CALLBACK;

struct ST_VALUE {
	char *pchValue;	//null is double
	double doValue;
} ST_VALUE;

/*
	SUIM struct
*/
struct ST_SUIM
{
	char *pchSuimName;
	char *pchBuffer;

	int inMaxLineNum;	//max line num
	int inMaxTokenNum;	//max line token num
	int inLineNo;		//now line no

	char **ppchLines;

	int *pinListJumpNo;		//jump line no
	char *pchListStartPos;	//line start position
	char **ppchListCmp;
	double *pdoListToken;
	char *pchListTokenType;	//0:error 1:double 2:variable 3:-variable 4:+ 5:== 6:!= 7:- 8:* 9:/ 10:% 11:>= 12:<= 13:> 14:< 15:&& 16:||
	char *pchListTokenWord;	//line token word list
	int *pinListTokenSt;	//line token start no list
	int *pinListTokenNum;	//line token num list

	int inStackBackNo;
	int pinStackBackNo[MAX_STACK_FUNCTION];
	int inCountFunction;
	struct ST_VALUE_FUNC **ppstListFunction;

	int inCountCallBack;
	struct ST_VALUE_CALLBACK **ppstListCallBack;

	int inCountVariable;
	struct ST_VALUE **ppstListVariable;

	char *pchInArgs;
	int inNoOut;
	char *pchOutArgsType;
	int inCountOut;
	struct ST_VALUE **ppstListOut;

	char *ppchStack[MAX_STACK];	//string stack
	double pdoStack[MAX_STACK];	//double stack
	double doCalcWork;
	char *pchCalcWork;

	int inErrCode;
	int inFlgError;
	char pchErrMsg[MAX_ERRMSG];
} ST_SUIM;

//----------------------------------------------------------------
// Global
//----------------------------------------------------------------
extern void Calc(struct ST_SUIM *);
extern int ExecLine(struct ST_SUIM *, const char *);

/*
	ClearError function
	Get error code.
*/
SUIMLIB_API int ClearError(const int *S)
{
	if(S== NULL) return (ERR_S_NULL);

	struct ST_SUIM *pS = (struct ST_SUIM *)S;
	pS->inErrCode = SUCCESS;
	strcpy(pS->pchErrMsg, "");

	return (SUCCESS);
}

/*
	GetErrorCode function
	Get error code.
*/
SUIMLIB_API int GetErrorCode(const int *S)
{
	if(S== NULL) return (ERR_S_NULL);

	struct ST_SUIM *pS = (struct ST_SUIM *)S;

	return (pS->inErrCode);
}

/*
	LibGetError function
	Get error message.
*/
SUIMLIB_API char * LibGetError(const int *S)
{
	if(S== NULL) return (NULL);

	struct ST_SUIM *pS = (struct ST_SUIM *)S;

	return (pS->pchErrMsg);
}

/*
	SetError function
*/
void SetError1(struct ST_SUIM *pS, const char *pchErrMsg)
{
	if(pS->inFlgError== 1) return;

	char pchLineNo[MAX_LINENO];
	memset(pchLineNo, 0x00, MAX_LINENO);
	char *pchLine = pS->ppchLines[pS->inLineNo];
	char *pchIndex = strchr(pchLine, ':');
	if(pchIndex== NULL) {
		strcpy(pchLineNo, "---");
	}
	else {
		strncpy(pchLineNo, pchLine, (int)(pchIndex - pchLine));
	}

	pS->inFlgError = 1;
	pS->inErrCode = ERR_NONE;
	sprintf(pS->pchErrMsg, "[%s,%s] %s", pS->pchSuimName, pchLineNo, pchErrMsg);
}

/*
	SetError function
	Set error info.
*/
void SetError2(struct ST_SUIM *pS, int inErrCode, char *pchErrMsg)
{
	if(pS->inFlgError== 1) return;

	char pchLineNo[MAX_LINENO];
	memset(pchLineNo, 0x00, MAX_LINENO);
	char *pchLine = pS->ppchLines[pS->inLineNo];
	char *pchIndex = strchr(pchLine, ':');
	if(pchIndex== NULL) {
		strcpy(pchLineNo, "---");
	}
	else {
		strncpy(pchLineNo, pchLine, (int)(pchIndex - pchLine));
	}

	pS->inFlgError = 1;
	pS->inErrCode = inErrCode;
	sprintf(pS->pchErrMsg, "[%s,%s] %s", pS->pchSuimName, pchLineNo, pchErrMsg);
}

/*
	LibGetName function
	Get suim name.
*/
SUIMLIB_API char * LibGetName(const int *S)
{
	if(S== NULL) return (NULL);

	struct ST_SUIM *pS = (struct ST_SUIM *)S;

	return (pS->pchSuimName);
}

/*
	GetLineNo function
	Get line no.
*/
SUIMLIB_API int GetLineNo(const int *S)
{
	if(S== NULL) return (0);

	struct ST_SUIM *pS = (struct ST_SUIM *)S;

	char *pchLine = pS->ppchLines[pS->inLineNo];
	char *pchIndex = strchr(pchLine, ':');
	int inLineNo = 0;
	if(pchIndex!= NULL) {
		char pchLineNo[MAX_LINENO];
		memset(pchLineNo, 0x00, MAX_LINENO);
		strncpy(pchLineNo, pchLine, (int)(pchIndex - pchLine));
		inLineNo = atoi(pchLineNo);
	}

	return (inLineNo);
}

/*
	CngKey2No function
	Change variable name to No.
*/
inline int CngKey2No(const char *pchKey)
{
	int inNo = 0;
	if(pchKey[0]>= 'a')
	{
		inNo = (pchKey[0] - 'a') * 52;
	}
	else {
		inNo = (pchKey[0] - 'A' + 26) * 52;
	}

	if(pchKey[1]>= 'a')
	{
		inNo += (pchKey[1] - 'a');
	}
	else {
		inNo += (pchKey[1] - 'A' + 26);
	}

	return (inNo);
}

/*
	GetTypeValue function
	Return double type (0 or 1)
*/
inline int IsDouble(const char *pchValue)
{
	if(pchValue== NULL) return (0);

	int inLength = strlen(pchValue);
	if(pchValue[0]== '-' && inLength== 1) return (0);

	int inSt = (pchValue[0]== '-') ? 1 : 0;

	int inCnt;
	for(inCnt= inSt;inCnt< inLength;inCnt++)
	{
		if(isdigit(pchValue[inCnt])) continue;
		if(pchValue[inCnt]== '.') continue;

		return (0);
	}

	return (1);
}

/*
	FreeAllStack function
	Free stack strings.
*/
inline void FreeAllStack(char **ppchStack)
{
	int inCnt;
	for(inCnt= 0;inCnt< MAX_STACK;inCnt++)
	{
		if(ppchStack[inCnt]== NULL) continue;

		free(ppchStack[inCnt]);
		ppchStack[inCnt] = NULL;
	}
}

/*
	SetStackString1 function
	Set stack string.
*/
inline void SetStackString1(char **ppchStack, int inStackNo, char *pchValue)
{
	char *pchStack = ppchStack[inStackNo];

	if(pchValue!= NULL)
	{
		int inLength = strlen(pchValue);
		ppchStack[inStackNo] = malloc(inLength + 1);
		strcpy(ppchStack[inStackNo], pchValue);
	}
	else
	{
		ppchStack[inStackNo] = NULL;
	}

	if(pchStack!= NULL)
	{
		free(pchStack);
		pchStack = NULL;
	}
}

/*
	SetStackString2 function
	Set stack string.
*/
inline void SetStackString2(char **ppchStack, int inStackNo, char *pchValue1, char *pchValue2)
{
	char *pchStack = ppchStack[inStackNo];

	if(pchValue1!= NULL && pchValue2!= NULL)
	{
		int inLength = strlen(pchValue1) + strlen(pchValue2);
		ppchStack[inStackNo] = malloc(inLength + 1);
		sprintf(ppchStack[inStackNo], "%s%s", pchValue1, pchValue2);
	}
	else
	{
		ppchStack[inStackNo] = NULL;
	}

	if(pchStack!= NULL)
	{
		free(pchStack);
		pchStack = NULL;
	}
}

/*
	SetStackString3 function
	Set stack string.
*/
inline void SetStackString3(char **ppchStack, int inStackNo, char *pchValue1, double doValue2)
{
	char *pchStack = ppchStack[inStackNo];

	if(pchValue1!= NULL)
	{
		int inLength = strlen(pchValue1) + MAX_WORD;
		ppchStack[inStackNo] = malloc(inLength + 1);
		sprintf(ppchStack[inStackNo], "%s%d", pchValue1, (int)doValue2);
	}
	else
	{
		ppchStack[inStackNo] = NULL;
	}

	if(pchStack!= NULL)
	{
		free(pchStack);
		pchStack = NULL;
	}
}

/*
	SetStackString4 function
	Set stack string.
*/
inline void SetStackString4(char **ppchStack, int inStackNo, double doValue1, char *pchValue2)
{
	char *pchStack = ppchStack[inStackNo];

	if(pchValue2!= NULL)
	{
		int inLength = MAX_WORD + strlen(pchValue2);
		ppchStack[inStackNo] = malloc(inLength + 1);
		sprintf(ppchStack[inStackNo], "%d%s", (int)doValue1, pchValue2);
	}
	else
	{
		ppchStack[inStackNo] = NULL;
	}

	if(pchStack!= NULL)
	{
		free(pchStack);
		pchStack = NULL;
	}
}

/*
	Calc function
*/
inline void Calc(struct ST_SUIM *pS)
{
	//initialize
	char **ppchStack = pS->ppchStack;	//string stack
	double *pdoStack = pS->pdoStack;	//double stack
	int inStackNo = 0;					//stack position
	char *pchWord;
	int inWordLen;
	int isHit = 0;
	int isHitString = 0;

	pS->doCalcWork = 0.0;
	if(pS->pchCalcWork!= NULL) {
		free(pS->pchCalcWork);
		pS->pchCalcWork = NULL;
	}

	int inLineNo = pS->inLineNo;
	int inTokenSt = pS->pinListTokenSt[inLineNo];
	int inCnt, inMax = pS->pinListTokenNum[inLineNo];
	for(inCnt= 0;inCnt< inMax;inCnt++)
	{
		pchWord = &pS->pchListTokenWord[MAX_WORD * (inTokenSt + inCnt)];

		isHit = 0;
		int inType = pS->pchListTokenType[inTokenSt + inCnt];
		if(inType== 1) {
			pdoStack[inStackNo] = pS->pdoListToken[inTokenSt + inCnt];
			SetStackString1(ppchStack, inStackNo, NULL);

			inStackNo++;

			isHit = 1;
		}
		else {
			if(inType== 2 || inType== 3)
			{
				int inSt = (inType== 2) ? 0 : 1;

				int inNo = CngKey2No(&pchWord[inSt]);
				if(inNo< 0 || pS->inCountVariable<= inNo) {
					SetError2(pS, ERR_CALCULATE, "Variable count error.");
					FreeAllStack(ppchStack);
					return;
				}

				const struct ST_VALUE *pstValue = pS->ppstListVariable[inNo];
				if(pstValue== NULL) {
					SetError2(pS, ERR_CALCULATE, "Variable name is not defined.");
					FreeAllStack(ppchStack);
					return;
				}

				if(pstValue->pchValue== NULL) {	//null is double
					pdoStack[inStackNo] = (inType== 2) ? pstValue->doValue : -pstValue->doValue;
					SetStackString1(ppchStack, inStackNo, NULL);

					inStackNo++;
				}
				else {
					if(inType== 2) {
						pdoStack[inStackNo] = 0.0;
						SetStackString1(ppchStack, inStackNo, pstValue->pchValue);

						inStackNo++;

						if(pstValue->pchValue!= NULL) {
							isHitString = 1;
						}
					}
					else {
						//syntax error. '-"hoge"' token error.
						SetError2(pS, ERR_CALCULATE, "Calculate error.");
						FreeAllStack(ppchStack);
						return;
					}
				}

				isHit = 1;
			}
			else {
				if(inType== 4) {
					//string + string
					//string + double
					//double + string
					//double + double

					if(inStackNo < 2) {
						SetError2(pS, ERR_STACK, "Stack error. (+)");
						FreeAllStack(ppchStack);
						return;
					}

					inStackNo--;

					if(ppchStack[inStackNo-1]== NULL && ppchStack[inStackNo]== NULL) {
						//double + double
						pdoStack[inStackNo-1] += pdoStack[inStackNo];
						SetStackString1(ppchStack, inStackNo-1, NULL);
					}
					else {
						if(ppchStack[inStackNo-1]!= NULL && ppchStack[inStackNo]!= NULL) {
							//string + string
							pdoStack[inStackNo-1] = 0.0;
							SetStackString2(ppchStack, inStackNo-1, ppchStack[inStackNo-1], ppchStack[inStackNo]);

							isHitString = 1;
						}
						else {
							if(ppchStack[inStackNo-1]!= NULL && ppchStack[inStackNo]== NULL) {
								//string + double
								pdoStack[inStackNo-1] = 0.0;
								SetStackString3(ppchStack, inStackNo-1, ppchStack[inStackNo-1], pdoStack[inStackNo]);

								isHitString = 1;
							}
							else {
								if(ppchStack[inStackNo-1]== NULL && ppchStack[inStackNo]!= NULL) {
									//double + string
									pdoStack[inStackNo-1] = 0.0;
									SetStackString4(ppchStack, inStackNo-1, pdoStack[inStackNo-1], ppchStack[inStackNo]);

									isHitString = 1;
								}
							}
						}
					}

					pdoStack[inStackNo] = 0;
					SetStackString1(ppchStack, inStackNo, NULL);

					isHit = 1;
				}
				else {
					if(inType== 5 || inType== 6) {

						//string and string
						//double and double

						if(inStackNo < 2) {
							char pchErrMsg[MAX_ERRMSG];
							sprintf(pchErrMsg, "Stack error. (%s)", pchWord);
							SetError2(pS, ERR_STACK, pchErrMsg);
							FreeAllStack(ppchStack);
							return;
						}

						if( (ppchStack[inStackNo-2]!= NULL && ppchStack[inStackNo-1]== NULL) ||
							(ppchStack[inStackNo-2]== NULL && ppchStack[inStackNo-1]!= NULL)) {

							char pchErrMsg[MAX_ERRMSG];
							if(ppchStack[inStackNo-2]!= NULL && ppchStack[inStackNo-1]== NULL) {
								sprintf(pchErrMsg, "Stack type error. (%s) %s %s double", pchWord, ppchStack[inStackNo-2], pchWord);
							}
							else {
								sprintf(pchErrMsg, "Stack type error. (%s) double %s %s", pchWord, pchWord, ppchStack[inStackNo-2]);
							}

							SetError2(pS, ERR_STACK, pchErrMsg);
							FreeAllStack(ppchStack);
							return;
						}

						inStackNo--;

						if(ppchStack[inStackNo-1]== NULL && ppchStack[inStackNo]== NULL) {
							if(inType== 5) {
								//double == double
								pdoStack[inStackNo-1] = (pdoStack[inStackNo-1] == pdoStack[inStackNo]);
							}
							else {
								if(inType== 6) {
									//double != double
									pdoStack[inStackNo-1] = (pdoStack[inStackNo-1] != pdoStack[inStackNo]);
								}
							}
						}
						else {
							if(inType== 5) {
								//string == string
								pdoStack[inStackNo-1] = !strcmp(ppchStack[inStackNo-1], ppchStack[inStackNo]);
							}
							else {
								if(inType== 6) {
									//string != string
									pdoStack[inStackNo-1] = strcmp(ppchStack[inStackNo-1], ppchStack[inStackNo]);
								}
							}
						}
						SetStackString1(ppchStack, inStackNo-1, NULL);

						pdoStack[inStackNo] = 0;
						SetStackString1(ppchStack, inStackNo, NULL);

						isHit = 1;
					}
					else {
						if(7<= inType && inType<= 16) {

							//double and double

							if(inStackNo < 2) {
								char pchErrMsg[MAX_ERRMSG];
								sprintf(pchErrMsg, "Stack error. (%s)", pchWord);
								SetError2(pS, ERR_STACK, pchErrMsg);
								FreeAllStack(ppchStack);
								return;
							}

							if(ppchStack[inStackNo-1]!= NULL || ppchStack[inStackNo]!= NULL) {
								char pchErrMsg[MAX_ERRMSG];
								sprintf(pchErrMsg, "Stack type error. (%s)", pchWord);
								SetError2(pS, ERR_STACK, pchErrMsg);
								FreeAllStack(ppchStack);
								return;
							}

							inStackNo--;

							if(inType== 7) {
								//double - double
								pdoStack[inStackNo-1] -= pdoStack[inStackNo];
							}
							else {
								if(inType== 8) {
									//double * double
									pdoStack[inStackNo-1] *= pdoStack[inStackNo];
								}
								else {
									if(inType== 9) {
										//double / double
										if(pdoStack[inStackNo]== 0.0) {
											char pchErrMsg[MAX_ERRMSG];
											sprintf(pchErrMsg, "Division by zero. (%s)", pchWord);
											SetError2(pS, ERR_STACK, pchErrMsg);
											FreeAllStack(ppchStack);
											return;
										}

										pdoStack[inStackNo-1] /= pdoStack[inStackNo];
									}
									else {
										if(inType== 10) {
											//double % double
											if(pdoStack[inStackNo]== 0.0) {
												char pchErrMsg[MAX_ERRMSG];
												sprintf(pchErrMsg, "Division by zero. (%s)", pchWord);
												SetError2(pS, ERR_STACK, pchErrMsg);
												FreeAllStack(ppchStack);
												return;
											}

											pdoStack[inStackNo-1] = fmod(pdoStack[inStackNo-1], pdoStack[inStackNo]);
										}
										else {
											if(inType== 11) {
												//double >= double
												pdoStack[inStackNo-1] = (pdoStack[inStackNo-1] >= pdoStack[inStackNo]);
											}
											else {
												if(inType== 12) {
													//double <= double
													pdoStack[inStackNo-1] = (pdoStack[inStackNo-1] <= pdoStack[inStackNo]);
												}
												else {
													if(inType== 13) {
														//double > double
														pdoStack[inStackNo-1] = (pdoStack[inStackNo-1] > pdoStack[inStackNo]);
													}
													else {
														if(inType== 14) {
															//double < double
															pdoStack[inStackNo-1] = (pdoStack[inStackNo-1] < pdoStack[inStackNo]);
														}
														else {
															if(inType== 15) {
																//double && double
																pdoStack[inStackNo-1] = (pdoStack[inStackNo-1] && pdoStack[inStackNo]);
															}
															else {
																if(inType== 16) {
																	//double || double
																	pdoStack[inStackNo-1] = (pdoStack[inStackNo-1] || pdoStack[inStackNo]);
																}
															}
														}
													}
												}
											}
										}
									}
								}
							}
							SetStackString1(ppchStack, inStackNo - 1, NULL);

							pdoStack[inStackNo] = 0;
							SetStackString1(ppchStack, inStackNo, NULL);

							isHit = 1;
						}
					}
				}
			}
		}

		if(isHit== 0) {
			SetError2(pS, ERR_TOKEN, "Token error.");
			FreeAllStack(ppchStack);
			return;
		}
	}

	if(ppchStack[0]== NULL) {
		pS->doCalcWork = pdoStack[0];
	}
	else {
		int inLength = strlen(ppchStack[0]);
		pS->pchCalcWork = malloc(inLength + 1);
		strcpy(pS->pchCalcWork, ppchStack[0]);
	}

	if(isHitString!= 0) {
		FreeAllStack(ppchStack);
	}
}

//----------------------------------------------------------------
// main
//----------------------------------------------------------------

/*
	StrNullOrEmpty function
*/
int IsNullOrEmpty(const char *pchStr)
{
	if(pchStr== NULL) return (-1);

	size_t szLength = strlen(pchStr);
	if(szLength<= 0) return (-2);

	return (0);
}

/*
	New function
	Create ST_SUIM.
*/
SUIMLIB_API const int * New()
{
	struct ST_SUIM *pstSuim = malloc(sizeof(ST_SUIM));
	if(pstSuim== NULL) return (NULL);
	memset(pstSuim, 0x00, sizeof(ST_SUIM));

	return ((int *)pstSuim);
}

/*
	Dispose function
	Remove ST_SUIM.
*/
SUIMLIB_API int Dispose(const int *S)
{
	if(S== NULL) return (ERR_S_NULL);

	struct ST_SUIM *pS = (struct ST_SUIM *)S;

	if(pS->pinListJumpNo!= NULL)
	{
		free(pS->pinListJumpNo);
		pS->pinListJumpNo = NULL;
	}

	if(pS->pchListStartPos!= NULL)
	{
		free(pS->pchListStartPos);
		pS->pchListStartPos = NULL;
	}

	if(pS->pdoListToken!= NULL)
	{
		free(pS->pdoListToken);
		pS->pdoListToken = NULL;
	}

	if(pS->pchListTokenType!= NULL)
	{
		free(pS->pchListTokenType);
		pS->pchListTokenType = NULL;
	}

	if(pS->pchListTokenWord!= NULL)
	{
		free(pS->pchListTokenWord);
		pS->pchListTokenWord = NULL;
	}

	if(pS->pinListTokenSt!= NULL)
	{
		free(pS->pinListTokenSt);
		pS->pinListTokenSt = NULL;
	}

	if(pS->pinListTokenNum!= NULL)
	{
		free(pS->pinListTokenNum);
		pS->pinListTokenNum = NULL;
	}

	if(pS->ppchListCmp!= NULL)
	{
		int inCnt, inMax = pS->inMaxLineNum;
		for(inCnt= 0;inCnt< inMax;inCnt++)
		{
			if(pS->ppchListCmp[inCnt]== NULL) continue;

			free(pS->ppchListCmp[inCnt]);
			pS->ppchListCmp[inCnt] = NULL;
		}

		free(pS->ppchListCmp);
		pS->ppchListCmp = NULL;
	}

	if(pS->pchCalcWork!= NULL)
	{
		free(pS->pchCalcWork);
		pS->pchCalcWork = NULL;
	}

	if(pS->ppstListFunction!= NULL)
	{
		int inCnt, inMax = pS->inCountFunction;
		for(inCnt= 0;inCnt< inMax;inCnt++)
		{
			if(pS->ppstListFunction[inCnt]== NULL) continue;

			free(pS->ppstListFunction[inCnt]);
			pS->ppstListFunction[inCnt] = NULL;
		}

		free(pS->ppstListFunction);
		pS->ppstListFunction = NULL;
	}

	if(pS->ppstListCallBack!= NULL)
	{
		int inCnt, inMax = pS->inCountCallBack;
		for(inCnt= 0;inCnt< inMax;inCnt++)
		{
			if(pS->ppstListCallBack[inCnt]== NULL) continue;

			free(pS->ppstListCallBack[inCnt]);
			pS->ppstListCallBack[inCnt] = NULL;
		}

		free(pS->ppstListCallBack);
		pS->ppstListCallBack = NULL;
	}

	if(pS->ppstListVariable!= NULL)
	{
		int inCnt, inMax = pS->inCountVariable;
		for(inCnt= 0;inCnt< inMax;inCnt++)
		{
			if(pS->ppstListVariable[inCnt]== NULL) continue;

			if(pS->ppstListVariable[inCnt]->pchValue!= NULL) {
				free(pS->ppstListVariable[inCnt]->pchValue);
				pS->ppstListVariable[inCnt]->pchValue = NULL;
			}
			free(pS->ppstListVariable[inCnt]);
			pS->ppstListVariable[inCnt] = NULL;
		}

		free(pS->ppstListVariable);
		pS->ppstListVariable = NULL;
	}

	if(pS->pchInArgs!= NULL)
	{
		free(pS->pchInArgs);
		pS->pchInArgs = NULL;
	}

	if(pS->pchOutArgsType!= NULL)
	{
		free(pS->pchOutArgsType);
		pS->pchOutArgsType = NULL;
	}

	if(pS->ppstListOut!= NULL)
	{
		int inCnt, inMax = pS->inCountOut;
		for(inCnt= 0;inCnt< inMax;inCnt++)
		{
			if(pS->ppstListOut[inCnt]== NULL) continue;

			if(pS->ppstListOut[inCnt]->pchValue!= NULL) {
				free(pS->ppstListOut[inCnt]->pchValue);
				pS->ppstListOut[inCnt]->pchValue = NULL;
			}
			free(pS->ppstListOut[inCnt]);
			pS->ppstListOut[inCnt] = NULL;
		}

		free(pS->ppstListOut);
		pS->ppstListOut = NULL;
	}

	if(pS->ppchLines!= NULL)
	{
		free(pS->ppchLines);
		pS->ppchLines = NULL;
	}

	if(pS->pchBuffer!= NULL)
	{
		free(pS->pchBuffer);
		pS->pchBuffer = NULL;
	}

	if(pS->pchSuimName!= NULL)
	{
		free(pS->pchSuimName);
		pS->pchSuimName = NULL;
	}

	free(pS);

	return (SUCCESS);
}

/*
	AddFunctionDefine function
	Function line no set.
*/
int AddFunctionDefine(struct ST_SUIM *pS, const char *pchKey, int inLineNo)
{
	int inCnt, inMax = pS->inCountFunction;
	for(inCnt= 0;inCnt< inMax;inCnt++)
	{
		if(pS->ppstListFunction[inCnt]== NULL)
		{
			pS->ppstListFunction[inCnt] = malloc(sizeof(struct ST_VALUE_FUNC));
			if(pS->ppstListFunction[inCnt]== NULL) return (ERR_MEMORY);
			memset(pS->ppstListFunction[inCnt], 0x00, sizeof(struct ST_VALUE_FUNC));

			strcpy(pS->ppstListFunction[inCnt]->pchKey, pchKey);

			pS->ppstListFunction[inCnt]->inLineNo = inLineNo;

			return (SUCCESS);
		}

		if(!strcmp(pchKey, pS->ppstListFunction[inCnt]->pchKey))
		{
			pS->ppstListFunction[inCnt]->inLineNo = inLineNo;

			return (SUCCESS);
		}
	}

	return (ERR_OVERFLOW);
}

/*
	GetFunctionDefine function
	Get function line no.
*/
int GetFunctionDefine(struct ST_SUIM *pS, const char *pchKey)
{
	int inCnt, inMax = pS->inCountFunction;
	for(inCnt= 0;inCnt< inMax;inCnt++)
	{
		if(pS->ppstListFunction[inCnt]== NULL) continue;

		if(!strcmp(pchKey, pS->ppstListFunction[inCnt]->pchKey))
		{
			return (pS->ppstListFunction[inCnt]->inLineNo);
		}
	}

	return (0);
}

/*
	AddCallBack function
	CallBack set.
*/
int AddCallBack(struct ST_SUIM *pS, const char *pchKey, CallBack pclCallBack)
{
	int inCnt, inMax = pS->inCountCallBack;
	for(inCnt= 0;inCnt< inMax;inCnt++)
	{
		if(pS->ppstListCallBack[inCnt]== NULL)
		{
			pS->ppstListCallBack[inCnt] = malloc(sizeof(struct ST_VALUE_CALLBACK));
			if(pS->ppstListCallBack[inCnt]== NULL) return (ERR_MEMORY);
			memset(pS->ppstListCallBack[inCnt], 0x00, sizeof(struct ST_VALUE_CALLBACK));

			strcpy(pS->ppstListCallBack[inCnt]->pchKey, pchKey);

			pS->ppstListCallBack[inCnt]->pclCallBack = pclCallBack;

			return (SUCCESS);
		}

		if(!strcmp(pchKey, pS->ppstListCallBack[inCnt]->pchKey))
		{
			pS->ppstListCallBack[inCnt]->pclCallBack = pclCallBack;

			return (SUCCESS);
		}
	}

	return (ERR_OVERFLOW);
}

/*
	GetCallBack function
	Get callback pointer.
*/
CallBack GetCallBack(struct ST_SUIM *pS, const char *pchKey)
{
	int inCnt, inMax = pS->inCountCallBack;
	for(inCnt= 0;inCnt< inMax;inCnt++)
	{
		if(pS->ppstListCallBack[inCnt]== NULL) continue;

		if(!strcmp(pchKey, pS->ppstListCallBack[inCnt]->pchKey))
		{
			return (pS->ppstListCallBack[inCnt]->pclCallBack);
		}
	}

	return (NULL);
}

/*
	LibGetInArgs function
*/
SUIMLIB_API char * LibGetInArgs(const int *S)
{
	if(S== NULL) return (NULL);

	struct ST_SUIM *pS = (struct ST_SUIM *)S;

	return (pS->pchInArgs);
}

/*
	LibGetArgType function
*/
SUIMLIB_API char * LibGetArgType(const int *S)
{
	if(S== NULL) return (NULL);

	struct ST_SUIM *pS = (struct ST_SUIM *)S;
	if(pS->pchOutArgsType== NULL) return (NULL);

	pS->pchOutArgsType[0] = '\0';

	int inCnt, inMax = pS->inNoOut;
	for(inCnt= 0;inCnt< inMax;inCnt++)
	{
		struct ST_VALUE *pstValue = pS->ppstListOut[inCnt];

		if(pstValue->pchValue== NULL) {
			strcat(pS->pchOutArgsType, "d");
		}
		else {
			strcat(pS->pchOutArgsType, "s");
		}
	}

	return (pS->pchOutArgsType);
}

/*
	SetDouble function
*/
SUIMLIB_API void SetDouble(const int *S, char *pchKey, double doValue)
{
	if(S== NULL) return;

	struct ST_SUIM *pS = (struct ST_SUIM *)S;

	int inNo = CngKey2No(pchKey);
	if(inNo< 0) return;
	if(inNo>= pS->inCountVariable) return;

	struct ST_VALUE *pstValue = pS->ppstListVariable[inNo];
	if(pstValue== NULL) return;

	if(pstValue->pchValue!= NULL) {
		free(pstValue->pchValue);
		pstValue->pchValue = NULL;
	}

	pstValue->doValue = doValue;
}

/*
	SetString function
*/
SUIMLIB_API void SetString(const int *S, char *pchKey, char *pchValue)
{
	if(S== NULL) return;

	struct ST_SUIM *pS = (struct ST_SUIM *)S;

	int inNo = CngKey2No(pchKey);
	if(inNo< 0) return;
	if(inNo>= pS->inCountVariable) return;

	struct ST_VALUE *pstValue = pS->ppstListVariable[inNo];
	if(pstValue== NULL) return;

	if(pstValue->pchValue!= NULL) {
		free(pstValue->pchValue);
		pstValue->pchValue = NULL;
	}

	pstValue->doValue = 0.0;
	int inLength = strlen(pchValue);
	pstValue->pchValue = malloc(inLength + 1);
	strcpy(pstValue->pchValue, pchValue);
}

/*
	GetDouble function
*/
SUIMLIB_API double GetDouble(const int *S, char *pchKey)
{
	if(S== NULL) return (0.0);

	struct ST_SUIM *pS = (struct ST_SUIM *)S;

	int inNo = CngKey2No(pchKey);
	if(inNo< 0) return (0.0);
	if(inNo>= pS->inCountVariable) return (0.0);

	struct ST_VALUE *pstValue = pS->ppstListVariable[inNo];
	if(pstValue== NULL) return (0.0);

	return (pstValue->doValue);
}

/*
	LibGetString function
*/
SUIMLIB_API char * LibGetString(const int *S, char *pchKey)
{
	if(S== NULL) return (NULL);

	struct ST_SUIM *pS = (struct ST_SUIM *)S;

	int inNo = CngKey2No(pchKey);
	if(inNo< 0) return (NULL);
	if(inNo>= pS->inCountVariable) return (NULL);

	struct ST_VALUE *pstValue = pS->ppstListVariable[inNo];
	if(pstValue== NULL) return (NULL);

	return (pstValue->pchValue);
}

/*
	GetOutDouble function
*/
SUIMLIB_API double GetOutDouble(const int *S, int inNo)
{
	if(S== NULL) return (0.0);

	struct ST_SUIM *pS = (struct ST_SUIM *)S;

	if(inNo< 0) return (0.0);
	if(inNo>= pS->inCountOut) return (0.0);

	struct ST_VALUE *pstValue = pS->ppstListOut[inNo];
	if(pstValue== NULL) return (0.0);

	return (pstValue->doValue);
}

/*
	LibGetOutString function
*/
SUIMLIB_API char * LibGetOutString(const int *S, int inNo)
{
	if(S== NULL) return (NULL);

	struct ST_SUIM *pS = (struct ST_SUIM *)S;

	if(inNo< 0) return (NULL);
	if(inNo>= pS->inCountOut) return (NULL);

	struct ST_VALUE *pstValue = pS->ppstListOut[inNo];
	if(pstValue== NULL) return (NULL);

	return (pstValue->pchValue);
}

/*
	SetValueDouble function
	Set string variable.
*/
int SetValueDouble(struct ST_VALUE **ppstList, int inNo, const double doValue)
{
	if(ppstList[inNo]== NULL)
	{
		ppstList[inNo] = malloc(sizeof(struct ST_VALUE));
		if(ppstList[inNo]== NULL) return (ERR_MEMORY);
		memset(ppstList[inNo], 0x00, sizeof(struct ST_VALUE));

		ppstList[inNo]->doValue = doValue;

		return (SUCCESS);
	}

	if(ppstList[inNo]->pchValue!= NULL) {
		free(ppstList[inNo]->pchValue);
		ppstList[inNo]->pchValue = NULL;
	}

	ppstList[inNo]->doValue = doValue;

	return (SUCCESS);
}

/*
	SetValueString function
	Set string variable.
*/
int SetValueString(struct ST_VALUE **ppstList, int inNo, char *pchValue)
{
	if(pchValue== NULL) {
		pchValue = "\0";
	}

	if(ppstList[inNo]== NULL)
	{
		ppstList[inNo] = malloc(sizeof(struct ST_VALUE));
		if(ppstList[inNo]== NULL) return (ERR_MEMORY);
		memset(ppstList[inNo], 0x00, sizeof(struct ST_VALUE));

		int inLength = strlen(pchValue);
		ppstList[inNo]->pchValue = malloc(inLength + 1);
		if(ppstList[inNo]->pchValue== NULL) return (ERR_MEMORY);
		strcpy(ppstList[inNo]->pchValue, pchValue);

		return (SUCCESS);
	}

	if(ppstList[inNo]->pchValue!= NULL) {
		free(ppstList[inNo]->pchValue);
		ppstList[inNo]->pchValue = NULL;
	}
	ppstList[inNo]->doValue = 0.0;

	int inLength = strlen(pchValue);
	ppstList[inNo]->pchValue = malloc(inLength + 1);
	if(ppstList[inNo]->pchValue== NULL) return (ERR_MEMORY);
	strcpy(ppstList[inNo]->pchValue, pchValue);

	return (SUCCESS);
}

/*
	GetToken function
	Get token from line
*/
int CountToken(char *pchLine)
{
	int inFlag = IsNullOrEmpty(pchLine);
	if(inFlag!= 0) return (0);

	int inCntToken = 0;
	char *pchSt = pchLine;
	char *pchEd = strchr(pchSt, ' ');
	while(1)
	{
		inCntToken++;

		if(pchEd== NULL) break;
		pchSt = pchEd + 1;
		pchEd = strchr(pchSt, ' ');
	}

	return (inCntToken);
}

/*
	SetTokenDouble function
*/
void SetTokenData(struct ST_SUIM *pS, char *pchLine, int inTokenSt)
{
	int inFlag = IsNullOrEmpty(pchLine);
	if(inFlag!= 0) return;

	int inCntToken = 0;
	char *pchSt = pchLine;
	char *pchEd = strchr(pchSt, ' ');
	while(1)
	{
		char pchWord[MAX_WORD];
		if(pchEd== NULL) {
			strcpy(pchWord, pchSt);
		}
		else {
			int inLength = pchEd- pchSt;
			strncpy(pchWord, pchSt, inLength);
			pchWord[inLength] = '\0';
		}

		//copy token word
		strcpy(&pS->pchListTokenWord[MAX_WORD * (inTokenSt + inCntToken)], pchWord);

		//setup token type
		if(isalpha(pchWord[0]) && isalpha(pchWord[1])) {
			pS->pchListTokenType[inTokenSt + inCntToken] = 2;
		}
		else {
			if(pchWord[0]== '-' && isalpha(pchWord[1]) && isalpha(pchWord[2])) {
				pS->pchListTokenType[inTokenSt + inCntToken] = 3;
			}
			else {
				int isDouble = IsDouble(pchWord);
				if(isDouble) {
					pS->pchListTokenType[inTokenSt + inCntToken] = 1;
					pS->pdoListToken[inTokenSt + inCntToken] = atof(pchSt);
				}
				else {
					if(pchWord[0]== '+') {
						pS->pchListTokenType[inTokenSt + inCntToken] = 4;
					}
					else {
						if(pchWord[0]== '=' && pchWord[1]== '=') {
							pS->pchListTokenType[inTokenSt + inCntToken] = 5;
						}
						else {
							if(pchWord[0]== '!' && pchWord[1]== '=') {
								pS->pchListTokenType[inTokenSt + inCntToken] = 6;
							}
							else {
								if(pchWord[0]== '-') {
									pS->pchListTokenType[inTokenSt + inCntToken] = 7;
								}
								else {
									if(pchWord[0]== '*') {
										pS->pchListTokenType[inTokenSt + inCntToken] = 8;
									}
									else {
										if(pchWord[0]== '/') {
											pS->pchListTokenType[inTokenSt + inCntToken] = 9;
										}
										else {
											if(pchWord[0]== '%') {
												pS->pchListTokenType[inTokenSt + inCntToken] = 10;
											}
											else {
												if(pchWord[0]== '>' && pchWord[1]== '=') {
													pS->pchListTokenType[inTokenSt + inCntToken] = 11;
												}
												else {
													if(pchWord[0]== '<' && pchWord[1]== '=') {
														pS->pchListTokenType[inTokenSt + inCntToken] = 12;
													}
													else {
														if(pchWord[0]== '>') {
															pS->pchListTokenType[inTokenSt + inCntToken] = 13;
														}
														else {
															if(pchWord[0]== '<') {
																pS->pchListTokenType[inTokenSt + inCntToken] = 14;
															}
															else {
																if(pchWord[0]== '&' && pchWord[1]== '&') {
																	pS->pchListTokenType[inTokenSt + inCntToken] = 15;
																}
																else {
																	if(pchWord[0]== '|' && pchWord[1]== '|') {
																		pS->pchListTokenType[inTokenSt + inCntToken] = 16;
																	}
																}
															}
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}

		inCntToken++;

		if(pchEd== NULL) break;
		pchSt = pchEd + 1;
		pchEd = strchr(pchSt, ' ');
	}
}

/*
	SetFunction function
	Setup function.
*/
SUIMLIB_API int SetFunction(const int *S, const char *pchFuncName, CallBack pclCallBack)
{
	if(S== NULL) return (ERR_S_NULL);
	int inFlag = IsNullOrEmpty(pchFuncName);
	if(inFlag!= 0) return (ERR_NAME);
	if(pclCallBack== NULL) return (ERR_S_NULL);
	int inLength = strlen(pchFuncName);
	if(inLength>= MAX_KEY_LENGTH - 1) return (ERR_SET_NAME);

	struct ST_SUIM *pS = (struct ST_SUIM *)S;

	int inErrCode = AddCallBack(pS, pchFuncName, pclCallBack);

	return (inErrCode);
}

/*
	SetBuffer function
	Setup suim code.
*/
SUIMLIB_API int SetBuffer(const int *S, const char *pchSuimName, const char *pchBuffer, const int inVarCount, const int inOutCount, int inCallBackCount)
{
	if(S== NULL) return (ERR_S_NULL);
	int inFlag = IsNullOrEmpty(pchSuimName);
	if(inFlag!= 0) return (ERR_NAME);
	inFlag = IsNullOrEmpty(pchBuffer);
	if(inFlag!= 0) return (ERR_BUFFLER);
	if(inVarCount>= MAX_VARIABLE_COUNT) return (ERR_OVERFLOW);

	struct ST_SUIM *pS = (struct ST_SUIM *)S;
	if(pS->pchSuimName!= NULL) return (ERR_SET_NAME);
	if(pS->pchBuffer!= NULL) return (ERR_SET_BUFFER);

	size_t szLength = strlen(pchSuimName);
	pS->pchSuimName = malloc(szLength + 1);
	if(pS->pchSuimName== NULL) return (ERR_MEMORY);
	strcpy(pS->pchSuimName, pchSuimName);

	szLength = strlen(pchBuffer);
	pS->pchBuffer = malloc(szLength + 1);
	if(pS->pchBuffer== NULL) return (ERR_MEMORY);
	strcpy(pS->pchBuffer, pchBuffer);

	int inMaxLine = 0;
	int inCnt, inMax = strlen(pchBuffer);
	for(inCnt= 0;inCnt< inMax;inCnt++)
	{
		if(pchBuffer[inCnt]== '\n') inMaxLine++;
	}

	pS->inMaxLineNum = inMaxLine;

	pS->ppchLines = malloc(sizeof(char *) * (inMaxLine));
	if(pS->ppchLines== NULL) return (ERR_MEMORY);
	memset(pS->ppchLines, 0x00, sizeof(char *) * (inMaxLine));

	//split lines
	inCnt = 0;
	char *pchPos = strtok( pS->pchBuffer, "\n" );
	while( pchPos != NULL && inCnt < inMaxLine )
	{
		pS->ppchLines[inCnt] = pchPos;
		inCnt++;

		pchPos = strtok( NULL, "\n" );
	}

	int inCountFunction = 0;
	for(inCnt= 0;inCnt< inMaxLine;inCnt++)
	{
		if(pS->ppchLines[inCnt][0]== 'f' && pS->ppchLines[inCnt][1]== '|') inCountFunction++;
	}

	//define setting
	pS->inCountVariable = inVarCount;
	if(inVarCount> 0) {
		pS->ppstListVariable = malloc(sizeof(struct ST_VALUE *) * inVarCount);
		memset(pS->ppstListVariable, 0x00, sizeof(struct ST_VALUE *) * inVarCount);
		for(inCnt= 0;inCnt< inVarCount;inCnt++)
		{
			SetValueDouble(pS->ppstListVariable, inCnt, 0.0);
		}

		for(inCnt= 0;inCnt< inMaxLine;inCnt++)
		{
			if(!(pS->ppchLines[inCnt][0]== 's' && pS->ppchLines[inCnt][1]== '|')) continue;

			char *pchKey = strtok(&pS->ppchLines[inCnt][2], ":");
			char *pchValue = strtok(NULL, ":");

			const int inNo = CngKey2No(pchKey);
			if(inNo< 0) return (ERR_OVERFLOW);
			if(inNo>= inMax) return (ERR_OVERFLOW);
			int inErrCode = SetValueString(pS->ppstListVariable, inNo, pchValue);
			if(inErrCode!= SUCCESS) return (inErrCode);
		}
	}

	//callback setting
	pS->inCountCallBack = inCallBackCount;
	if(inCallBackCount> 0) {
		pS->ppstListCallBack = malloc(sizeof(struct ST_VALUE_CALLBACK *) * inCallBackCount);
		memset(pS->ppstListCallBack, 0x00, sizeof(struct ST_VALUE_CALLBACK *) * inCallBackCount);
	}

	//out setting
	pS->pchInArgs = NULL;

	pS->inNoOut = 0;

	pS->inCountOut = inOutCount;
	if(inOutCount> 0) {
		pS->pchOutArgsType = malloc(inOutCount + 1);
		memset(pS->pchOutArgsType, 0x00, inOutCount + 1);

		pS->ppstListOut = malloc(sizeof(struct ST_VALUE *) * inOutCount);
		memset(pS->ppstListOut, 0x00, sizeof(struct ST_VALUE *) * inOutCount);
		for(inCnt= 0;inCnt< inOutCount;inCnt++)
		{
			SetValueDouble(pS->ppstListOut, inCnt, 0.0);
		}
	}

	//function setting
	pS->inCountFunction = inCountFunction;
	if(inCountFunction> 0) {
		pS->ppstListFunction = malloc(sizeof(struct ST_VALUE *) * inCountFunction);
		memset(pS->ppstListFunction, 0x00, sizeof(struct ST_VALUE *) * inCountFunction);
		for(inCnt= 0;inCnt< inMaxLine;inCnt++)
		{
			if(!(pS->ppchLines[inCnt][0]== 'f' && pS->ppchLines[inCnt][1]== '|')) continue;

			char *pchKey = strtok(&pS->ppchLines[inCnt][2], ":");
			char *pchValue = strtok(NULL, ":");
			int inLineNo = atoi(pchValue);

			int inRes = AddFunctionDefine(pS, pchKey, inLineNo);
			if(inRes!= SUCCESS) return (inRes);
		}
	}

	//set start position
	//set jump line no
	//create cmp string
	pS->pchListStartPos = malloc(sizeof(char) * inMaxLine);
	pS->pinListJumpNo = malloc(sizeof(int) * inMaxLine);
	pS->ppchListCmp = malloc(sizeof(char *) * inMaxLine);
	memset(pS->ppchListCmp, 0x00, sizeof(char *) * inMaxLine);
	for(inCnt= 0;inCnt< inMaxLine;inCnt++)
	{
		const char *pchLine = pS->ppchLines[inCnt];

		pS->pchListStartPos[inCnt] = 0;
		pS->pinListJumpNo[inCnt] = 0;
		pS->ppchListCmp[inCnt] = NULL;

		if(pchLine[0]== '-' && pchLine[1]== '-') continue;
		if(pchLine[1]== '|') continue;

		char *pchIndex = strchr(pchLine, ':');
		if(pchIndex== NULL) continue;
		pchIndex+= 1;

		pS->pchListStartPos[inCnt] = (char)(pchIndex - pchLine);

		if(pchIndex[0]== 'i' && pchIndex[1]== ':')
		{
			char *pchIndex2 = strstr(pchIndex, ")e");
			if(pchIndex2!= NULL)
			{
				pS->pinListJumpNo[inCnt] = atoi(pchIndex2 + 2);

				int inSize = (int)(pchIndex2 - &pchIndex[3]);
				pS->ppchListCmp[inCnt] = malloc(inSize + 1);
				memset(pS->ppchListCmp[inCnt], 0x00, inSize + 1);
				strncpy(pS->ppchListCmp[inCnt], &pchIndex[3], inSize);
			}
			continue;
		}

		if(pchIndex[0]== 'g' && pchIndex[1]== ':')
		{
			pchIndex = strchr(pchIndex, ':');
			if(pchIndex!= NULL)
			{
				pS->pinListJumpNo[inCnt] = atoi(pchIndex + 1);
			}
			continue;
		}
	}

	//memory allocate
	int inSize = sizeof(int) * inMaxLine;
	pS->pinListTokenNum = malloc(inSize);
	memset(pS->pinListTokenNum, 0x00, inSize);

	inSize = sizeof(int) * inMaxLine;
	pS->pinListTokenSt = malloc(inSize);
	memset(pS->pinListTokenSt, 0x00, inSize);

	//count token
	int inStToken = 0;
	int inMaxToken = 0;
	for(inCnt= 0;inCnt< inMaxLine;inCnt++)
	{
		const char *pchLine = pS->ppchLines[inCnt];

		if(pchLine[0]== '-' && pchLine[1]== '-') continue;
		if(pchLine[1]== '|') continue;

		if(pS->pchListStartPos[inCnt]== 0) continue;

		char *pchIndex = (char *)(pchLine + pS->pchListStartPos[inCnt]);
		if(pchIndex[0]== 'g' && pchIndex[1]== ':') continue;
		if(pchIndex[0]== 'e' && pchIndex[1]== ':') continue;
		if(pchIndex[0]== 'f' && pchIndex[1]== ':') continue;
		if(pchIndex[0]== 'c' && pchIndex[1]== ':') continue;
		if(pchIndex[0]== 'r' && pchIndex[1]== ':') continue;

		int inCntToken = 0;
		if(pchIndex[0]== 'o' && pchIndex[1]== ':') {
			inCntToken = CountToken(&pchIndex[2]);
			if(inMaxToken< inCntToken) inMaxToken = inCntToken;
		}
/*
		if(pchIndex[0]== 's' && pchIndex[1]== ':') {
			inCntToken = CountToken(&pchIndex[5]);
			if(inMaxToken< inCntToken) inMaxToken = inCntToken;
		}
		if(pchIndex[0]== 'd' && pchIndex[1]== ':') {
			inCntToken = CountToken(&pchIndex[5]);
			if(inMaxToken< inCntToken) inMaxToken = inCntToken;
		}
*/
		if(pchIndex[0]== 'i' && pchIndex[1]== ':') {
			inCntToken = CountToken(pS->ppchListCmp[inCnt]);
			if(inMaxToken< inCntToken) inMaxToken = inCntToken;
		}
		if(pchIndex[2]== '=') {
			inCntToken = CountToken(&pchIndex[3]);
			if(inMaxToken< inCntToken) inMaxToken = inCntToken;
		}

		pS->pinListTokenSt[inCnt] = inStToken;
		inStToken+= inCntToken;

		pS->pinListTokenNum[inCnt] = inCntToken;
	}

	//memory allocate
	pS->inMaxTokenNum = inMaxToken;

	inSize = sizeof(double) * inMaxToken * inMaxLine;
	pS->pdoListToken = malloc(inSize);
	memset(pS->pdoListToken, 0x00, inSize);

	inSize = sizeof(char) * inMaxToken * inMaxLine;
	pS->pchListTokenType = malloc(inSize);
	memset(pS->pchListTokenType, 0x00, inSize);

	inSize = sizeof(char) * MAX_WORD * inMaxToken * inMaxLine;
	pS->pchListTokenWord = malloc(inSize);
	memset(pS->pchListTokenWord, 0x00, inSize);

	//setup token data
	for(inCnt= 0;inCnt< inMaxLine;inCnt++)
	{
		const char *pchLine = pS->ppchLines[inCnt];

		if(pchLine[0]== '-' && pchLine[1]== '-') continue;
		if(pchLine[1]== '|') continue;

		if(pS->pchListStartPos[inCnt]== 0) continue;

		char *pchIndex = (char *)(pchLine + pS->pchListStartPos[inCnt]);
		if(pchIndex[0]== 'g' && pchIndex[1]== ':') continue;
		if(pchIndex[0]== 'e' && pchIndex[1]== ':') continue;
		if(pchIndex[0]== 'f' && pchIndex[1]== ':') continue;
		if(pchIndex[0]== 'c' && pchIndex[1]== ':') continue;
		if(pchIndex[0]== 'r' && pchIndex[1]== ':') continue;

		int inStToken = pS->pinListTokenSt[inCnt];
		if(pchIndex[0]== 'o' && pchIndex[1]== ':') {
			SetTokenData(pS, &pchIndex[2], inStToken);
		}
/*
		if(pchIndex[0]== 's' && pchIndex[1]== ':') {
			SetTokenData(pS, &pchIndex[5], inStToken);
		}
		if(pchIndex[0]== 'd' && pchIndex[1]== ':') {
			SetTokenData(pS, &pchIndex[5], inStToken);
		}
*/
		if(pchIndex[0]== 'i' && pchIndex[1]== ':') {
			SetTokenData(pS, pS->ppchListCmp[inCnt], inStToken);
		}
		if(pchIndex[2]== '=') {
			SetTokenData(pS, &pchIndex[3], inStToken);
		}
	}

	//global line execute
	int inFunc = 0;
	for(inCnt= 0;inCnt< inMaxLine;inCnt++)
	{
		const char *pchLine = pS->ppchLines[inCnt];

		if(pchLine[0]== '-' && pchLine[1]== '-') {
			int inLength = strlen(pchLine);
			if(inLength<= 2) continue;

			int inVersion = atoi(&pchLine[2]);
			if(inVersion== VERSION) continue;

			return (ERR_VERSION);
		}

		if(pchLine[1]== '|') continue;

		pS->inLineNo = inCnt;

		char *pchIndex = (char *)(pchLine + pS->pchListStartPos[inCnt]);

		if(pchIndex[0]== 'f') inFunc = 1;
		if(pchIndex[0]== 'e') inFunc = 0;
		if(inFunc== 1) continue;

		if(pchIndex[0]== 'e' && pchIndex[1]== ':') continue;

		int inLineNo = ExecLine(pS, pchIndex);
		if(inLineNo< 0) return (inLineNo);
	}

	return (SUCCESS);
}

/*
	ExecLine function
*/
inline int ExecLine(struct ST_SUIM *pS, const char *pchLine)
{
	if(pS->inFlgError== 1) return (ERR_NONE);

	if(pchLine[0]== 'i' && pchLine[1]== ':') {
		//if
		Calc(pS);
		if(pS->inErrCode!= SUCCESS) return (pS->inErrCode);

		if(pS->doCalcWork== 1.0) {
			return (pS->inLineNo + 1);
		}
		else {
			int inLineNo = pS->pinListJumpNo[pS->inLineNo];
			return (inLineNo);
		}
	}
	else {
		if(pchLine[0]== 'g' && pchLine[1]== ':') {
			//goto
			int inLineNo = pS->pinListJumpNo[pS->inLineNo];
			return (inLineNo);
		}
		else {
			if(pchLine[0]== 'c' && pchLine[1]== ':') {
				//call function
				if(pS->pchInArgs!= NULL) {
					free(pS->pchInArgs);
					pS->pchInArgs = NULL;
				}

				char pchFuncName[MAX_WORD] = "";

				CallBack pclCallBack = NULL;
				char *pchIndex = strchr(&pchLine[2], ' ');
				if(pchIndex== NULL) {
					strcpy(pchFuncName, &pchLine[2]);

					pclCallBack = GetCallBack(pS, &pchLine[2]);
				}
				else {
					char *pchInArgs = pchIndex + 1;

					int inSize = pchIndex - &pchLine[2];
					memset(pchFuncName, 0x00, MAX_WORD);
					strncpy(pchFuncName, &pchLine[2], inSize);

					pclCallBack = GetCallBack(pS, pchFuncName);
					if(pclCallBack!= NULL) {
						int inLength = strlen(pchInArgs);
						pS->pchInArgs = malloc(inLength + 1);
						strcpy(pS->pchInArgs, pchInArgs);
					}
				}

				if(pclCallBack!= NULL) {
					//exe side function
					int inErrCode = (*pclCallBack)((int *)pS);
					if(inErrCode!= SUCCESS) {
						SetError2(pS, inErrCode, "function call error.");
						return (inErrCode);
					}
				}
				else {
					//suim function
					if(pS->inStackBackNo>= MAX_STACK_FUNCTION) {
						SetError2(pS, ERR_OVERFLOW, "function stack overflow.");
						return (ERR_OVERFLOW);
					}

					pS->pinStackBackNo[pS->inStackBackNo] = pS->inLineNo;
					pS->inStackBackNo++;

					int inLineNo = GetFunctionDefine(pS, pchFuncName);
					if(inLineNo== 0) {
						SetError2(pS, ERR_FUNC_NAME, "can't find function name.");
						return (ERR_FUNC_NAME);
					}

					pS->inLineNo = inLineNo;
				}

				pS->inNoOut = 0;

				return (pS->inLineNo + 1);
			}
			else {
				if(pchLine[0]== 'o' && pchLine[1]== ':') {
					//setup out arg
					Calc(pS);
					if(pS->inErrCode!= SUCCESS) return (pS->inErrCode);

					if(pS->pchCalcWork== NULL) {
						int inErrCode = SetValueDouble(pS->ppstListOut, pS->inNoOut, pS->doCalcWork);
						if(inErrCode!= SUCCESS) return (inErrCode);
					}
					else {
						int inErrCode = SetValueString(pS->ppstListOut, pS->inNoOut, pS->pchCalcWork);
						if(inErrCode!= SUCCESS) return (inErrCode);
					}
					pS->inNoOut++;

					return (pS->inLineNo + 1);
				}
				else {
					if(pchLine[0]== '-' && pchLine[1]== '-') {
						//Separator
						return (ORDER_SEPARATOR);
					}
					else {
						if(pchLine[0]== 'r' && pchLine[1]== ':') {
							//return
							pS->inNoOut = 0;

							return (ORDER_RETURN);
						}
						else {
							if(pchLine[0]== 'e' && pchLine[1]== ':') {
								//function end
								if(pS->inStackBackNo>= 1) {
									pS->inLineNo = pS->pinStackBackNo[pS->inStackBackNo - 1];
									pS->pinStackBackNo[pS->inStackBackNo - 1] = 0;
									pS->inStackBackNo--;

									return (pS->inLineNo + 1);
								}

								return (ORDER_END);
							}
						}
					}
				}
			}
		}
	}

	//calculation
	int inNo = CngKey2No(&pchLine[0]);
	if(inNo< 0) return (ERR_OVERFLOW);
	if(inNo>= pS->inCountVariable) return (ERR_OVERFLOW);

	Calc(pS);
	if(pS->inErrCode!= SUCCESS) return (pS->inErrCode);

	int inErrCode = SUCCESS;
	if(pS->pchCalcWork== NULL) {
		inErrCode = SetValueDouble(pS->ppstListVariable, inNo, pS->doCalcWork);
	}
	else {
		inErrCode = SetValueString(pS->ppstListVariable, inNo, pS->pchCalcWork);

		free(pS->pchCalcWork);
		pS->pchCalcWork = NULL;
	}
	if(inErrCode!= SUCCESS) return (inErrCode);

	return (pS->inLineNo + 1);
}

/*
	IsExistFunction function
	Check exist function.
*/
SUIMLIB_API const int IsExistFunction(const int *S, const char *pchFuncName)
{
	char *pchLine;

	if(S== NULL) return (0);
	int inFlag = IsNullOrEmpty(pchFuncName);
	if(inFlag!= 0) return (0);

	struct ST_SUIM *pS = (struct ST_SUIM *)S;

	int inLineNo = GetFunctionDefine(pS, pchFuncName);
	if(inLineNo== 0) return (0);

	return (1);
}

/*
	ExecFunction function
	Execute function.
*/
SUIMLIB_API const int ExecFunction(const int *S, const char *pchFuncName)
{
	char *pchLine;

	if(S== NULL) return (ERR_S_NULL);
	int inFlag = IsNullOrEmpty(pchFuncName);
	if(inFlag!= 0) return (ERR_S_NULL);

	struct ST_SUIM *pS = (struct ST_SUIM *)S;

	int inLineNo = GetFunctionDefine(pS, pchFuncName);
	if(inLineNo== 0) return (ERR_FUNC_NAME);

	inLineNo++;

	while(1)
	{
		pS->inLineNo = inLineNo;
		inLineNo = ExecLine(pS, (char *)(pS->ppchLines[inLineNo] + pS->pchListStartPos[inLineNo]));
		if(inLineNo< 0) {
			if(inLineNo== ORDER_SEPARATOR || inLineNo== ORDER_RETURN || inLineNo== ORDER_END) {
				//finish
				break;
			}
			else {
				//error
				return (inLineNo);
			}
		}
	}

/*
	int inLineNoBase = inLineNo;
	double doTime = 0.0;
	int inCnt;
	for(inCnt= 0;inCnt< 20;inCnt++) {
		clock_t start = clock();

		inLineNo = inLineNoBase;

		while(1)
		{
			pS->inLineNo = inLineNo;
			inLineNo = ExecLine(pS, (char *)(pS->ppchLines[inLineNo] + pS->pchListStartPos[inLineNo]));
			if(inLineNo< 0) break;
		}

		clock_t end = clock();
		doTime += (double)(end - start) / CLOCKS_PER_SEC;
	}
	printf("time= %f ms\n", doTime / 20.0 * 1000.0);

	if(pS->inFlgError) {
		printf("err:%s", pS->pchErrMsg);
	}
*/

/*
--01
s|ad:------ end ------
s|ab:level
s|ac:----- start -----
s|ae:BREAK_EFFECT
s|aa:resonance
s|af:data_prim_key=
f|init:20
f|exec:33
--
8:ag=0
9:ah=0
10:ai=0
11:aj=0
12:ak=0
13:al=0
14:am=0
15:an=0.1
16:ao=0
18:ap=0.1

22:f:init
23:ai=aq
24:aj=ar
25:o:aq
25:o:ar
25:c:WorldPos2BlockPos ag,ah
26:al=as
27:am=25.0 as /
28:an=0.1
29:ao=0.1
31:o:aa
31:c:PlaySE
32:e:

35:f:exec
36:ak=ak ap +
38:o:at
38:o:au
38:c:WorldPos2BlockPos av,aw
39:i:(ag av != ah aw != ||)e41
40:o:0
40:r:
43:i:(ax ab !=)e44
44:o:0
44:r:
47:an=an am ap * +
49:o:-an
49:o:an
49:c:MathRandom ay
50:aq=ai ay 100.0 / +
51:o:-an
51:o:an
51:c:MathRandom az
52:ar=aj az 100.0 / +
55:ao=ao ap -
56:i:(ao 0 <=)e63
57:o:ac
57:c:Printf
58:ao=aA
59:o:ad
59:c:Printf
61:o:ae
61:o:af aB +
61:c:AddEffect
64:i:(ak al >)e77
65:o:aC
65:o:ag
65:o:ah
65:c:SetBreak
68:aD=0
70:i:(aD 8 >)e75
71:o:ae
71:o:af aB +
71:c:AddEffect
72:aD=aD 1 +
73:g:69
76:o:0
76:r:
79:o:1
79:r:
80:e:
--
*/

	return (SUCCESS);
}



//----------------------------------------------------------------
//----------------------------------------------------------------



#if defined(DEBUG)

int main(int argc, char **argv)
{
	const int *S = New();

//	char pchBuffer[] = "--01\nf|test:3\n--\n9:f:test\n11:d:aa=0\n13:i:(aa 1000000 <)e8\n14:aa=aa 1 +\n15:g:5\n18:e:\n--";
	char pchBuffer[] = "--01\ns|ad:------ end ------\ns|ab:level\ns|ac:----- start -----\ns|ae:BREAK_EFFECT\ns|aa:resonance\ns|af:data_prim_key=\nf|init:20\nf|exec:33\n--\n8:ag=0\n9:ah=0\n10:ai=0\n11:aj=0\n12:ak=0\n13:al=0\n14:am=0\n15:an=0.1\n16:ao=0\n18:ap=0.1\n22:f:init\n23:ai=aq\n24:aj=ar\n25:o:aq\n25:o:ar\n25:c:WorldPos2BlockPos ag,ah\n26:al=as\n27:am=25.0 as /\n28:an=0.1\n29:ao=0.1\n31:o:aa\n31:c:PlaySE\n32:e:\n35:f:exec\n36:ak=ak ap +\n38:o:at\n38:o:au\n38:c:WorldPos2BlockPos av,aw\n39:i:(ag av != ah aw != ||)e41\n40:o:0\n40:r:\n43:i:(ax ab !=)e44\n44:o:0\n44:r:\n47:an=an am ap * +\n49:o:-an\n49:o:an\n49:c:MathRandom ay\n50:aq=ai ay 100.0 / +\n51:o:-an\n51:o:an\n51:c:MathRandom az\n52:ar=aj az 100.0 / +\n55:ao=ao ap -\n56:i:(ao 0 <=)e63\n57:o:ac\n57:c:Printf\n58:ao=aA\n59:o:ad\n59:c:Printf\n61:o:ae\n61:o:af aB +\n61:c:AddEffect\n64:i:(ak al >)e77\n65:o:aC\n65:o:ag\n65:o:ah\n65:c:SetBreak\n68:aD=0\n70:i:(aD 8 >)e75\n71:o:ae\n71:o:af aB +\n71:c:AddEffect\n72:aD=aD 1 +\n73:g:69\n76:o:0\n76:r:\n79:o:1\n79:r:\n80:e:\n--";
	SetBuffer(S, "test", pchBuffer, 28, 3, 64);

	ExecFunction(S, "init");

	ExecFunction(S, "exec");

	double doRes = GetOutDouble(S, 0);
	char *pchRes = LibGetOutString(S, 1);

	Dispose(S);

	return (0);
}

#endif



//----------------------------------------------------------------
//----------------------------------------------------------------
