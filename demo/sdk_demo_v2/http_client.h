#pragma once

#pragma comment(lib, "winhttp.lib")

#include <Windows.h>
#include <tchar.h>
#include <winhttp.h>
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
		namespace detail {
			struct InternetHandleDeleter {
				void operator()(HINTERNET handle) const {
					::WinHttpCloseHandle(handle);
				}
			};
		}

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
			/*static std::string UrlEncode(const std::string& value) {
				DWORD size = value.size() * 2;
				auto key = std::make_unique<char[]>(size);
				UrlEscapeA(value.c_str(), key.get(), &size, URL_ESCAPE_SEGMENT_ONLY);
				return { key.get(), key.get() + size };
			}*/

		public:
			std::vector<std::uint8_t> toBytes() const override {
				std::vector<std::string> kvs;
				for (const auto& kv : values_) {
					for (const auto& v : kv.second) {
						kvs.emplace_back(kv.first + "=" + v);
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
			HttpResponse doRequest(const tstring& method, const tstring& server, const tstring& path, const std::vector<tstring>& headers, const HttpUrlEncodedBody& body) {
#define HINTERNET_SMART_PTR(e) ::std::unique_ptr<void, ::saz::http::detail::InternetHandleDeleter>(e, ::saz::http::detail::InternetHandleDeleter{})
				static const TCHAR* ACCEPT_TYPE[] = {
					_T("application/json"),
					_T("text/json"),
					nullptr,
				};

				const auto handle = HINTERNET_SMART_PTR(::WinHttpOpen(nullptr, WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0));

				const auto conn = HINTERNET_SMART_PTR(::WinHttpConnect(handle.get(), server.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0));
				const auto req = HINTERNET_SMART_PTR(::WinHttpOpenRequest(conn.get(), method.c_str(), path.c_str(), nullptr, WINHTTP_NO_REFERER, ACCEPT_TYPE, WINHTTP_FLAG_SECURE));

				for (const auto& header : headers) {
					::WinHttpAddRequestHeaders(req.get(), header.c_str(), wcslen(header.c_str()), 0);
				}

				tstring header = _T("Content-Type: application/x-www-form-urlencoded");
				
				const auto bytes = body.toBytes();
				auto bytes_p = std::make_unique<char[]>(bytes.size());
				std::copy(std::begin(bytes), std::end(bytes), bytes_p.get());

				::WinHttpSendRequest(req.get(), header.c_str(), header.size(), bytes_p.get(), bytes.size(), bytes.size(), 0);
				::WinHttpReceiveResponse(req.get(), nullptr);

				DWORD status_code = 0;
				DWORD stauts_code_size = sizeof(status_code);
				::WinHttpQueryHeaders(req.get(), WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &status_code, &stauts_code_size, WINHTTP_NO_HEADER_INDEX);

				std::string res_body;
				while (true) {
					DWORD size = 0;
					if (!WinHttpQueryDataAvailable(req.get(), &size)) {
						break;
					}
					if (size == 0) {
						break;
					}
					auto buffer = std::make_unique<char[]>(size + 1);
					DWORD number_of_bytes = 0;
					if (!WinHttpReadData(req.get(), buffer.get(), size, &number_of_bytes)) {
						break;
					}
					res_body.append(buffer.get(), buffer.get() + number_of_bytes);
				}

				return { static_cast<int>(status_code), res_body };
#undef HINTERNET_SMART_PTR
			}

		public:
			HttpResponse post(const tstring& server, const tstring& path, const std::vector<tstring>& headers, const HttpUrlEncodedBody& body) {
				return doRequest(_T("POST"), server, path, headers, body);
			}
		};
	}
}