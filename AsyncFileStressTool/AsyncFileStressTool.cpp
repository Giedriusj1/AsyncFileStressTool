// Author: Giedrius Jonikas

#include "stdafx.h"
#include "ReadDirectoryChanges.h"
#include <random>


// Number of file operations that each thread should attempt:
int file_operation_num = 10;

// Sleep in ms
const int sleep_after_each_operation = 100;

const DWORD event_notify_filter =
	  FILE_NOTIFY_CHANGE_LAST_WRITE
 	| FILE_NOTIFY_CHANGE_ATTRIBUTES
	| FILE_NOTIFY_CHANGE_CREATION
	| FILE_NOTIFY_CHANGE_SIZE
	| FILE_NOTIFY_CHANGE_FILE_NAME
	| FILE_NOTIFY_CHANGE_SECURITY;

// Implemented in FileOperations.cpp:
void read_from_beginning(std::wstring* path, int iteration);
void read_from_end(std::wstring* path, int iteration);
void get_file_attributes(std::wstring* path, int iteration);
void lock_file_range(std::wstring* path, int iteration);
void get_file_type(std::wstring* path, int iteration);
void get_binary_type(std::wstring* path, int iteration);

// Array of functions that will be called on file:
void(*file_ops[])(std::wstring*, int) = { read_from_beginning,
										  read_from_end,
										  get_file_attributes,
										  lock_file_range,
										  get_file_type,
										  get_binary_type };

const int file_ops_count = sizeof(file_ops) / sizeof(file_ops[0]);

std::mt19937_64 rand_gen(0); //Use the 64-bit Mersenne Twister 19937 generator

// rand() is not thread safe, so we'll add thread id to make sure threads get different numbers
LONG gen_random(LONG max) {
	return (rand_gen() + GetCurrentThreadId()) % max;
}

// This thead will be spun for each notify event we get
DWORD WINAPI file_access_thread_func(LPVOID lpParam)
{
	std::wstring* path = (std::wstring*)lpParam;

	wprintf(L"tid %8d| %s\n", GetCurrentThreadId(), path->c_str());

	for (int i = 1; i <= file_operation_num; i++) {
		file_ops[gen_random(file_ops_count)](path, i);

		Sleep(sleep_after_each_operation);
	}

	return 0;
}

int wmain(int argc, wchar_t *argv[], wchar_t *envp[])
{
	if(argc <= 1) {
		wprintf(L"Monitoring path not provided, aborting");
		exit(1);
	}
	std::wstring monitor_path(argv[1]);
	wprintf(L"Monitoring path: %s", monitor_path.c_str());


	CReadDirectoryChanges changes;
	changes.AddDirectory(monitor_path.c_str(), true,
						 event_notify_filter);

	HANDLE hStdIn = ::GetStdHandle(STD_INPUT_HANDLE);
	const HANDLE handles[] = { changes.GetWaitHandle() };

	while (true)
	{
		DWORD rc = ::WaitForMultipleObjectsEx(_countof(handles), handles, false, INFINITE, true);
		switch (rc)
		{
		case WAIT_OBJECT_0:
		{
			DWORD dwAction;
			CStringW wstrFilename;
			if (changes.CheckOverflow())
				wprintf(L"Queue overflowed.\n");
			else
			{
				changes.Pop(dwAction, wstrFilename);
				std::wstring* my_data = new std::wstring(wstrFilename);

				HANDLE thread_handle = CreateThread(NULL,0,file_access_thread_func,my_data,0,NULL);

				CloseHandle(thread_handle);
			}
		}
		break;

		default:
			break;
		}
	}

	return EXIT_SUCCESS;
}
