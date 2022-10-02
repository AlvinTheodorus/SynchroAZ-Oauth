#include "oauth_code.h"

#include <string>
#include <cstdlib>
#include <sstream>
#include <random>
#include <cstddef>
#include <unordered_map>
#include <thread>

// cryptopp
#include "sha.h"
#include "base64.h"
#include "hex.h"

#include "stdafx.h"

namespace {
	std::string GenerateRandomString(std::size_t length) {
		static constexpr char CHARS[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

		std::random_device rand_device;
		std::uniform_int_distribution<std::uint_fast16_t> rand(0, std::size(CHARS) - 1);

		std::string s(length, '\0');
		for (std::size_t i = 0; i < length; ++i) {
			s[i] = CHARS[rand(rand_device)];
		}
		return s;
	}

	std::string Sha256HashString(std::string aString) {
		std::string digest;
		CryptoPP::SHA256 hash;

		CryptoPP::StringSource foo9(aString, true,
			new CryptoPP::HashFilter(hash,
				new CryptoPP::StringSink(digest)));

		return digest;
	}

	std::string Base64Encode(std::string aString) {
		std::string result;

		CryptoPP::StringSource foo8(aString, true,
			new CryptoPP::Base64Encoder(
				new CryptoPP::StringSink(result)));

		return result;
	}

	constexpr char OAUTH2_RESPONSE_TYPE[] = "code";
	constexpr char OAUTH2_REDIRECT_URI[] = "com.5saz://v1/auth/oauth2";
	constexpr char OAUTH2_CLIENT_ID[] = "iVpmSZ5wSyqqcFTPxkDGDg";
	constexpr char OAUTH2_CODE_CHALLENGE_METHOD[] = "S256";
	constexpr TCHAR OAUTH2_CODE_EXCHANGE_PIPE_PATH[] = _T(R"(\\.\pipe\synchroaz\com.5saz.auth.oauth2)");

	class Uri {
	public:
		using QueryType = std::unordered_map<std::string, std::vector<std::string>>;

	private:
		std::string scheme_;
		std::string path_;
		QueryType query_;

	public:
		Uri(std::string scheme, std::string path, QueryType query) : scheme_(std::move(scheme)), path_(std::move(path)), query_(std::move(query)) {}

		Uri() = default;
		Uri(const Uri&) = default;
		Uri(Uri&&) = default;

		Uri& operator=(const Uri&) = default;
		Uri& operator=(Uri&&) = default;

		~Uri() = default;

	public:
		const std::string& scheme() const noexcept { return scheme_; }
		const std::string& path() const noexcept { return path_; }
		const QueryType& query() const noexcept { return query_; }

	public:
		static std::unique_ptr<Uri> Parse(std::string uri) {
			const auto scheme_sep_pos = uri.find("://");
			if (scheme_sep_pos == std::string::npos) {
				return nullptr;
			}
			auto scheme = uri.substr(0, scheme_sep_pos);

			const auto path_start_pos = scheme_sep_pos + 3;
			const auto path_end_pos = uri.find('?', path_start_pos);
			if (path_end_pos == std::string::npos) {
				return std::make_unique<Uri>(scheme, uri.substr(path_start_pos-1), QueryType{});
			}

			auto path = uri.substr(path_start_pos-1, path_end_pos - path_start_pos + 1);

			QueryType queries;
			auto previous_end_pos = path_end_pos + 1;
			while (true) {
				const auto end_pos = uri.find('&', previous_end_pos);
				auto query = uri.substr(previous_end_pos, end_pos - previous_end_pos);

				const auto eq_pos = query.find('=');
				auto key = query.substr(0, eq_pos);
				auto value = query.substr(eq_pos + 1);

				queries[key].emplace_back(value);

				if (end_pos == std::string::npos) {
					break;
				}
				previous_end_pos = end_pos + 1;
			}

			return std::make_unique<Uri>(scheme, path, queries);
		}
	};
}

namespace saz {
	std::wstring StringToWideString(const std::string& str) {
		auto wide = std::make_unique<wchar_t[]>(str.size() + 1);

		std::size_t converted = 0;
		mbstowcs_s(&converted, wide.get(), str.size() + 1, str.c_str(), _TRUNCATE);
		return std::wstring(wide.get());
	}

