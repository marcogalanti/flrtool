/*
 * flrtool.c
 *
 *
 * DISCLAIMER:
 * This program is provided AS IS with no warranty of any kind, and
 * the author makes no representation with respect to the adequacy of this
 * program for any particular purpose or with respect to its adequacy to
 * produce any particular result.
 * The author shall not be liable for loss or damage arising out of
 * the use of this program regardless of how sustained, and
 * in no event shall the author be liable for special, direct, indirect
 * or consequential damage, loss, costs or fees or expenses of any
 * nature or kind.
 *
 *
 *
 * v0.9 27th July 2005  - Initial coding 	 (Marco Galanti)
 * v1.0 31st July 2005  - First official Release (Marco Galanti)
 * v1.1 21st April 2006 - Changed name and args  (Marco Galanti)
 * v1.2 29th April 2020 - open sourced code (Marco Galanti)
 *
 *
 *
 */

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

enum MESI { JAN, FEB, MAR, APR, MAY, JUN, JUL, AUG, SET, OCT, NOV, DIC };
int MONTHS[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
HANDLE          hFile;
FILETIME        at, mt, ct;
const FILETIME *lpFileTime;
SYSTEMTIME      st, stL;
char           *lpszString = NULL;
char           *command;

void usage(char *cmd) {
	 printf("\n- flrtool v1.2 - \n\
             Marco Galanti - marco.galanti@gmail.com\n\
             Dell EMC Unity file-level retention CLI tool");
	 printf("\nUsage:\n%s [FILENAME] [-s] [-m | -a | -c] YYYYMMDDHHmm", cmd);
	 printf("\n *** Set create/modify/access time of a file\n");
	 printf("\n%s [FILENAME] [-x] [-m | -a | -c] number[Y|M|D|H|m]", cmd);
	 printf("\n *** Xtend create/modify/access time of a file\n");
	 printf("\n%s [FILENAME] [-flr]", cmd);
	 printf("\n *** Commit File Level Retention\n");
	 printf("\n%s [ -h ]", cmd);
	 printf("\n *** Print this help\n\n");
}

int             displayfile(HANDLE hf)
{
	 if (!GetFileTime(hf, &ct, &at, &mt))
			return FALSE;
	 char           *lpszString = (char *) malloc(128);

// display creation time
	 lpFileTime = &ct;
	 FileTimeToSystemTime(lpFileTime, &st);
	 SystemTimeToTzSpecificLocalTime(NULL, &st, &stL);
	 wsprintf(lpszString, TEXT("%02d/%02d/%d  %02d:%02d"), stL.wMonth, stL.wDay,
						stL.wYear, stL.wHour, stL.wMinute);
	 printf("Creation Time\t\t%s\n", lpszString);
// display access time
	 lpFileTime = &at;
	 FileTimeToSystemTime(lpFileTime, &st);
	 SystemTimeToTzSpecificLocalTime(NULL, &st, &stL);
	 wsprintf(lpszString, TEXT("%02d/%02d/%d  %02d:%02d"), stL.wMonth, stL.wDay,
						stL.wYear, stL.wHour, stL.wMinute);
	 printf("Access Time\t\t%s\n", lpszString);
// display modification time
	 lpFileTime = &mt;
	 FileTimeToSystemTime(lpFileTime, &st);
	 SystemTimeToTzSpecificLocalTime(NULL, &st, &stL);
	 wsprintf(lpszString, TEXT("%02d/%02d/%d  %02d:%02d"), stL.wMonth, stL.wDay,
						stL.wYear, stL.wHour, stL.wMinute);
	 printf("Modification Time\t%s\n", lpszString);
	 free(lpszString);
	 exit(0);	// end
						// of
						// display
}

//increase - increase sec,min,hour etc. in a date//
void            increase(char t_elem, SYSTEMTIME * st_elem)
{
	 if (!t_elem)
	 {
			printf("Missing time argument!\n");
	 } else
	 {
			switch (t_elem)
			{
			case 'Y':
				 st_elem->wYear++;
				 break;
			case 'M':
				 if (st_elem->wMonth + 1 > 12)
				 {
						st_elem->wMonth = 1;
						increase('Y', &st_elem);
						break;
				 }
				 st_elem->wMonth++;
				 break;
			case 'D':
				 if (!st_elem->wYear % 4)
						MONTHS[FEB] = 29;
				 if (st_elem->wDay + 1 > MONTHS[(st_elem->wMonth) - 1])
				 {
						st_elem->wDay = 1;
						increase('M', &st_elem);
						break;
				 }
				 st_elem->wDay++;
				 break;
			case 'H':
				 if (st_elem->wHour + 1 > 23)
				 {
						st_elem->wHour = 0;
						increase('D', &st_elem);
						break;
				 }
				 st_elem->wHour++;
				 break;
			case 'm':
				 if (st_elem->wMinute + 1 > 59)
				 {
						st_elem->wMinute = 0;
						increase('H', &st_elem);
						break;
				 }
				 st_elem->wMinute++;
				 break;
			default:
				 printf("Wrong increase value!\n");
				 exit(-2);
			}
	 }
}	// end // of // increase

//extendfile - extends file date in the future//
int             extendfile(char *opt, char *time, HANDLE hf)
{
	 if (!time)
	 {
			printf("Missing time argument!\n");
			usage(command);
			return (-4);
	 }
	 char           *str1, *str2;

	 str1 = (char *) malloc(1);
	 int             num, len = strlen(time);

	 str1[0] = time[len - 1];
	 str2 = (char *) malloc(len - 1);
	 strncpy(str2, time, len - 1);
	 num = atoi(str2);
	 if (!num)
	 {
			printf("Wrong time argument!\n");
			usage(command);
			exit(-4);
	 }
	 if (!GetFileTime(hf, &ct, &at, &mt))
			return FALSE;
	 if (!strcmp("-m", opt))
	 {
			lpFileTime = &mt;
			FileTimeToSystemTime(lpFileTime, &st);
			// SystemTimeToTzSpecificLocalTime(NULL,
			// &st,
			// &stL);
			// printf("\nY=%d\nM=%d\nD=%d\nH=%d\nm=%d\ns=%d\n",
			// \
			// stL.wYear,stL.wMonth,stL.wDay,stL.wHour,stL.wMinute,stL.wSecond);
			switch (str1[0])
			{
			case 'Y':
				 st.wYear += num;
				 break;
			case 'M':
				 while (num > 12)
				 {
						num -= 12;
						increase('Y', &st);
				 }
				 if (st.wMonth + num > 12)
				 {
						st.wMonth = (st.wMonth + num - 12);
						increase('Y', &st);
						break;
				 }
				 st.wMonth += num;
				 break;
			case 'D':
				 if (!st.wYear % 4)
						MONTHS[FEB] = 29;
				 while (num > MONTHS[st.wMonth - 1])
				 {
						num -= MONTHS[st.wMonth - 1];
						increase('M', &st);
				 }
				 if (st.wDay + num > MONTHS[st.wMonth - 1])
				 {
						st.wDay = (st.wDay + num - MONTHS[st.wMonth - 1]);
						increase('M', &st);
						break;
				 }
				 st.wDay += num;
				 break;
			case 'H':
				 while (num > 23)
				 {
						num -= 24;
						increase('D', &st);
				 }
				 if (st.wHour + num > 23)
				 {
						st.wHour = (st.wHour + num - 24);
						increase('D', &st);
						break;
				 }
				 st.wHour += num;
				 break;
			case 'm':
				 while (num > 59)
				 {
						num -= 60;
						increase('H', &st);
				 }
				 if (st.wMinute + num > 59)
				 {
						st.wMinute = (st.wMinute + num - 60);
						increase('H', &st);
						break;
				 }
				 st.wMinute += num;
				 break;
			default:
				 printf("Wrong time size!\n");
				 usage(command);
				 CloseHandle(hf);
				 exit(-2);
			}
			// TzSpecificLocalTimeToSystemTime(NULL,&stL,&st);
			SystemTimeToFileTime(&st, &mt);
			SetFileTime(hf, NULL, NULL, &mt);
			printf("Modification Time Extended!\n");
			CloseHandle(hFile);
	 }
	 if (!strcmp("-a", opt))
	 {
			lpFileTime = &at;
			FileTimeToSystemTime(lpFileTime, &st);
			// SystemTimeToTzSpecificLocalTime(NULL,
			// &st,
			// &stL);
			switch (str1[0])
			{
			case 'Y':
				 st.wYear += num;
				 break;
			case 'M':
				 while (num > 12)
				 {
						num -= 12;
						increase('Y', &st);
				 }
				 if (st.wMonth + num > 12)
				 {
						st.wMonth = (st.wMonth + num - 12);
						increase('Y', &st);
						break;
				 }
				 st.wMonth += num;
				 break;
			case 'D':
				 if (!st.wYear % 4)
						MONTHS[FEB] = 29;
				 while (num > MONTHS[st.wMonth - 1])
				 {
						num -= MONTHS[st.wMonth - 1];
						increase('M', &st);
				 }
				 if (st.wDay + num > MONTHS[st.wMonth - 1])
				 {
						st.wDay = (st.wDay + num - MONTHS[st.wMonth - 1]);
						increase('M', &st);
						break;
				 }
				 st.wDay += num;
				 break;
			case 'H':
				 while (num > 23)
				 {
						num -= 24;
						increase('D', &st);
				 }
				 if (st.wHour + num > 23)
				 {
						st.wHour = (st.wHour + num - 24);
						increase('D', &st);
						break;
				 }
				 st.wHour += num;
				 break;
			case 'm':
				 while (num > 59)
				 {
						num -= 60;
						increase('H', &st);
				 }
				 if (st.wMinute + num > 59)
				 {
						st.wMinute = (st.wMinute + num - 60);
						increase('H', &st);
						break;
				 }
				 st.wMinute += num;
				 break;
			default:
				 printf("Wrong time size!\n");
				 usage(command);
				 CloseHandle(hf);
				 exit(-2);
			}
			// TzSpecificLocalTimeToSystemTime(NULL,&stL,&st);
			SystemTimeToFileTime(&st, &at);
			SetFileTime(hf, NULL, &at, NULL);
			printf("Access Time Extended!\n");
			CloseHandle(hFile);
	 }
	 if (!strcmp("-c", opt))
	 {
			lpFileTime = &ct;
			FileTimeToSystemTime(lpFileTime, &st);
			// SystemTimeToTzSpecificLocalTime(NULL,
			// &st,
			// &stL);
			switch (str1[0])
			{
			case 'Y':
				 st.wYear += num;
				 break;
			case 'M':
				 while (num > 12)
				 {
						num -= 12;
						increase('Y', &st);
				 }
				 if (st.wMonth + num > 12)
				 {
						st.wMonth = (st.wMonth + num - 12);
						increase('Y', &st);
						break;
				 }
				 st.wMonth += num;
				 break;
			case 'D':
				 if (!st.wYear % 4)
						MONTHS[FEB] = 29;
				 while (num > MONTHS[st.wMonth - 1])
				 {
						num -= MONTHS[st.wMonth - 1];
						increase('M', &st);
				 }
				 if (st.wDay + num > MONTHS[st.wMonth - 1])
				 {
						st.wDay = (st.wDay + num - MONTHS[st.wMonth - 1]);
						increase('M', &st);
						break;
				 }
				 st.wDay += num;
				 break;
			case 'H':
				 while (num > 23)
				 {
						num -= 24;
						increase('D', &st);
				 }
				 if (st.wHour + num > 23)
				 {
						st.wHour = (st.wHour + num - 24);
						increase('D', &st);
						break;
				 }
				 st.wHour += num;
				 break;
			case 'm':
				 while (num > 59)
				 {
						num -= 60;
						increase('H', &st);
				 }
				 if (st.wMinute + num > 59)
				 {
						st.wMinute = (st.wMinute + num - 60);
						increase('H', &st);
						break;
				 }
				 st.wMinute += num;
				 break;
			default:
				 printf("Wrong time size!\n");
				 usage(command);
				 CloseHandle(hf);
				 exit(-2);
			}
			// TzSpecificLocalTimeToSystemTime(NULL,&stL,&st);
			SystemTimeToFileTime(&st, &ct);
			SetFileTime(hf, &ct, NULL, NULL);
			printf("Creation Time Extended!\n");
			CloseHandle(hFile);
	 }
	 exit(0);
}	// ---End-of-Extend---//

//setfile - set file date in the future//
int             setfile(char *opt, char *time, HANDLE hf)
{
	 char           *year, *month, *day, *hour, *minute;
	 int             dyear, dmonth, dday, dhour, dminute, index, bias;

	 if (!time)
	 {
			printf("Missing time argument!\n");
			usage(command);
			CloseHandle(hf);
			return (-4);
	 }
	 if (strlen(time) != 12)
	 {
			printf("Wrong time argument!\n");
			usage(command);
			CloseHandle(hf);
			exit(-5);
	 }
	 if (opt[0] != '-')
	 {
			printf("Wrong Parameter!\n");
			usage(command);
			CloseHandle(hf);
			exit(-6);
	 }
	 for (index = 0; index < 12; index++)
	 {
			if (!isdigit((int) time[index]))
			{
				 printf("Wrong Time Parameter!\n");
				 usage(command);
				 CloseHandle(hf);
				 exit(-8);
			}
	 }
	 year = (char *) malloc(4);
	 month = (char *) malloc(2);
	 day = (char *) malloc(2);
	 hour = (char *) malloc(2);
	 minute = (char *) malloc(2);
	 year[0] = time[0];
	 year[1] = time[1];
	 year[2] = time[2];
	 year[3] = time[3];
	 month[0] = time[4];
	 month[1] = time[5];
	 day[0] = time[6];
	 day[1] = time[7];
	 hour[0] = time[8];
	 hour[1] = time[9];
	 minute[0] = time[10];
	 minute[1] = time[11];
	 dyear = atoi(year);
	 dmonth = atoi(month);
	 dday = atoi(day);
	 dhour = atoi(hour);
	 dminute = atoi(minute);
	 st.wYear = dyear;
	 st.wMonth = dmonth;
	 st.wDay = dday;
	 st.wHour = dhour;
	 st.wMinute = dminute;
	 st.wSecond = 0;
	 switch (opt[1])
	 {
	 case 'm':
			SystemTimeToTzSpecificLocalTime(NULL, &st, &stL);
			bias = stL.wHour - st.wHour;
			// printf("\nDIFF=%d\n",bias);
			// TzSpecificLocalTimeToSystemTime(NULL,&stL,&st);
			st.wHour -= bias;
			SystemTimeToFileTime(&st, &mt);
			// printf("\nDUTC=%d\nDLOCAL=%d\nHUTC=%d\nHLOCAL=%d\n",
			// \
			// st.wDay,st.wHour,stL.wDay,stL.wHour);
			SetFileTime(hf, NULL, NULL, &mt);
			printf("Modification Time Set!\n");
			break;
	 case 'c':
			SystemTimeToTzSpecificLocalTime(NULL, &st, &stL);
			bias = stL.wHour - st.wHour;
			// printf("\nDIFF=%d\n",bias);
			// TzSpecificLocalTimeToSystemTime(NULL,&stL,&st);
			st.wHour -= bias;
			SystemTimeToFileTime(&st, &ct);
			// printf("\nDUTC=%d\nDLOCAL=%d\nHUTC=%d\nHLOCAL=%d\n",
			// \
			// st.wDay,st.wHour,stL.wDay,stL.wHour);
			SetFileTime(hf, &ct, NULL, NULL);
			printf("Creation Time Set!\n");
			break;
	 case 'a':
			SystemTimeToTzSpecificLocalTime(NULL, &st, &stL);
			bias = stL.wHour - st.wHour;
			// printf("\nDIFF=%d\n",bias);
			// TzSpecificLocalTimeToSystemTime(NULL,&stL,&st);
			st.wHour -= bias;
			SystemTimeToFileTime(&st, &at);
			// printf("\nDUTC=%d\nDLOCAL=%d\nHUTC=%d\nHLOCAL=%d\n",
			// \
			// st.wDay,st.wHour,stL.wDay,stL.wHour);
			SetFileTime(hf, NULL, &at, NULL);
			printf("Access Time Set!\n");
			break;
	 default:
			printf("Wrong Parameter!\n");
			CloseHandle(hf);
			exit(-7);
			break;
	 }
}	// -end-of-setfile-//

///////////////////////
//      M A I N      //
///////////////////////

int             main(int argc, char *argv[])
{
	 int             retcode;

	 command = argv[0];
	 if ((argc == 1) || (!strcmp("-h", argv[1])))
	 {
			usage(command);
			return (0);
	 }
	 hFile =
			CreateFile(TEXT(argv[1]), GENERIC_WRITE, FILE_SHARE_WRITE, NULL,
								 OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
								 NULL);
	 // if
	 // (hFile
	 // ==
	 // INVALID_HANDLE_VALUE)
	 // {
	 retcode = GetLastError();
	 switch (retcode)
	 {
	 case 0:
			break;
	 case 1:
			printf("\nError in function!\n\n");
			exit(retcode);
	 case 2:
			printf("\nFile %s not found!\n\n", argv[1]);
			exit(retcode);
	 case 3:
			printf("\nPath not found!\n\n");
			exit(retcode);
	 case 4:
			printf("\nFile open failed!\n\n");
			exit(retcode);
	 case 5:
			printf("\nAccess denied!\n\n");
			exit(retcode);
	 default:
			printf("\nUnknow Error!\n\n");
			exit(retcode);
	 }
	 if (argc == 2)
	 {	// Show
			// file
			// time
			// stamps
			displayfile(hFile);
			CloseHandle(hFile);
			exit(0);
	 }
	 // option
	 // isn't
	 // Xtend
	 // nor
	 // set,
	 // so
	 // it's
	 // wrong
	 if (
			 ((strcmp
				 ("-s", argv[2]) && strcmp("-x", argv[2])
				 && strcmp("-flr", argv[2]))))
	 {
			printf("Wrong option!\n");
			usage(command);
			CloseHandle(hFile);
			exit(-1);
	 }
	 // Set
	 // file
	 // time
	 // Read-Only
	 // for
	 // CWORM
	 // Commit
	 if (!strcmp("-flr", argv[2]))
	 {
			CloseHandle(hFile);
			if (SetFileAttributes(argv[1], FILE_ATTRIBUTE_READONLY))
			{
				 printf("\nFile %s committed Retention !\n", argv[1]);
				 exit(0);
			} else
			{
				 printf("\nFailed commit: Error %d\n", GetLastError());
				 exit(-9);
			}
	 }
	 // Extend
	 // file
	 // time
	 if (!strcmp("-x", argv[2]))
	 {
			// -------------------//
			if (argc > 3)
			{
				 int             ret = extendfile(argv[3], argv[4], hFile);

				 CloseHandle(hFile);
				 exit(ret);
			} else
			{
				 printf("Missing argument!\n");
				 CloseHandle(hFile);
				 exit(-2);
			}
	 }
	 // Set
	 // file
	 // time
	 if (!strcmp("-s", argv[2]))
	 {
			// -------------------//
			if (argc > 3)
			{
				 int             ret = setfile(argv[3], argv[4], hFile);

				 CloseHandle(hFile);
				 exit(ret);
			} else
			{
				 printf("Missing argument!\n");
				 CloseHandle(hFile);
				 exit(-3);
			}
			CloseHandle(hFile);
			exit(0);
	 }
}
