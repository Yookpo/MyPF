#pragma once
#include <string>
#include "ModelData.h"


namespace My
{
	class ModelLoader
	{
	public:
		static bool Load(const std::string& filePath, ModelData& outModelData);


	};
}


