#include <string.h>  
#include <stdint.h>
#include "ustdlib.h"

//*****************************************************************************
//
//! \addtogroup ustdlib_api
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
// A mapping from an integer between 0 and 15 to its ASCII character
// equivalent.
//
//*****************************************************************************
static const char * const g_pcHex = "0123456789abcdef";

//*****************************************************************************
//
//! Copies a certain number of characters from one string to another.
//!
//! \param s1 is a pointer to the destination buffer into which characters
//! are to be copied.
//! \param s2 is a pointer to the string from which characters are to be
//! copied.
//! \param n is the number of characters to copy to the destination buffer.
//!
//! This function copies at most \e n characters from the string pointed to
//! by \e s2 into the buffer pointed to by \e s1.  If the end of \e s2 is found
//! before \e n characters have been copied, remaining characters in \e s1
//! will be padded with zeroes until \e n characters have been written.  Note
//! that the destination string will only be NULL terminated if the number of
//! characters to be copied is greater than the length of \e s2.
//!
//! \return Returns \e s1.
//
//*****************************************************************************
char *
ustrncpy(char * restrict s1, const char * restrict s2, size_t n)
{
    size_t count;


    //
    // Start at the beginning of the source string.
    //
    count = 0;

    //
    // Copy the source string until we run out of source characters or
    // destination space.
    //
    while(n && s2[count])
    {
        s1[count] = s2[count];
        count++;
        n--;
    }

    //
    // Pad the destination if we are not yet done.
    //
    while(n)
    {
        s1[count++] = (char)0;
        n--;
    }

    //
    // Pass the destination pointer back to the caller.
    //
    return(s1);
}

