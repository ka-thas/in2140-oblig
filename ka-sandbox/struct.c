#include <stdio.h>
#include <string.h>

// Define a struct
struct Person
{
    char name[50];
    int age;
};

int main()
{
    // Create an instance of the struct
    struct Person *person1;

    // Assign values to the struct members
    strcpy(person1->name, "John Doe");
    person1->age = 25;

    // Print the values of the struct members
    printf("Name: %s\n", person1->name);
    printf("Age: %d\n", person1->age);

    return 0;
}
