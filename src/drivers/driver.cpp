/*
 * My Experience Operating System
 * Copyright (C) 2019, Andreas Lindström
 */
#include <drivers/driver.h>
#include <console.h>

using namespace myxp;

/*
 *  Driver
 */
myxp::drivers::Driver::Driver()
{
    driverName = "(No Driver)";
}

myxp::drivers::Driver::~Driver()
{

}

void myxp::drivers::Driver::Activate()
{

}

void myxp::drivers::Driver::Deactivate()
{

}

int myxp::drivers::Driver::Reset()
{
    return 0;
}


/*
 *  Driver Manager
 */
myxp::drivers::DriverManager::DriverManager()
{
    numOfDrivers = 0;
}

void myxp::drivers::DriverManager::ActivateAll()
{
    for(int i = 0; i < numOfDrivers; i++)
    {
        drivers[i]->Activate();
        printf("Driver ");
        printf(drivers[i]->GetDriverName());
        printf(" loaded...\n");
    }
}

char* myxp::drivers::Driver::GetDriverName()
{
    return driverName;
}

void myxp::drivers::DriverManager::AddDriver(Driver* driver)
{
    drivers[numOfDrivers] = driver;
    numOfDrivers++;
}