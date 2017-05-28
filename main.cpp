//Handle files > 2 Go
#define _FILE_OFFSET_BITS 64
#define _LARGEFILE64_SOURCE
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdint.h>

#define VERSION "1.3"

#ifdef WIN32
#define ftell _ftelli64
#define fseek _fseeki64
#define int64_t __int64
#endif

enum MODE
{
    MODE_ANALYSE=   0x1,
    MODE_REPAIR=    0x2,
    MODE_VERBOSE=   0x4,
    MODE_DEBUG=     0x8,
    MODE_COMPAT=    0x10
};
FILE *mbox_in;
char line_in[4096];
int64_t data = 0;
unsigned int lines=0;
int64_t fileSize = 0;

int get_line_in(void)
{
	unsigned int  i = 0;
	char  c;
	memset(line_in,0,sizeof(line_in));

	while(i < sizeof(line_in))
	{
        c = fgetc(mbox_in);
        data++;
		if (c == '\0')
		{
		    //Could occur, not a reason to block
			fprintf(stderr,"Warning: null character at line: %i col: %i\n", lines + 1, i + 1);
        }
		if (c == EOF)
		{
			//Check if the file is bigger than we think
			if (fileSize > data)
			{
				//ASCII char 0xFF translates to -1 integer, so EOF
				fprintf(stderr, "Warning: -1 character at line: %i col: %i\n", lines + 1, i + 1);
				return 0;
			}
			//The 'real' eof has been reached
			return 1;
		}
		line_in[i] = c;
        i++;
		if (c == '\n')
		{
            line_in[i]='\0';//null terminated
			return 0;
		}
	}
	fprintf(stderr,"Error: line buffer overflow (max 4096 bytes)\n");
    return 1;
}
void printUsage()
{
    printf("Necro Mail v. %s\nUsage:\nnecromail OPTIONS MBOX_FILE\nOptions:\n-v\tVerbose mode\n-r\tRepair mode (read-only by default)\n-c\tCompatibily mode (\"From \" mail separator instead of \"From - \")\n-d\tDebug mode\n-h\tHelp\n", VERSION);
    return;
}

