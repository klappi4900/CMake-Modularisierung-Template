//
// Created by Dennis on 20.08.2026.
//
#pragma once
#include "I_Runtime/IRuntime.h"
namespace runtime {
    class Runtime{
        public:
        Runtime();
        ~Runtime();
        private:
        int cycletime = 100; //mu also mikrosekunden
        void run();

    };
}