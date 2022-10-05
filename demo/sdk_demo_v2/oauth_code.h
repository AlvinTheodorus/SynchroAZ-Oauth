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
			bool isAccessTokenExpired() const noexcept { return false; }
			bool isRefreshTokenExpired() const noexcept { return false; }

			Token refreshIfExpired() {
				return *this;
			}
		};

		struct User {
		private:
			std::string id_;
			std::string first_name_;
			std::string last_name_;
			std::string email_;
			std::uint_fast64_t personal_meeting_id_;

		public:
			User(std::string id, std::string first_name, std::string last_name, std::string email, std::uint_fast64_t personal_meeting_id)
				: id_(std::move(id))
				, first_name_(std::move(first_name))
				, last_name_(std::move(last_name))
				, email_(std::move(email_))
				, personal_meeting_id_(personal_meeting_id) {}

			const std::string& id() const noexcept { return id_; }
			const std::string& firstName() const noexcept { return first_name_; }
			const std::string& lastName() const noexcept { return last_name_; }
			const std::string& email() const noexcept { return email_; }
			std::uint_fast64_t personalMeetingId() const noexcept { return personal_meeting_id_; }
		};

		void PassOAuthCodeToMainProcess();
		void StartOAuthSequence(std::function<void(Token)> token_callback);
		Token RunOAuthSequence();
		Token RunPkceSequence(const std::string& code, const std::string& code_verifier);
		std::string GetZakToken(Token token);
		User GetUser(Token token);
	}
}