int main(int argc, char *argv[])
{
	char file_name_in[255];
	char file_name_simple[255];
	char * pch;
	int path_offset;
	unsigned int mails=0;
	unsigned int deleted=0;
	int mode=MODE_ANALYSE;
	bool nameIsSet=false;
	unsigned short separatorLenght=7;

	if(argc < 2 || argc > 6 )
	{
        fprintf(stderr, "Error: bad syntax\n");
		printUsage();
		return 1;
	}
	
  	//Options
  	for(int i=1;i<argc;i++)
  	{
        if(strcmp(argv[i], "-r") == 0)
        {
            mode |= MODE_REPAIR;
            if(mode&MODE_DEBUG) puts("repair mode");
        }
        else if(strcmp(argv[i], "-v") == 0)
        {
            mode |= MODE_VERBOSE;
            if(mode&MODE_DEBUG) puts("verbose mode");
        }
        else if(strcmp(argv[i], "-c") == 0)
        {
            mode |= MODE_COMPAT;
            if(mode&MODE_DEBUG) puts("compat mode");
        }
        else if(strcmp(argv[i], "-d") == 0)
        {
            mode |= MODE_DEBUG;
            if(mode&MODE_DEBUG) puts("debug mode");
        }
        else if(strcmp(argv[i], "-h") == 0)
        {
            printUsage();
            return 0;
        }
        else if (strlen(argv[i]) > 254)
        {
            fprintf(stderr,"Error: filename too long (max 255 chars)\n");
            return 1;
        }
        else
        {
            if(!nameIsSet)
            {
                strcpy(file_name_in, argv[i]);
                nameIsSet=true;
            }
            else
            {
                printUsage();
                return 1;
            }
        }
    }
    //At least 1 option but no file name specified
    if(!nameIsSet)
    {
        printUsage();
        return 1;
    }
	//Last occurrence of character
	pch=strrchr(file_name_in,'/');
	if (pch != NULL) {
		path_offset = pch-file_name_in+1;
	}
	else {
		path_offset = 0;
	}
	sprintf (file_name_simple,"%s",&file_name_in[path_offset]);

    //Open file for update
	mbox_in = fopen(file_name_in,"rb+");
	if (!mbox_in )
	{
		printf("\nFile '%s' was not found or locked.\n", file_name_in);
		return 1;
	}

	//Get the file size
	fseek(mbox_in, 0, SEEK_END);
	fileSize = ftell(mbox_in);
	fseek(mbox_in, 0, SEEK_SET);

	//Compatibility mode for mbox files with the "From " mail separator instead of "From - "
	if(mode&MODE_COMPAT) separatorLenght=5;
	
    //Line by line read
	while (get_line_in() == 0)
	{
        lines++;
        //New mail separator
        if(memcmp(line_in,"From - ",separatorLenght) == 0)
        {
            mails++;
            if(mode&MODE_DEBUG)
            {
				unsigned long long pos = ftell(mbox_in);
                printf("%lld %s", pos-strlen(line_in), line_in);
            }
        }
        //status
        if(memcmp(line_in,"X-Mozilla-Status: ",18) == 0)
		{
            char x_mozilla_status[5];
            x_mozilla_status[4]='\0';
            memcpy(&x_mozilla_status, &line_in[18], 4);
            if(mode&MODE_VERBOSE) printf("%s",x_mozilla_status);
            //Deleted flag detected
            if(x_mozilla_status[3]>='8')
            {
                if(mode&MODE_VERBOSE) printf("D");
                deleted++;
                if(mode&MODE_REPAIR)
                {
                    //Modify the value
                    switch(x_mozilla_status[3])
                    {
                        //0-7 nothing
                        case 56://8
                        {
                            x_mozilla_status[3]='0';
                            break;
                        }
                        case 57://9
                        {
                            x_mozilla_status[3]='1';//char 49
                            break;
                        }
                        case 97://a
                        {
                            x_mozilla_status[3]='2';//char 50
                            break;
                        }
                        case 98://b
                        {
                            x_mozilla_status[3]='3';//char 51
                            break;
                        }
                        case 99://c
                        {
                            x_mozilla_status[3]='4';//char 52
                            break;
                        }
                        case 100://d
                        {
                            x_mozilla_status[3]='5';//char 53
                            break;
                        }
                        case 101://e
                        {
                            x_mozilla_status[3]='6';//char 54
                            break;
                        }
                        case 102://f
                        {
                            x_mozilla_status[3]='7';//char 55
                            break;
                        }
                        default:
                        {
                            fprintf(stderr,"Error: invalid X-Mozilla-Status range\n");
                            return 1;
                        }
                    }
                    //Remember the current position
                    fpos_t pos;
                    fgetpos(mbox_in, &pos);
                    //Seek the to the last byte position
                    /*  Special char counter
                    "X-Mozilla-Status: "    18
                    "xxxx"                  4
                    "\r? \n"                offset
                    Total 22 + offset    */
                    long offset = strlen(line_in) - 22;
                    //Back one char
                    fseek(mbox_in, -(offset+1), SEEK_CUR);
                    //Write the modified value
                    fputc(x_mozilla_status[3],mbox_in);
                    //Reset to the last position
                    fsetpos(mbox_in, &pos);
                }
            }
            else
            {
                if(mode&MODE_VERBOSE) printf("%s", " ");
            }
        }
        if(memcmp(line_in,"Subject:",8) == 0)
        {
            if(mode&MODE_VERBOSE) printf("%s", line_in);
        }
    }
    if(mode&MODE_VERBOSE) printf("\n");
    printf("%s (%i/%i) %i lines %lli bytes\n",file_name_in, deleted, mails, lines, data - 1);
    fclose(mbox_in);
	return 0;
};
