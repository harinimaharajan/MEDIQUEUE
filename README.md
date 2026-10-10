# MEDIQUEUE
### Intelligent Hospital Patient Queue & Resource Management System

MEDIQUEUE is a Data Structures based hospital management system developed to organize patient queues, prioritize emergency cases, allocate beds, and manage patient records efficiently.

The project demonstrates the practical use of core Data Structures concepts through a C-based backend with a web-based frontend.

## Key Features

- Patient record management
- Patient ID based search
- Emergency patient prioritization
- Priority Queue based emergency handling
- Smart hospital bed allocation
- Doctor management
- Queue and patient analytics
- Activity history tracking
- C backend with HTTP server
- Web-based dashboard interface
- CSV-based patient data processing

## Data Structures Used

| Data Structure | Application |
|---|---|
| Queue | Managing normal patient flow |
| Priority Queue | Handling emergency and critical patients |
| Linked List | Storing patient records |
| Stack | Maintaining activity history |
| Searching | Finding patient records |
| Sorting | Organizing priority and queue information |

## System Architecture

```text
                    MEDIQUEUE
                        |
        +---------------+---------------+
        |                               |
    Frontend                        C Backend
 HTML / CSS / JavaScript          mediqueue.c
        |                          server.c
        |                               |
        +---------------+---------------+
                        |
                   patients.csv
