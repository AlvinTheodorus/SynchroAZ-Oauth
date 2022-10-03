#pragma once
#include "sdk_util.h"
#include "UIlib.h"
#include "resource.h"
#include "LOGIN_login_with_sso_workflow.h"
#include "LOGIN_join_meeting_only_workflow.h"
#include "LOGIN_restapi_without_login_workflow.h"
#include "sdk_demo_app_common.h"
#include "mess_info.h"
#include "sha.h"
#include "filters.h"
#include "base64.h"
#include "hex.h"
#include "files.h"
#include <iostream>
#include <cstddef>
#include <cstring>
#include <algorithm>
#include <vector>

/////////////////////////

enum loginTabPage
{
	login_RestAPI_Page,
	login_UseSSO_Page,
	login_JoinMeetingOnly_Page
};

class CSDKLoginUIMgr;
class CSDKLoginWithSSOUIGroup
{
public:
	CSDKLoginWithSSOUIGroup();
	virtual ~CSDKLoginWithSSOUIGroup();
	void InitWindow(CPaintManagerUI& ui_mgr, CSDKLoginUIMgr* main_frame_);
	void UninitWindow();
	void Show();
	void Hide();
	void Notify( TNotifyUI& msg );

	void DoLoginWithSSOBtnClick();

	std::string SHA256HashString(std::string aString) {
		std::string digest;
		CryptoPP::SHA256 hash;

		CryptoPP::StringSource foo9(aString, true,
			new CryptoPP::HashFilter(hash,
					new CryptoPP::StringSink(digest)));

		return digest;
	}

	std::string Base64URLconvert(std::string aString) {
		std::string result;
		/*char const* c = aString.c_str();
		byte bytes[c.size()];
		std::memcpy(bytes, aString.data(), aString.length());*/

		/*std::vector<char> bytes(aString.begin(), aString.end());
		bytes.push_back('\0');
		char* c = &bytes[0];*/

		/*CryptoPP::StringSource foo8((byte*)aString.data(), aString.size(), true,
				new CryptoPP::Base64URLEncoder(
					new CryptoPP::StringSink(result)));*/
					
		CryptoPP::StringSource foo8(aString, true,
				new CryptoPP::Base64Encoder(
					new CryptoPP::StringSink(result)));
		

		return result;

		/*byte decoded[] = { 0xad, 0xf1, 0x52, 0x4fd, 0x86, 0xe01, 0xb1, 0x28, 0xe4, 0x53, 0xf4, 0x71, 0x1a, 0xcd, 0x5b, 0xcd, 0x76, 0x5e, 0x6e, 0x4c, 0x06, 0x11, 0xec, 0xbb, 0xbf, 0xfa, 0x2c, 0xdc, 0x4a, 0x9f, 0x28 };
		std::string encoded;

		CryptoPP::StringSource ss(decoded, sizeof(decoded), true,
			new CryptoPP::Base64URLEncoder(
				new CryptoPP::StringSink(encoded)
			) 
		); 
		std::cout << encoded << std::endl;
		return encoded;*/
	}

	std::string lower(std::string aString) {
		std::transform(aString.begin(), aString.end(), aString.begin(), ::tolower);
		return aString;
	}


	std::string pack256(std::string aString) {
		/*using namespace CryptoPP;
		SHA256 hash;*/
		
		std::string digest;
		CryptoPP::SHA256 hash;

		/*StringSource(aString, true, new HashFilter(hash, new StringSink(digest)));*/

		CryptoPP::StringSource foo1(aString, true,
			new CryptoPP::HashFilter(hash,
				new CryptoPP::HexEncoder(
						new CryptoPP::StringSink(digest))));

		return digest;
	}

