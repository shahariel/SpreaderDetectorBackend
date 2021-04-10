/**
 * @file SpreaderDetectorBackend.c
 * @author  Shahar Ariel <shahar.ariel1@mail.huji.ac.il>
 * @version 1.0
 * @date 03 August 2020
 *
 * @brief A part of a program that analyses meetings between people and detects the probability for
 * infection
 *
 * @section LICENSE
 * This program is a free software.
 *
 * @section DESCRIPTION
 * The program gets a file with a list of people from videos, and a file with information about the
 * meetings seen in those videos, starting by the id of the sick person. Then it's calculates each
 * person's probabilty to get infected, and outputs a list with the results and the medical
 * instructions.
 */

//-----------------------------------------  includes  ---------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "SpreaderDetectorParams.h"

//-------------------------------------  const definitions  ----------------------------------------
/**
 * @def SUCCESS- a declaration for success in a function.
 */
#define SUCCESS 1

/**
 * @def FAILURE- a declaration for failure in a function.
 */
#define FAILURE 0

/**
 * @def ARGS_AMOUNT- the number of arguments in argv[]
 */
#define ARGS_AMOUNT 3

/**
 * @def PEOPLE_FILE_INDEX- the people's file index in argv[]
 */
#define PEOPLE_FILE_INDEX 1

/**
 * @def MEETINGS_FILE_INDEX- the meetings' file index in argv[]
 */
#define MEETINGS_FILE_INDEX 2

/**
 * @def SEPARATOR- the character that separates the fields in each line in the files
 */
#define SEPARATOR " "

/**
 * @def ALLOC_SIZE- the default size of  memory allocation
 */
#define ALLOC_SIZE 2

/**
 * @def DECIMAL_BASE- a numeric base for the int convertion
 */
#define DECIMAL_BASE 10

/**
 * @def EPSILON- the accuracy for float comparision
 */
#define EPSILON 0.000000001

/**
 * @def EQUAL- the objects are equal
 */
#define EQUAL 0

/**
 * @def GREATER- the first object greater than the second one
 */
#define GREATER 1

/**
 * @def SMALLER- the first object smaller than the second one
 */
#define SMALLER -1

/**
 * @def USAGE_ERROR- the massage to print when there is a usage error
 */
#define USAGE_ERROR "USAGE: ./SpreaderDetectorBackend <Path to People.in> <Path to Meetings.in>\n"

/**
 * @def IN_FILE_ERROR- the massage to print when there is an error in the input files
 */
#define IN_FILE_ERROR "Error in input files.\n"

/**
 * @def OUT_FILE_ERROR- the massage to print when there is an error in opening the output file
 */
#define OUT_FILE_ERROR "Error in output file.\n"

/**
 * @def MAX_LINE_LENGTH- the maximum length for each line in the file
 */
#define MAX_LINE_LENGTH 1024

/**
 * @def Person- a struct that contains information about a person: name (dinamically allocates array
 * of chars), id, age and the probability to get infected
 */
typedef struct Person
{
	char *name;
	unsigned long int id;
	float age;
	float probability;
} Person;

/**
 * @def compFunc- a typdef to a function that compares two persons
 */
typedef int (*compFunc)(const Person, const Person);

//-----------------------------------------  functions  --------------------------------------------
/**
 * This function is responsible to free the array of Person's structs, including the name field in
 * each struct
 * @param peopleList - the array of structs
 * @param length - the length of the array
 */
void freePeople(Person *peopleList, int length);

/**
 * This function is doing the required actions before exiting the program with EXIT_FAILURE code.
 * @param fileToClose - a pointer to the file needed to be closed. NULL if there isn't any open file
 * @param errorToPrint - the error needed to be print to the stderr
 * @param peopleList - the array of Persons needed to be freed. NULL if the array doesn't exist
 * @param peopleListSize - the size of the array
 */
void beforeExitFailure(FILE *fileToClose, const char *errorToPrint, Person *peopleList,
					   int peopleListSize);

