
#include <StdFuncs.h>
#include <Args.h>
#include <Dir.h>
#include <ctype.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

#ifdef __amigaos4__

/* Lovely version structure.  Only Amiga makes it possible! */

static const struct Resident g_oROMTag __attribute__((used)) =
{
	RTC_MATCHWORD,
	(struct Resident *) &g_oROMTag,
	(struct Resident *) (&g_oROMTag + 1),
	RTF_AUTOINIT,
	0,
	NT_LIBRARY,
	0,
	"ls",
	"\0$VER: ls 2.00 (11.07.2009)\r\n",
	NULL
};

#endif /* __amigaos4__ */

/* Template for use in obtaining command line parameters */

#define ARGS_WILDCARD 0
#define ARGS_PAUSE 1
#define ARGS_NUM_ARGS 2

/* Macros for printing the results in a platform independent way.  This is not particularly */
/* nice but it's the easiest way of dealing with Amiga specific CSI codes */

#ifdef __amigaos4__

#define PRINT_DIR printf("%c0;33;40mDir", 0x9b);
#define PRINT_LINK_PREFIX printf("%c0;34;40m", 0x9b);
#define PRINT_LINK printf("%d", a_poEntry->iSize);
#define PRINT_NAME printf(" %s%c0;31;40m\n", a_poEntry->iName, 0x9b);

#else /* ! __amigaos4__ */

#define PRINT_DIR printf("Dir");
#define PRINT_LINK_PREFIX
#define PRINT_LINK printf("Link");
#define PRINT_NAME printf(" %s\n", a_poEntry->iName);

#endif /* ! __amigaos4__ */

static bool g_bBreak;					/* Set to true if when ctrl-c is hit by the user */
static int g_iShellHeight;				/* Height of the shell window in lines */
static char *g_apcArgs[ARGS_NUM_ARGS];	/* Array of arguments */

/* Written: Sunday 29-Mar-2009 5:38 pm */

static void PrintDetails(const TEntry *a_poEntry)
{
	unsigned Index, Length;
	char Date[20], Time[20];

	/* Print "Dir", "Link" or the file's size as appropriate */

	if (a_poEntry->IsDir())
	{
		PRINT_DIR;
		Length = 3;
	}
	else if (a_poEntry->IsLink())
	{
		PRINT_LINK_PREFIX;
		Length = PRINT_LINK;
	}
	else
	{
		Length = printf("%d", a_poEntry->iSize);
	}

	/* Print a number of spaces after the entry's type or size, ensuring that we don't go into an */
	/* infinite loop if a DivX movie is found */

	for (Index = 0; Index < (11 - Length); ++Index)
	{
		putchar(' ');
	}

	/* Determine the entry's attributes and print them on the screen */

	putchar((a_poEntry->IsReadable()) ? 'r' : '-');
	putchar((a_poEntry->IsWriteable()) ? 'w' : '-');
	putchar((a_poEntry->IsExecutable()) ? 'e' : '-');
	putchar((a_poEntry->IsDeleteable()) ? 'd' : '-');

	/* Convert the entry's date and time to a string and display it */

	DEBUGCHECK((Utils::TimeToString(Date, Time, *a_poEntry) != EFalse), "PrintDetails() => Utils::TimeToString() returned failure");
	printf(" %s %s", Time, Date);

	/* Calculate the length of the date and time that was printed */

	Length = (strlen(Time) + strlen(Date) + 2);

	/* And print a number of spaces after the entry's date and time, to ensure that "short" dates such as "today" */
	/* are padded out to the correct length */

	for (Index = 0; Index < (19 - Length); ++Index)
	{
		putchar(' ');
	}

	/* And finally print the name of the file itself */

	PRINT_NAME;
}

/* Written: Saturday 04-Jul-2009 13:11 pm */

static void SignalHandler(int /*a_iSignal*/)
{
	/* Signal that ctrl-c has been pressed so that we break out of the listing routine */

	g_bBreak = true;
}

/* Written: Sunday 29-Mar-2009 5:24 pm */

