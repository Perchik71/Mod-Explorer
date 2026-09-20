#include "meError.h"

#include <windows.h>
#include <memory.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

static constexpr auto SIZE_ASSERT_BUFFERS = 4 * 1024;
static char meAssertBuffer[SIZE_ASSERT_BUFFERS]{};
static char meAssertMessage[SIZE_ASSERT_BUFFERS]{};

int meError::ReportAssertion(const char* SourceFile, int SourceLine, const char* Function, 
	const char* Message) noexcept
{
	return MessageBoxA(nullptr, Message, "Assertion", MB_ABORTRETRYIGNORE | MB_ICONERROR |
		MB_SETFOREGROUND | MB_TOPMOST);
}

void meError::AssertMsg(const char* SourceFile, int SourceLine, const char* Function, 
	const char* FormattedMessage, ...) noexcept
{
    memset(meAssertBuffer, 0, 1024);
    memset(meAssertMessage, 0, 1024);

    va_list ap;
    va_start(ap, FormattedMessage);
    vsnprintf(meAssertBuffer, SIZE_ASSERT_BUFFERS, FormattedMessage, ap);
    snprintf(meAssertMessage, SIZE_ASSERT_BUFFERS, "%s(%d) %s():\n\n%s", SourceFile, SourceLine,
        Function, meAssertBuffer);
    va_end(ap);

    while (1)
    {
        const auto sdl_assert_state = ReportAssertion(SourceFile, SourceLine, Function, meAssertMessage);

        if (sdl_assert_state == IDRETRY)
            // Fuck user
            continue;
        else if (sdl_assert_state == IDABORT)
        {
            // For debugger
            __debugbreak();
            // For Wine
            abort();
            // AGAIN!!!
            TerminateProcess(GetCurrentProcess(), EXIT_FAILURE);
            // CTD
            *((int*)0) = 0;
        }

        // Skip
        break;
    }
}