	std::string UTF8toISO8859_1(const char* in)
	{
		std::string out;
		if (in == NULL)
			return out;

		unsigned int codepoint;
		while (*in != 0)
		{
			unsigned char ch = static_cast<unsigned char>(*in);
			if (ch <= 0x7f)
				codepoint = ch;
			else if (ch <= 0xbf)
				codepoint = (codepoint << 6) | (ch & 0x3f);
			else if (ch <= 0xdf)
				codepoint = ch & 0x1f;
			else if (ch <= 0xef)
				codepoint = ch & 0x0f;
			else
				codepoint = ch & 0x07;
			++in;
			if (((*in & 0xc0) != 0x80) && (codepoint <= 0x10ffff))
			{
				if (codepoint <= 255)
				{
					out.append(1, static_cast<char>(codepoint));
				}
				else
				{
					// do whatever you want for out-of-bounds characters
				}
			}
		}
		return out;
	}

	std::string hexconvert(std::string aString) {
		std::string result;

		CryptoPP::StringSource foo8(aString, true,
			new CryptoPP::HexEncoder(
				new CryptoPP::StringSink(result)));

		return result;
	}

	std::string new_pack(std::string aString) {
		std::string digest;
		CryptoPP::SHA256 hash;
		CryptoPP::HexEncoder encoder(new CryptoPP::StringSink(digest));

		hash.Update((const byte*)aString.data(), aString.size());
		digest.resize(hash.DigestSize());
		hash.Final((byte*)&digest[0]);

		CryptoPP::StringSource(digest, true,
			new CryptoPP::HexEncoder(
				new CryptoPP::Base64Encoder(
					new CryptoPP::StringSink(digest))));

		return digest;
	}


protected:
	CVerticalLayoutUI* m_loginWithSSOPage;
	CRichEditUI*	   m_editSSOtoken;
	CSDKLoginUIMgr*	   m_parentFrame;
	CSDKLoginWithSSOFlow   m_loginSSOWorkFlow;
	CButtonUI*		   m_btnLogin;
	CButtonUI* m_btnGetSSOLoginUrl;
	CRichEditUI* m_editPrefixOfVanityUrl;
};

class CSDKLoginCBHandler : public ZOOM_SDK_NAMESPACE::IAuthServiceEvent
{
public:
	CSDKLoginCBHandler();
	virtual ~CSDKLoginCBHandler();
	void SetUIEvent(CSDKLoginUIMgr* main_frame_);
	virtual void onAuthenticationReturn(ZOOM_SDK_NAMESPACE::AuthResult ret) {};
	virtual void onLoginReturnWithReason(ZOOM_SDK_NAMESPACE::LOGINSTATUS ret, ZOOM_SDK_NAMESPACE::IAccountInfo* pAccountInfo, ZOOM_SDK_NAMESPACE::LoginFailReason reason);
	virtual void onLogout();
	virtual void onZoomIdentityExpired();
	virtual void onZoomAuthIdentityExpired() {}

protected:
	CSDKLoginUIMgr* m_parentFrame;
};

class CSDKWithoutLoginStartJoinMeetingUIGroup :public CSDKJoinMeetingOnlyFlowUIEvent
{
public:
	CSDKWithoutLoginStartJoinMeetingUIGroup();
	virtual ~CSDKWithoutLoginStartJoinMeetingUIGroup();
	void InitWindow(CPaintManagerUI& ui_mgr, CSDKLoginUIMgr* main_frame_);
	void UninitWindow();
	void Cleanup();
	void Show();
	void Hide();
	void Notify(TNotifyUI& msg);

	void DoWithoutLoginStartJoinMeetingBtnClick();
public:
	virtual void onMeetingStatusChanged(ZOOM_SDK_NAMESPACE::MeetingStatus status, int iResult = 0);
	virtual void onMeetingStatisticsWarningNotification(ZOOM_SDK_NAMESPACE::StatisticsWarningType type) {};
	virtual void onMeetingParameterNotification(const ZOOM_SDK_NAMESPACE::MeetingParameter* meeting_param) {};
  
protected:
	CVerticalLayoutUI* m_WithoutLoginStartJoinMeetingPage;
	CRichEditUI*	   m_editMeetingNumber;
	CRichEditUI*	   m_editScreenName;
	CRichEditUI*	   m_editMeetingPassword;
	CButtonUI*		   m_btnJoin;
	CSDKLoginUIMgr*	   m_parentFrame;
	CSDKWithoutLoginStartJoinMeetingFlow  m_withoutLoginJoinMeetingWorkFlow;
	bool m_bInMeeting;
};

