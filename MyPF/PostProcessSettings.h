#pragma once
#include <cstdint>

namespace My
{
	enum class ToneMapper : std::uint32_t
	{
		Reinhard = 0,
		ACES = 1,
	};

	// CPU only: picks which texture EndScene copies to the back buffer.
	enum class PostProcessDebugView
	{
		Final,
		Bright,
		BlurX,
		Blur,
	};

	struct PostProcessSettings
	{
		float				 exposure{ 1.0f };
		float				 threshold{ 1.0f };
		float				 bloomStrength{ 0.5f };
		int					 bloomBlurIterations{ 1 };
		ToneMapper			 toneMapper{ ToneMapper::ACES };
		PostProcessDebugView debugView{ PostProcessDebugView::Final };
	};
} // namespace My