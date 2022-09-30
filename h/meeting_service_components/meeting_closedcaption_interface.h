/*!
* \file meeting_claosecaptioin_interface.h
* \brief Meeting Service Closed Caption Interface. 
* \remarks Valid for both ZOOM style and user custom interface mode.
*/
#ifndef _MEETING_CLOSEDCAPTION_INTERFACE_H_
#define _MEETING_CLOSEDCAPTION_INTERFACE_H_
#include "zoom_sdk_def.h"

BEGIN_ZOOM_SDK_NAMESPACE
/**
 * @brief Enumerations of the type for live transcription status.
 */
typedef enum
{
	SDK_LiveTranscription_Status_Stop = 0,   //not start
	SDK_LiveTranscription_Status_Start = 1,  //start
	SDK_LiveTranscription_Status_Connecting = 10,	 //connecting
}SDKLiveTranscriptionStatus;

/**
 * @brief Enumerations of the type for live transcription operation type.
 */
typedef enum
{
	SDK_LiveTranscription_OperationType_None = 0,
	SDK_LiveTranscription_OperationType_Add,
	SDK_LiveTranscription_OperationType_Update,
	SDK_LiveTranscription_OperationType_Delete,
}SDKLiveTranscriptionOperationType;

/// \brief Closed Caption controller callback event.
///
class IClosedCaptionControllerEvent
{
public:
	virtual ~IClosedCaptionControllerEvent() {}
	/// \brief Callback event when the user is assigned to send Closed Caption messages.
	virtual void onAssignedToSendCC(bool bAssigned) = 0;
	/// \brief Callback event when the user receives Closed Caption messages.
	/// \param ccMsg: an object pointer to the closed caption message content. 
	/// \param sender_id: the sender id of closed caption message. 
	/// \param time: the time of closed caption message.
	virtual void onClosedCaptionMsgReceived(const wchar_t* ccMsg, unsigned int sender_id, time_t time) = 0;
	/// \brief live transcription status changed callback.
	/// \param status: the live transcription status. For more details, see \link SDKLiveTranscriptionStatus \endlink.
	virtual void onLiveTranscriptionStatus(SDKLiveTranscriptionStatus status) = 0;
	/// \brief live transcription message received callback.
	/// \param ltMsg: an object pointer to the live transcription message content. 
	/// \param ltSpearkId: the speark id of the live transcription message. 
	/// \param type: the live transcription operation type. For more details, see \link SDKLiveTranscriptionOperationType \endlink.
	virtual void onLiveTranscriptionMsgReceived(const wchar_t* ltMsg, unsigned int speaker_id, SDKLiveTranscriptionOperationType type) = 0;
	/// \brief only host can receive this callback.
	/// \param requester_id: the request user id, if param bAnonymous is true, the requester_id is 0, no meanings. 
	/// \param bAnonymous: specify the request whether user anonymous request.
	virtual void onRequestForLiveTranscriptReceived(unsigned int requester_id, bool bAnonymous) = 0;
	/// \brief request live transcription status changed callback.
	/// \param bEnabled Specify the request live transcription status is enable, true enable, false disabled.
	virtual void onRequestLiveTranscriptionStatusChange(bool bEnabled) = 0;
};

/// \brief Closed caption controller interface.
class IClosedCaptionController
{
public:
	/// Set the controller event of closed caption(CC).
	virtual SDKError SetEvent(IClosedCaptionControllerEvent* pEvent) = 0;
	/// Determine if the current meeting supports the CC feature.
	virtual bool IsMeetingSupportCC() = 0;
	/// Query if it is able to assign others to send CC.
	virtual bool CanAssignOthersToSendCC() = 0;
	/// Assign a user to send CC.
	/// Zero(0) means to assign the current user himself.
	virtual SDKError AssignCCPriviledge(unsigned int userid, bool bAssigned) = 0;
	/// Determine if the user can be assigned as a CC sender.
	virtual bool CanBeAssignedToSendCC(unsigned int userid) = 0;
	/// Query if the user can send CC.
	virtual bool CanSendClosedCaption() = 0;
	/// Send the CC message.
	virtual SDKError SendClosedCaption(const wchar_t* ccMsg) = 0;
	/// Determine if it is enabled to save CC.
	virtual bool IsSaveCCEnabled() = 0;
	/// History of saving CC.
	virtual SDKError SaveCCHistory() = 0;
	/// Get the path of saving CC.
	virtual const wchar_t* GetClosedCaptionHistorySavedPath() = 0;
	/// Get the CC URL used by the third party service.
	virtual const wchar_t* GetClosedCaptionUrlFor3rdParty() = 0;
	/// \brief Determine whether the legal notice for Live transcript is available
	/// \return True indicates the legal notice for Live transcript is available. Otherwise False.
	virtual bool IsLiveTranscriptLegalNoticeAvailable() = 0;
	/// Get the CC legal notices prompt.
	virtual const wchar_t* getLiveTranscriptLegalNoticesPrompt() = 0;
	/// Get the CC legal notices explained.
	virtual const wchar_t* getLiveTranscriptLegalNoticesExplained() = 0;
	/// brief Determine whether the live transcription feature is enabled.
	virtual bool IsLiveTranscriptionFeatureEnabled() = 0;
	/// \brief Get the current live transcription status.
	//  \return type: the current live transcription status type. For more details, see \link SDKLiveTranscriptionStatus \endlink.
	virtual SDKLiveTranscriptionStatus GetLiveTranscriptionStatus() = 0;
	/// Query if the user can start live transcription.
	virtual bool CanStartLiveTranscription() = 0;
	/// \brief start live transcription.
	/// \return If the function succeeds, the return value is SDKErr_Success.
	/// Otherwise failed. To get extended error information, see \link SDKError \endlink enum.
	virtual SDKError StartLiveTranscription() = 0;
	/// \brief stop live transcription.
	/// \return If the function succeeds, the return value is SDKErr_Success.
	/// Otherwise failed. To get extended error information, see \link SDKError \endlink enum.
	virtual SDKError StopLiveTranscription() = 0;
	/// brief Enable or disable the attendees to request live transcription.
	/// \return If the function succeeds, the return value is SDKErr_Success.
	/// Otherwise failed. To get extended error information, see \link SDKError \endlink enum.
	virtual SDKError EnableRequestLiveTranscription(bool bEnable) = 0;
	/// brief Determine whether request to start live transcription is enabled, return true enabled, false disabled
	virtual bool IsRequestToStartLiveTranscriptionEnalbed() = 0;
	/// \brief request host to start live transcription.
	/// \param bRequestAnonymous: true indicate the user anonymous request.
	/// \return If the function succeeds, the return value is SDKErr_Success.
	/// Otherwise failed. To get extended error information, see \link SDKError \endlink enum.
	virtual SDKError RequestToStartLiveTranscription(bool bRequestAnonymous) = 0;
};

END_ZOOM_SDK_NAMESPACE
#endif