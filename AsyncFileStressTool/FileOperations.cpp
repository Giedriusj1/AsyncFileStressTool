// Author: Giedrius Jonikas

#include "stdafx.h"

extern int file_operation_num;
extern LONG gen_random(LONG);

// Simple CreateFileW() -> ReadFile() up to 1 page from beginning -> CloseHandle() case
void read_from_beginning(std::wstring* path, int iteration) {
	HANDLE file_handle = CreateFileW(path->c_str(),
		GENERIC_READ,
		FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (file_handle != INVALID_HANDLE_VALUE) {
		// Try to read something:
		char buff[4096];
		DWORD bytes_read = 0;

		BOOL read_successful = ReadFile(file_handle, &buff, gen_random(4096), &bytes_read, NULL);
		if (read_successful) {
			wprintf(L"tid %8d|%4i/%4i| ReadFile() from file beginning OK  \n", GetCurrentThreadId(), iteration, file_operation_num);
		}
		else {
			wprintf(L"tid %8d|%4i/%4i| ReadFile() from file beginning Failed  \n", GetCurrentThreadId(), iteration, file_operation_num);
		}
		CloseHandle(file_handle);
	}
	else {
		wprintf(L"tid %8d|%4i/%4i| read_from_beginning() failed to open file  \n", GetCurrentThreadId(), iteration, file_operation_num);
	}
}

// Reads up to 1 page from file end. Does nothing if file is less than 4kb in size.
void read_from_end(std::wstring* path, int iteration) {
	HANDLE file_handle = CreateFileW(path->c_str(),
		GENERIC_READ,
		FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (file_handle != INVALID_HANDLE_VALUE) {
		LONG read_size = gen_random(4096);
		DWORD ret = SetFilePointer(
			file_handle,
			0 - read_size,
			0, // dist high
			FILE_END);

		DWORD bytes_read = 0;
		char buff[4096];
		BOOL read_successful = ReadFile(file_handle, &buff, read_size, &bytes_read, NULL);

		if (read_successful) {
			wprintf(L"tid %8d|%4i/%4i| ReadFile() from file end OK\n", GetCurrentThreadId(), iteration, file_operation_num);
		}
		else {
			wprintf(L"tid %8d|%4i/%4i| ReadFile() from file end failed\n", GetCurrentThreadId(), iteration, file_operation_num);
		}

		CloseHandle(file_handle);
	}
	else {
		wprintf(L"tid %8d|%4i/%4i| read_from_end() failed to open file\n", GetCurrentThreadId(), iteration, file_operation_num);
	}
}

// We'll try to get file attributes (and do nothing with them)
void get_file_attributes(std::wstring* path, int iteration) {
	WIN32_FILE_ATTRIBUTE_DATA attr;
	if (GetFileAttributesEx(path->c_str(), GetFileExInfoStandard, &attr) == 0) {
		wprintf(L"tid %8d|%4i/%4i| GetFileAttributesEx() OK\n", GetCurrentThreadId(), iteration, file_operation_num);
	}
	else {
		wprintf(L"tid %8d|%4i/%4i| GetFileAttributesEx() has failed\n", GetCurrentThreadId(), iteration, file_operation_num);
	}
}

// We'll try to LockFile() for random valid range, and then UnlockFile() after a while.
void lock_file_range(std::wstring* path, int iteration) {
	HANDLE file_handle = CreateFileW(path->c_str(),
		GENERIC_READ,
		FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (file_handle != INVALID_HANDLE_VALUE) {
		LARGE_INTEGER file_size;

		BOOL res = GetFileSizeEx(
			file_handle,
			&file_size);

		if (res && file_size.QuadPart > 0) {
			LARGE_INTEGER lock_start;
			lock_start.QuadPart = gen_random(file_size.QuadPart);

			LARGE_INTEGER lock_bytes;
			lock_bytes.QuadPart = gen_random(file_size.QuadPart - lock_start.QuadPart);

			if (LockFile(file_handle, lock_start.LowPart, lock_start.HighPart, lock_bytes.LowPart, lock_bytes.HighPart)) {
				wprintf(L"tid %8d|%4i/%4i| LockFile() range %I64d to %I64d OK\n", GetCurrentThreadId(), iteration, file_operation_num, lock_start.QuadPart, lock_bytes.QuadPart);

				//sleep up to 100ms with lock held
				Sleep(gen_random(100));

				if (UnlockFile(file_handle, lock_start.LowPart, lock_start.HighPart, lock_bytes.LowPart, lock_bytes.HighPart)) {
					wprintf(L"tid %8d|%4i/%4i| UnlockFile() OK\n", GetCurrentThreadId(), iteration, file_operation_num);
				}
				else {
					wprintf(L"tid %8d|%4i/%4i| UnlockFile() has failed\n", GetCurrentThreadId(), iteration, file_operation_num);
				}
			}
			else {
				wprintf(L"tid %8d|%4i/%4i| LockFile() range %I64d to %I64d has failed\n", GetCurrentThreadId(), iteration, file_operation_num, lock_start.QuadPart, lock_bytes.QuadPart);
			}
		}
		CloseHandle(file_handle);
	}
	else {
		wprintf(L"tid %8d|%4i/%4i| lock_file_range() failed to open file\n", GetCurrentThreadId(), iteration, file_operation_num);
	}

}


void get_file_type(std::wstring* path, int iteration) {
	HANDLE file_handle = CreateFileW(path->c_str(),
		GENERIC_READ,
		FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (file_handle != INVALID_HANDLE_VALUE) {
		if (FILE_TYPE_UNKNOWN != GetFileType(file_handle)) {
			wprintf(L"tid %8d|%4i/%4i| GetFileType() OK\n", GetCurrentThreadId(), iteration, file_operation_num);
		}
		else {
			wprintf(L"tid %8d|%4i/%4i| GetFileType() has failed\n", GetCurrentThreadId(), iteration, file_operation_num);
		}
		CloseHandle(file_handle);
	}
}

// Exercises GetBinaryType(). This should fail, unless file is actualy an executable.
void get_binary_type(std::wstring* path, int iteration) {
	DWORD binary_type;
	if (GetBinaryType(
		path->c_str(),
		&binary_type))
	{
		wprintf(L"tid %8d|%4i/%4i| GetBinaryType() OK\n", GetCurrentThreadId(), iteration, file_operation_num);
	}
	else {
		wprintf(L"tid %8d|%4i/%4i| GetBinaryType() has failed\n", GetCurrentThreadId(), iteration, file_operation_num);
	}

}
