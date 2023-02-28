

class KernelLogger
{
public:
	static NTSTATUS LogToFile(PCSTR msg);

	static DK::DString<CHAR>* GetTime()
	{
		static LARGE_INTEGER time;
		static TIME_FIELDS TimeFieldsa;
		PTIME_FIELDS TimeFields = &TimeFieldsa;

		KeQuerySystemTime(&time);
		ExSystemTimeToLocalTime(&time, &time);
		RtlTimeToTimeFields(&time, TimeFields);

		static PCHAR Months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
									"Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
		static PCHAR Days[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

		return new DK::DString<CHAR>("[%s %02d - %02d:%02d:%02d.%03d]",
			Months[TimeFields->Month - 1],
			TimeFields->Day,
			TimeFields->Hour,
			TimeFields->Minute,
			TimeFields->Second,
			TimeFields->Milliseconds);
	}

	template<class ... Types>
	static void MyDbgPrint(const char* txt, Types ... args)
	{

		static bool Inited = false;
		static KGUARDED_MUTEX LoggerLock;

		if (!Inited)
		{
			Inited = true;
			KeInitializeGuardedMutex(&LoggerLock);
		}

		KeAcquireGuardedMutex(&LoggerLock);

		auto LogStr = new DK::DString<CHAR>(txt, (args)...);
		auto TimeStr = GetTime();

		// Log With Time	
		auto Log = new DK::DString<CHAR>("%hs %hs", TimeStr->Buffer, LogStr->Buffer);		
		LogToFile(Log->Buffer);
		
		delete LogStr;
		delete TimeStr;
		delete Log;

		KeReleaseGuardedMutex(&LoggerLock);

	}
};

#define DEBUG_PRINT KernelLogger::MyDbgPrint
#define DEBUG_PRINTE


#define DEBUG_RESULT_WITH_RETURN2(MYDEBUG,status,resultname)  \
if (!status) \
{  \
	DEBUG_PRINT("[%ws]::%hs Error: " resultname " Status: 0x%X \r\n", PROJECT_NAME, __FUNCTION__,status);  \
	return STATUS_ACCESS_DENIED;  \
}   \
else  \
MYDEBUG("[%ws]::%hs Successfull: " resultname " \r\n", PROJECT_NAME, __FUNCTION__); 


#define DEBUG_RESULT_WITH_RETURN(MYDEBUG,status,resultname)  \
if (!NT_SUCCESS(status)) \
{  \
	DEBUG_PRINT("[%ws]::%hs Error: " resultname " Status: 0x%X \r\n", PROJECT_NAME, __FUNCTION__,status);  \
	return status;  \
}   \
else  \
MYDEBUG("[%ws]::%hs Successfull: " resultname " \r\n", PROJECT_NAME, __FUNCTION__); 

#define DEBUG_RESULT_WITH_GO_TO(MYDEBUG,status,resultname,tooo)  \
if (!NT_SUCCESS(status)) \
{  \
	DEBUG_PRINT("[%ws]::%hs Error: " resultname " Status: 0x%X \r\n", PROJECT_NAME, __FUNCTION__,status);  \
	goto tooo;  \
}   \
else  \
MYDEBUG("[%ws]::%hs Successfull: " resultname " \r\n", PROJECT_NAME, __FUNCTION__); 


#define DEBUG_RESULT(MYDEBUG,status,resultname)  \
if (!NT_SUCCESS(status)) \
{  \
	DEBUG_PRINT("[%ws]::%hs Error: " resultname " Status: 0x%X \r\n", PROJECT_NAME, __FUNCTION__,status);  \
}   \
else  \
MYDEBUG("[%ws]::%hs Successfull: " resultname " \r\n", PROJECT_NAME, __FUNCTION__); 