	namespace oauth2 {
		void PassOAuthCodeToMainProcess() {
			if (__argc > 1) {
				const std::string argv1 = __argv[1];
				if (argv1.find("--url") == 0) {

					const auto eq_pos = argv1.find('=');
					std::string url;
					if (eq_pos != std::string::npos) {
						url = argv1.substr(eq_pos + 1);
					}
					else if (__argc > 2) {
						url = __argv[2];

					}
					else {
						std::exit(EXIT_FAILURE);
					}

					const auto hPipe = ::CreateFile(OAUTH2_CODE_EXCHANGE_PIPE_PATH, GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
					if (hPipe == INVALID_HANDLE_VALUE) {
						std::exit(EXIT_FAILURE);
					}

					::WriteFile(hPipe, url.c_str(), url.size(), nullptr, nullptr);
					std::exit(EXIT_SUCCESS);
				}
			}
		}

		void StartOAuthSequence(std::function<void(std::string)> code_callback) {
			std::string code_verifier = GenerateRandomString(43);//"OoRepCjjX8P4perVeWHK-5TKSfyZLPuCjirkjY3hh7I";
			std::string code_challenge = Base64Encode(Sha256HashString(code_verifier));

			std::stringstream ss;
			ss << "https://zoom.us/oauth/authorize"
				<< "?response_type=" << OAUTH2_RESPONSE_TYPE
				<< "&redirect_uri=" << OAUTH2_REDIRECT_URI
				<< "&client_id=" << OAUTH2_CLIENT_ID
				<< "&code_challenge=" << code_challenge
				<< "&code_challenge_method=" << OAUTH2_CODE_CHALLENGE_METHOD;
			std::string url_naive = ss.str();

			auto url_wide = StringToWideString(url_naive);

			//::ShellExecute(nullptr, _T("open"), url_wide.get(), nullptr, nullptr, SW_SHOWNORMAL);
			::ShellExecute(nullptr, _T("open"), _T("com.5saz://v1/auth/oauth2?code=abcdefghijklmnopqrstuvwxyz"), nullptr, nullptr, SW_SHOWNORMAL);
			std::thread thread([code_callback] {

				const auto hPipe = ::CreateNamedPipe(OAUTH2_CODE_EXCHANGE_PIPE_PATH, PIPE_ACCESS_INBOUND, PIPE_TYPE_BYTE | PIPE_WAIT, 1, 0, 0, 100, nullptr);
				if (hPipe == INVALID_HANDLE_VALUE) {
					std::exit(EXIT_FAILURE);
				}
				std::string url;
				while (true) {
					char buffer[256] = {};
					DWORD bytes_read = 0;
					if (::ReadFile(hPipe, buffer, sizeof(buffer), &bytes_read, nullptr)) {
						url.assign(buffer, bytes_read);
						break;
					}
				}

				const auto uri = Uri::Parse(url);
				if (!uri) {
					printf("Parse failed\n");
					std::exit(EXIT_FAILURE);
				}

				printf("scheme %s\n", uri->scheme().c_str());
				if (uri->scheme() != "com.5saz") {
					printf("Invalid scheme %s\n", uri->scheme().c_str());
					std::exit(EXIT_FAILURE);
				}

				printf("path %s\n", uri->path().c_str());
				if (uri->path() != "/v1/auth/oauth2") {
					printf("Invalid path %s\n", uri->path().c_str());
					std::exit(EXIT_FAILURE);
				}
				const auto& query = uri->query();
				for (const auto& kv : query) {
					printf("  key: %s\n", kv.first.c_str());
					for (const auto& v : kv.second) {
						printf("    value: %s\n", v.c_str());
					}
				}
				if (query.find("code") == std::end(query)) {
					printf("Cannot find query named 'code'\n");
					for (const auto& kv : query) {
						printf("  key: %s\n", kv.first.c_str());
						for (const auto& v : kv.second) {
							printf("    value: %s\n", v.c_str());
						}
					}
					std::exit(EXIT_FAILURE);
				}

				const auto code = query.at("code")[0];
				printf("code: %s\n", code.c_str());
				code_callback(code);
			});
			thread.detach();
		}
	}
}