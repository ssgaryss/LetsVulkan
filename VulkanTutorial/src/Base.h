#pragma once


#ifdef VK_TUTORIAL_DEBUG
	const bool IsEnableValidationLayer = true;
#else
	const bool IsEnableValidationLayer = false;
#endif