//*****************************************************************************
//
//! A simple vsnprintf function supporting \%c, \%d, \%p, \%s, \%u, \%x, and
//! \%X.
//!
//! \param s points to the buffer where the converted string is stored.
//! \param n is the size of the buffer.
//! \param format is the format string.
//! \param arg is the list of optional arguments, which depend on the
//! contents of the format string.
//!
//! This function is very similar to the C library <tt>vsnprintf()</tt>
//! function.  Only the following formatting characters are supported:
//!
//! - \%c to print a character
//! - \%d or \%i to print a decimal value
//! - \%s to print a string
//! - \%u to print an unsigned decimal value
//! - \%x to print a hexadecimal value using lower case letters
//! - \%X to print a hexadecimal value using lower case letters (not upper case
//! letters as would typically be used)
//! - \%p to print a pointer as a hexadecimal value
//! - \%\% to print out a \% character
//!
//! For \%d, \%i, \%p, \%s, \%u, \%x, and \%X, an optional number may reside
//! between the \% and the format character, which specifies the minimum number
//! of characters to use for that value; if preceded by a 0 then the extra
//! characters will be filled with zeros instead of spaces.  For example,
//! ``\%8d'' will use eight characters to print the decimal value with spaces
//! added to reach eight; ``\%08d'' will use eight characters as well but will
//! add zeroes instead of spaces.
//!
//! The type of the arguments after \e format must match the requirements of
//! the format string.  For example, if an integer was passed where a string
//! was expected, an error of some kind will most likely occur.
//!
//! The \e n parameter limits the number of characters that will be
//! stored  in the buffer pointed to by \e s to prevent the possibility of
//! a buffer  overflow.  The buffer size should be large enough to hold the
//! expected converted output string, including the null termination character.
//!
//! The function will return the number of characters that would be converted
//! as if there were no limit on the buffer size.  Therefore it is possible for
//! the function to return a count that is greater than the specified buffer
//! size.  If this happens, it means that the output was truncated.
//!
//! \return Returns the number of characters that were to be stored, not
//! including the NULL termination character, regardless of space in the
//! buffer.
//
//*****************************************************************************
int
uvsnprintf(char * restrict s, size_t n, const char * restrict format,
           va_list arg)
{
    unsigned long ulIdx, ulValue, ulCount, ulBase, ulNeg;
    char *pcStr, cFill;
    int iConvertCount = 0;

    //
    // Check the arguments.
    //
//    ASSERT(s);
//    ASSERT(n);
//    ASSERT(format);

    //
    // Adjust buffer size limit to allow one space for null termination.
    //
    if(n)
    {
        n--;
    }

    //
    // Initialize the count of characters converted.
    //
    iConvertCount = 0;

    //
    // Loop while there are more characters in the format string.
    //
    while(*format)
    {
        //
        // Find the first non-% character, or the end of the string.
        //
        for(ulIdx = 0; (format[ulIdx] != '%') && (format[ulIdx] != '\0');
            ulIdx++)
        {
        }

        //
        // Write this portion of the string to the output buffer.  If there are
        // more characters to write than there is space in the buffer, then
        // only write as much as will fit in the buffer.
        //
        if(ulIdx > n)
        {
            ustrncpy(s, format, n);
            s += n;
            n = 0;
        }
        else
        {
            ustrncpy(s, format, ulIdx);
            s += ulIdx;
            n -= ulIdx;
        }

        //
        // Update the conversion count.  This will be the number of characters
        // that should have been written, even if there was not room in the
        // buffer.
        //
        iConvertCount += ulIdx;

        //
        // Skip the portion of the format string that was written.
        //
        format += ulIdx;

        //
        // See if the next character is a %.
        //
        if(*format == '%')
        {
            //
            // Skip the %.
            //
            format++;

            //
            // Set the digit count to zero, and the fill character to space
            // (that is, to the defaults).
            //
            ulCount = 0;
            cFill = ' ';

            //
            // It may be necessary to get back here to process more characters.
            // Goto's aren't pretty, but effective.  I feel extremely dirty for
            // using not one but two of the beasts.
            //
again:

            //
            // Determine how to handle the next character.
            //
            switch(*format++)
            {
                //
                // Handle the digit characters.
                //
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                {
                    //
                    // If this is a zero, and it is the first digit, then the
                    // fill character is a zero instead of a space.
                    //
                    if((format[-1] == '0') && (ulCount == 0))
                    {
                        cFill = '0';
                    }

                    //
                    // Update the digit count.
                    //
                    ulCount *= 10;
                    ulCount += format[-1] - '0';

                    //
                    // Get the next character.
                    //
                    goto again;
                }

                //
                // Handle the %c command.
                //
                case 'c':
                {
                    //
                    // Get the value from the varargs.
                    //
                    ulValue = va_arg(arg, unsigned long);

                    //
                    // Copy the character to the output buffer, if there is
                    // room.  Update the buffer size remaining.
                    //
                    if(n != 0)
                    {
                        *s++ = (char)ulValue;
                        n--;
                    }

                    //
                    // Update the conversion count.
                    //
                    iConvertCount++;

                    //
                    // This command has been handled.
                    //
                    break;
                }

                //
                // Handle the %d and %i commands.
                //
                case 'd':
                case 'i':
                {
                    //
                    // Get the value from the varargs.
                    //
                    ulValue = va_arg(arg, unsigned long);

                    //
                    // If the value is negative, make it positive and indicate
                    // that a minus sign is needed.
                    //
                    if((long)ulValue < 0)
                    {
                        //
                        // Make the value positive.
                        //
                        ulValue = -(long)ulValue;

                        //
                        // Indicate that the value is negative.
                        //
                        ulNeg = 1;
                    }
                    else
                    {
                        //
                        // Indicate that the value is positive so that a
                        // negative sign isn't inserted.
                        //
                        ulNeg = 0;
                    }

                    //
                    // Set the base to 10.
                    //
                    ulBase = 10;

                    //
                    // Convert the value to ASCII.
                    //
                    goto convert;
                }

                //
                // Handle the %s command.
                //
                case 's':
                {
                    //
                    // Get the string pointer from the varargs.
                    //
                    pcStr = va_arg(arg, char *);

                    //
                    // Determine the length of the string.
                    //
                    for(ulIdx = 0; pcStr[ulIdx] != '\0'; ulIdx++)
                    {
                    }

                    //
                    // Update the convert count to include any padding that
                    // should be necessary (regardless of whether we have space
                    // to write it or not).
                    //
                    if(ulCount > ulIdx)
                    {
                        iConvertCount += (ulCount - ulIdx);
                    }

                    //
                    // Copy the string to the output buffer.  Only copy as much
                    // as will fit in the buffer.  Update the output buffer
                    // pointer and the space remaining.
                    //
                    if(ulIdx > n)
                    {
                        ustrncpy(s, pcStr, n);
                        s += n;
                        n = 0;
                    }
                    else
                    {
                        ustrncpy(s, pcStr, ulIdx);
                        s += ulIdx;
                        n -= ulIdx;

                        //
                        // Write any required padding spaces assuming there is
                        // still space in the buffer.
                        //
                        if(ulCount > ulIdx)
                        {
                            ulCount -= ulIdx;
                            if(ulCount > n)
                            {
                                ulCount = n;
                            }
                            n = -ulCount;

                            while(ulCount--)
                            {
                                *s++ = ' ';
                            }
                        }
                    }

                    //
                    // Update the conversion count.  This will be the number of
                    // characters that should have been written, even if there
                    // was not room in the buffer.
                    //
                    iConvertCount += ulIdx;

                    //
                    // This command has been handled.
                    //
                    break;
                }

                //
                // Handle the %u command.
                //
                case 'u':
                {
                    //
                    // Get the value from the varargs.
                    //
                    ulValue = va_arg(arg, unsigned long);

                    //
                    // Set the base to 10.
                    //
                    ulBase = 10;

                    //
                    // Indicate that the value is positive so that a minus sign
                    // isn't inserted.
                    //
                    ulNeg = 0;

                    //
                    // Convert the value to ASCII.
                    //
                    goto convert;
                }

                //
                // Handle the %x and %X commands.  Note that they are treated
                // identically; that is, %X will use lower case letters for a-f
                // instead of the upper case letters is should use.  We also
                // alias %p to %x.
                //
                case 'x':
                case 'X':
                case 'p':
                {
                    //
                    // Get the value from the varargs.
                    //
                    ulValue = va_arg(arg, unsigned long);

                    //
                    // Set the base to 16.
                    //
                    ulBase = 16;

                    //
                    // Indicate that the value is positive so that a minus sign
                    // isn't inserted.
                    //
                    ulNeg = 0;

                    //
                    // Determine the number of digits in the string version of
                    // the value.
                    //
convert:
                    for(ulIdx = 1;
                        (((ulIdx * ulBase) <= ulValue) &&
                         (((ulIdx * ulBase) / ulBase) == ulIdx));
                        ulIdx *= ulBase, ulCount--)
                    {
                    }

                    //
                    // If the value is negative, reduce the count of padding
                    // characters needed.
                    //
                    if(ulNeg)
                    {
                        ulCount--;
                    }

                    //
                    // If the value is negative and the value is padded with
                    // zeros, then place the minus sign before the padding.
                    //
                    if(ulNeg && (n != 0) && (cFill == '0'))
                    {
                        //
                        // Place the minus sign in the output buffer.
                        //
                        *s++ = '-';
                        n--;

                        //
                        // Update the conversion count.
                        //
                        iConvertCount++;

                        //
                        // The minus sign has been placed, so turn off the
                        // negative flag.
                        //
                        ulNeg = 0;
                    }

                    //
                    // See if there are more characters in the specified field
                    // width than there are in the conversion of this value.
                    //
                    if((ulCount > 1) && (ulCount < 65536))
                    {
                        //
                        // Loop through the required padding characters.
                        //
                        for(ulCount--; ulCount; ulCount--)
                        {
                            //
                            // Copy the character to the output buffer if there
                            // is room.
                            //
                            if(n != 0)
                            {
                                *s++ = cFill;
                                n--;
                            }

                            //
                            // Update the conversion count.
                            //
                            iConvertCount++;
                        }
                    }

                    //
                    // If the value is negative, then place the minus sign
                    // before the number.
                    //
                    if(ulNeg && (n != 0))
                    {
                        //
                        // Place the minus sign in the output buffer.
                        //
                        *s++ = '-';
                        n--;

                        //
                        // Update the conversion count.
                        //
                        iConvertCount++;
                    }

                    //
                    // Convert the value into a string.
                    //
                    for(; ulIdx; ulIdx /= ulBase)
                    {
                        //
                        // Copy the character to the output buffer if there is
                        // room.
                        //
                        if(n != 0)
                        {
                            *s++ = g_pcHex[(ulValue / ulIdx) % ulBase];
                            n--;
                        }

                        //
                        // Update the conversion count.
                        //
                        iConvertCount++;
                    }

                    //
                    // This command has been handled.
                    //
                    break;
                }

                //
                // Handle the %% command.
                //
                case '%':
                {
                    //
                    // Simply write a single %.
                    //
                    if(n != 0)
                    {
                        *s++ = format[-1];
                        n--;
                    }

                    //
                    // Update the conversion count.
                    //
                    iConvertCount++;

                    //
                    // This command has been handled.
                    //
                    break;
                }

                //
                // Handle all other commands.
                //
                default:
                {
                    //
                    // Indicate an error.
                    //
                    if(n >= 5)
                    {
                        ustrncpy(s, "ERROR", 5);
                        s += 5;
                        n -= 5;
                    }
                    else
                    {
                        ustrncpy(s, "ERROR", n);
                        s += n;
                        n = 0;
                    }

                    //
                    // Update the conversion count.
                    //
                    iConvertCount += 5;

                    //
                    // This command has been handled.
                    //
                    break;
                }
            }
        }
    }

    //
    // Null terminate the string in the buffer.
    //
    *s = 0;

    //
    // Return the number of characters in the full converted string.
    //
    return(iConvertCount);
}

