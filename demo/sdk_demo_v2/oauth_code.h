#pragma once

#include <functional>
#include <string>

namespace saz {
	std::wstring StringToWideString(const std::string& str);

	namespace oauth2 {
		struct Token {
		private:
			static Token current_;

		public:
			static Token& GetCurrent() {
				return current_;
			}
			void setAsCurrent() {
				current_ = *this;
			}

		private:
			std::string access_token_;
			std::string token_type_;
			std::string refresh_token_;
			std::uint_fast64_t expires_in_;
			std::string scope_;

		public:
			Token() = default;
			Token(const Token&) = default;
			Token(Token&&) = default;
			Token& operator=(const Token&) = default;
			Token& operator=(Token&&) = default;

			~Token() = default;

			Token(
				std::string access_token,
				std::string token_type,
				std::string refresh_token,
				std::uint_fast64_t expires_in,
				std::string scope
			) : access_token_(std::move(access_token))
				, token_type_(std::move(token_type))
				, refresh_token_(std::move(refresh_token))
				, expires_in_(expires_in)
				, scope_(std::move(scope)) {}

		public:
			const std::string& accessToken() const noexcept { return access_token_; }
			const std::string& tokenType() const noexcept { return token_type_; }
			const std::string& refreshToken() const noexcept { return refresh_token_; }
			std::uint_fast64_t expiresIn() const noexcept { return expires_in_; }
			const std::string& scope() const noexcept { return scope_; }

		public:
			Token refresh() {
				return *this;
			}
			bool isAccessTokenExpired() const noexcept {}
			bool isRefreshTokenExpired() const noexcept {}

			Token refreshIfExpired() {
				return *this;
			}
		};

		void PassOAuthCodeToMainProcess();
		void StartOAuthSequence(std::function<void(Token)> token_callback);
		Token RunPkceSequence(const std::string& code, const std::string& code_verifier);
		std::string GetZakToken(Token token);
	}
}