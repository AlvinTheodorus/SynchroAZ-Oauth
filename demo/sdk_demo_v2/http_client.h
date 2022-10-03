#pragma once

#pragma comment(lib, "wininet.lib")

#include <Windows.h>
#include <tchar.h>
#include <wininet.h>
#include <Shlwapi.h>

#include <algorithm>
#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <memory>
#include <sstream>

namespace saz {
	using tstring = std::basic_string<TCHAR>;

	namespace http {
		class IHttpBody {
		public:
			virtual std::vector<std::uint8_t> toBytes() const = 0;
		};

		class HttpUrlEncodedBody : public IHttpBody {
		public:
			std::unordered_map<std::string, std::vector<std::string>> values_;

		public:
			HttpUrlEncodedBody& add(std::string key, std::string value) {
				values_[std::move(key)].emplace_back(std::move(value));
				return *this;
			}

		private:
			static std::string UrlEncode(const std::string& value) {
				DWORD size = value.size() * 2;
				auto key = std::make_unique<char[]>(size);
				UrlEscapeA(value.c_str(), key.get(), &size, URL_ESCAPE_SEGMENT_ONLY);
				return { key.get(), key.get() + size };
			}

		public:
			std::vector<std::uint8_t> toBytes() const override {
				std::vector<std::string> kvs;
				for (const auto& kv : values_) {
					auto key = UrlEncode(kv.first);
					for (const auto& v : kv.second) {
						auto value = UrlEncode(v);
						kvs.emplace_back(key + "=" + value);
					}
				}
				auto itr = std::begin(kvs);
				std::stringstream ss;
				ss << *itr;
				std::for_each(itr + 1, std::end(kvs), [&ss](const std::string& s) {
					ss << "&" << s;
					});
				ss << std::flush;
				
				const auto value = ss.str();

				std::vector<std::uint8_t> bytes;
				std::copy(std::begin(value), std::end(value), std::back_inserter(bytes));
				return bytes;
			}
		};

		class HttpResponse {
		private:
			int status_code_;
			std::string body_;

		public:
			HttpResponse(int status_code, std::string body) : status_code_(status_code), body_(std::move(body)) {}

		public:
			int statusCode() const noexcept { return status_code_; }
			const std::string& body() const noexcept { return body_; }
		};

		class HttpClient {
		private:
			HINTERNET handle_;
			HINTERNET connection_handle_;

		public:
			HttpClient() :handle_(::InternetOpen(
				nullptr,
				INTERNET_OPEN_TYPE_PRECONFIG,
				nullptr, // proxy server
				nullptr,
				0)
			),
				connection_handle_(nullptr) {}

			~HttpClient() {
				::InternetCloseHandle(handle_);
				::InternetCloseHandle(connection_handle_);
				handle_ = nullptr;
				connection_handle_ = nullptr;
			}

		public:
			void connect(const tstring& server) {
				connection_handle_ = ::InternetConnect(handle_, server.c_str(), INTERNET_DEFAULT_HTTPS_PORT, nullptr, nullptr, INTERNET_SERVICE_HTTP, 0, 0);
			}

		private:
			HINTERNET openRequest(const tstring& method, const tstring& path) {
				static const TCHAR* ACCEPT_TYPE[] = {
					_T("application/json"),
					_T("text/json"),
					nullptr,
				};

				return ::HttpOpenRequest(
					connection_handle_,
					method.c_str(),
					path.c_str(),
					_T("HTTP/1.1"),
					nullptr,
					ACCEPT_TYPE,
					INTERNET_FLAG_RELOAD,
					0
				);
			}

			void sendRequest(HINTERNET request_handle, const std::vector<tstring>& headers, const IHttpBody& body) {
				const auto bytes = body.toBytes();
				auto bytes_p = std::make_unique<char[]>(bytes.size());
				std::copy(std::begin(bytes), std::end(bytes), bytes_p.get());


				for (const auto& header : headers) {
					::HttpAddRequestHeaders(request_handle, header.c_str(), wcslen(header.c_str()), 0);
				}

				tstring header = _T("Content-Type: application/x-www-form-urlencoded");
				::HttpSendRequest(request_handle, header.c_str(), wcslen(header.c_str()), bytes_p.get(), bytes.size());
			}

			HttpResponse getResponse(HINTERNET request_handle) {
				DWORD status_code = 0;
				DWORD length = sizeof(status_code);
				::HttpQueryInfo(request_handle, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &status_code, &length, 0);

				std::string body;
				while (true) {
					char buffer[1024] = {};
					DWORD read_size = 0;
					::InternetReadFile(request_handle, buffer, sizeof(buffer), &read_size);
					if (read_size == 0) {
						break;
					}
					body.append(buffer, buffer + read_size);
				}

				return {status_code, body};
			}

		public:
			HttpResponse post(const tstring& path, const std::vector<tstring>& headers, const HttpUrlEncodedBody& body) {
				auto request = openRequest(_T("POST"), path);
				sendRequest(request, headers, body);
				return getResponse(request);
			}
		};
	}
}