class CSDKRestAPIUserUIGroup : public CSDKRestAPIUserUIEvent
{
public:
	CSDKRestAPIUserUIGroup();
	virtual ~CSDKRestAPIUserUIGroup();
	void InitWindow(CPaintManagerUI& ui_mgr, CSDKLoginUIMgr* main_frame_);
	void UninitWindow();
	void Cleanup();
	void Show();
	void Hide();
	void Notify(TNotifyUI& msg);

	void DoWithoutLoginStartMeetingBtnClick();
	void DoWithoutLoginJoinMeetingBtnClick();
public:
	virtual void onMeetingStatusChanged(ZOOM_SDK_NAMESPACE::MeetingStatus status, int iResult = 0);
	//handle the following status later. Just ignore them for a moment
	virtual void onMeetingStatisticsWarningNotification(ZOOM_SDK_NAMESPACE::StatisticsWarningType type) {};
	virtual void onMeetingParameterNotification(const ZOOM_SDK_NAMESPACE::MeetingParameter* meeting_param) {};

private:
	CVerticalLayoutUI* m_WithoutLoginRestAPIPage;
	CRichEditUI*	   m_editRestAPIUserZAK;
	CRichEditUI*	   m_editMeetingNumber;
	CRichEditUI*	   m_editScreenName;

	CButtonUI*		   m_btnStartMeeting;
	CButtonUI*         m_btnJoinMeeting;
	CSDKLoginUIMgr*  m_parentFrame;
	CSDKRestAPIUserWorkFlow  m_RestAPIUserWorkFlow;
	bool m_bInMeeting;
};

class CSDKLoginUIMgr : 
	public CWindowWnd, 
	public INotifyUI
{
public:
	CSDKLoginUIMgr();
	virtual ~CSDKLoginUIMgr();

	void SetEvent(CSDKDemoAppEvent* pAppEvent);
public:
	virtual LPCTSTR		GetWindowClassName() const   {   return _T("zSDKDemoUI");  }
	UINT GetClassStyle() const { return UI_CLASSSTYLE_FRAME | CS_DBLCLKS ; };
	virtual UINT		GetSkinRes()				 {	 return IDXML_LOGINFRAME_UI; };
	UILIB_RESOURCETYPE GetResourceType() const{	return UILIB_RESOURCE; }

	virtual void		InitWindow();
	virtual void		OnFinalMessage(HWND) {}

	virtual void		Notify( TNotifyUI& msg );
	virtual LRESULT		HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	void SwitchToWaitingPage(const wchar_t* waiting_message, bool show);
	void ShowErrorMessage(const wchar_t* error_message);
	void SwitchToPage(loginTabPage nPage);
	
	CSDKDemoAppEvent* GetAppEvent();
	void SetCurrentPage(CVerticalLayoutUI* current_) {m_currentPage = current_;}

	bool LogOut();
	void ChangeUIforLoginFailed();
	void ChangeUIforJoinFailed();

	void NotifyAuthDone();
    void SwitchToPage(SwitchToLoginUIType);

	void CleanUp();

protected:
	CPaintManagerUI m_PaintManager;
	CSDKLoginWithSSOUIGroup m_LoginWithSSOUIGroup;
	CSDKWithoutLoginStartJoinMeetingUIGroup m_WithoutLoginStartJoinMeetingUIGroup;
	CSDKRestAPIUserUIGroup m_RestAPIUserUIGroup;
	CSDKLoginCBHandler     m_LoginCBHandler;
	CSDKDemoAppEvent* m_pAppEvent;
	CVerticalLayoutUI* m_waitingPage;
	CLabelUI*          m_waitingLabelUI;
	CGifAnimUI*		   m_gifWaiting;
	CVerticalLayoutUI* m_currentPage;

	COptionUI* m_btnRestAPI;
	COptionUI* m_btnLoginWithSSO;
	COptionUI* m_btnJoinMeetingOnly;
	CVerticalLayoutUI* m_panel_login_content;

	CVerticalLayoutUI* m_sso_login_page;
	CVerticalLayoutUI* m_restApi_login_page;
	CVerticalLayoutUI* m_only_join_page;
};
