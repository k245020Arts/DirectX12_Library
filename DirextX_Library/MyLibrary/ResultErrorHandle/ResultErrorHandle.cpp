#include "ResultErrorHandle.h"

bool ResultErrorHandle::ErrorCodeInit()const
{
	return errorCode == ErrorCode::NotInitializationErrorCode;
}

bool ResultErrorHandle::IsSuccess()const
{
	if (errorCode == ErrorCode::OK) {
		return true;
	}
	
	return false;
}