//*****************************************************************************
//
//! A simple sprintf function supporting \%c, \%d, \%p, \%s, \%u, \%x, and \%X.
//!
//! \param s is the buffer where the converted string is stored.
//! \param format is the format string.
//! \param ... are the optional arguments, which depend on the contents of the
//! format string.
//!
//! This function is very similar to the C library <tt>sprintf()</tt> function.
//! Only the following formatting characters are supported:
//!
//! - \%c to print a character
//! - \%d or \%i to print a decimal value
//! - \%s to print a string
//! - \%u to print an unsigned decimal value
//! - \%x to print a hexadecimal value using lower case letters
//! - \%X to print a hexadecimal value using lower case letters (not upper case
//! letters as would typically be used)
//! - \%p to print a pointer as a hexadecimal value
//! - \%\% to print out a \% character
//!
//! For \%d, \%i, \%p, \%s, \%u, \%x, and \%X, an optional number may reside
//! between the \% and the format character, which specifies the minimum number
//! of characters to use for that value; if preceded by a 0 then the extra
//! characters will be filled with zeros instead of spaces.  For example,
//! ``\%8d'' will use eight characters to print the decimal value with spaces
//! added to reach eight; ``\%08d'' will use eight characters as well but will
//! add zeros instead of spaces.
//!
//! The type of the arguments after \e format must match the requirements of
//! the format string.  For example, if an integer was passed where a string
//! was expected, an error of some kind will most likely occur.
//!
//! The caller must ensure that the buffer \e s is large enough to hold the
//! entire converted string, including the null termination character.
//!
//! \return Returns the count of characters that were written to the output
//! buffer, not including the NULL termination character.
//
//*****************************************************************************
int
usprintf(char * restrict s, const char *format, ...)
{
    va_list arg;
    int ret;

    //
    // Start the varargs processing.
    //
    va_start(arg, format);

    //
    // Call vsnprintf to perform the conversion.  Use a large number for the
    // buffer size.
    //
    ret = uvsnprintf(s, 0xffff, format, arg);

    //
    // End the varargs processing.
    //
    va_end(arg);

    //
    // Return the conversion count.
    //
    return(ret);
}

