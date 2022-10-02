#pragma once

#include <functional>
#include <string>

namespace saz {
	std::wstring StringToWideString(const std::string& str);

	namespace oauth2 {
		void PassOAuthCodeToMainProcess();
		void StartOAuthSequence(std::function<void(std::string)> code_callback);
	}
}