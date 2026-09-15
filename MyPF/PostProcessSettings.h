#pragma once
#include <cstdint>

namespace My
{
	enum class ToneMapper : std::uint32_t
	{
		Reinhard = 0,
		ACES = 1,
	};

	struct PostProcessSettings
	{
		float	   exposure{ 1.0f };
		ToneMapper toneMapper{ ToneMapper::ACES };
	};
} // namespace My