//*****************************************************************************
//
//! A simple snprintf function supporting \%c, \%d, \%p, \%s, \%u, \%x, and
//! \%X.
//!
//! \param s is the buffer where the converted string is stored.
//! \param n is the size of the buffer.
//! \param format is the format string.
//! \param ... are the optional arguments, which depend on the contents of the
//! format string.
//!
//! This function is very similar to the C library <tt>sprintf()</tt> function.
//! Only the following formatting characters are supported:
//!
//! - \%c to print a character
//! - \%d or \%i to print a decimal value
//! - \%s to print a string
//! - \%u to print an unsigned decimal value
//! - \%x to print a hexadecimal value using lower case letters
//! - \%X to print a hexadecimal value using lower case letters (not upper case
//! letters as would typically be used)
//! - \%p to print a pointer as a hexadecimal value
//! - \%\% to print out a \% character
//!
//! For \%d, \%i, \%p, \%s, \%u, \%x, and \%X, an optional number may reside
//! between the \% and the format character, which specifies the minimum number
//! of characters to use for that value; if preceded by a 0 then the extra
//! characters will be filled with zeros instead of spaces.  For example,
//! ``\%8d'' will use eight characters to print the decimal value with spaces
//! added to reach eight; ``\%08d'' will use eight characters as well but will
//! add zeros instead of spaces.
//!
//! The type of the arguments after \e format must match the requirements of
//! the format string.  For example, if an integer was passed where a string
//! was expected, an error of some kind will most likely occur.
//!
//! The function will copy at most \e n - 1 characters into the buffer
//! \e s.  One space is reserved in the buffer for the null termination
//! character.
//!
//! The function will return the number of characters that would be converted
//! as if there were no limit on the buffer size.  Therefore it is possible for
//! the function to return a count that is greater than the specified buffer
//! size.  If this happens, it means that the output was truncated.
//!
//! \return Returns the number of characters that were to be stored, not
//! including the NULL termination character, regardless of space in the
//! buffer.
//
//*****************************************************************************
int
usnprintf(char * restrict s, size_t n, const char * restrict format, ...)
{
    va_list arg;
    int ret;

    //
    // Start the varargs processing.
    //
    va_start(arg, format);

    //
    // Call vsnprintf to perform the conversion.
    //
    ret = uvsnprintf(s, n, format, arg);

    //
    // End the varargs processing.
    //
    va_end(arg);

    //
    // Return the conversion count.
    //
    return(ret);
}


