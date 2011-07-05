/* config.c a Config library*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Constants */
#define MAXLEN 127

/* Typedefs */
typedef char * pchar;
typedef struct nv * pnv;
typedef struct section * psection;

/*types */
struct nv {
	pchar name;
	pchar value;
	pnv   next;
};

struct section {
	pchar   name;    /* section name eg [fred] - stores fred */
	pnv	nvlist;  /* list of name value (nv) pairs */
	psection next;
};


/* Global variables */
psection head; /* Points to first section in list */
psection current;

/*Functions */
void trim(pchar s) /* Remove tabs/spaces/lf/cr  both ends */
{
	/* Trim from start */
	size_t i=0,j;
	while((s[i] == ' ' || s[i] == '\t' || s[i] == '\n' || s[i] == '\r')) {
		i++;
	}
	if (i > 0) {
		for( j = 0; j < strlen(s); j++) {
			s[j] = s[j + i];
		}
	s[j] = '\0';
	}

	/* Trim from end */
	i = strlen(s) - 1;
	while((s[i] == ' ' || s[i] == '\t'|| s[i] == '\n' || s[i] == '\r')) {
		i--;
	}
	if ( i < (strlen(s) - 1)) {
		s[i + 1] = '\0';
	}
}

/* Found [section] */
void newsection(pchar config) {
	psection newsect = malloc(sizeof(struct section));

	if (head == NULL) /* start the chain off */
		head = newsect;
	else
		current->next = newsect; /* add on the end of the last section */
	current = newsect;
	newsect->name = malloc(strlen(config));
	strncpy(newsect->name, config + 1,strlen(config) - 1);
	newsect->name[strlen(config) - 2] = '\0';
	newsect->nvlist = NULL; 
	newsect->next   = NULL;
}

/* this is added in the section pointed to by current */
void newnvpair(pchar config) {
	pchar name = NULL;
	pchar value = NULL;
	pnv   newnv = NULL;
	pnv   lastnv;
	size_t valuelen;
	size_t p = 0;
	int err = 2;

	if (current == NULL)
	  exit(1); /* Error 1 - Found Name=value before a section was defined */

	for (p = 0; (p < strlen(config)); p++) {
		if (config[p] == '=')
		{
			err = 0;
			break;
		}
	}

    if (err == 2)
		exit(2); /* Error 2 No = in name = value line */

	newnv = malloc(sizeof(struct nv));
	name = malloc(p + 1);
	strncpy(name, config, p);
	name[p] = '\0';

	valuelen = strlen(config) - p - 1; /* length of string eg 'alpha' len = 5 */
	value = malloc(valuelen + 1); /* add 1 for \0 */
	strncpy(value, config + p + 1, valuelen);
	value[valuelen] = '\0';

	newnv->name = name;
	newnv->value = value;
	newnv->next = NULL;

	if (current->nvlist == NULL) /* no name/values in this section yet */
		current->nvlist = newnv;
	else {
		lastnv = current->nvlist; /* follow list of nv pairs until end */
		while ((lastnv->next ) != NULL)
			lastnv = lastnv->next;
		lastnv->next = newnv; /* add on the end of the list */
	}
}

/* Searches through all sections for matching sectionname. */
psection findsection(pchar sectionname) {
	psection result = NULL;
	current = head;
	while (current) {
		if (strcmp(current->name, sectionname) == 0) { /* 0 = march */
			result = current;
			break;
		}
		current = current->next;
	}
	return result;
}

/* This assumes current points to the section, call findsection first */
extern pchar getvalue (pchar name) {
	pchar result = NULL;
	pnv   currnv = current->nvlist;
	while (currnv) {
		if ((strcmp(name, currnv->name) == 0)) { /* 0 = match */
			result = currnv->value;
			break;
		}
		currnv = currnv->next;
	}
	return result;
}

/* Process the string configline */
void process(pchar  configline) {
	if (strlen(configline) == 0)  /* Ignore empty lines */
		return;
    if (configline[0] == ';')  /* ignore lines starting with a ; - they are comments */
		return; 
	if (configline[0] == '[')
		newsection(configline);
	else 
		newnvpair(configline);
}

int isname(pchar section, pchar name) {
	int result = 0;
	trim(section);
	current = findsection(section);
	if (current) {
		if (getvalue(name))
			result = 1;
	}
	return result;
}


/* return pointer to first section ot 0 if error) */
extern int initlib(pchar filename) {
	FILE * f;
	char buff[MAXLEN];
	f = fopen(filename, "rt");
	if (f == NULL)
		return -1; /* error */
	head = NULL;
	while (!feof(f)) {
      if (fgets(buff, MAXLEN - 2, f) == NULL)
		  break;
	  trim(buff);
	  process(buff);
	}
	fclose(f);

	return 0;
}

/* iterate through sections and free each up, and nvs in it */
extern void tidyup()
{	
	pnv   currentnv;
	pnv	  nextnv;
	psection nextsection;
	current = head;
	do  {
		currentnv = current->nvlist;
		do { /* Free up the chain of nv pairs for current section */
			if (currentnv) {
				nextnv = currentnv->next;				
				free(currentnv->value);
				free(currentnv->name);
				free(currentnv);
				currentnv = nextnv;
			} 
		}
		while (currentnv);

		if (current) { /* Now Free this section */
			nextsection = current->next;
			free(current->name);
			free(current);
			current = nextsection;
		}
	}
	while (current);
	head=NULL;
}
