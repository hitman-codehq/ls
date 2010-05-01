
#include <StdFuncs.h>
#include <Args.h>
#include <Dir.h>
#include <ctype.h>
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

// TODO: CAW - Sort entries by name and add switches -o:s -:d -o:n and reverse versions
// TODO: CAW - ls nonexistentdir doesn't give an error

/* Written: Saturday 04-Jul-2009 13:11 pm */

void SignalHandler(int /*a_iSignal*/)
{
	/* Signal that ctrl-c has been pressed so that we break out of the display routine */

	g_bBreak = true;
}

/* Written: Sunday 29-Mar-2009 5:38 pm */

static void PrintDetails(const TEntry *a_poEntry)
{
	unsigned Attrib, Index, Length, Bit;
	char Date[20], Time[20], Protect[] = { 'r', 'w', 'e', 'd' };

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

	/* Iterate through the entry's attributes and print them on the screen.  Set attributes are */
	/* indicated by their flag being 0 and cleared ones by the flag being 1 */

	Bit = 8;

	for (Attrib = 0; Attrib < 4; ++Attrib, Bit >>= 1)
	{
		putchar(((a_poEntry->iAttributes & Bit) ? '-' : Protect[Attrib]));
	}

	/* Convert the entry's date and time to a string and display it */

	Utils::TimeToString(Date, Time, *a_poEntry);
	printf(" %s %s", Time, Date);

	/* And finally print the name of the file itself */

	PRINT_NAME;
}

/* Written: Sunday 29-Mar-2009 5:24 pm */

int main(int a_iArgC, char *a_ppcArgV[])
{
	char Char;
	int Index, NumEntries, NumFiles, NumLines, RetVal;
	unsigned int TotalSize;
	RDir Dir;
	TEntryArray *DirEntries;

	/* Assume failure */

	RetVal = RETURN_ERROR;

	/* First off, install a ctrl-c handler */

	signal(SIGINT, SignalHandler);

	/* Find out the height of the shell, for use by the -p option */

	//if (Utils::GetShellHeight(&g_iShellHeight)) // TODO: CAW
	g_iShellHeight = 40;
	if (1)
	{
		/* Assume that no arguments have been passed in */

		memset(g_apcArgs, 0, sizeof(g_apcArgs));

		/* Iterate through all of the arguments passed in and find any options that we recognise. */
		/* Anything not recognised will be treated as a path to be used, with the last one found */
		/* being the one that is actually used */

		for (Index = 1; Index < a_iArgC; ++Index)
		{
			if (!(stricmp(a_ppcArgV[Index], "-p")))
			{
				g_apcArgs[ARGS_PAUSE] = a_ppcArgV[Index];
			}
			else
			{
				g_apcArgs[ARGS_WILDCARD] = a_ppcArgV[Index];
			}
		}

		/* Scan the requested directory for entries */

		if (Dir.Open((g_apcArgs[0]) ? g_apcArgs[0] : "") == KErrNone)
		{
			if (Dir.Read(DirEntries) == KErrNone)
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
						Char = getchar();

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
				//Utils::Error("Unable to read directory."); // TODO: CAW - Here and below
			}

			Dir.Close();
		}
		else
		{
			//Utils::Error("Unable to open directory for reading.");
		}
	}
	else
	{
		//Utils::Error("Unable to obtain window dimensions");
	}

	return(RetVal);
}
