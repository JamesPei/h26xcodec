#pragma once
#ifndef __H26XCODEC_EXCEPTION__
#define __H26XCODEC_EXCEPTION__

class H26xException : public std::runtime_error
{
public:
  H26xException(const char* s) : std::runtime_error(s) {}
};

class H26xInitFailure : public H26xException
{
public:
    H26xInitFailure(const char* s) : H26xException(s) {}
};

class H26xDecodeFailure : public H26xException
{
public:
    H26xDecodeFailure(const char* s) : H26xException(s) {}
};

#endif