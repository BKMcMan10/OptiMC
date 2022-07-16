#include "core/DataParser.h"

using json = nlohmann::json;


DataParser::DataParser(const char* path)
{
	std::cout << path << std::endl;
	std::ifstream jem_file{ path };

	json jem;
	jem_file >> jem;

	for (const auto& element : jem) 
	{
		std::cout << element << std::endl;
	}

	for (json::iterator it = jem.begin(); it != jem.end(); ++it) 
	{
		std::cout << it.key() << " : " << it.value() << "\n";
	}

	for (json::iterator ji = jem.begin(); ji != jem.end(); ++ji) 
	{
		if (jem.contains("models"))
		{
			size_t numBoxes = jem["models"][0]["boxes"].size();
			std::cout << "Number of boxes: " << numBoxes << "\n" << std::endl;
		}
		else
		{
			std::cout << "Unable To Load Json  Beep Boop" << "\n" << std::endl;
		}
	}

	std::cout << "\n\n" << std::endl;

	const auto coords3 = jem["models"][0]["boxes"][2]["coordinates"];
	if (jem["models"][0]["part"] == "B9ody")
	{
		std::cout << "models[0].boxes[3].coordinates: " << coords3 << std::endl;
	}
	else if (jem["models"][10]["part"] == "Head")
	{
		std::cout << "YEEEEEEE" << std::endl;
	}
}