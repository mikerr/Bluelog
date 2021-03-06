/*
 *  readconfig.c - Read configuration file and
 *  load settings into struct.
 *
 */
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

// Longest line in config file
#define MAX_LINE_LEN 256

// Struct to hold configuration variables
struct cfg
{
	// Strings
	char *outfilename;
	
	// Basic
	int verbose;	
	int quiet;	
	int daemon;	
	int bluelive;	
	
	// Logging 
	int showtime;
	int obfuscate;
	int encode;
	int showclass;
	int friendlyclass;
	int bluepropro;
	int getname;
	int amnesia;
	int syslogonly;
	int getmanufacturer;
	
	// Advanced
	int retry_count;
	int scan_window;
	
	// System
	int bt_socket;
};

// Define global struct, set default values
struct cfg config = 
{
	.verbose = 0,
	.quiet = 0,
	.daemon = 0,
	.bluelive = 0,
	.showtime = 0,
	.obfuscate = 0,
	.encode = 0,
	.showclass = 0,
	.friendlyclass = 0,
	.bluepropro = 0,
	.getname = 0,
	.amnesia = -1,
	.syslogonly = 0,
	.getmanufacturer = 0,
	.retry_count = 3,
	.scan_window = 8,
};

// Determine if config file is present
int cfg_exists (void)
{
  struct stat buffer;   
  return (stat(CFG_FILE, &buffer) == 0);
}

// Make sure everybody plays nice
static void cfg_check (void)
{
	// Check for out of range values
	if ((config.retry_count < 0) || ((config.amnesia < 0) && (config.amnesia != -1)))
	{	
		printf("Error, arguments must be positive numbers!\n");
		exit(1);
	}
	
	// Make sure window is reasonable
	if (config.scan_window > MAX_SCAN || config.scan_window < MIN_SCAN)
	{
		printf("Scan window is out of range. See README.\n");
		exit(1);
	}	
	
	// Override some options that don't play nice with others
	// If retry is set, assume names are on. Default retry value
	if (config.retry_count > 0)
		config.getname = 1;
	else
		config.retry_count = 3;
	
	// No verbose for daemon
	if (config.daemon)
		config.verbose = 0;
		
	// No Bluelog Live when running BPP, names on, syslog off
	if (config.bluepropro)
	{
		config.bluelive = 0;
		config.getname = 1;
		config.syslogonly = 0;
	}

	// Showing raw class ID turns off friendly names
	if (config.showclass)
		config.friendlyclass = 0;
			
	// No timestamps for Bluelog Live, names on, syslog off
	if (config.bluelive)
	{
		config.showtime = 0;
		config.getname = 1;
		config.syslogonly = 0;
	}
	
	// No timestamps in syslog mode, disable other modes
	if (config.syslogonly)
	{
		config.showtime = 0;
		config.bluelive = 0;
		config.bluepropro = 0;
	}
	
	// Encode trumps obfuscate
	if (config.encode)
		config.obfuscate = 0;
}

static void cfg_read (void)
{
	FILE* cfgfile;
	char line[MAX_LINE_LEN + 1];
	char* token;
	char* value;
	int linenum = 1;
        
    // Open file, show error if we can't
	if (!config.quiet)		
		printf("Opening config file: %s...", CFG_FILE); 
    if ((cfgfile = fopen(CFG_FILE, "r")) == NULL)
	{
		printf("Error!\n");
		exit(1);
	}
    
	// Continue until file is done
	while(fgets(line, MAX_LINE_LEN, cfgfile) != NULL)
	{
		token = strtok(line, "\t =\n\r");
		if(token != NULL && token[0] != '#')
		{			
			// Get token's associated value
			value = strtok(NULL,"\t =\n\r");
  
			if (value != NULL)
			{
				// See if token matches something
				if (strcmp(token, "VERBOSE") == 0)
					config.verbose = (atoi(value));
				else if (strcmp(token, "QUIET") == 0)
					config.quiet = atoi(value);
				else if (strcmp(token, "DAEMON") == 0)
					config.daemon = (atoi(value));
				else if (strcmp(token, "LIVEMODE") == 0)
					config.bluelive = (atoi(value));
				else if (strcmp(token, "SHOWTIME") == 0)
					config.showtime = (atoi(value));		
				else if (strcmp(token, "OBFUSCATE") == 0)
					config.obfuscate = (atoi(value));
				else if (strcmp(token, "ENCODE") == 0)
					config.encode = (atoi(value));				
				else if (strcmp(token, "SHOWCLASS") == 0)
					config.showclass = (atoi(value));
				else if (strcmp(token, "FRIENDLYCLASS") == 0)
					config.friendlyclass = (atoi(value));
				else if (strcmp(token, "BLUEPROPRO") == 0)
					config.bluepropro = (atoi(value));
				else if (strcmp(token, "GETNAME") == 0)
					config.getname = atoi(value);
				else if (strcmp(token, "AMNESIA") == 0)
					config.amnesia = (atoi(value));
				else if (strcmp(token, "SYSLOGONLY") == 0)
					config.syslogonly = (atoi(value));
				else if (strcmp(token, "GETMANUFACTURER") == 0)
					config.getmanufacturer = (atoi(value));			
				else if (strcmp(token, "SCANWINDOW") == 0)
					config.scan_window = (atoi(value));
				else if (strcmp(token, "RETRYCOUNT") == 0)
					config.retry_count = (atoi(value));
				else
				{
					printf("FAILED\n");
					printf("Syntax error or unknown option in configuration file on line %i!\n", linenum);
					exit(1);
				}
			}
			else
			{
				printf("FAILED\n");
				printf("Value missing in configuration file on line %i!\n", linenum);
				exit(1);
			}
		}
	// Increment line number
	linenum++;
	}
	if (!config.quiet)
		printf("OK\n");
}
