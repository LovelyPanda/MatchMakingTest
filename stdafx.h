#pragma once

#ifndef _WIN32_WINNT		               
#define _WIN32_WINNT 0x0501	
#endif						

#define WIN32_LEAN_AND_MEAN	

#define _CRT_RAND_S
#define _CRT_SECURE_NO_WARNINGS

#define XML_STATIC 

#include <windows.h>
#include <WinSock2.h>
#include <MSWSock.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