//*****************************************************************************
//
//! Converts a string into its numeric equivalent.
//!
//! \param nptr is a pointer to the string containing the integer.
//! \param endptr is a pointer that will be set to the first character past
//! the integer in the string.
//! \param base is the radix to use for the conversion; can be zero to
//! auto-select the radix or between 2 and 16 to explicitly specify the radix.
//!
//! This function is very similar to the C library <tt>strtoul()</tt> function.
//! It scans a string for the first token (that is, non-white space) and
//! converts the value at that location in the string into an integer value.
//!
//! \return Returns the result of the conversion.
//
//*****************************************************************************
unsigned long
ustrtoul(const char * restrict nptr, const char ** restrict endptr, int base)
{
    unsigned long ulRet, ulDigit, ulNeg, ulValid;
    const char *pcPtr;

    //
    // Check the arguments.
    //
//    ASSERT(nptr);
//    ASSERT((base == 0) || ((base > 1) && (base <= 16)));

    //
    // Initially, the result is zero.
    //
    ulRet = 0;
    ulNeg = 0;
    ulValid = 0;

    //
    // Skip past any leading white space.
    //
    pcPtr = nptr;
    while((*pcPtr == ' ') || (*pcPtr == '\t'))
    {
        pcPtr++;
    }

    //
    // Take a leading + or - from the value.
    //
    if(*pcPtr == '-')
    {
        ulNeg = 1;
        pcPtr++;
    }
    else if(*pcPtr == '+')
    {
        pcPtr++;
    }

    //
    // See if the radix was not specified, or is 16, and the value starts with
    // "0x" or "0X" (to indicate a hex value).
    //
    if(((base == 0) || (base == 16)) && (*pcPtr == '0') &&
       ((pcPtr[1] == 'x') || (pcPtr[1] == 'X')))
    {
        //
        // Skip the leading "0x".
        //
        pcPtr += 2;

        //
        // Set the radix to 16.
        //
        base = 16;
    }

    //
    // See if the radix was not specified.
    //
    if(base == 0)
    {
        //
        // See if the value starts with "0".
        //
        if(*pcPtr == '0')
        {
            //
            // Values that start with "0" are assumed to be radix 8.
            //
            base = 8;
        }
        else
        {
            //
            // Otherwise, the values are assumed to be radix 10.
            //
            base = 10;
        }
    }

    //
    // Loop while there are more valid digits to consume.
    //
    while(1)
    {
        //
        // See if this character is a number.
        //
        if((*pcPtr >= '0') && (*pcPtr <= '9'))
        {
            //
            // Convert the character to its integer equivalent.
            //
            ulDigit = *pcPtr++ - '0';
        }

        //
        // Otherwise, see if this character is an upper case letter.
        //
        else if((*pcPtr >= 'A') && (*pcPtr <= 'Z'))
        {
            //
            // Convert the character to its integer equivalent.
            //
            ulDigit = *pcPtr++ - 'A' + 10;
        }

        //
        // Otherwise, see if this character is a lower case letter.
        //
        else if((*pcPtr >= 'a') && (*pcPtr <= 'z'))
        {
            //
            // Convert the character to its integer equivalent.
            //
            ulDigit = *pcPtr++ - 'a' + 10;
        }

        //
        // Otherwise, this is not a valid character.
        //
        else
        {
            //
            // Stop converting this value.
            //
            break;
        }

        //
        // See if this digit is valid for the chosen radix.
        //
        if(ulDigit >= base)
        {
            //
            // Since this was not a valid digit, move the pointer back to the
            // character that therefore should not have been consumed.
            //
            pcPtr--;

            //
            // Stop converting this value.
            //
            break;
        }

        //
        // Add this digit to the converted value.
        //
        ulRet *= base;
        ulRet += ulDigit;

        //
        // Since a digit has been added, this is now a valid result.
        //
        ulValid = 1;
    }

    //
    // Set the return string pointer to the first character not consumed.
    //
    if(endptr)
    {
        *endptr = ulValid ? pcPtr : nptr;
    }

    //
    // Return the converted value.
    //
    return(ulNeg ? (0 - ulRet) : ulRet);
}

