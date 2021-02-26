#pragma once
#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>


//constant data

const int Ncook = 2;
const int Noven = 5;
const int Ndeliverer = 10;

const unsigned int Torderlow = 1;
const unsigned int Torderhigh = 5;

const int Norderlow = 1;
const int Norderhigh = 5;

const unsigned int Tprep = 1;
const unsigned int Tbake = 10;

const unsigned int Tlow = 5;
const unsigned int Thigh = 15;


//Functions Decleration

void assert_successful_mutex_action(int response_code);
unsigned int sleep(unsigned int seconds);