/**
 * This function checks if the amount of arguments is ok
 * @param argc - the amount of arguments in the program
 * @return - 1 if succeeded, 0 if failed
 */
int argcCheck(int argc);

/**
 * This function calculates the probability for infection between two people
 * @param dist - the distance between them in their meeting (float)
 * @param time - the duration of their meeting (float)
 * @return a float with the probability
 */
float crna(float dist, float time);

/**
 * This function compares between two persons' id
 * @param a - the first Person
 * @param b - the second Person
 * @return - 1 if a is greater, -1 if a is smaller, 0 if a and b are equal
 */
int idCompare(Person a, Person b);

/**
 * This function compares between two persons' probability of infection
 * @param a - the first Person
 * @param b - the second Person
 * @return - 1 if a is greater, -1 if a is smaller, 0 if a and b are equal
 */
int probCompare(Person a, Person b);

/**
 * Merges two sorted sub-arrays into one sorted array
 * @param peopleList - the main array to sort
 * @param a - a pointer to the first sub-array
 * @param aLen - the length of the first sub-array
 * @param b - a pointer to the second sub-array
 * @param bLen - the length of the second sub-array
 * @param start - the index (in the original list) to start with
 * @param comp - a compare function
 */
void merge(Person *peopleList, Person *a, int aLen, Person *b, int bLen, int start, compFunc comp);

/**
 * Sorts the people's list, using the Merge-Sort algorithm
 * @param peopleList - the main array to sort
 * @param draftList - an array of structs (type Person)
 * @param length - an int with the length of the array
 * @param start - an int with the index to start with (in the original list)
 * @param comp - a compare function
 */
void mergeSort(Person *peopleList, Person *draftList, int length, int start, compFunc comp);

/**
 * This function sorts an array of struct Person by the id attribute
 * @param peopleList - the array to sort
 * @param peopleListSize - the length of the array
 * @return nothing, if fails- frees all memory and exits the program
 */
void sortById(Person **peopleList, int peopleListSize);

/**
 * This function sorts an array of struct Person by the probability attribute
 * @param peopleList - the array to sort
 * @param peopleListSize - the length of the array
 * @return nothing, if fails- frees all memory and exits the program
 */
void sortByProbability(Person **peopleList, int peopleListSize);

/**
 * This function gets an array of struct Person sorted by id and an id to search in it, and finds
 * the index of the id in the array
 * @param peopleList - the array to search in
 * @param idToFind - the id to search
 * @param start - the start index
 * @param end - the end index
 * @return - the index of the desired id in the array
 */
int binarySearchById(Person *peopleList, unsigned long int idToFind, int start, int end);

/**
 * This function gets an empty struct of type Person and a line with information, and fills the
 * struct's fields
 * @param line - a string with the information from the file
 * @param person - the struct to fill
 * @return 1 if succeeded, 0 if failed
 */
int fillPerson(char *line, Person *person);

/**
 * This function reads the people's file and put the data in an array of struct Person
 * @param peopleFile - pointer to the people's file
 * @param peopleList - type Person**, a pointer to the array to fill
 * @return an int with the number of people in the file (and in the array). if fails- frees all
 * memory and exits the program
 */
int readPeopleFile(FILE *peopleFile, Person **peopleList);

/**
 * This function allocates more memory to the array of struct Person, using realloc
 * @param peopleList - the array of struct Person
 * @param capacity - the new size for the array
 * @param counter - the current size of the array
 * @param peopleFile - a pointer to the people's file
 */
void allocateMore(Person **peopleList, int capacity, int counter, FILE *peopleFile);

/**
 * This function reads the data from the meetings' file and accordingly updates the array of Persons
 * @param meetingsFile - the meetings' file
 * @param peopleList - the array to update
 * @param peopleListSize - the size of the array
 */
void readMeetingsFile(FILE *meetingsFile, Person *peopleList, int peopleListSize);

