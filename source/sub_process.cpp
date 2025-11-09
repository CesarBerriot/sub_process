#include "sub_process.hpp"
#include <optional>
#include <filesystem>
#include <windows.h>
#include <string_theory/string>
#include <string_theory/format>

static void throw_win32_function_failure(std::string function)
{	throw
		std::runtime_error
		(	ST::format
			(	"{} failure with error : {} ({#X}).",
				function,
				GetLastError(),
				GetLastError()
			).to_std_string()
		);
}

static void close_handle(HANDLE handle, bool allow_invalid_handles = false)
{	if
	(	!CloseHandle(handle) &&
		(	!allow_invalid_handles ||
			(GetLastError() != ERROR_INVALID_HANDLE)
		)
	)
		throw_win32_function_failure("CloseHandle");
}

namespace sub_process
{	result run(std::string application, std::string arguments)
	{	char * raw_path = getenv("PATH");
		if(!raw_path)
			throw std::runtime_error("PATH environment variable missing.");

		std::optional<std::string> application_path;
		for(ST::string path : ST::string(raw_path).split(';'))
		{	std::filesystem::path potential_application_path =
				std::filesystem::path(path.to_std_string())
				/ (application + ".exe");
			if(std::filesystem::is_regular_file(potential_application_path))
			{	application_path = potential_application_path.string();
				break;
			}
		}
		if(!application_path)
			throw std::runtime_error("Couldn't find application executable in PATH.");

		struct pipe
		{	HANDLE read, write;
			pipe()
			{	SECURITY_ATTRIBUTES security_attributes =
					{	.nLength = sizeof(SECURITY_ATTRIBUTES),
						.bInheritHandle = true
					};
				if(!CreatePipe(&read, &write, &security_attributes, 0))
					throw_win32_function_failure("CreatePipe");
			}
			~pipe()
			{	close_handle(read);
				close_handle(write);
			}
			std::string read_output()
			{	DWORD size = GetFileSize(read, NULL);
				if(size == INVALID_FILE_SIZE)
					throw_win32_function_failure("GetFileSize");
				if(size)
				{	std::unique_ptr<char[]> buffer = std::make_unique<char[]>(size);
					if(!ReadFile(read, buffer.get(), size, NULL, 0))
						throw_win32_function_failure("ReadFile");
					return
						{	buffer.get(),
							(size_t)
							(	std::find(buffer.get(), buffer.get() + size - 1, EOF)
								- buffer.get()
							)
						};
				}
				else
					return "";
			}
		} cout, cerr;

		STARTUPINFOA startup_information;
		GetStartupInfoA(&startup_information);
		startup_information.hStdOutput = cout.write;
		startup_information.hStdError = cerr.write;
		startup_information.dwFlags = STARTF_USESTDHANDLES;
		PROCESS_INFORMATION process_information;
		if
		(	!CreateProcessA
			(	application_path->c_str(),
				ST::format("\"{}\" {}", std::filesystem::current_path(), arguments).to_std_string().data(),
				NULL,
				NULL,
				TRUE,
				CREATE_NEW_PROCESS_GROUP,
				NULL,
				NULL,
				&startup_information,
				&process_information
			)
		)
			throw_win32_function_failure("CreateProcessA");
		if(WaitForSingleObject(process_information.hProcess, INFINITE) == WAIT_FAILED)
			throw_win32_function_failure("WaitForSingleObject");
		for(HANDLE handle : { process_information.hProcess, process_information.hThread })
			close_handle(handle, true);

		return { .cout = cout.read_output(), .cerr = cerr.read_output() };
	}
}