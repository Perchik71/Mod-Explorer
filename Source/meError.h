#pragma once

namespace meError
{
    int ReportAssertion([[maybe_unused]] const char* SourceFile, [[maybe_unused]] int SourceLine,
        [[maybe_unused]] const char* Function, const char* Message) noexcept;

    void AssertMsg(const char* SourceFile, int SourceLine, const char* Function, 
        const char* FormattedMessage, ...) noexcept;
};

#define meAssert(Cond)									if(!(Cond)) meError::AssertMsg(__FILE__, __LINE__, __FUNCTION__, #Cond)
#define meAssertWithFormattedMessage(Cond, Msg, ...)	if(!(Cond)) meError::AssertMsg(__FILE__, __LINE__, __FUNCTION__, "%s\n\n" Msg, #Cond, ##__VA_ARGS__)
#define meAssertWithMessage(Cond, Msg)					meAssertWithFormattedMessage(Cond, Msg)