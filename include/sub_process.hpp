#include <string>

namespace sub_process
{	struct result
	{	std::string cout, cerr;
	};
	result run(std::string application, std::string arguments);
}
