/*
 * My Experience Operating System
 * Copyright (C) 2019, Andreas Lindström
 */
#ifndef __MYXP__DRIVERS__DRIVER_H
#define __MYXP__DRIVERS__DRIVER_H

namespace myxp
{
    namespace drivers
    {
        class Driver
        {
        protected:
            char* driverName;

        public:
            Driver();
            ~Driver();

            virtual void Activate();
            virtual void Deactivate();
            virtual int Reset();
            virtual char* GetDriverName();
        };

        class DriverManager
        {
        private:
            Driver* drivers[512];
            int numOfDrivers;

        public:
            DriverManager();
            void AddDriver(Driver*);
            void ActivateAll();
        };
    }
}

#endif