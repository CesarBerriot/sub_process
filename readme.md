### Usage
```cpp
static void log_if_non_empty(std::string label, std::string message)
{	if(!message.empty())
		std::cout << label << " : " << message << std::endl;
}

try
{	sub_process::result result = sub_process::run("cmd", "/k echo Hello, World! && exit");
	log_if_non_empty("cout", result.cout);
	log_if_non_empty("cerr", result.cerr);
}
catch(const std::exception exception)
{	log_if_non_empty("exception", exception.what());
}
```