//*****************************************************************************
//
// An array of the value of ten raised to the power-of-two exponents.  This is
// used for converting the decimal exponent into the floating-point value of
// 10^exp.
//
//*****************************************************************************
static const float g_pfExponents[] =
{
    1.0e+01,
    1.0e+02,
    1.0e+04,
    1.0e+08,
    1.0e+16,
    1.0e+32,
};

//*****************************************************************************
//
//! Converts a string into its floating-point equivalent.
//!
//! \param nptr is a pointer to the string containing the floating-point
//! value.
//! \param endptr is a pointer that will be set to the first character past
//! the floating-point value in the string.
//!
//! This function is very similar to the C library <tt>strtof()</tt> function.
//! It scans a string for the first token (that is, non-white space) and
//! converts the value at that location in the string into a floating-point
//! value.
//!
//! \return Returns the result of the conversion.
//
//*****************************************************************************
float
ustrtof(const char *nptr, const char **endptr)
{
    unsigned long ulNeg, ulExp, ulExpNeg, ulValid, ulIdx;
    float fRet, fDigit, fExp;
    const char *pcPtr;

    //
    // Check the arguments.
    //
//    ASSERT(nptr);

    //
    // Initially, the result is zero.
    //
    fRet = 0;
    ulNeg = 0;
    ulValid = 0;

    //
    // Skip past any leading white space.
    //
    pcPtr = nptr;
    while((*pcPtr == ' ') || (*pcPtr == '\t'))
    {
        pcPtr++;
    }

    //
    // Take a leading + or - from the value.
    //
    if(*pcPtr == '-')
    {
        ulNeg = 1;
        pcPtr++;
    }
    else if(*pcPtr == '+')
    {
        pcPtr++;
    }

    //
    // Loop while there are valid digits to consume.
    //
    while((*pcPtr >= '0') && (*pcPtr <= '9'))
    {
        //
        // Add this digit to the converted value.
        //
        fRet *= 10;
        fRet += *pcPtr++ - '0';

        //
        // Since a digit has been added, this is now a valid result.
        //
        ulValid = 1;
    }

    //
    // See if the next character is a period and the character after that is a
    // digit, indicating the start of the fractional portion of the value.
    //
    if((*pcPtr == '.') && (pcPtr[1] >= '0') && (pcPtr[1] <= '9'))
    {
        //
        // Skip the period.
        //
        pcPtr++;

        //
        // Loop while there are valid fractional digits to consume.
        //
        fDigit = 0.1;
        while((*pcPtr >= '0') && (*pcPtr <= '9'))
        {
            //
            // Add this digit to the converted value.
            //
            fRet += (*pcPtr++ - '0') * fDigit;
            fDigit /= (float)10.0;

            //
            // Since a digit has been added, this is now a valid result.
            //
            ulValid = 1;
        }
    }

    //
    // See if the next character is an "e" and a valid number has been
    // converted, indicating the start of the exponent.
    //
    if(((pcPtr[0] == 'e') || (pcPtr[0] == 'E')) && (ulValid == 1) &&
       (((pcPtr[1] >= '0') && (pcPtr[1] <= '9')) ||
        (((pcPtr[1] == '+') || (pcPtr[1] == '-')) &&
         (pcPtr[2] >= '0') && (pcPtr[2] <= '9'))))
    {
        //
        // Skip the "e".
        //
        pcPtr++;

        //
        // Take a leading + or - from the exponenet.
        //
        ulExpNeg = 0;
        if(*pcPtr == '-')
        {
            ulExpNeg = 1;
            pcPtr++;
        }
        else if(*pcPtr == '+')
        {
            pcPtr++;
        }

        //
        // Loop while there are valid digits in the exponent.
        //
        ulExp = 0;
        while((*pcPtr >= '0') && (*pcPtr <= '9'))
        {
            //
            // Add this digit to the converted value.
            //
            ulExp *= 10;
            ulExp += *pcPtr++ - '0';
        }

        //
        // Raise ten to the power of the exponent.  Do this via binary
        // decomposition; for each binary bit set in the exponent, multiply the
        // floating-point representation by ten raised to that binary value
        // (extracted from the table above).
        //
        fExp = 1;
        for(ulIdx = 0; ulIdx < 7; ulIdx++)
        {
            if(ulExp & (1 << ulIdx))
            {
                fExp *= g_pfExponents[ulIdx];
            }
        }

        //
        // If the exponent is negative, then the exponent needs to be inverted.
        //
        if(ulExpNeg == 1)
        {
            fExp = 1 / fExp;
        }

        //
        // Multiply the result by the computed exponent value.
        //
        fRet *= fExp;
    }

    //
    // Set the return string pointer to the first character not consumed.
    //
    if(endptr)
    {
        *endptr = ulValid ? pcPtr : nptr;
    }

    //
    // Return the converted value.
    //
    return(ulNeg ? (0 - fRet) : fRet);
}

