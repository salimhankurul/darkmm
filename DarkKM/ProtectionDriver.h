
//Test proc
//
push rax
push rbx
push rcx
push rdx

push rsi
push rdi
push rbp
push rsp

; push r8
; push r9
; push r10
; push r11
; push r12
; push r13
; push r14
; push r15


pop rax
pop rbx
pop rcx
pop rdx

pop rsi
pop rdi
pop rbp
pop rsp

; pop r8
; pop r9
; pop r10
; pop r11
; pop r12
; pop r13
; pop r14
; pop r15
//
//ret
//Test endp
//#pragma once
//
//#include "Main.h"
//
//class ProtectionDriver
//{
//public:
//	ProtectionDriver();
//	~ProtectionDriver();
//
//	static OB_PREOP_CALLBACK_STATUS ProtectionPreProcessCallback(void* RegistrationContext, POB_PRE_OPERATION_INFORMATION OperationInformation);
//	static OB_PREOP_CALLBACK_STATUS ProtectionPreThreadCallback(void* RegistrationContext, POB_PRE_OPERATION_INFORMATION OperationInformation);
//
//	static NTSTATUS InitProtectedProcess();
//	static void UnHookProtection();
//
//	static NTSTATUS InitCallBacks();
//
//
//	static CALLBACK_HOOK Protection_Hook;
//	static WCHAR m_process_name[64];
//	static UNICODE_STRING CallBackHookAltitute;
//
//	static PEPROCESS m_process;
//	static HANDLE  m_processId;
//};
//