int main(int a_iArgC, char *a_ppcArgV[])
{
	char Char;
	int Index, NumEntries, NumFiles, NumLines, RetVal;
	unsigned int TotalSize;
	enum TDirSortOrder SortOrder;
	RDir Dir;
	TEntryArray *DirEntries;

	/* Assume failure */

	RetVal = RETURN_ERROR;

	/* First off, install a ctrl-c handler */

	signal(SIGINT, SignalHandler);

	/* Find out the height of the shell, for use by the -p option */

	if ((g_iShellHeight = Utils::GetShellHeight()) != -1)
	{
		/* Assume that no arguments have been passed in */

		memset(g_apcArgs, 0, sizeof(g_apcArgs));

		/* By default, sort alphabetically */

		SortOrder = EDirSortNameAscending;

		/* Iterate through all of the arguments passed in and find any options that we recognise. */
		/* Anything not recognised will be treated as a path to be used, with the last one found */
		/* being the one that is actually used */

		for (Index = 1; Index < a_iArgC; ++Index)
		{
			if (stricmp(a_ppcArgV[Index], "-p") == 0)
			{
				g_apcArgs[ARGS_PAUSE] = a_ppcArgV[Index];
			}
			else if (strnicmp(a_ppcArgV[Index], "-o:", 3) == 0)
			{
				if (strnicmp(&a_ppcArgV[Index][3], "-n", 2) == 0)
				{
					SortOrder = EDirSortNameDescending;
				}
				else if (strnicmp(&a_ppcArgV[Index][3], "d", 2) == 0)
				{
					SortOrder = EDirSortDateAscending;
				}
				else if (strnicmp(&a_ppcArgV[Index][3], "-d", 2) == 0)
				{
					SortOrder = EDirSortDateDescending;
				}
				else if (strnicmp(&a_ppcArgV[Index][3], "s", 2) == 0)
				{
					SortOrder = EDirSortSizeAscending;
				}
				else if (strnicmp(&a_ppcArgV[Index][3], "-s", 2) == 0)
				{
					SortOrder = EDirSortSizeDescending;
				}
			}
			else
			{
				g_apcArgs[ARGS_WILDCARD] = a_ppcArgV[Index];
			}
		}

		/* Scan the requested directory for entries */

		if (Dir.Open((g_apcArgs[0]) ? g_apcArgs[0] : "") == KErrNone)
		{
			if (Dir.Read(DirEntries, SortOrder) == KErrNone)
			{
				/* Indicate success */

				RetVal = RETURN_OK;

				/* Loop around and display the entries in the directory, keeping track of the */
				/* # of files found and their total size */

				NumFiles = NumLines = TotalSize = 0;
				NumEntries = DirEntries->Count();

				for (Index = 0; Index < NumEntries; ++Index)
				{
					PrintDetails(&(*DirEntries)[Index]);
					++NumFiles;
					++NumLines;

					/* Only add the size of this is not a directory, as the value of TEntry::iSize */
					/* is undefined for directories */

					if (!((*DirEntries)[Index].IsDir()))
					{
						TotalSize += (*DirEntries)[Index].iSize;
					}

					/* If the user has requested to pause between screens and we are displaying the */
					/* last line of the listing, pause and see if the user wishes to continue */

					if ((g_apcArgs[ARGS_PAUSE]) && (NumLines == (g_iShellHeight - 1)))
					{
						/* See if the user wants to continue and break out if not */

						printf("More? ");

						/* Loop around until we have the answer we want.  We do it like this as some */
						/* implementations of getchar() return a line feed as a separate character and */
						/* some don't */

						do
						{
							Char = (char) toupper(getchar());
						}
						while ((Char != 'N') && (Char != 'Y'));

						/* If the user has chosen to discontinue then break out of the loop */

						if (toupper(Char) == 'N')
						{
							break;
						}

						/* Reset the line count for the next screenfull */

						NumLines = 0;
					}

					/* Check to see if ctrl-c was pressed and if so, break out of the loop */

					if (g_bBreak)
					{
						printf("LS: ***Break\n");

						break;
					}
				}

				/* Only print out a summary if ctrl-c wasn't pressed */

				if (Index == NumEntries)
				{
					printf("%d Files - %d bytes used Bro!\n", NumFiles, TotalSize);
				}
			}
			else
			{
				Utils::Error("Unable to read directory.");
			}

			Dir.Close();
		}
		else
		{
			Utils::Error("Unable to open file or directory for reading.");
		}
	}
	else
	{
		Utils::Error("Unable to obtain window dimensions");
	}

	return(RetVal);
}