//*****************************************************************************
//
//! Returns the length of a null-terminated string.
//!
//! \param s is a pointer to the string whose length is to be found.
//!
//! This function is very similar to the C library <tt>strlen()</tt> function.
//! It determines the length of the null-terminated string passed and returns
//! this to the caller.
//!
//! This implementation assumes that single byte character strings are passed
//! and will return incorrect values if passed some UTF-8 strings.
//!
//! \return Returns the length of the string pointed to by \e s.
//
//*****************************************************************************
size_t
ustrlen(const char *s)
{
    size_t len;

    //
    // Check the arguments.
    //
//    ASSERT(s);

    //
    // Initialize the length.
    //
    len = 0;

    //
    // Step throug the string looking for a zero character (marking its end).
    //
    while(s[len])
    {
        //
        // Zero not found so move on to the next character.
        //
        len++;
    }

    return(len);
}

//*****************************************************************************
//
//! Finds a substring within a string.
//!
//! \param s1 is a pointer to the string that will be searched.
//! \param s2 is a pointer to the substring that is to be found within
//! \e s1.
//!
//! This function is very similar to the C library <tt>strstr()</tt> function.
//! It scans a string for the first instance of a given substring and returns
//! a pointer to that substring.  If the substring cannot be found, a NULL
//! pointer is returned.
//!
//! \return Returns a pointer to the first occurrence of \e s2 within
//! \e s1 or NULL if no match is found.
//
//*****************************************************************************
char *
ustrstr(const char *s1, const char *s2)
{
    size_t n;

    //
    // Get the length of the string to be found.
    //
    n = ustrlen(s2);

    //
    // Loop while we have not reached the end of the string.
    //
    while(*s1)
    {
        //
        // Check to see if the substring appears at this position.
        //
        if(ustrncmp(s2, s1, n) == 0)
        {
            //
            // It does so return the pointer.
            //
            return((char *)s1);
        }

        //
        // Move to the next position in the string being searched.
        //
        s1++;
    }

    //
    // We reached the end of the string without finding the substring so
    // return NULL.
    //
    return((char *)0);
}

//*****************************************************************************
//
//! Compares two strings without regard to case.
//!
//! \param s1 points to the first string to be compared.
//! \param s2 points to the second string to be compared.
//! \param n is the maximum number of characters to compare.
//!
//! This function is very similar to the C library <tt>strncasecmp()</tt>
//! function.  It compares at most \e n characters of two strings without
//! regard to case.  The comparison ends if a terminating NULL character is
//! found in either string before \e n characters are compared.  In this case,
//! the shorter string is deemed the lesser.
//!
//! \return Returns 0 if the two strings are equal, -1 if \e s1 is less
//! than \e s2 and 1 if \e s1 is greater than \e s2.
//
//*****************************************************************************
int
ustrncasecmp(const char *s1, const char *s2, size_t n)
{
    char c1, c2;

    //
    // Loop while there are more characters to compare.
    //
    while(n)
    {
        //
        // If we reached a NULL in both strings, they must be equal so
        // we end the comparison and return 0
        //
        if(!*s1 && !*s2)
        {
            return(0);
        }

        //
        // Lower case the characters at the current position before we compare.
        //
        c1 = (((*s1 >= 'A') && (*s1 <= 'Z')) ? (*s1 + ('a' - 'A')) : *s1);
        c2 = (((*s2 >= 'A') && (*s2 <= 'Z')) ? (*s2 + ('a' - 'A')) : *s2);

        //
        // Compare the two characters and, if different, return the relevant
        // return code.
        //
        if(c2 < c1)
        {
            return(1);
        }
        if(c1 < c2)
        {
            return(-1);
        }

        //
        // Move on to the next character.
        //
        s1++;
        s2++;
        n--;
    }

    //
    // If we fall out, the strings must be equal for at least the first n
    // characters so return 0 to indicate this.
    //
    return(0);
}