/**
 * This function calculates the probability for a certain Person (taken from a line) and updates it
 * in its struct
 * @param peopleList - the array of Persons
 * @param peopleListSize - the size of the array
 * @param line - a string with the line from the meetings' file
 */
void probUpdater(Person *peopleList, int peopleListSize, char *line);

/**
 * This function is responsible to write to the output file the medical conclusions for the people
 * in the program by their probability of infection
 * @param outputFile - the file to write to
 * @param peopleList - the array of struct Person
 * @param peopleListSize - the size of the array
 */
void writeOutput(FILE *outputFile, Person *peopleList, int peopleListSize);

//-------------------------------------------- code  -----------------------------------------------

void freePeople(Person *peopleList, int length)
{
	for (int i = 0; i < length; ++i)
	{
		free(peopleList[i].name);
		peopleList[i].name = NULL;
	}
	free(peopleList);
	peopleList = NULL;
}

void beforeExitFailure(FILE *fileToClose, const char *errorToPrint, Person *peopleList,
					   int peopleListSize)
{
	fprintf(stderr, "%s", errorToPrint);
	if (peopleList != NULL)
	{
		freePeople(peopleList, peopleListSize);
	}
	if (fileToClose != NULL && fclose(fileToClose) == EOF)
	{
		fprintf(stderr, STANDARD_LIB_ERR_MSG);
	}
}

float crna(float dist, float time)
{
	float numerator = time * MIN_DISTANCE;
	float denominator = dist * MAX_TIME;
	return numerator / denominator;
}

int idCompare(const Person a, const Person b)
{
	const unsigned long int id1 = a.id;
	const unsigned long int id2 = b.id;
	return id1 - id2;
}

int probCompare(const Person a, const Person b)
{
	const float id1 = a.probability;
	const float id2 = b.probability;
	if (fabsf(id1 - id2) < EPSILON)
	{
		return EQUAL;
	}
	else if (id1 > id2)
	{
		return GREATER;
	}
	else
	{
		return SMALLER;
	}
}

void merge(Person *peopleList, Person *a, int aLen, Person *b, int bLen, int start, compFunc comp)
{
	int aI = 0;
	int bI = 0;
	while (aI < aLen && bI < bLen)
	{
		if (comp(a[aI], b[bI]) < 0)
		{
			peopleList[start + aI + bI] = a[aI];
			aI++;
		}
		else
		{
			peopleList[start + aI + bI] = b[bI];
			bI++;
		}
	}
	if (aI < aLen)
	{
		for (int i = aI; i < aLen; i++)
		{
			peopleList[start + i + bI] = a[i];
		}
	}
	else
	{
		for (int j = bI; j < bLen; j++)
		{
			peopleList[start + aI + j] = b[j];
		}
	}
}

void mergeSort(Person *peopleList, Person *draftList, int length, int start, compFunc comp)
{
	if (length < 1)
	{
		return;
	}
	int aLen = length / 2;
	int bLen = length - aLen;
	if (aLen == 0)
	{
		bLen = 0;
	}
	mergeSort(peopleList, draftList, aLen, start, comp);
	mergeSort(peopleList, &draftList[aLen], bLen, aLen + start, comp);
	for (int i = 0; i < (length); i++)
	{
		draftList[i] = peopleList[i + start];
	}
	merge(peopleList, draftList, aLen, &draftList[aLen], bLen, start, comp);
}

int fillPerson(char *line, Person *const person)
{
	char *temp = strtok(line, SEPARATOR);
	person->name = (char *) calloc(sizeof(char), strlen(temp) + 1);
	if (person->name == NULL)
	{
		return FAILURE;
	}
	strcpy(person->name, temp); // copy name into person
	person->id = strtol(strtok(NULL, SEPARATOR), NULL, DECIMAL_BASE);
	person->age = strtof(strtok(NULL, SEPARATOR), NULL);
	person->probability = 0;
	return SUCCESS;
}

