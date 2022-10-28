// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022 - Present Romain Augier
// All rights reserved.

#include <iostream>

class MediaInputError : public std::exception
{
public:
    MediaInputError(char* msg) : message(msg) {}
    
    MediaInputError(const char* msg) : message(strcpy(this->message, msg)) {}
    
    MediaInputError(const std::string& msg) : message(strcpy(this->message, msg.c_str())) {}

    char* what() { return message; }
private:
    char* message = nullptr;
};

class ImageInputError : public MediaInputError 
{
    using MediaInputError::MediaInputError;
};

class VideoInputError : public MediaInputError 
{
    using MediaInputError::MediaInputError;
};