//*****************************************************************************
//
//! Compares two strings without regard to case.
//!
//! \param s1 points to the first string to be compared.
//! \param s2 points to the second string to be compared.
//!
//! This function is very similar to the C library <tt>strcasecmp()</tt>
//! function.  It compares two strings without regard to case.  The comparison
//! ends if a terminating NULL character is found in either string.  In this
//! case, the int16_ter string is deemed the lesser.
//!
//! \return Returns 0 if the two strings are equal, -1 if \e s1 is less
//! than \e s2 and 1 if \e s1 is greater than \e s2.
//
//*****************************************************************************
int
ustrcasecmp(const char *s1, const char *s2)
{
    //
    // Just let ustrncasecmp() handle this.
    //
    return(ustrncasecmp(s1, s2, (size_t)-1));
}

//*****************************************************************************
//
//! Compares two strings.
//!
//! \param s1 points to the first string to be compared.
//! \param s2 points to the second string to be compared.
//! \param n is the maximum number of characters to compare.
//!
//! This function is very similar to the C library <tt>strncmp()</tt> function.
//! It compares at most \e n characters of two strings taking case into
//! account.  The comparison ends if a terminating NULL character is found in
//! either string before \e n characters are compared.  In this case, the
//! int16_ter string is deemed the lesser.
//!
//! \return Returns 0 if the two strings are equal, -1 if \e s1 is less
//! than \e s2 and 1 if \e s1 is greater than \e s2.
//
//*****************************************************************************
int
ustrncmp(const char *s1, const char *s2, size_t n)
{
    //
    // Loop while there are more characters.
    //
    while(n)
    {
        //
        // If we reached a NULL in both strings, they must be equal so we end
        // the comparison and return 0
        //
        if(!*s1 && !*s2)
        {
            return(0);
        }

        //
        // Compare the two characters and, if different, return the relevant
        // return code.
        //
        if(*s2 < *s1)
        {
            return(1);
        }
        if(*s1 < *s2)
        {
            return(-1);
        }

        //
        // Move on to the next character.
        //
        s1++;
        s2++;
        n--;
    }

    //
    // If we fall out, the strings must be equal for at least the first n
    // characters so return 0 to indicate this.
    //
    return(0);
}

//*****************************************************************************
//
//! Compares two strings.
//!
//! \param s1 points to the first string to be compared.
//! \param s2 points to the second string to be compared.
//!
//! This function is very similar to the C library <tt>strcmp()</tt>
//! function.  It compares two strings, taking case into account.  The
//! comparison ends if a terminating NULL character is found in either string.
//! In this case, the int16_ter string is deemed the lesser.
//!
//! \return Returns 0 if the two strings are equal, -1 if \e s1 is less
//! than \e s2 and 1 if \e s1 is greater than \e s2.
//
//*****************************************************************************
int
ustrcmp(const char *s1, const char *s2)
{
    //
    // Pass this on to ustrncmp.
    //
    return(ustrncmp(s1, s2, (size_t)-1));
}

//*****************************************************************************
//
// Random Number Generator Seed Value
//
//*****************************************************************************
static unsigned int g_iRandomSeed = 1;

//*****************************************************************************
//
//! Set the random number generator seed.
//!
//! \param seed is the new seed value to use for the random number
//! generator.
//!
//! This function is very similar to the C library <tt>srand()</tt> function.
//! It will set the seed value used in the <tt>urand()</tt> function.
//!
//! \return None
//
//*****************************************************************************
void
usrand(unsigned int seed)
{
    g_iRandomSeed = seed;
}

//*****************************************************************************
//
//! Generate a new (pseudo) random number
//!
//! This function is very similar to the C library <tt>rand()</tt> function.
//! It will generate a pseudo-random number sequence based on the seed value.
//!
//! \return A pseudo-random number will be returned.
//
//*****************************************************************************
int
urand(void)
{
    //
    // Generate a new pseudo-random number with a linear congruence random
    // number generator.  This new random number becomes the seed for the next
    // random number.
    //
    g_iRandomSeed = (g_iRandomSeed * 1664525) + 1013904223;

    //
    // Return the new random number.
    //
    return((int)g_iRandomSeed);
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