void allocateMore(Person **peopleList, int capacity, int counter, FILE *peopleFile)
{
	Person *tempList = (Person *) realloc(*peopleList, sizeof(Person) * capacity);
	if (tempList == NULL)
	{
		beforeExitFailure(peopleFile, STANDARD_LIB_ERR_MSG, *peopleList, counter);
		exit(EXIT_FAILURE);
	}
	*peopleList = tempList;
}

int readPeopleFile(FILE *peopleFile, Person **peopleList)
{
	int capacity = ALLOC_SIZE;
	*peopleList = (Person *) calloc(sizeof(Person), capacity);
	if (*peopleList == NULL)
	{
		beforeExitFailure(peopleFile, STANDARD_LIB_ERR_MSG, NULL, 0);
		exit(EXIT_FAILURE);
	}
	char line[MAX_LINE_LENGTH];
	int personsCounter = 0;
	while (fgets(line, MAX_LINE_LENGTH, peopleFile))
	{
		Person person;
		int fillResult = fillPerson(line, &person);
		if (fillResult == FAILURE)
		{
			beforeExitFailure(peopleFile, STANDARD_LIB_ERR_MSG, *peopleList, personsCounter);
			exit(EXIT_FAILURE);
		}
		(*peopleList)[personsCounter] = person;
		++personsCounter;
		if (personsCounter == capacity)
		{
			capacity *= ALLOC_SIZE;
			allocateMore(peopleList, capacity, personsCounter, peopleFile);
		}
	}
	if (fclose(peopleFile) == EOF)
	{
		beforeExitFailure(NULL, STANDARD_LIB_ERR_MSG, *peopleList, personsCounter);
		exit(EXIT_FAILURE);
	}
	return personsCounter;
}

void sortById(Person **peopleList, int peopleListSize)
{
	Person *draft = (Person *) calloc(sizeof(Person), peopleListSize);
	if (draft == NULL)
	{
		beforeExitFailure(NULL, STANDARD_LIB_ERR_MSG, *peopleList, peopleListSize);
		exit(EXIT_FAILURE);
	}
	mergeSort(*peopleList, draft, peopleListSize, 0, idCompare);
	free(draft);
	draft = NULL;
}

void sortByProbability(Person **peopleList, int peopleListSize)
{
	Person *draft = (Person *) calloc(sizeof(Person), peopleListSize);
	if (draft == NULL)
	{
		beforeExitFailure(NULL, STANDARD_LIB_ERR_MSG, *peopleList, peopleListSize);
		exit(EXIT_FAILURE);
	}
	mergeSort(*peopleList, draft, peopleListSize, 0, probCompare);
	free(draft);
	draft = NULL;
}

int binarySearchById(Person *const peopleList, unsigned long int idToFind, int start, int end)
{
	int mid = start + ((end - start) / 2);
	if (peopleList[mid].id == idToFind)
	{
		return mid;
	}
	if (peopleList[mid].id > idToFind) // id is smaller than the middle id
	{
		return binarySearchById(peopleList, idToFind, start, mid - 1);
	}
	else // id is greater than the middle id
	{
		return binarySearchById(peopleList, idToFind, mid + 1, end);
	}
}

void probUpdater(Person *const peopleList, int peopleListSize, char *line)
{
	unsigned long int infectorId = strtol(strtok(line, SEPARATOR), NULL, DECIMAL_BASE);
	unsigned long int infectedId = strtol(strtok(NULL, SEPARATOR), NULL, DECIMAL_BASE);
	int infectorIdx = binarySearchById(peopleList, infectorId, 0, peopleListSize - 1);
	int infectedIdx = binarySearchById(peopleList, infectedId, 0, peopleListSize - 1);
	float distance = strtof(strtok(NULL, SEPARATOR), NULL);
	float time = strtof(strtok(NULL, SEPARATOR), NULL);
	float prob = crna(distance, time);
	peopleList[infectedIdx].probability = peopleList[infectorIdx].probability * prob;
}

