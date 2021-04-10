# SpreaderDetectorBackend

This is a program I wrote for my C exam, and was graded 100.

The purpose is to analyze data from videos and use it to stop Covid-19 infections.

It gets 2 files, produced from videos:
1. A list of people, where each line contains a name, ID and an age.
2. A file of meetings: The first line is an ID of a Covid-infected person, and the next lines
 describe meetings: 2 ID's (belongs to the persons in the meeting), the distance between them and
 the duration of the meeting.
 
The output is a file with a diagnose for each one of the persons from the first file:
Hospitalization Required/ 14-days-Quarantine Required/ No serious chance for infection