void readMeetingsFile(FILE *meetingsFile, Person *const peopleList, int peopleListSize)
{
	char line[MAX_LINE_LENGTH];
	if (fgets(line, MAX_LINE_LENGTH, meetingsFile) == NULL) //if file is empty, close it and return
	{
		if (fclose(meetingsFile) == EOF)
		{
			fprintf(stderr, STANDARD_LIB_ERR_MSG);
		}
		return;
	}
	unsigned long int sickId = strtol(line, NULL, DECIMAL_BASE);
	int sickPersonIndex = binarySearchById(peopleList, sickId, 0, peopleListSize - 1);
	peopleList[sickPersonIndex].probability = 1;
	while (fgets(line, MAX_LINE_LENGTH, meetingsFile))
	{
		probUpdater(peopleList, peopleListSize, line);
	}
	if (fclose(meetingsFile) == EOF)
	{
		beforeExitFailure(NULL, STANDARD_LIB_ERR_MSG, peopleList, peopleListSize);
		exit(EXIT_FAILURE);
	}
}

void writeOutput(FILE *outputFile, Person *const peopleList, int peopleListSize)
{
	for (int i = peopleListSize - 1; i >= 0; --i)
	{
		if (peopleList[i].probability >= MEDICAL_SUPERVISION_THRESHOLD ||
			fabsf(peopleList[i].probability - MEDICAL_SUPERVISION_THRESHOLD) < EPSILON)
		{
			fprintf(outputFile, MEDICAL_SUPERVISION_THRESHOLD_MSG, peopleList[i].name,
					peopleList[i].id);
		}
		else if (peopleList[i].probability >= REGULAR_QUARANTINE_THRESHOLD ||
				 fabsf(peopleList[i].probability - REGULAR_QUARANTINE_THRESHOLD) < EPSILON)
		{
			fprintf(outputFile, REGULAR_QUARANTINE_MSG, peopleList[i].name, peopleList[i].id);
		}
		else
		{
			fprintf(outputFile, CLEAN_MSG, peopleList[i].name, peopleList[i].id);
		}
	}
	if (fclose(outputFile) == EOF)
	{
		beforeExitFailure(NULL, STANDARD_LIB_ERR_MSG, peopleList, peopleListSize);
		exit(EXIT_FAILURE);
	}
}

int argcCheck(int argc)
{
	if (argc != ARGS_AMOUNT)  // ERROR- wrong number of arguments
	{
		fprintf(stderr, USAGE_ERROR);
		return FAILURE;
	}
	return SUCCESS;
}

int main(int argc, char *argv[])
{
	if (argcCheck(argc) == FAILURE)
	{
		return EXIT_FAILURE;
	}
	FILE *peopleFile = fopen(argv[PEOPLE_FILE_INDEX], "r");
	if (peopleFile == NULL)
	{
		fprintf(stderr, IN_FILE_ERROR);
		return EXIT_FAILURE;
	}
	Person *peopleList = NULL;
	int peopleListSize = readPeopleFile(peopleFile, &peopleList); //after this, peopleFile is closed
	sortById(&peopleList, peopleListSize); // so it would be quicker to update the probabilities
	FILE *meetingsFile = fopen(argv[MEETINGS_FILE_INDEX], "r");
	if (meetingsFile == NULL)
	{
		beforeExitFailure(NULL, IN_FILE_ERROR, peopleList, peopleListSize);
		return EXIT_FAILURE;
	}
	readMeetingsFile(meetingsFile, peopleList, peopleListSize); //after this, meetingsFile's closed
	sortByProbability(&peopleList, peopleListSize); // so we know what order to print in
	FILE *outputFile = fopen(OUTPUT_FILE, "w");
	if (outputFile == NULL)
	{
		beforeExitFailure(NULL, OUT_FILE_ERROR, peopleList, peopleListSize);
		return EXIT_FAILURE;
	}
	writeOutput(outputFile, peopleList, peopleListSize); //after this, outputFile is closed
	freePeople(peopleList, peopleListSize);
	return EXIT_SUCCESS;
}
